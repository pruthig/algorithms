
/***************************************************************************
 *
 *
 * groupingExtIf.c
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
#include <sys/time.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/stat.h>

#include "types.h"
#include "defines.h"
#include "logger.h"

#include "WemoDB.h"

#include "utlist.h"

#include "groupingDS.h"
#include "groupingExtIf.h"
#include "groupingDefs.h"
#include "stdbool.h"

#define NO_OF_CHARS 256
#define GROUPING_DB_FILE "grouping.db"

extern sqlite3 *g_GroupingDB;
int s_groupInitiallized=PLUGIN_SUCCESS;

typedef struct dev_node{
		char value[SIZE_256B];
		struct dev_node *next;
}dev_node_t;




SEndDevicesGroup *gGroupListHead=NULL;
void *groupingInit(void *);
void  *groupingInit(void * param)
{
		int retStatus=PLUGIN_SUCCESS;

		retStatus=initGroupingDB(GROUPING_DB_FILE);

		if(PLUGIN_FAILURE == retStatus)
		{
				s_groupInitiallized=PLUGIN_FAILURE;
		}

		return NULL;
}
																						
//comapare group  id
 int compareGroupIds( SEndDevicesGroup *group1, SEndDevicesGroup *group2) {
		if (group1->groupInfo.groupId == group2->groupInfo.groupId) {
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}
static int compare_value( struct dev_node *group1, struct dev_node *group2) {
		if (strcmp(group1->value ,group2->value)==0) {
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}
/* CreateGroup: API to create new group
	Arguments:
		groupId: shared by App to uniquely identify a group
		groupDetails: contains the details of the group like end devices index, capabilities etc.
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/
int CreateGroup(unsigned int groupId, SGroupDetails *groupDetails, int *pErrorCode)
{
		SEndDevicesGroup *mgroup=NULL;
		int retStatus=PLUGIN_FAILURE;
		char Condition[SIZE_256B];
		char TimeStamp[SIZE_32B];
		time_t UTCcurTime;

		if(!groupDetails) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Detail of groupId:%d is NULL", groupId);
				goto func_exit;
		}

		if(groupId <=PLUGIN_SUCCESS)
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not valid", groupId);
				goto func_exit;
		}		
		APP_LOG("GROUPING", LOG_DEBUG, "Request to create group instance by Id:%u\n", groupId);
		mgroup=(SEndDevicesGroup*)malloc(sizeof(SEndDevicesGroup));

		if(mgroup){
				memset(mgroup,0,sizeof(SEndDevicesGroup));
				mgroup->groupInfo.groupId=groupId;
				/*TODO 1. Creates association for end devices with received "GroupID" in "End Devices Schema Table". */
				memcpy(&(mgroup->groupInfo),groupDetails,sizeof(SGroupDetails));
				LL_APPEND(gGroupListHead,mgroup); 	
				memset(Condition, 0, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "ROWID in (%s)", mgroup->groupInfo.endDeviceIdList);
				char groupId_s[SIZE_8B];
				memset(groupId_s, 0x00, sizeof(groupId_s));
				snprintf(groupId_s,sizeof(groupId_s),"%u",groupId);
				APP_LOG("GROUPING", LOG_DEBUG, " group id in string is %s\n", groupId_s);

				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				ColDetails GroupId[]={{"GroupID",groupId_s},{"TimeStamp",TimeStamp}};
				WeMoDBUpdateTable(&g_GroupingDB, "DevicesSchemaTable", GroupId,2 , Condition);
				/*TODO 2. Creating a new entry in "Group Schema Table" for this new group along with "GroupID" and its elements which are mandatory elements while creating a new group. */
				char    delims[]        = ",";
				unsigned int     numberOfCapabilities=0;
				char		*result=NULL;
				char     tmp_str[SIZE_256B];

				memset(tmp_str,0,sizeof(tmp_str));
				memcpy(tmp_str,mgroup->groupInfo.capabilityList,strlen(mgroup->groupInfo.capabilityList));
				result=strtok(tmp_str,delims);
				if(result) {numberOfCapabilities++;}
				while((result != NULL) )
				{
						numberOfCapabilities++;
						result = strtok(NULL, delims);
				}
				APP_LOG("GROUPING", LOG_DEBUG, "No of Capabilities found as %u",numberOfCapabilities);
				char NumberOfCapabilities[SIZE_8B],groupIconVer_s[SIZE_8B];
				memset(NumberOfCapabilities,0,sizeof(NumberOfCapabilities));
				snprintf(NumberOfCapabilities,sizeof(NumberOfCapabilities),"%u",numberOfCapabilities);
				memset(groupIconVer_s,0,sizeof(groupIconVer_s));
				snprintf(groupIconVer_s,sizeof(groupIconVer_s),"%u",mgroup->groupInfo.groupIconVer);

				char capabilityList[SIZE_256B];
				memset(capabilityList,0,sizeof(capabilityList));
				sprintf(capabilityList,"\"%s\"",mgroup->groupInfo.capabilityList);

				char groupName_s[SIZE_256B];
				memset(groupName_s,0,sizeof(groupName_s));
				sprintf(groupName_s,"\"%s\"",mgroup->groupInfo.groupName);

				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				ColDetails GroupInfo[]={{"GroupID",groupId_s},{"GroupCapability",capabilityList},{"NumberOfCapabilities",NumberOfCapabilities},{"GroupIconName",groupName_s},{"GroupIconversion",groupIconVer_s},{"TimeStamp",TimeStamp}};
				if(WeMoDBInsertInTable(&g_GroupingDB, "GroupingsSchemaTable",GroupInfo ,6))
				{
						APP_LOG("GROUPING", LOG_DEBUG, "Entry failed to store in GroupingsSchemaTable ");
						retStatus=PLUGIN_FAILURE;
						goto func_exit;
				}
				APP_LOG("GROUPING", LOG_DEBUG, "Entry Successfully  stored in GroupingsSchemaTable for groupId=%d ",groupId);
				retStatus=PLUGIN_SUCCESS;
		}
func_exit:
		return retStatus;
}

int DeleteGroup(unsigned int groupId, int *pErrorCode) 
{
		SEndDevicesGroup	*mgroup = NULL, tmpgroup;
		int retStatus = PLUGIN_FAILURE;
		char **s8ResultArray=NULL;
		char query[SIZE_256B];
		char Condition[SIZE_256B];
		char rowids[30];
		int NoOfRows=0,NoOfCols=0;
		char TimeStamp[SIZE_32B];
		time_t UTCcurTime;

		if (groupId <= PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE; 
				APP_LOG("GROUPING", LOG_ERR, "Grouping core instance of componentId:%d is not valid", groupId);
				goto func_exit;
		}

		APP_LOG("GROUPING", LOG_DEBUG, "Request to destroy Group instance by componentId:%d", groupId);
		tmpgroup.groupInfo.groupId = groupId;
		LL_SEARCH(gGroupListHead, mgroup, &tmpgroup, compareGroupIds);
		if (mgroup) {
				LL_DELETE(gGroupListHead, mgroup);
				free(mgroup);
				mgroup = 0;
				memset(query, 0, sizeof(query));
				snprintf(query, sizeof(query), "select DeviceID from DevicesSchemaTable where GroupId =%u ",groupId);
				if(WeMoDBGetTableData(&g_GroupingDB,query,&s8ResultArray,&NoOfRows,&NoOfCols))
				{
						APP_LOG("GROUPING", LOG_DEBUG, "Failed to get DeviceID for groupid %d ", groupId);
						retStatus=PLUGIN_FAILURE;
						goto func_exit;
				}
				if(NoOfRows)
				{
						APP_LOG("GROUPING", LOG_DEBUG, "No of Rows rcvd from DBQuery is =%d",NoOfRows);
						int counter=1;
						memset(rowids,0,sizeof(rowids));
						strncpy(rowids,s8ResultArray[++counter],sizeof(s8ResultArray[counter]));
						for(;counter < NoOfRows;counter++)
						{
								strcat(rowids,",");
								strcat(rowids,s8ResultArray[counter]);
								APP_LOG("GROUPING", LOG_DEBUG, "Row list is %s for groupid %d is", rowids,groupId);
						}
				}
				memset(Condition, 0x00, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "ROWID in (%s)", rowids);
				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				ColDetails GroupId[]={{"GroupID","0"},{"TimeStamp",TimeStamp}};
				if(WeMoDBUpdateTable(&g_GroupingDB, "DevicesSchemaTable", GroupId,2 , Condition))
				{
						retStatus=PLUGIN_FAILURE;
						goto func_exit; 
				}
				APP_LOG("GROUPING", LOG_DEBUG, "Deleting GroupID: %d \n",groupId);
				char GroupCondition[SIZE_256B];
				memset(GroupCondition,0,sizeof(GroupCondition));
				snprintf(GroupCondition, sizeof(GroupCondition), "GroupId = %d", groupId);
				retStatus= WeMoDBDeleteEntry(&g_GroupingDB,"GroupingsSchemaTable",GroupCondition);
				APP_LOG("GROUPING", LOG_DEBUG, "Grouping core instance of componentId:%d deleted successfull", groupId);
		} else {
				APP_LOG("GROUPING", LOG_ERR, "Grouping core instance of componentId:%d not found ", groupId);
				retStatus = PLUGIN_FAILURE; 
		}

func_exit:
		return retStatus;
}
/* AddToGroup: API to add new end devices to a existing group
	Arguments:
		groupId: shared by App to uniquely identify a group
		addDeviceList: comma seperated list of end device indices
		updatedCapabilityList: comma seperated absolute list of capability ids
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/
int AddToGroup(unsigned int groupId, char* addDeviceList, char* updatedCapabilityList, int *pErrorCode)
{
		SEndDevicesGroup *mgroup=NULL,tmpgroup;
		int retStatus=PLUGIN_FAILURE;
		char Condition[SIZE_256B];
		char TimeStamp[SIZE_32B];
		time_t UTCcurTime;

		if(groupId <=PLUGIN_SUCCESS)
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not valid", groupId);
				goto func_exit;
		}		
		if(!addDeviceList) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Updated DeviceList  of groupId:%d is NULL", groupId);
				goto func_exit;
		}

		if(!updatedCapabilityList) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Updated Capability List  of groupId:%d is NULL", groupId);
				goto func_exit;
		}
		APP_LOG("GROUPING", LOG_DEBUG, "Request to create group instance by Id:%d\n", groupId);
		tmpgroup.groupInfo.groupId=groupId;
		LL_SEARCH(gGroupListHead,mgroup,&tmpgroup,compareGroupIds);
		if(mgroup){
				strcat(mgroup->groupInfo.endDeviceIdList,",");
				strncat(mgroup->groupInfo.endDeviceIdList,addDeviceList,strlen(addDeviceList));
				strncpy(mgroup->groupInfo.capabilityList,updatedCapabilityList,sizeof(mgroup->groupInfo.capabilityList)-1);
				memset(Condition, 0x00, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "ROWID in (%s)", mgroup->groupInfo.endDeviceIdList);
				char groupId_s[SIZE_4B];
				memset(groupId_s, 0x00, sizeof(groupId_s));
				snprintf(groupId_s,sizeof(groupId_s),"%u",groupId);
				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				ColDetails GroupId[]={{"GroupID",groupId_s},{"TimeStamp",TimeStamp}};
				if(WeMoDBUpdateTable(&g_GroupingDB, "DevicesSchemaTable", GroupId,2 , Condition))
				{
						retStatus=PLUGIN_FAILURE;
						APP_LOG("GROUPING", LOG_DEBUG, "AddToGroup Action failed to update DB for groupId =%d", groupId);
						goto func_exit;
				}
				retStatus=PLUGIN_SUCCESS;
				APP_LOG("GROUPING", LOG_DEBUG, "Grouping core instance of groupId:%d created successfully", groupId);
		}
		else
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not exist", groupId);
				goto func_exit;
		}
func_exit:
		return retStatus;
}
/* EditGroupDetails: API to change group details like friendly name, icon etc.
	Arguments:
		groupId: shared by App to uniquely identify a group
		groupName: Group friendly name, NULL in case no change required
		groupIconVersion: Group icon version, -1 in case no change required
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/
int EditGroupDetails(unsigned int groupId, char *groupName, int groupIconVersion, int *pErrorCode)
{
		SEndDevicesGroup *mgroup=NULL,tmpgroup;
		int retStatus=PLUGIN_FAILURE;
		char Condition[SIZE_256B];
		char TimeStamp[SIZE_32B];
		time_t UTCcurTime;

		if(groupId <=PLUGIN_SUCCESS)
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not valid", groupId);
				goto func_exit;
		}		
		if(!groupName) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Updated Name  of groupId:%d is NULL", groupId);
				goto func_exit;
		}

		if(groupIconVersion <=PLUGIN_SUCCESS)
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "New icon version  of groupId:%d is not valid", groupId);
				goto func_exit;
		}		
		APP_LOG("GROUPING", LOG_DEBUG, "Request to create group instance by Id:%d\n", groupId);
		tmpgroup.groupInfo.groupId=groupId;
		LL_SEARCH(gGroupListHead,mgroup,&tmpgroup,compareGroupIds);
		if(mgroup){
				memset(&mgroup->groupInfo.groupName,0,sizeof(mgroup->groupInfo.groupName));
				strncpy(mgroup->groupInfo.groupName,groupName,sizeof(mgroup->groupInfo.groupName)-1);
				mgroup->groupInfo.groupIconVer=groupIconVersion;
				memset(Condition, 0x00, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "GroupId in (%d)", groupId);
				char groupIconVer_s[SIZE_8B];
				memset(groupIconVer_s,0,sizeof(groupIconVer_s));
				snprintf(groupIconVer_s,sizeof(groupIconVer_s),"%u",groupIconVersion);

				char groupName_s[SIZE_256B];
				memset(groupName_s,0,sizeof(groupName_s));
				sprintf(groupName_s,"\"%s\"",groupName );
				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				ColDetails GroupDetails[]={{"GroupIconName",groupName_s},{"GroupIconversion",groupIconVer_s},{"TimeStamp",TimeStamp}};
				if(WeMoDBUpdateTable(&g_GroupingDB, "GroupingsSchemaTable", GroupDetails,3 , Condition))
				{
						APP_LOG("GROUPING", LOG_DEBUG, "Failed to Update Group Info for groupId=%d", groupId);
						retStatus=PLUGIN_FAILURE;
						goto func_exit;
				}
				retStatus=PLUGIN_SUCCESS;
				APP_LOG("GROUPING", LOG_DEBUG, "Grouping core instance of groupId:%d updated successfully", groupId);
		}
		else
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not exist", groupId);
				goto func_exit;
		}
func_exit:
		return retStatus;
}
/* EditGroupDetails: API to change group details like friendly name, icon etc.
	Arguments:
		groupId: shared by App to uniquely identify a group
		groupName: Group friendly name, NULL in case no change required
		groupIconVersion: Group icon version, -1 in case no change required
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/
int EditGroupCapabilities(unsigned int groupId, char* groupCapabilityList, int *pErrorCode)
{
		SEndDevicesGroup *mgroup=NULL,tmpgroup;
		int retStatus=PLUGIN_FAILURE;
		char Condition[SIZE_256B];
		char *result=NULL;
		char TimeStamp[SIZE_32B];
		time_t UTCcurTime;

		if(!groupCapabilityList) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Updated CapabilityList  of groupId:%d is NULL", groupId);
				goto func_exit;
		}

		if(groupId <=PLUGIN_SUCCESS)
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not valid", groupId);
				goto func_exit;
		}		
		APP_LOG("GROUPING", LOG_DEBUG, "Request to create group instance by Id:%d\n", groupId);
		tmpgroup.groupInfo.groupId=groupId;
		LL_SEARCH(gGroupListHead,mgroup,&tmpgroup,compareGroupIds);
		if(mgroup){
				memset(&mgroup->groupInfo.capabilityList,0,sizeof(mgroup->groupInfo.capabilityList));
				strncpy(mgroup->groupInfo.capabilityList,groupCapabilityList,sizeof(mgroup->groupInfo.capabilityList)-1);//TBD
				char    delims[]        = ",";
				int     numberOfCapabilities=0;
				result=strtok(mgroup->groupInfo.capabilityList,delims);
				if(result)numberOfCapabilities++;
				while((result != NULL) )
				{
						numberOfCapabilities++;
						result = strtok(NULL, delims);
				}
				memset(Condition, 0x00, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "GroupId in (%d)", groupId);
				char NumberOfCapabilities[SIZE_256B];
				memset(NumberOfCapabilities,0,sizeof(NumberOfCapabilities));
				snprintf(NumberOfCapabilities,sizeof(NumberOfCapabilities),"%u",numberOfCapabilities);

				char capabilityList[SIZE_256B];
				memset(capabilityList,0,sizeof(capabilityList));
				sprintf(capabilityList,"\"%s\"",mgroup->groupInfo.capabilityList);
				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				ColDetails GroupDetails[]={{"GroupCapability",capabilityList},{"NumberOfCapabilities",NumberOfCapabilities},{"TimeStamp",TimeStamp}};
				if(WeMoDBUpdateTable(&g_GroupingDB, "GroupingsSchemaTable", GroupDetails,3 , Condition))
				{
						APP_LOG("GROUPING", LOG_DEBUG, "DB update fail for GroupCapability groupId=%d", groupId);
						retStatus=PLUGIN_FAILURE;
						goto func_exit;
				}
				retStatus=PLUGIN_SUCCESS;
				APP_LOG("GROUPING", LOG_DEBUG, "Grouping core instance of groupId:%d updated successfully", groupId);
		}
		else
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not exist", groupId);
				goto func_exit;
		}
func_exit:
		return retStatus;
}
char* removeDuplicate(char*str,char* result)
{
		int ip_ind = 0, res_ind = 0; 
		char temp;    

		if(!result){ return NULL;}
		/* In place removal of duplicate characters*/ 
		while(*(str + ip_ind))
		{
				temp = *(str + ip_ind);
				if( (*result != temp) )
				{
						if((*(str + ip_ind)==',' ) && (*(str+res_ind)==','))
								ip_ind++;
						*(str + res_ind) = *(str + ip_ind);
						res_ind++;         
				}
				ip_ind++;
		}      

		/* After above step string is stringiittg.
			 Removing extra iittg after string*/       
		*(str+res_ind) = '\0';   

		return str;
}

struct dev_node * create_dev_node()
{
		struct dev_node *tmp=NULL;

		tmp=(struct dev_node*)malloc(sizeof(struct dev_node));

		return tmp;

}
/* RemoveFromGroup: API to remove existing end devices from a existing group
	Arguments:
		groupId: shared by App to uniquely identify a group
		removeDeviceList: comma seperated list of end device indices
		updatedCapabilityList: comma seperated absolute list of capability ids
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/

int RemoveFromGroup(unsigned int groupId, char* removeDeviceList,char* updatedCapabilityList, int *pErrorCode)
{
		SEndDevicesGroup *mgroup=NULL,tmpgroup;
		int retStatus=PLUGIN_FAILURE;
		char Condition[SIZE_256B];
		char    delims[]= ",";
		struct dev_node *head=NULL;
		struct dev_node *lnode=NULL;
		struct dev_node *find_node=NULL;
		char op_str[SIZE_256B];
		char TimeStamp[SIZE_32B];
		time_t UTCcurTime;

		if(!updatedCapabilityList) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Updated CapabilityList  of groupId:%d is NULL", groupId);
				goto func_exit;
		}

		if(!removeDeviceList) {
				retStatus=PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Updated DeviceList  of groupId:%d is NULL", groupId);
				goto func_exit;
		}
		if(groupId <=PLUGIN_SUCCESS)
		{
				retStatus = PLUGIN_FAILURE;
				APP_LOG("GROUPING", LOG_ERR, "Grouping Instance  of groupId:%d is not valid", groupId);
				goto func_exit;
		}		
		tmpgroup.groupInfo.groupId=groupId;
		LL_SEARCH(gGroupListHead,mgroup,&tmpgroup,compareGroupIds);
		if(mgroup){
				if(mgroup->groupInfo.endDeviceIdList)
				{
						struct dev_node *temp_node;
						char *temp=NULL;
						char tmp_devstr[SIZE_256B];

						memset(tmp_devstr,0,sizeof(tmp_devstr));
						memcpy(tmp_devstr,mgroup->groupInfo.endDeviceIdList,strlen(mgroup->groupInfo.endDeviceIdList));
						temp=strtok(tmp_devstr,delims);
						while(temp!=NULL)
						{
								temp_node=create_dev_node();
								if(!temp_node)
								{
										retStatus=PLUGIN_FAILURE;
										goto func_exit;
								}
								memset(temp_node->value,0,sizeof(temp_node->value));
								memcpy(temp_node->value,temp,strlen(temp));
								LL_APPEND(head,temp_node);
								temp=strtok(NULL,delims);
						}
				}
				char temp_str[SIZE_256B];
				char *result=NULL;
				memset(temp_str,0,sizeof(temp_str));
				memcpy(temp_str,removeDeviceList,strlen(removeDeviceList));
				APP_LOG("GROUPING", LOG_DEBUG, "temp_str is %s", temp_str);
				result=strtok(temp_str,delims);
				while(result !=NULL){ 
						struct dev_node trav_node;
						memset(trav_node.value,0,sizeof(trav_node.value));
						memcpy(trav_node.value,result,strlen(result));

						struct dev_node *mnode;
						LL_SEARCH(head,mnode,&trav_node,compare_value);
						if(mnode)
						{
								LL_DELETE(head,mnode);
						}
						result=strtok(NULL,delims);
				}

				memset(op_str,0,sizeof(op_str));
				if(head)
				{
						strncpy(op_str,head->value,sizeof(op_str)-1);
				}
				LL_FOREACH_SAFE(head, lnode,find_node)
				{

						if(lnode->next!=NULL)
						{
								strcat(op_str,",");
								strncat(op_str,lnode->next->value,strlen(lnode->next->value));
						}
						APP_LOG("GROUPING", LOG_DEBUG, "op_str is %s", op_str);
				}
				APP_LOG("GROUPING", LOG_DEBUG, "Updated EndDevices after removal is %s", op_str);
				memset(mgroup->groupInfo.endDeviceIdList,0,sizeof(mgroup->groupInfo.endDeviceIdList));
				memcpy(mgroup->groupInfo.endDeviceIdList,op_str,strlen(op_str));
				memset(mgroup->groupInfo.capabilityList,0,sizeof(mgroup->groupInfo.capabilityList));
				memcpy(mgroup->groupInfo.capabilityList,updatedCapabilityList,strlen(updatedCapabilityList));
				APP_LOG("GROUPING", LOG_DEBUG, "Updated EndDevices after removal is %s", mgroup->groupInfo.endDeviceIdList);
				APP_LOG("GROUPING", LOG_DEBUG, "Updated EndDevice Capabilities after removal is %s", mgroup->groupInfo.capabilityList);
				UTCcurTime = GetUTCTime();
				memset(TimeStamp, 0x00, sizeof(TimeStamp));
				snprintf(TimeStamp, sizeof(TimeStamp), "%lu", UTCcurTime);
				
				char capabilityList[SIZE_256B];
				memset(capabilityList,0,sizeof(capabilityList));
				sprintf(capabilityList,"\"%s\"",mgroup->groupInfo.capabilityList);
			
				memset(Condition, 0x00, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "GroupID in (%u)", mgroup->groupInfo.groupId);
				ColDetails GroupCapability[]={{"TimeStamp",TimeStamp},{"GroupCapability",capabilityList}};
				if(WeMoDBUpdateTable(&g_GroupingDB, "GroupingsSchemaTable", GroupCapability,2 , Condition))
				{
						APP_LOG("GROUPING", LOG_DEBUG, "Request to delete device[DB] from group:%d\n failed", groupId);
						retStatus=PLUGIN_FAILURE;
						goto func_exit;
				}

				memset(Condition, 0x00, sizeof(Condition));
				snprintf(Condition, sizeof(Condition), "DeviceID in (%s)", removeDeviceList);
				ColDetails GroupId[]={{"GroupID","0"},{"TimeStamp",TimeStamp}};
				if(WeMoDBUpdateTable(&g_GroupingDB, "DevicesSchemaTable", GroupId,2 , Condition))
				{
						APP_LOG("GROUPING", LOG_DEBUG, "Request to delete device[DB] from group:%d\n failed", groupId);
						retStatus=PLUGIN_FAILURE;
						goto func_exit;
				}

		}	
func_exit:
		return retStatus;
}
