/***************************************************************************
 *
 * groupingExtIf.h
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
#ifndef __GROUPINGEXTIF_H
#define __GROUPINGEXTIF_H


/* MACROS for various grouping COMMANDS */

#define CMD_CREATE_GRP		0x00000001
#define CMD_ADD_TO_GRP		0x00000002
#define CMD_REM_FROM_GRP	0x00000004
#define CMD_DEL_GRP		0x00000008
#define CMD_EDIT_GRP_CAP	0x00000010
#define CMD_EDIT_GRP_DETAILS	0x00000020



//Initialization interface Grouping Module
void *groupingInit(void *);

//Destroy interface for Grouping Module
void* groupingDestroy(void *);

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
int SetGroupDetails(void* transaction, void *RequestXML, void **ResponseXML); 

/* GetGroupDetails: API to get details of a group

Argument details:
 transaction: Reserved for future use, for now should be NULL
 RequestXML: XML data containing grouping action and data. Format detailed below:
	<GetGroupDetails>
      		<GroupIDs>Existing GroupID1, GroupID2, GroupID3</GroupIDs>
 	</GetGroupDetails>

 ResponseXML: XML data for results returned by this API
    <GetGroupDetailsResponse>
      <GroupResult>Success or Partial Success or Failure</GroupResult>
      <ErrorCode>Specific error code (say -10)</ErrorCode>
      <ErrorDescription>Specific Description (such as DB couldn't be Accessed)</ErrorDescription>
      <GroupDetails>
               <GroupID>GroupID1</GroupID>
               <DeviceIDs>List of comma separated device indexes for this existing group</AddDeviceIDs>
               <GroupCapabilityIDs>Capabilities Id along with their current values</GroupCapabilityIDs>
               <GroupName>Group Friendly Name</GroupName>
               <GroupIconVer>Group Icon Version</GroupIconVer>
      </GroupDetails>
      <GroupDetails>
               <GroupID>Existing GroupID</GroupID>
               <DeviceIDs>List of comma separated device indexes for this existing group</AddDeviceIDs>
               <GroupCapabilityIDs>Capabilities Id along with their current values</GroupCapabilityIDs>
               <GroupName>Group Friendly Name</GroupName>
               <GroupIconVer>Group Icon Version</GroupIconVer>
      </GroupDetails>
    </GetGroupDetailsResponse>


Return values: 0 in case of success, <0 in case of failure

*/

int GetGroupDetails(void* transaction, void *RequestXML, void **ResponseXML);

/* Atomic APIs for various group actions follow */

/* CreateGroup: API to create new group
	Arguments:
		groupId: shared by App to uniquely identify a group
		groupDetails: contains the details of the group like end devices index, capabilities etc.
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/
int CreateGroup(unsigned int groupId, SGroupDetails *groupDetails, int *pErrorCode);


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
int AddToGroup(unsigned int groupId, char* addDeviceList, char* updatedCapabilityList, int *pErrorCode);


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

int RemoveFromGroup(unsigned int groupId, char* removeDeviceList, char* updatedCapabilityList, int *pErrorCode);


/* DeleteGroup: API to delete existing group
	Arguments:
		groupId: shared by App to uniquely identify a group
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/


int DeleteGroup(unsigned int groupId, int *pErrorCode);

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


int EditGroupDetails(unsigned int groupId, char *groupName, int groupIconVersion, int *pErrorCode);


/* EditGroupCapabilities: API to change group capabilities
	Arguments:
		groupId: shared by App to uniquely identify a group
		groupCapabilityList: comma seperated absolute list of capabilities ids
		pErrorCode: to return specific error code in case of failure

	Return Values:
		0: success
		<0 : failure
*/


int EditGroupCapabilities(unsigned int groupId, char* groupCapabilityList, int *pErrorCode);

#endif
