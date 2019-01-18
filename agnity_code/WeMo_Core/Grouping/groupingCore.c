
/***************************************************************************
 *
 *
 * groupingCore.c
 *
 * Created by Belkin International, Software Engineering on XX/XX/XX.
 * Copyright (c) 2012-2013 Belkin International, Inc. and/or its affiliates. All rights reserved.
 *
 * Belkin International, Inc. retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have obtained
 * a separate written license from Belkin International, Inc., you are not authorized
 * to utilize all or a part of this computer program for any purpose (including
 * reproduction, distribution, modification, and compilation into object code)
 * and you must immediately destroy or return to Belkin International, Inc
 * all copies of this computer program.  If you are licensed by Belkin International, Inc., your
 * rights to utilize this computer program are limited by the terms of that license.
 *
 * To obtain a license, please contact Belkin International, Inc.
 *
 * This computer program contains trade secrets owned by Belkin International, Inc.
 * and, unless unauthorized by Belkin International, Inc. in writing, you agree to
 * maintain the confidentiality of this computer program and related information
 * and to not disclose this computer program and related information to any
 * other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND BELKIN INTERNATIONAL, INC.
 * EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING THE WARRANTIES OF
 * MERCHANTIBILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND NON-INFRINGEMENT.
 *
 *
 ***************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/stat.h>

#include "types.h"
#include "defines.h"
#include "logger.h"

#include "WemoDB.h"

#include "utlist.h"
#include "mxml.h"
#include "groupingDS.h"
#include "groupingExtIf.h"
#include "groupingDefs.h"
#define NO_OF_CHARS 256
extern sqlite3 *g_GroupingDB;
extern SEndDevicesGroup *gGroupListHead;
/* SetGroupDetails: Common API to
   1. Create group
   2. Add members to group
   3. Remove members from group
   4. Edit group details (icon, name etc.)
   5. Modify member capabilities
   6. Delete group

   Argument details:
transaction: Reserved for future use, for now should be NULL
RequestXML: XML data containing grouping action and data. Format detailed below:
<SetGroupDetails>
<GroupID>New or Existing GroupID</GroupID>
<GroupActions>ORed Actions Values to handle multiple action requests in one XML /Command exchange</GroupActions>		
<AddDeviceIDs>List of comma separated device indexes for creating a new group or adding to an existing group</AddDeviceIDs>
<RemoveDeviceIDs>List of comma separated device indexes for removing device to an existing group</RemoveDeviceIDs>
<GroupCapabilityIDs>Capabilities Id along with their current values</GroupCapabilityIDs>
<GroupIconDetails>Group Icon Name along with Group Icon Version</GroupIconDetails>
</SetGroupDetails>

ResponseXML: XML data for results returned by this API
<SetGroupDetailsResponse>
<GroupID>New or Existing GroupID</GroupID>
<GroupActions>ORed Actions Values to handle multiple action requests in one XML /Command exchange</GroupActions>
<GroupActionsResult>Success or Failure</GroupActionsResult>
<ErrorCode>Specific error code (say -10)</ErrorCode>
<ErrorDescription>Error code Description (such as DB couldn't be Accessed)</ErrorDescription>
</SetGroupDetailsResponse>	

Return values: 0 in case of success, <0 in case of failure

 */
extern  int compareGroupIds( SEndDevicesGroup *group1, SEndDevicesGroup *group2);
int EditGroup(int groupId,char* AddDeviceIDs,char *RemoveDeviceIDs,char*GroupCapabilityIDs,char*GroupIconDetails,int Action);

int SetGroupDetails(void* transaction, void *RequestXML, void **ResponseXML)
{
    unsigned int groupId=0;
    unsigned int Action=0;
    char AddDeviceIDs[SIZE_256B];
    char RemoveDeviceIDs[SIZE_256B];
    char GroupCapabilityIDs[SIZE_256B];
    char GroupIconDetails[SIZE_256B];
    unsigned retStatus=PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;
    char resp[SIZE_512B];
    char *RspXML=NULL;

    if(!RequestXML)
    {
	retStatus=PLUGIN_FAILURE;	
	APP_LOG("GROUPING", LOG_ERR, "Request XML received is NULL ");
	goto func_exit;
    }
    tree = mxmlLoadString(NULL, (char*)RequestXML, MXML_OPAQUE_CALLBACK);
    if(tree){

	find_node = mxmlFindElement(tree, tree, "GroupID", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    groupId=atoi(find_node->child->value.opaque);
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}

	find_node = mxmlFindElement(tree, tree, "GroupActions", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    Action=atoi(find_node->child->value.opaque);
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}
	memset(AddDeviceIDs,0,sizeof(AddDeviceIDs));
	find_node = mxmlFindElement(tree, tree, "AddDeviceIDs", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    strcpy(AddDeviceIDs,(find_node->child->value.opaque));
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}
	memset(RemoveDeviceIDs,0,sizeof(RemoveDeviceIDs));
	find_node = mxmlFindElement(tree, tree, "RemoveDeviceIDs", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    strncpy(RemoveDeviceIDs,(find_node->child->value.opaque),sizeof(RemoveDeviceIDs)-1);
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}
	memset(GroupCapabilityIDs,0,sizeof(GroupCapabilityIDs));
	find_node = mxmlFindElement(tree, tree, "GroupCapabilityIDs", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    strncpy(GroupCapabilityIDs,(find_node->child->value.opaque),sizeof(GroupCapabilityIDs)-1);
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}
	memset(GroupIconDetails,0,sizeof(GroupIconDetails));
	find_node = mxmlFindElement(tree, tree, "GroupIconDetails", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    strncpy(GroupIconDetails,(find_node->child->value.opaque),sizeof(GroupIconDetails)-1);
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}
    }

    switch(Action)
    {

	case CMD_CREATE_GRP : 
	    {
		int errorCode=PLUGIN_SUCCESS;
		SGroupDetails *pGroupInfo=NULL;

		pGroupInfo=(SGroupDetails*)malloc(sizeof(SGroupDetails));
		if(!pGroupInfo)
		{
		    retStatus=PLUGIN_FAILURE;
		    goto func_exit;
		}
		memset(pGroupInfo,0,sizeof(SGroupDetails));
		pGroupInfo->groupId=groupId;
		if(strlen(GroupIconDetails) >0)
		{
		    char GroupName[SIZE_256B];
		    unsigned int IconVersion;
		    memset(GroupName,0,sizeof(GroupName));
		    sscanf(GroupIconDetails,"%[^:]:%u",GroupName,&IconVersion);
		    strncpy(pGroupInfo->groupName,GroupName,sizeof(pGroupInfo->groupName)-1);
		    pGroupInfo->groupIconVer=IconVersion;
		}
		if((strlen(GroupCapabilityIDs)>0))
		{
		    strncpy(pGroupInfo->capabilityList,GroupCapabilityIDs,sizeof(pGroupInfo->capabilityList)-1);
		}
		if(strlen(AddDeviceIDs)>0)
		{
		    strncpy(pGroupInfo->endDeviceIdList,AddDeviceIDs,sizeof(pGroupInfo->endDeviceIdList)-1);
		}
		CreateGroup(groupId,pGroupInfo,&errorCode);
		free(pGroupInfo);
	    }
	    break;
	case CMD_DEL_GRP :
	    {
		int errorcode=PLUGIN_SUCCESS;
		DeleteGroup(groupId,&errorcode);
	    }
	    break;
	default :								
	    /*check whether 
	      1.Add Device Ids are available or Remove Device is available 
	      depending on that actions would be taken 
	      2.If both are NULL and only GroupCapabilityIds or GroupName is present done edit */ 
	    EditGroup(groupId,AddDeviceIDs,RemoveDeviceIDs,GroupCapabilityIDs,GroupIconDetails,Action); 
	    break;
    }

func_exit:	

    memset(resp,0,sizeof(resp));
    sprintf(resp,"<SetGroupDetailsResponse><GroupID>%d</GroupID><GroupActions>%d</GroupActions><GroupActionsResult>%d</GroupActionsResult><ErrorCode>0</ErrorCode></SetGroupDetailsResponse>",groupId,Action,retStatus);

    RspXML=((char*)malloc(sizeof(char)*strlen(resp)));

    if(RspXML ==NULL)
    {
	retStatus=PLUGIN_FAILURE;
	goto func_exit;
    }

    memset(RspXML,0,strlen(resp));
    memcpy(RspXML,resp,strlen(resp));
    *ResponseXML = (void*)RspXML;

    APP_LOG("GROUPING", LOG_ERR, "Response XML formed as %s ",RspXML);
    return retStatus;
}
int EditGroup(int groupId,char* AddDeviceIDs,char *RemoveDeviceIDs,char*GroupCapabilityIDs,char*GroupIconDetails,int Action)
{
    int errorcode=PLUGIN_SUCCESS;

    while(Action)
    {
	if((Action & CMD_ADD_TO_GRP) == CMD_ADD_TO_GRP)
	{
	    Action &= ~(CMD_ADD_TO_GRP);
	    AddToGroup( groupId,AddDeviceIDs,GroupCapabilityIDs, &errorcode);

	}
	else if ((Action & CMD_REM_FROM_GRP )== CMD_REM_FROM_GRP)
	{
	    Action &= ~(CMD_REM_FROM_GRP);	
	    RemoveFromGroup( groupId, RemoveDeviceIDs,GroupCapabilityIDs, &errorcode);
	}
	else if ((Action & CMD_EDIT_GRP_CAP)== CMD_EDIT_GRP_CAP)
	{
	    Action &= ~(CMD_EDIT_GRP_CAP);	
	    EditGroupCapabilities(groupId, GroupCapabilityIDs, &errorcode);
	}
	else if((Action & CMD_EDIT_GRP_DETAILS) == CMD_EDIT_GRP_DETAILS)
	{
	    Action &= ~(CMD_EDIT_GRP_DETAILS);	
	    char GroupName[SIZE_256B];
	    unsigned int IconVersion;
	    memset(GroupName,0,sizeof(GroupName));
	    sscanf(GroupIconDetails,"%[^:]:%u",GroupName,&IconVersion);
	    EditGroupDetails( groupId, GroupName, IconVersion, &errorcode);

	}
	else
	{
	    APP_LOG("GROUPING", LOG_DEBUG, "Invalid Request received:%d\n", Action);				
	}
    }
    return errorcode;
}

int GetGroupDetails(void* transaction, void *RequestXML, void **ResponseXML)
{
    char  groupIds[SIZE_50B];
    mxml_node_t *tree=NULL;
    mxml_node_t *find_node=NULL;
    unsigned int retStatus=PLUGIN_SUCCESS;
    char resp[SIZE_512B];
    char *RspXML=NULL;



    memset(groupIds,0,sizeof(groupIds));
    tree = mxmlLoadString(NULL, (char*)RequestXML, MXML_OPAQUE_CALLBACK);
    if(tree){
	find_node = mxmlFindElement(tree, tree, "GroupIDs", NULL, NULL, MXML_DESCEND);
	if((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    strcpy(groupIds,find_node->child->value.opaque);
	    mxmlDelete(find_node);
	    find_node=NULL;
	}	else{
	    if(find_node){ mxmlDelete(find_node); find_node=NULL;}
	}
	if(tree)	{free(tree);tree=NULL;}
    }

    memset(resp,0,sizeof(resp));
    sprintf(resp,"<GetGroupDetailsResponse><GroupResult>Success</GroupResult>");
    if(strlen(groupIds)>0)
    {		
	char    delims[]        = ", ";
	char *result=NULL;
	SEndDevicesGroup	*mgroup = NULL, tmpgroup;
	char tmp_str[SIZE_256B];
	char save_str[SIZE_50B];

	memset(save_str,0,sizeof(save_str));
	memcpy(save_str,groupIds,sizeof(save_str));
	result=strtok(groupIds,delims);
	if(result) {
	    tmpgroup.groupInfo.groupId = atoi(result);
	    LL_SEARCH(gGroupListHead, mgroup, &tmpgroup, compareGroupIds);

	    if(mgroup)
	    {
		memset(tmp_str,0,sizeof(tmp_str));

		sprintf(tmp_str,"<GroupDetails><GroupId>%u</GroupId><DeviceIDs>%s</DeviceIDs><GroupCapabilityIDs>%s</GroupCapabilityIDs><GroupName>%s</GroupName><GroupIconVer>%u</GroupIconVer></GroupDetails>",mgroup->groupInfo.groupId,mgroup->groupInfo.endDeviceIdList,mgroup->groupInfo.capabilityList,mgroup->groupInfo.groupName,mgroup->groupInfo.groupIconVer);
		strcat(resp,tmp_str);
	    }
	}
	while((result != NULL) )
	{
	    result = strtok(NULL, delims);
	    if(!result){break;}
	    tmpgroup.groupInfo.groupId = atoi(result);
	    LL_SEARCH(gGroupListHead, mgroup, &tmpgroup, compareGroupIds);

	    if(mgroup)
	    {
		memset(tmp_str,0,sizeof(tmp_str));
		sprintf(tmp_str,"<GroupDetails><GroupId>%u</GroupId><DeviceIDs>%s</DeviceIDs><GroupCapabilityIDs>%s</GroupCapabilityIDs><GroupName>%s</GroupName><GroupIconVer>%u</GroupIconVer></GroupDetails>",mgroup->groupInfo.groupId,mgroup->groupInfo.endDeviceIdList,mgroup->groupInfo.capabilityList,mgroup->groupInfo.groupName,mgroup->groupInfo.groupIconVer);
		strcat(resp,tmp_str);
	    }
	}
    }
    strcat(resp," </GetGroupDetailsResponse>");
    APP_LOG("GROUPING", LOG_ERR, "Response and length of XML formed as %s  %d",resp,strlen(resp));
    RspXML=((char*)malloc(sizeof(char)*strlen(resp)));

    if(RspXML ==NULL)
    {
	retStatus=PLUGIN_FAILURE;
	goto func_exit;
    }

    memset(RspXML,0,strlen(resp));
    memcpy(RspXML,resp,strlen(resp));
    *ResponseXML = (void*)RspXML;

    APP_LOG("GROUPING", LOG_ERR, "Response XML formed as %s ",RspXML);
func_exit:
    return retStatus;
}
