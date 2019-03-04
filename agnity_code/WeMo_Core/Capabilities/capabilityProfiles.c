/***************************************************************************
 *
 *
 * capabilityProfiles.c
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

#include "types.h"
#include "defines.h"
#include "logger.h"

#include "utlist.h"

#include "capabilityStructs.h"
#include "capabilityIface.h"
#include "capabilityDefs.h"
#include "capabilityCore.h"

//Global Profiles
CapabilityProfileData gCapabilityGlobalProfiles[] = {
		{BinaryCtl, BinaryCtlS, WiFi|ZigBee, Boolean, "ON:1,OFF:0", RW, "NULL", NULL, NULL},
		{LinearCtl, LinearCtlS, WiFi|ZigBee, IntegerRange, "MIN:0,MAX:100,RATE:120000", RW, "NULL", NULL, NULL},
		{ToggleCtl, ToggleCtlS, WiFi|ZigBee, Integer, "TOGGLE:2", RW, "NULL", NULL, NULL},
		{DiscreteCtl, DiscreteCtlS, WiFi|ZigBee, IntegerSet, "VAL1:2,VAL2:5,VAL3:7", RW, "NULL", NULL, NULL},
		{MessageCtl, MessageCtlS, WiFi|ZigBee, String, "MSG:Alert From Sensor", RO, "NULL", NULL, NULL},
};

//List of available capability created at runtime based on the joined devices
CapabilityProfiles *gCapabilityRunTimeProfiles = NULL;

//Last assigned runtime capability id
unsigned int gLastRuntimeCapbilityId = 0x00000000;

#if 0
//Total number of available profiles at any point of time
unsigned int gNumOfProfiles = 0;
#endif

//Interface to get last used capability runtime id
//IN:reserved - Reserved for future use, for now should be NULL
//This interface will return last used run time capability id on success,
//0 if no capability is present or <0 in case of any error.
int getLastRunTimeCapabilityId(void *reserved)
{
		int retStatus = PLUGIN_SUCCESS;
		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Last stored capability id is %d", gLastRuntimeCapbilityId);
		return gLastRuntimeCapbilityId;
}

//Interface to get count of available Capabilities profile
//IN:reserved - Reserved for future use, for now should be NULL
//This interface will return number of availabale capability profiles,
//0 if no capability is present or <0 in case of any error.
int getNumOfCapabilityProfiles(void *reserved)
{
		int count = 0;
		CapabilityProfiles *lprofiles = NULL;
		int retStatus = PLUGIN_SUCCESS;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		//get the counts of CapabilityProfiles node in gCapabilityRunTimeProfiles 
		LL_COUNT(gCapabilityRunTimeProfiles, lprofiles, count);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Number of capability profiles available is %d", count);
		return count;
}

//Interface to get available Capability IDs
//IN:capabilityIds - Intger Array for capability ids which will be filled by Ids from this core,
//size of the array should be same as the one returned from getNumOfCapabilities interface
//IN:NumberOfEntries - Size of array capabilityIds
//OUT:capabilityIds - This array will be filled with capability ids available
//This interface will also return number of capability ids entries filled in capabilityIds,
//0 if no id is present or <0 in case of any error.
//If return number of capability ids is less than the return of getNumOfCapabilities then caller of
//getCapabilityProfileIDs can again call this interface with correct size of capabilityIds
int getCapabilityProfileIDs(unsigned int capabilityIds[], unsigned int numOfEntries) 
{
		int retStatus = PLUGIN_FAILURE, i=0, count = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if (numOfEntries <= PLUGIN_SUCCESS) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Number of Entries should always be greater than Zero:%d\n", numOfEntries);
				retStatus = PLUGIN_FAILURE; 
				goto func_exit;
		}
		
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Number of entries requested for capability id is %d", numOfEntries);
		LL_COUNT(gCapabilityRunTimeProfiles, lprofiles, count);
		if (count <= 0) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Number of counts in gCapabilityRunTimeProfiles is Zero:%d\n", count);
				retStatus = PLUGIN_SUCCESS; 
				goto func_exit;
		}
		APP_LOG("CAPABILITIES", LOG_ERR, "Number of counts in gCapabilityRunTimeProfiles is :%d\n", count);

		LL_FOREACH_SAFE(gCapabilityRunTimeProfiles, lprofiles, tprofiles)
		{
				if (lprofiles) {
						capabilityIds[i] = lprofiles->capabilityData.capabilityId;
						APP_LOG("CAPABILITIES", LOG_ERR, "gCapabilityRunTimeProfiles index %d:capabilityId 0x%x\n", i, capabilityIds[i]);
						++i;
				} else {
						retStatus = i;
						break;
				}
				if (i >= numOfEntries) {
						retStatus = i;
						break;
				}
		}
func_exit:
		APP_LOG("CAPABILITIES", LOG_ERR, "Number of counts returned in capabilityIds[] and i is:%d-%d\n", retStatus, i);
		return retStatus;
}

//Compare capability Ids of two profiles
static int compareCapabilityIds(CapabilityProfiles *profile1, CapabilityProfiles *profile2) {
		if (profile1->capabilityData.capabilityId == profile2->capabilityData.capabilityId) {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "compareCapabilityIds = profile1:%d - profile2:%d\n", \
						profile1->capabilityData.capabilityId, profile2->capabilityData.capabilityId);
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

//Print profile attributes of this capability profile
void printCapabilityProfiles(CapabilityProfiles *elmProfile) {
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability Id 0x%x", elmProfile->capabilityData.capabilityId);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability Profile Name %s", elmProfile->capabilityData.capabilityProfileName);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability Spec %d", elmProfile->capabilityData.capabilitySpec);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability Data Type %d", elmProfile->capabilityData.capabilityDataType);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability Name Value %s", elmProfile->capabilityData.capabilityNameValue);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability Control %d", elmProfile->capabilityData.capabilityControl);
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability SubProfile Name %s", elmProfile->capabilityData.capabilitySubProfileName);
		if (elmProfile->capabilityData.on_set_action) {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability on_set_action is set");
		}
		if (elmProfile->capabilityData.on_get_action) {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability on_get_action is set");
		}
}

//Interface to get Capability Profiles Data
//IN:capabilityIds - Intger Array of capability ids for which capability profile is requested
//IN:NumberOfEntries - Number of entries in capabilityIds, if 0 all all available profiles will be returned
//OUT:CapabilityProfiles - List of Capability profiles of requested devices or all profiles
//This interface will also return number of capability entries for which CapabilityProfiles list is returned,
//0 if no profile is present or <0 in case of any error.
int getCapabilityProfiles(unsigned int capabilityIds[], unsigned int numOfEntries, CapabilityProfiles **profileData) 
{
		int retStatus = PLUGIN_FAILURE, i = 0, count = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;
		CapabilityProfiles *retProfiles = NULL, *elmProfile = NULL, cmpProfile;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		LL_COUNT(gCapabilityRunTimeProfiles, lprofiles, count);
		if (count <= 0) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Number of counts in gCapabilityRunTimeProfiles is Zero:%d\n", count);
				retStatus = PLUGIN_SUCCESS; 
				goto func_exit;
		}

		if (numOfEntries <= PLUGIN_SUCCESS) {
				//Get all available capability profiles from gCapabilityRunTimeProfiles list
				retStatus = getAllCapabilityProfilesI(profileData);
		} else {
				for (i = 0; i < numOfEntries; i++) {
						cmpProfile.capabilityData.capabilityId = capabilityIds[i];
						LL_SEARCH(gCapabilityRunTimeProfiles, lprofiles, &cmpProfile, compareCapabilityIds);
						if (lprofiles) {
								elmProfile = (CapabilityProfiles*)calloc(1, sizeof(CapabilityProfiles));
								if (elmProfile) {
										memcpy((CapabilityProfileData*)&elmProfile->capabilityData, (CapabilityProfileData *)&lprofiles->capabilityData, sizeof(CapabilityProfileData));
#if 0
										elmProfile->capabilityData.capabilityId = lprofiles->capabilityData.capabilityId;
										strncpy(elmProfile->capabilityData.capabilityProfileName, lprofiles->capabilityData.capabilityProfileName, strlen(lprofiles->capabilityData.capabilityProfileName));
										elmProfile->capabilityData.capabilitySpec = lprofiles->capabilityData.capabilitySpec;
										elmProfile->capabilityData.capabilityDataType = lprofiles->capabilityData.capabilityDataType;
										strncpy(elmProfile->capabilityData.capabilityNameValue, lprofiles->capabilityData.capabilityNameValue, strlen(lprofiles->capabilityData.capabilityNameValue));
										elmProfile->capabilityData.capabilityControl = lprofiles->capabilityData.capabilityControl;
										strncpy(elmProfile->capabilityData.capabilitySubProfileName, lprofiles->capabilityData.capabilitySubProfileName, strlen(lprofiles->capabilityData.capabilitySubProfileName));
#endif
										LL_APPEND(retProfiles, elmProfile);
										printCapabilityProfiles(elmProfile);
								}
						}
				}
				*profileData = retProfiles;
				LL_COUNT(retProfiles, tprofiles, count);
				retStatus = count;
		}
func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Number of capability profiles count returned is :%d\n", retStatus);
		return retStatus;
}

//Get all available capability profiles from gCapabilityRunTimeProfiles list
//Internal interface to get all available capability profiles,
//internal to capability core module
//This interface on successfull call returns all available capability profiles pointed by
//profileData along with number of profile counts else failure (-ve) will be returned.
int getAllCapabilityProfilesI(CapabilityProfiles **allprofiles) 
{
		int retStatus = PLUGIN_FAILURE, count = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;
		CapabilityProfiles *retProfiles = NULL, *elmProfile = NULL;

		LL_FOREACH_SAFE(gCapabilityRunTimeProfiles, lprofiles, tprofiles)
		{
				if (lprofiles) {
						elmProfile = (CapabilityProfiles*)calloc(1, sizeof(CapabilityProfiles));
						if (elmProfile) {
								memcpy((CapabilityProfileData*)&elmProfile->capabilityData, (CapabilityProfileData *)&lprofiles->capabilityData, sizeof(CapabilityProfileData));
#if 0
								elmProfile->capabilityData.capabilityId = lprofiles->capabilityData.capabilityId;
								strncpy(elmProfile->capabilityData.capabilityProfileName, lprofiles->capabilityData.capabilityProfileName, strlen(lprofiles->capabilityData.capabilityProfileName));
								elmProfile->capabilityData.capabilitySpec = lprofiles->capabilityData.capabilitySpec;
								elmProfile->capabilityData.capabilityDataType = lprofiles->capabilityData.capabilityDataType;
								strncpy(elmProfile->capabilityData.capabilityNameValue, lprofiles->capabilityData.capabilityNameValue, strlen(lprofiles->capabilityData.capabilityNameValue));
								elmProfile->capabilityData.capabilityControl = lprofiles->capabilityData.capabilityControl;
								strncpy(elmProfile->capabilityData.capabilitySubProfileName, lprofiles->capabilityData.capabilitySubProfileName, strlen(lprofiles->capabilityData.capabilitySubProfileName));
#endif
								LL_APPEND(retProfiles, elmProfile);
								printCapabilityProfiles(elmProfile);
						}
				} else {
						break;
				}
		}

		*allprofiles = retProfiles;
		LL_COUNT(retProfiles, tprofiles, count);
		retStatus = count;
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Number of capability profiles count returned is :%d\n", retStatus);

		return retStatus;
}

//Get global capability id corresponding to the name
//IN:profileName - Name of the global profile for which profile attributes are needed.
//On success this interface returns CapabilityProfileData or else NULL is returned.
CapabilityProfileData* getGlobalCapbilityIdByName(char *profileName) {
		int i = 0;
		int retStatus = PLUGIN_SUCCESS;
		
		CHECK_CAPABILITYCORE_STATUS_P(capabilityCoreStatus());
		for (i = 0; i < MAX_GLOBAL_PROFILES; i++) {
				if (!strncmp(gCapabilityGlobalProfiles[i].capabilityProfileName, profileName, strlen(profileName))) {
						APP_LOG("CAPABILITIES", LOG_DEBUG, "Global capability profile match found at index %d for profile name in global list %s \
						and profileName %s", i, gCapabilityGlobalProfiles[i].capabilityProfileName, profileName);
						break;
				}
		}
		return &gCapabilityGlobalProfiles[i];
}

//Compare profile, a new profile will added to profile list if this match fails
static int checkCapabilityProfileExists(CapabilityProfiles *profile1, CapabilityProfiles *profile2) {
		if ((!strncmp(profile1->capabilityData.capabilityProfileName, profile2->capabilityData.capabilityProfileName, strlen(profile2->capabilityData.capabilityProfileName))) \
						&& (profile1->capabilityData.capabilitySpec == profile2->capabilityData.capabilitySpec) \
						&& (!strncmp(profile1->capabilityData.capabilityNameValue, profile2->capabilityData.capabilityNameValue, strlen(profile2->capabilityData.capabilityNameValue))) \
						&& (profile1->capabilityData.capabilityControl == profile2->capabilityData.capabilityControl) \
						&& (!strncmp(profile1->capabilityData.capabilitySubProfileName, profile2->capabilityData.capabilitySubProfileName, strlen(profile2->capabilityData.capabilitySubProfileName)))) {
						APP_LOG("CAPABILITIES", LOG_DEBUG, "Profile requested matches with an existing profile");
#if 0
						printCapabilityProfiles(profile1);
						printCapabilityProfiles(profile2);
#endif
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

//Interface to add Capability Profiles Data
//IN:CapabilityProfiles - List of Capability profiles of requested device
//This interface will return success(0) if CapabilityProfiles is added successfully or
//<0 in case of any error.
int addCapabilityProfiles(CapabilityProfiles *profileData)
{
		int retStatus = PLUGIN_SUCCESS;
		CapabilityProfileData *profile;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;
		CapabilityProfiles *retProfiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if (!profileData) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Profile data provided as argument is NULL");
				goto func_exit;
		}

		if (!profileData->next) {
				//If profileData has only one node
				profile = getGlobalCapbilityIdByName(profileData->capabilityData.capabilityProfileName);
				LL_SEARCH(gCapabilityRunTimeProfiles, lprofiles, profileData, checkCapabilityProfileExists); 
				if (!lprofiles) {
						profileData->capabilityData.capabilityId = ((++gLastRuntimeCapbilityId)&(0x00FFFFFF));
						profileData->capabilityData.capabilityId |= profile->capabilityId;
						++profileData->refCount;
						LL_APPEND(gCapabilityRunTimeProfiles, profileData);
						APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability profile added with capability id 0x%x", profileData->capabilityData.capabilityId);
						printCapabilityProfiles(profileData);
				} else {
						++lprofiles->refCount;
						APP_LOG("CAPABILITIES", LOG_DEBUG, "Matched capability profile already exists with capability id 0x%x", lprofiles->capabilityData.capabilityId);
				}
		} else {
				//If profileData has a list of profiles
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						profile = getGlobalCapbilityIdByName(lprofiles->capabilityData.capabilityProfileName);
						LL_SEARCH(gCapabilityRunTimeProfiles, retProfiles, lprofiles, checkCapabilityProfileExists); 
						if (!retProfiles) {
								lprofiles->capabilityData.capabilityId = ((++gLastRuntimeCapbilityId)&(0x00FFFFFF));
								lprofiles->capabilityData.capabilityId |= profile->capabilityId;
								++lprofiles->refCount;
								LL_APPEND(gCapabilityRunTimeProfiles, lprofiles);
								APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability profile added with capability id 0x%x", lprofiles->capabilityData.capabilityId);
								printCapabilityProfiles(lprofiles);
						}else {
								++retProfiles->refCount;
								APP_LOG("CAPABILITIES", LOG_DEBUG, "Matched capability profile already exists with capability id 0x%x", retProfiles->capabilityData.capabilityId);
						}
				}
		}
func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "capability profiles add returned :%d\n", retStatus);
		return retStatus;
}

//Interface to add Capability Profiles Data
//IN:capabilityId - Capability id for which adding capability profile is requested,
//usually capabilityId will not be known to the modules using this API, hence this
//should be set to 0 in that case and capability core module will generate id and
//add against this profile
//IN:CapabilityProfiles - List of Capability profiles of requested device
//This interface will return success(0) if CapabilityProfiles is added successfully or
//<0 in case of any error.
int addCapabilityProfilesById(unsigned int capabilityId, CapabilityProfiles *profileData) 
{
		int retStatus = PLUGIN_SUCCESS;
		CapabilityProfileData *profile;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;
		CapabilityProfiles *retProfiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if (!profileData) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Profile data provided as argument is NULL");
				goto func_exit;
		}

		if (capabilityId <= 0) {
				retStatus = addCapabilityProfiles(profileData);
		} else {
				if (!profileData->next) {
						profile = getGlobalCapbilityIdByName(profileData->capabilityData.capabilityProfileName);
						LL_SEARCH(gCapabilityRunTimeProfiles, lprofiles, profileData, checkCapabilityProfileExists); 
						if (!lprofiles) {
								profileData->capabilityData.capabilityId = ((capabilityId)&(0x00FFFFFF));
								gLastRuntimeCapbilityId = profileData->capabilityData.capabilityId;
								profileData->capabilityData.capabilityId |= profile->capabilityId;
								++profileData->refCount;
								LL_APPEND(gCapabilityRunTimeProfiles, profileData);
								APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability profile added with capability id 0x%x", profileData->capabilityData.capabilityId);
								printCapabilityProfiles(profileData);
						} else {
								APP_LOG("CAPABILITIES", LOG_DEBUG, "Matched capability profile already exists with capability id 0x%x", lprofiles->capabilityData.capabilityId);
						}
				} else {
						LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
								profile = getGlobalCapbilityIdByName(lprofiles->capabilityData.capabilityProfileName);
								LL_SEARCH(gCapabilityRunTimeProfiles, retProfiles, lprofiles, checkCapabilityProfileExists); 
								if (!retProfiles) {
										lprofiles->capabilityData.capabilityId = ((capabilityId)&(0x00FFFFFF));
										gLastRuntimeCapbilityId = profileData->capabilityData.capabilityId;
										lprofiles->capabilityData.capabilityId |= profile->capabilityId;
										++lprofiles->refCount;
										LL_APPEND(gCapabilityRunTimeProfiles, lprofiles);
										APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability profile added with capability id 0x%x", lprofiles->capabilityData.capabilityId);
										printCapabilityProfiles(lprofiles);
								} else {
										++retProfiles->refCount;
										APP_LOG("CAPABILITIES", LOG_DEBUG, "Matched capability profile already exists with capability id 0x%x", retProfiles->capabilityData.capabilityId);
								}
						}
				}	
		}
func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "capability profiles add returned :%d\n", retStatus);
		return retStatus;
}

//Interface to remove Capability Profiles Data
//IN:capabilityIds - Intger Array of capability ids for which capability profile removal is requested
//IN:NumberOfEntries - Number of entries in capabilityIdList, this must be a value greater than 0
//This interface will also return number of capability entries for which CapabilityProfiles is removed,
//0 if no profile is present or <0 in case of any error.
//Also, use of this API is discouraged since removal of a profile should always be against a
//device not directly with capability Ids
int removeCapabilityProfiles(unsigned int capabilityIds[], unsigned int numOfEntries) {
		int retStatus = PLUGIN_FAILURE, i = 0, count = 0;
		CapabilityProfiles *lprofiles = NULL;
		CapabilityProfiles cmpProfile;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if (numOfEntries <= PLUGIN_SUCCESS) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Number of Entries should always be greater than Zero:%d\n", numOfEntries);
				retStatus = PLUGIN_FAILURE; 
				goto func_exit;
		}
		APP_LOG("CAPABILITIES", LOG_ERR, "Number of Entries in capabilityIds to remove :%d\n", numOfEntries);

		LL_COUNT(gCapabilityRunTimeProfiles, lprofiles, count);
		if (count <= 0) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Number of counts in gCapabilityRunTimeProfiles is Zero:%d\n", count);
				retStatus = PLUGIN_SUCCESS; 
				goto func_exit;
		}

		count = 0;
		for (i = 0; i < numOfEntries; i++) {
				cmpProfile.capabilityData.capabilityId = capabilityIds[i];
				LL_SEARCH(gCapabilityRunTimeProfiles, lprofiles, &cmpProfile, compareCapabilityIds);
				if (lprofiles) {
						APP_LOG("CAPABILITIES", LOG_ERR, "Delete request for profile with capability id 0x%x from gCapabilityRunTimeProfiles, refCount %d", \
								capabilityIds[i], lprofiles->refCount);
						--lprofiles->refCount;
						APP_LOG("CAPABILITIES", LOG_ERR, "Delete request for profile with capability id 0x%x from gCapabilityRunTimeProfiles, --refCount %d", \
								capabilityIds[i], lprofiles->refCount);
						if (lprofiles->refCount <= PLUGIN_SUCCESS) {
								LL_DELETE(gCapabilityRunTimeProfiles, lprofiles);
								free(lprofiles);
								lprofiles = NULL;
								APP_LOG("CAPABILITIES", LOG_ERR, "Profile with capability id 0x%x deleted from gCapabilityRunTimeProfiles", capabilityIds[i]);
						}
						++count;
				}
		}

		if (count > PLUGIN_SUCCESS) {
				retStatus = count;
		} else {
				retStatus = PLUGIN_FAILURE;
		}

func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "capability profiles remove returned :%d\n", retStatus);
		return retStatus;
}

//Internal interfaces to capability core module, should not be called from external
//modules interacting with capability core
//Internal interface to all capability profile
//IN:CapabilityProfiles - capability profile needs to be added, this must not be NULL
//This will return capability id after successfull addition of capability profile or else
//FAILURE (-ve)
int addCapabilityProfilesI(CapabilityProfiles *profileData)
{
		int retStatus = PLUGIN_SUCCESS;
		CapabilityProfileData *profile;
		CapabilityProfiles *lprofiles = NULL;

		if (!profileData) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Profile data provided as argument is NULL");
				goto func_exit;
		}
		profileData->next = NULL;

		profile = getGlobalCapbilityIdByName(profileData->capabilityData.capabilityProfileName);
		LL_SEARCH(gCapabilityRunTimeProfiles, lprofiles, profileData, checkCapabilityProfileExists); 
		if (!lprofiles) {
				profileData->capabilityData.capabilityId = ((++gLastRuntimeCapbilityId)&(0x00FFFFFF));
				profileData->capabilityData.capabilityId |= profile->capabilityId;
				++profileData->refCount;
				LL_APPEND(gCapabilityRunTimeProfiles, profileData);
				retStatus =  profileData->capabilityData.capabilityId;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability profile added with capability id 0x%x - refCount %d", \
						profileData->capabilityData.capabilityId, profileData->refCount);
				printCapabilityProfiles(profileData);
		} else {
				++lprofiles->refCount;
				retStatus =  lprofiles->capabilityData.capabilityId;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Matched capability profile already exists with capability id 0x%x - refCount %d", \
						lprofiles->capabilityData.capabilityId, lprofiles->refCount);
				printCapabilityProfiles(lprofiles);
		}

func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "capability profiles add returned :%d\n", retStatus);
		return retStatus;
}
