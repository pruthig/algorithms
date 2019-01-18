/***************************************************************************
 *
 *
 * capabilityDevs.c
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

//List of available capability created at runtime based on the joined devices
extern CapabilityProfiles *gCapabilityRunTimeProfiles;

//Global list of end devices list and their capability mapping
EndDevicesCapabilities *gEndDevToCapabilityMappingList = NULL;

//Global index for device index
//will be incremented for each new device publishing their capability to
//this module
unsigned int gEndDevicesIndex = 0;

//Interface to get an index for a new device trying to publish its capabilities profile
//IN:reserved - Reserved for future use, for now should be NULL
//This interface will returns a unique index for use in subsequent inetrfaces,
//or <0 in case of any error.
//This interface will return capability core generated device index only if
//capabilityCoreDevIdxFlag returns 1 i.e device index generator flag is set
//while initializing capability core.
unsigned int getNewDeviceIndex(void *reserved) {
		int retStatus = PLUGIN_SUCCESS;
		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		
		retStatus = capabilityCoreDevIdxFlag();
		if (retStatus <= PLUGIN_SUCCESS) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Not a valid call if capabilityCoreDevIdxFlag set to %d", capabilityCoreDevIdxFlag());
				return PLUGIN_FAILURE;
		}

		++gEndDevicesIndex;
		APP_LOG("CAPABILITIES", LOG_DEBUG, "New device index returned is gEndDevicesIndex %d", gEndDevicesIndex);
		return gEndDevicesIndex;
}

//Compare device indices of two EndDevicesCapabilities nodes
static int compareDeviceIndices(EndDevicesCapabilities *dev1, EndDevicesCapabilities *dev2) {
		if (dev1->endDeviceCapabilities.endDeviceIdx == dev2->endDeviceCapabilities.endDeviceIdx) {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "compareDeviceIndices = dev1:%d - dev2:%d\n", dev1->endDeviceCapabilities.endDeviceIdx, dev2->endDeviceCapabilities.endDeviceIdx);
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

//Compare device hardware addresses of two EndDevicesCapabilities nodes
static int compareDeviceHwAddr(EndDevicesCapabilities *dev1, EndDevicesCapabilities *dev2) {
		APP_LOG("CAPABILITIES", LOG_DEBUG, "compareDeviceHwAddr = dev1:%s - dev2:%s\n", dev1->endDeviceCapabilities.endDeviceIdentifier, dev2->endDeviceCapabilities.endDeviceIdentifier);
		if (!strncmp(dev1->endDeviceCapabilities.endDeviceIdentifier, dev2->endDeviceCapabilities.endDeviceIdentifier, strlen(dev2->endDeviceCapabilities.endDeviceIdentifier))) {
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

//Compare device hardware addresses as well device indices of two EndDevicesCapabilities nodes
static int compareDeviceIdxHwAddr(EndDevicesCapabilities *dev1, EndDevicesCapabilities *dev2) {
		if (!strncmp(dev1->endDeviceCapabilities.endDeviceIdentifier, dev2->endDeviceCapabilities.endDeviceIdentifier, strlen(dev2->endDeviceCapabilities.endDeviceIdentifier)) \
				&& (dev1->endDeviceCapabilities.endDeviceIdx == dev2->endDeviceCapabilities.endDeviceIdx)) {
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

//Interface to add Capability Profiles of a device
//IN:deviceIdx - Device index for which capability profile as well as its mapping with
//device is requested. This is a mandatory input from the caller of this interface.
//IN:hwAddress - Device hardware address for which capability profile as well as its mapping with
//device is requested. This is a mandatory input from the caller of this interface.
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//This interface will return device index same as the one passed to this interface if successfull
//else or <0 in case of any error.
int addDeviceCapabilities(unsigned int deviceIdx, char *hwAddress, CapabilityProfiles *profileData) {
		EndDevicesCapabilities *ldevCap = NULL, *ldevCap1 = NULL, cmpDev;
		int retStatus = PLUGIN_FAILURE, isPresent = 0, capId = 0, i = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		
		if ((deviceIdx <= PLUGIN_SUCCESS) && (!hwAddress) && (!profileData)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check deviceIdx or hwAddress or profileData");
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to add device capabilities for deviceIdx %d hwAddress %s \
				capabilityProfileName %s capabilitySubProfileName %s", deviceIdx, hwAddress, \
				profileData->capabilityData.capabilityProfileName, profileData->capabilityData.capabilitySubProfileName);

		cmpDev.endDeviceCapabilities.endDeviceIdx = deviceIdx;
		strncpy(cmpDev.endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress));
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevCap, &cmpDev, compareDeviceIdxHwAddr);
		if (ldevCap) {
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d hwAddress %s", deviceIdx, hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				retStatus = ldevCap->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x", deviceIdx, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap->endDeviceCapabilities, 1);
		} else {
				isPresent = 0;
				ldevCap1 = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
				if (!ldevCap1) {
						retStatus = PLUGIN_FAILURE;
						APP_LOG("CAPABILITIES", LOG_ERR, "Failed in memory allocation for adding capability profile for deviceIdx %d hwAddress %s", deviceIdx, hwAddress);
						goto func_exit;
				}
				ldevCap1->endDeviceCapabilities.endDeviceIdx = deviceIdx; 
				strncpy(ldevCap1->endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress));
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d hwAddress %s", deviceIdx, hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap1->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap1->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap1->endDeviceCapabilities.capabilityIds[ldevCap1->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap1->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				LL_APPEND(gEndDevToCapabilityMappingList, ldevCap1);
				retStatus = ldevCap1->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x", deviceIdx, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap1->endDeviceCapabilities, 1);
		}
func_exit:
		if (retStatus < PLUGIN_SUCCESS) {
				if (ldevCap1) {free(ldevCap1); ldevCap1 = NULL;}
		}
		return retStatus;
}

//Interface to add Capability Profiles of a device
//IN:deviceIdx - Device index for which capability profile as well as its mapping with
//device is requested. This is an optional input from the caller of this interface.
//IN:hwAddress - Device hardware address for which capability profile as well as its mapping with
//device is requested. This is a mandatory input from the caller of this interface.
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//If an entry with the hwAddress already exists in device to capability mapping list then this 
//interface will update the same entry and return the existing device index if successfull or
//else <0 in case of error.
//if entry with this hwAddress is not present in device to capability mapping list then this will
//check whether generating device index from capability core is allowed or not. If not then error
// <0 will be returned else this interface will generate a device index then adds the capability
//profile as well as updates device to capability mapping list and returns the newly generated
//device index.
int addDeviceCapabilitiesEx(unsigned int deviceIdx, char *hwAddress, CapabilityProfiles *profileData) {
		EndDevicesCapabilities *ldevCap = NULL, *ldevCap1 = NULL, cmpDev;
		int retStatus = PLUGIN_FAILURE, isPresent = 0, capId = 0, i = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if ((!hwAddress) && (!profileData)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check hwAddress or profileData");
				goto func_exit;
		}
		
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to add device capabilities for deviceIdx %d hwAddress %s \
				capabilityProfileName %s capabilitySubProfileName %s", deviceIdx, hwAddress, \
				profileData->capabilityData.capabilityProfileName, profileData->capabilityData.capabilitySubProfileName);

		strncpy(cmpDev.endDeviceCapabilities.endDeviceIdentifier, hwAddress, (strlen(hwAddress)+1));
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevCap, &cmpDev, compareDeviceHwAddr);
		if (ldevCap) {
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d hwAddress %s", deviceIdx, hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				retStatus = ldevCap->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x", retStatus, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap->endDeviceCapabilities, 1);
		} else {
				isPresent = 0;
				ldevCap1 = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
				if (!ldevCap1) {
						retStatus = PLUGIN_FAILURE;
						APP_LOG("CAPABILITIES", LOG_ERR, "Failed in memory allocation for adding capability profile for deviceIdx %d hwAddress %s", deviceIdx, hwAddress);
						goto func_exit;
				}

				retStatus = capabilityCoreDevIdxFlag();
				if (retStatus <= PLUGIN_SUCCESS) {
						APP_LOG("CAPABILITIES", LOG_ERR, "Not a valid call if capabilityCoreDevIdxFlag set to %d", capabilityCoreDevIdxFlag());
						retStatus = PLUGIN_FAILURE;
						goto func_exit;
				}

				ldevCap1->endDeviceCapabilities.endDeviceIdx = ++gEndDevicesIndex; 
				strncpy(ldevCap1->endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress));
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d hwAddress %s", \
										ldevCap1->endDeviceCapabilities.endDeviceIdx, hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap1->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap1->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap1->endDeviceCapabilities.capabilityIds[ldevCap1->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap1->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				LL_APPEND(gEndDevToCapabilityMappingList, ldevCap1);
				retStatus = ldevCap1->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x", retStatus, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap1->endDeviceCapabilities, 1);
		}
func_exit:
		if (retStatus < PLUGIN_SUCCESS) {
				if (ldevCap1) {free(ldevCap1); ldevCap1 = NULL;}
		}
		return retStatus;
}

//Print EndDeviceCapabilities details
void printDevCapabilities(EndDevicesCapabilities *devs, EndDeviceCapabilities *devCapability) {
		int i = 0;
		CapabilityValue *lcurr = NULL, *tcurr = NULL;

		APP_LOG("CAPABILITIES", LOG_DEBUG, "EndDeviceCapabilities endDeviceIdx-%d, endDeviceIdentifier-%s, numOfCapabilityId-%d", \
				devCapability->endDeviceIdx, devCapability->endDeviceIdentifier, devCapability->numOfCapabilityId);
		for (i = 0; i < devCapability->numOfCapabilityId; i++) {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "EndDeviceCapabilities capabilityId[%d]:0x%x", i, devCapability->capabilityIds[i]);
		}
		if (devs->currValues) {
				LL_FOREACH_SAFE(devs->currValues, lcurr, tcurr) {
						APP_LOG("CAPABILITIES", LOG_DEBUG, "EndDeviceCapabilities current values capabilityId[%d]:0x%x capabilityNameValue:%s", \
								i, lcurr->capabilityId, lcurr->capabilityNameValue);
				}
		}
}

//Get all devices to capabilities mappings
static int getAllDevCapabilityMappingsI(EndDevicesCapabilities **enddevsCapabilities) {
		int retStatus = PLUGIN_FAILURE, count = 0, vcount = 0;
		EndDevicesCapabilities *ldevmap = NULL, *tdevmap = NULL;
		EndDevicesCapabilities *retDevMap = NULL, *retDevMaps = NULL;
		CapabilityValue *tcvalue = NULL;

		LL_FOREACH_SAFE(gEndDevToCapabilityMappingList, ldevmap, tdevmap)
		{
				if (ldevmap) {
						retDevMap = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
						if (retDevMap) {
								retDevMap->endDeviceCapabilities.endDeviceIdx = ldevmap->endDeviceCapabilities.endDeviceIdx;
								strncpy(retDevMap->endDeviceCapabilities.endDeviceIdentifier, ldevmap->endDeviceCapabilities.endDeviceIdentifier, strlen(ldevmap->endDeviceCapabilities.endDeviceIdentifier));
								retDevMap->endDeviceCapabilities.numOfCapabilityId = ldevmap->endDeviceCapabilities.numOfCapabilityId;
								memcpy(retDevMap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.capabilityIds, (SIZE_8B * sizeof(unsigned int)));
								if (ldevmap->currValues) {
										LL_COUNT(ldevmap->currValues, tcvalue, vcount); 
										retDevMap->currValues = (CapabilityValue*)calloc(vcount, sizeof(CapabilityValue));
										if (retDevMap->currValues) {
												memcpy(retDevMap->currValues, ldevmap->currValues, (sizeof(CapabilityValue)*vcount));
										}
								}
								LL_APPEND(retDevMaps, retDevMap);
								printDevCapabilities(ldevmap, &ldevmap->endDeviceCapabilities);
						}
				} else {
						break;
				}
		}

		*enddevsCapabilities = retDevMaps;
		LL_COUNT(retDevMaps, tdevmap, count);
		retStatus = count;
		
		APP_LOG("CAPABILITIES", LOG_DEBUG, "End device to capability mapping count returned is %d", retStatus);

		return retStatus;
}

//Interface to get end devices Capability
//IN:deviceIdxs - Array of device indices for which capability is required
//IN:numOfDevices - Number of device indices entries in DeviceIdxs
//OUT:EndDevicesCapabilities - List of end devices with their capabilities
//This interface will return number of devices entries in OUT: EndDevicesCapabilities.
//This return will be 0 if capability can't be determined for any devices in deviceIdxs
//or <0 in case of any error.
//If this interface is called with NumberOfDevices set to 0, then OUT:EndDevicesCapabilities
//will be returned for all available sub devices.
int getDevicesCapabilitiesByIdx(unsigned int deviceIdxs[], unsigned int numOfDevices, EndDevicesCapabilities **enddevsCapabilities) {
		int retStatus = PLUGIN_FAILURE, count = 0, i = 0, vcount = 0;
		EndDevicesCapabilities *ldevmap = NULL, *tdevmap = NULL, cdevmap;
		EndDevicesCapabilities *retDevMap = NULL, *retDevMaps = NULL;
		CapabilityValue *tcvalue = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		LL_COUNT(gEndDevToCapabilityMappingList, ldevmap, tdevmap);
		if (count < 0) {
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_ERR, "No entry in gEndDevToCapabilityMappingList");
				goto func_exit;
		}

		if (numOfDevices <= PLUGIN_SUCCESS) {
				retStatus = getAllDevCapabilityMappingsI(enddevsCapabilities); 
		} else {
				for (i = 0; i < numOfDevices; i++) {
						cdevmap.endDeviceCapabilities.endDeviceIdx = deviceIdxs[i];
						LL_SEARCH(gEndDevToCapabilityMappingList, ldevmap, &cdevmap, compareDeviceIndices);
						if (ldevmap) {
								retDevMap = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
								if (retDevMap) {
										retDevMap->endDeviceCapabilities.endDeviceIdx = ldevmap->endDeviceCapabilities.endDeviceIdx;
										strncpy(retDevMap->endDeviceCapabilities.endDeviceIdentifier, ldevmap->endDeviceCapabilities.endDeviceIdentifier, strlen(ldevmap->endDeviceCapabilities.endDeviceIdentifier));
										retDevMap->endDeviceCapabilities.numOfCapabilityId = ldevmap->endDeviceCapabilities.numOfCapabilityId;
										memcpy(retDevMap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.capabilityIds, (SIZE_8B * sizeof(unsigned int)));
										if (ldevmap->currValues) {
												LL_COUNT(ldevmap->currValues, tcvalue, vcount); 
												retDevMap->currValues = (CapabilityValue*)calloc(vcount, sizeof(CapabilityValue));
												if (retDevMap->currValues) {
														memcpy(retDevMap->currValues, ldevmap->currValues, (sizeof(CapabilityValue)*vcount));
												}
										}
										LL_APPEND(retDevMaps, retDevMap);
										printDevCapabilities(ldevmap, &ldevmap->endDeviceCapabilities);
								}
						}
				}
				*enddevsCapabilities = retDevMaps;
				LL_COUNT(retDevMaps, tdevmap, count);
				retStatus = count;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "End device to capability mapping count returned is %d", retStatus);
		}

func_exit:
		return retStatus;
}

//Interface to get end devices Capability
//IN:devHwAddr - Array of device hardware address for which capability is required
//IN:numOfDevices - Number of device indices entries in devHwAddr
//OUT:EndDevicesCapabilities - List of end devices with their capabilities
//This interface will return number of devices entries in OUT: EndDevicesCapabilities.
//This return will be 0 if capability can't be determined for any devices in devHwAddr
//or <0 in case of any error.
//If this interface is called with NumberOfDevices set to 0, then OUT:EndDevicesCapabilities
//will be returned for all available sub devices.
int getDevicesCapabilitiesByHWAddr(char *devHwAddr[], unsigned int numOfDevices, EndDevicesCapabilities **enddevsCapabilities) {
		int retStatus = PLUGIN_FAILURE, count = 0, i = 0, vcount = 0;
		EndDevicesCapabilities *ldevmap = NULL, *tdevmap = NULL, cdevmap;
		EndDevicesCapabilities *retDevMap = NULL, *retDevMaps = NULL;
		char **devHwAddrs = NULL;
		CapabilityValue *tcvalue = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		LL_COUNT(gEndDevToCapabilityMappingList, ldevmap, tdevmap);
		if (count < 0) {
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_ERR, "No entry in gEndDevToCapabilityMappingList");
				goto func_exit;
		}

		if (numOfDevices <= PLUGIN_SUCCESS) {
				retStatus = getAllDevCapabilityMappingsI(enddevsCapabilities); 
		} else {
				for (i = 0,devHwAddrs = devHwAddr; ((i < numOfDevices) && (*devHwAddrs)); ++i,++devHwAddrs) {
						strncpy(cdevmap.endDeviceCapabilities.endDeviceIdentifier, *devHwAddrs, strlen(*devHwAddrs));
						LL_SEARCH(gEndDevToCapabilityMappingList, ldevmap, &cdevmap, compareDeviceHwAddr);
						if (ldevmap) {
								retDevMap = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
								if (retDevMap) {
										retDevMap->endDeviceCapabilities.endDeviceIdx = ldevmap->endDeviceCapabilities.endDeviceIdx;
										strncpy(retDevMap->endDeviceCapabilities.endDeviceIdentifier, ldevmap->endDeviceCapabilities.endDeviceIdentifier, strlen(ldevmap->endDeviceCapabilities.endDeviceIdentifier));
										retDevMap->endDeviceCapabilities.numOfCapabilityId = ldevmap->endDeviceCapabilities.numOfCapabilityId;
										memcpy(retDevMap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.capabilityIds, (SIZE_8B * sizeof(unsigned int)));
										if (ldevmap->currValues) {
												LL_COUNT(ldevmap->currValues, tcvalue, vcount); 
												retDevMap->currValues = (CapabilityValue*)calloc(vcount, sizeof(CapabilityValue));
												if (retDevMap->currValues) {
														memcpy(retDevMap->currValues, ldevmap->currValues, (sizeof(CapabilityValue)*vcount));
												}
										}
										LL_APPEND(retDevMaps, retDevMap);
										printDevCapabilities(ldevmap, &ldevmap->endDeviceCapabilities);
								}
						}
				}
				*enddevsCapabilities = retDevMaps;
				LL_COUNT(retDevMaps, tdevmap, count);
				retStatus = count;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "End device to capability mapping count returned is %d", retStatus);
		}

func_exit:
		return retStatus;
}

//Remove capability details corresponding to a end device index
//IN:deviceIdx - index of end devices need to be removed.
//Returns 0 in case of Success or < 0 in case of Failure
int removeDeviceCapabilitiesByIdx(unsigned int deviceIdx) {
		int retStatus = PLUGIN_FAILURE, count = 0;
		EndDevicesCapabilities *ldevmap = NULL, *tdevmap = NULL, cdevmap;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if (deviceIdx <= PLUGIN_SUCCESS) {
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_ERR, "Argument deviceId passed is in invalid");
				goto func_exit;
		}

		LL_COUNT(gEndDevToCapabilityMappingList, ldevmap, tdevmap);
		if (count < 0) {
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_ERR, "No entry in gEndDevToCapabilityMappingList");
				goto func_exit;
		}
		
		cdevmap.endDeviceCapabilities.endDeviceIdx = deviceIdx;
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevmap, &cdevmap, compareDeviceIndices);
		if (ldevmap) {
				LL_DELETE(gEndDevToCapabilityMappingList, ldevmap);
				retStatus = removeCapabilityProfiles(ldevmap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.numOfCapabilityId); 
				APP_LOG("CAPABILITIES", LOG_DEBUG, "deviceIdx %d is deleted from gEndDevToCapabilityMappingList, retStatus %d", deviceIdx, retStatus);
				invokeCoreCallbacks(&ldevmap->endDeviceCapabilities, 0);
				free(ldevmap);
				ldevmap = NULL;
				retStatus = PLUGIN_SUCCESS;
		} else {
				APP_LOG("CAPABILITIES", LOG_ERR, "deviceIdx %d is not in gEndDevToCapabilityMappingList", deviceIdx);
				retStatus = PLUGIN_FAILURE;
		}

func_exit:
		return retStatus;
}

//Remove capability details corresponding to a end device hardware address
//IN:deviceHwAddress - Hardware address of end devices need to be removed.
//Returns 0 in case of Success or < 0 in case of Failure
int removeDeviceCapabilitiesByHwAddress(char *deviceHwAddr) {
		int retStatus = PLUGIN_FAILURE, count = 0;
		EndDevicesCapabilities *ldevmap = NULL, *tdevmap = NULL, cdevmap;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if (!deviceHwAddr) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Argument deviceHwAddr passed is NULL");
				goto func_exit;
		}

		LL_COUNT(gEndDevToCapabilityMappingList, ldevmap, tdevmap);
		if (count < 0) {
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_ERR, "No entry in gEndDevToCapabilityMappingList");
				goto func_exit;
		}
		
		strncpy(cdevmap.endDeviceCapabilities.endDeviceIdentifier, deviceHwAddr, strlen(deviceHwAddr));
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevmap, &cdevmap, compareDeviceHwAddr);
		if (ldevmap) {
				LL_DELETE(gEndDevToCapabilityMappingList, ldevmap);
				retStatus = removeCapabilityProfiles(ldevmap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.numOfCapabilityId); 
				APP_LOG("CAPABILITIES", LOG_DEBUG, "deviceHwAddr %s is deleted from gEndDevToCapabilityMappingList, retStatus %d", deviceHwAddr, retStatus);
				invokeCoreCallbacks(&ldevmap->endDeviceCapabilities, 0);
				free(ldevmap);
				ldevmap = NULL;
				retStatus = PLUGIN_SUCCESS;
		} else {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "deviceHwAddr %d is not in gEndDevToCapabilityMappingList", deviceHwAddr);
		}

func_exit:
		return retStatus;
}

//-------------------------------------------------------------------------------------------------

//Interface to add Capability Profiles Data
//IN:deviceIdx - Device index for which capability profile as well as its mapping with device
//is requested. This is mandatory for this interface and must exists in the device to capability
//mappining list
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//This interface will return device index if successfull else or <0 in case of any error.
//This interface will return error if this device index is not already present in device
//to capability mapping list.
int addDeviceCapabilitiesByIdx(unsigned int deviceIdx, CapabilityProfiles *profileData) {
		EndDevicesCapabilities *ldevCap = NULL, cmpDev;
		int retStatus = PLUGIN_FAILURE, isPresent = 0, capId = 0, i = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if ((deviceIdx <= PLUGIN_SUCCESS) && (!profileData)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check deviceIdx or profileData");
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to add device capabilities for deviceIdx %d \
				capabilityProfileName %s capabilitySubProfileName %s", deviceIdx, \
				profileData->capabilityData.capabilityProfileName, profileData->capabilityData.capabilitySubProfileName);

		cmpDev.endDeviceCapabilities.endDeviceIdx = deviceIdx;
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevCap, &cmpDev, compareDeviceIndices);
		if (ldevCap) {
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d ", deviceIdx);
								goto func_exit;
						}
						for (i=0; i < ldevCap->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap->endDeviceCapabilities.numOfCapabilityId += 1;
								ldevCap->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
						}
				}
				retStatus = ldevCap->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d capabilityId 0x%x", deviceIdx, capId);
				invokeCoreCallbacks(&ldevCap->endDeviceCapabilities, 1);
		} else {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Failed to get an entry in gEndDevToCapabilityMappingList for deviceIdx %d ", deviceIdx);
		}
func_exit:
		return retStatus;
}

//Interface to add Capability Profiles Data
//IN:hwAdrress - Device hardware address for which capability profile as well as its mapping with
//device is requested. This is mandatory for this interface and must exists in the device to capability
//mappining list.
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//This interface will return device index if successfull else or <0 in case of any error.
//This interface will return error if this device hardware address is not already present in device
int addDeviceCapabilitiesByHWAddr(char *hwAddress, CapabilityProfiles *profileData) {
		EndDevicesCapabilities *ldevCap = NULL, cmpDev;
		int retStatus = PLUGIN_FAILURE, isPresent = 0, capId = 0, i = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if ((!hwAddress) && (!profileData)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check hwAddress or profileData");
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to add device capabilities for hwAddress %s \
				capabilityProfileName %s capabilitySubProfileName %s", hwAddress, profileData->capabilityData.capabilityProfileName,\
				profileData->capabilityData.capabilitySubProfileName);

		strncpy(cmpDev.endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress));
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevCap, &cmpDev, compareDeviceHwAddr);
		if (ldevCap) {
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for hwAddress %s", hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				retStatus = ldevCap->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x", retStatus, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap->endDeviceCapabilities, 1);
		} else {
				retStatus = PLUGIN_FAILURE;	
				APP_LOG("CAPABILITIES", LOG_ERR, "Failed to get an entry in gEndDevToCapabilityMappingList for hwAddress %s ", hwAddress);
		}
func_exit:
		return retStatus;
}

//Interface to add Capability Profiles Data
//IN:deviceIdx - Device index for which capability profile as well as its mapping with device
//is requested. This is mandatory for this interface and may or may not aleardy exists in device to
//capability mapping list.
//IN:CapabilityProfiles - List of Capability profiles for the requested device.
//If the device index already exists in the device to capability mapping list then it will be updated
//with the requested profile else a new entry with the device index will be created with the requested
//profile.
//This interface will return device index which will be same as the one passed to this interface 
//if successfull else or <0 in case of any error.
int addDeviceCapabilitiesByIdxEx(unsigned int deviceIdx, CapabilityProfiles *profileData) {
		EndDevicesCapabilities *ldevCap = NULL, *ldevCap1 = NULL, cmpDev;
		int retStatus = PLUGIN_FAILURE, isPresent = 0, capId = 0, i = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if ((deviceIdx <= PLUGIN_SUCCESS) && (!profileData)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check for deviceIdx or profileData");
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to add device capabilities for deviceIdx %d \
				capabilityProfileName %s capabilitySubProfileName %s", deviceIdx, \
				profileData->capabilityData.capabilityProfileName, profileData->capabilityData.capabilitySubProfileName);

		cmpDev.endDeviceCapabilities.endDeviceIdx = deviceIdx;
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevCap, &cmpDev, compareDeviceIndices);
		if (ldevCap) {
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);	
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d ", deviceIdx);
								goto func_exit;
						}
						for (i=0; i < ldevCap->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap->endDeviceCapabilities.numOfCapabilityId += 1;
								ldevCap->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
						}
				}
				retStatus = ldevCap->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d capabilityId 0x%x", deviceIdx, capId);
				invokeCoreCallbacks(&ldevCap->endDeviceCapabilities, 1);
		} else {
				isPresent = 0;
				ldevCap1 = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
				if (!ldevCap1) {
						retStatus = PLUGIN_FAILURE;
						APP_LOG("CAPABILITIES", LOG_ERR, "Failed in memory allocation for adding capability profile for deviceIdx %d", deviceIdx);
						goto func_exit;
				}
				ldevCap1->endDeviceCapabilities.endDeviceIdx = deviceIdx; 
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d", \
										ldevCap1->endDeviceCapabilities.endDeviceIdx);
								goto func_exit;
						}
						for (i=0; i < ldevCap1->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap1->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap1->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap1->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				LL_APPEND(gEndDevToCapabilityMappingList, ldevCap1);
				retStatus = ldevCap1->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d capabilityId 0x%x", retStatus, capId);
				invokeCoreCallbacks(&ldevCap1->endDeviceCapabilities, 1);
		}
func_exit:
		if (retStatus < PLUGIN_SUCCESS) {
				if (ldevCap1) {free(ldevCap1); ldevCap1 = NULL;}
		}
		return retStatus;
}

//Interface to add Capability Profiles Data
//IN:hwAddress - Device hardware address for which capability profile as well as its mapping with device
//is requested. This is mandatory for this interface and may or may not aleardy exists in device to
//capability mapping list.
//IN:CapabilityProfiles - List of Capability profiles for the requested device.
//If an entry with the hwAddress already exists in device to capability mapping list then this 
//interface will update the same entry and return the existing device index if successfull or
//else <0 in case of error.
//if entry with this hwAddress is not present in device to capability mapping list then this will
//check whether generating device index from capability core is allowed or not. If not then error
// <0 will be returned else this interface will generate a device index then adds the capability
//profile as well as updates device to capability mapping list and returns the newly generated
//device index.
int addDeviceCapabilitiesByHWAddrEx(char *hwAddress, CapabilityProfiles *profileData) {
		EndDevicesCapabilities *ldevCap = NULL, *ldevCap1 = NULL, cmpDev;
		int retStatus = PLUGIN_FAILURE, isPresent = 0, capId = 0, i = 0;
		CapabilityProfiles *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());
		if ((!hwAddress) && (!profileData)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check for hwAddress or profileData");
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to add device capabilities for hwAddress %s \
				capabilityProfileName %s capabilitySubProfileName %s", hwAddress, \
				profileData->capabilityData.capabilityProfileName, profileData->capabilityData.capabilitySubProfileName);

		strncpy(cmpDev.endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress));
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevCap, &cmpDev, compareDeviceHwAddr);
		if (ldevCap) {
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for hwAddress %s", hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				retStatus = ldevCap->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x", \
						retStatus, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap->endDeviceCapabilities, 1);
		} else {
				isPresent = 0;
				ldevCap1 = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
				if (!ldevCap1) {
						retStatus = PLUGIN_FAILURE;
						APP_LOG("CAPABILITIES", LOG_ERR, "Failed in memory allocation for adding capability profile for hwAddress %s", hwAddress);
						goto func_exit;
				}
				
				retStatus = capabilityCoreDevIdxFlag();
				if (retStatus <= PLUGIN_SUCCESS) {
						APP_LOG("CAPABILITIES", LOG_ERR, "Not a valid call if capabilityCoreDevIdxFlag set to %d", capabilityCoreDevIdxFlag());
						retStatus = PLUGIN_FAILURE;
						goto func_exit;
				}

				ldevCap1->endDeviceCapabilities.endDeviceIdx = ++gEndDevicesIndex; 
				strncpy(ldevCap1->endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress));
				LL_FOREACH_SAFE(profileData, lprofiles, tprofiles) {
						capId = addCapabilityProfilesI(lprofiles);
						if (capId <= PLUGIN_SUCCESS) {
								retStatus = PLUGIN_FAILURE;
								APP_LOG("CAPABILITIES", LOG_ERR, "Failed to add capability profile for deviceIdx %d, hwAddress %s",
										ldevCap1->endDeviceCapabilities.endDeviceIdx, hwAddress);
								goto func_exit;
						}
						for (i=0; i < ldevCap1->endDeviceCapabilities.numOfCapabilityId; ++i) {
								if (capId == ldevCap1->endDeviceCapabilities.capabilityIds[i]) {
										isPresent = 1;
										break;
								}
						}
						if (isPresent == 0) {
								ldevCap1->endDeviceCapabilities.capabilityIds[ldevCap->endDeviceCapabilities.numOfCapabilityId] = capId;
								ldevCap1->endDeviceCapabilities.numOfCapabilityId += 1;
						}
				}
				LL_APPEND(gEndDevToCapabilityMappingList, ldevCap1);
				retStatus = ldevCap1->endDeviceCapabilities.endDeviceIdx;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Successfully added capability profile for deviceIdx %d hwAddress %s capabilityId 0x%x",\
						retStatus, hwAddress, capId);
				invokeCoreCallbacks(&ldevCap1->endDeviceCapabilities, 1);
		}
func_exit:
		if (retStatus < PLUGIN_SUCCESS) {
				if (ldevCap1) {free(ldevCap1); ldevCap1 = NULL;}
		}
		return retStatus;
}

//Interface to get end device Capability
//IN:deviceIdx - Device Index for which capability is required
//IN:hwAddress - Device hardware address for which capability is required
//OUT:EndDevicesCapabilities - End device with their capabilities
//This interface will return device capabilities in OUT: EndDevicesCapabilities.
//This return will be success (0) if capability of device is determined successfully
//or <0 in case of any error.
int getDevicesCapability(unsigned int deviceIdx, char *hwAddress, EndDevicesCapabilities **enddevsCapabilities) {
		int retStatus = PLUGIN_FAILURE, count = 0, vcount = 0;
		EndDevicesCapabilities *ldevmap = NULL, *tdevmap = NULL, cdevmap;
		EndDevicesCapabilities *retDevMap = NULL;
		CapabilityValue *tcvalue = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		if ((deviceIdx <= PLUGIN_SUCCESS) && (!hwAddress)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check deviceIdx or hwAddress or profileData");
				goto func_exit;
		}

		LL_COUNT(gEndDevToCapabilityMappingList, ldevmap, tdevmap);
		if (count < 0) {
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_ERR, "No entry in gEndDevToCapabilityMappingList");
				goto func_exit;
		}
		
		memset(&cdevmap, 0x0, sizeof(EndDevicesCapabilities));
		cdevmap.endDeviceCapabilities.endDeviceIdx = deviceIdx;
		strncpy(cdevmap.endDeviceCapabilities.endDeviceIdentifier, hwAddress, strlen(hwAddress)+1);
		LL_SEARCH(gEndDevToCapabilityMappingList, ldevmap, &cdevmap, compareDeviceIdxHwAddr);
		if (ldevmap) {
				retDevMap = (EndDevicesCapabilities*)calloc(1, sizeof(EndDevicesCapabilities));
				if (retDevMap) {
						retDevMap->endDeviceCapabilities.endDeviceIdx = ldevmap->endDeviceCapabilities.endDeviceIdx;
						strncpy(retDevMap->endDeviceCapabilities.endDeviceIdentifier, ldevmap->endDeviceCapabilities.endDeviceIdentifier, strlen(ldevmap->endDeviceCapabilities.endDeviceIdentifier));
						retDevMap->endDeviceCapabilities.numOfCapabilityId = ldevmap->endDeviceCapabilities.numOfCapabilityId;
						memcpy(retDevMap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.capabilityIds, (SIZE_8B * sizeof(unsigned int)));
						if (ldevmap->currValues) {
								LL_COUNT(ldevmap->currValues, tcvalue, vcount); 
								retDevMap->currValues = (CapabilityValue*)calloc(vcount, sizeof(CapabilityValue));
								if (retDevMap->currValues) {
										memcpy(retDevMap->currValues, ldevmap->currValues, (sizeof(CapabilityValue)*vcount));
								}
						}
						printDevCapabilities(ldevmap, &ldevmap->endDeviceCapabilities);
						*enddevsCapabilities = retDevMap;
						retStatus = PLUGIN_SUCCESS;
				} else {
						retStatus = PLUGIN_FAILURE;
						APP_LOG("CAPABILITIES", LOG_ERR, "Memory allocation failed");
				}
		} else {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "No entry in gEndDevToCapabilityMappingList for deviceIdx %d with hwAddress %s", deviceIdx, hwAddress);
		}

func_exit:
		return retStatus;
}

//update current capability values under end devices capabilities struct once a set current capability
//value action is successfull
int updateDeviceCapabilityCurrentValues(EndDevicesCapabilities *endDevs, CapabilityValue *currValue) {
		int retStatus = PLUGIN_FAILURE;
		CapabilityValue *lcurrval = NULL, *tcurrval = NULL, *acurrval = NULL;
		EndDevicesCapabilities *ldevmap = NULL;

		if ((!endDevs) && (!currValue)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments EndDevicesCapabilities and CapabilityValue");
				goto func_exit;
		}

		LL_SEARCH(gEndDevToCapabilityMappingList, ldevmap, endDevs, compareDeviceIdxHwAddr);
		if (!ldevmap) {
				retStatus = PLUGIN_FAILURE;
				goto func_exit;
		}
#if 0
		if (!(ldevmap->currValues)) {
				acurrval = (CapabilityValue*)calloc(1, sizeof(CapabilityValue));
				if (acurrval) {
						acurrval->capabilityId = currValue->capabilityId;
						strncpy(acurrval->capabilityNameValue, currValue->capabilityNameValue, strlen(currValue->capabilityNameValue));
						LL_APPEND(ldevmap->currValues, acurrval);
						APP_LOG("CAPABILITIES", LOG_DEBUG, "First time current values entry capabilityId 0x%x capabilityNameValue %s", \
								acurrval->capabilityId, acurrval->capabilityNameValue);
						retStatus = PLUGIN_SUCCESS;
						goto func_exit;
				}
		}
#endif
		LL_FOREACH_SAFE(ldevmap->currValues, lcurrval, tcurrval) {
				if (lcurrval->capabilityId == currValue->capabilityId) {
						strncpy(lcurrval->capabilityNameValue, currValue->capabilityNameValue, strlen(currValue->capabilityNameValue));
						APP_LOG("CAPABILITIES", LOG_DEBUG, "Updating current values for entry capabilityId 0x%x capabilityNameValue %s", \
								acurrval->capabilityId, acurrval->capabilityNameValue);
						retStatus = PLUGIN_SUCCESS;
						break;
				} 
		}

		if (retStatus != PLUGIN_SUCCESS) {
				acurrval = (CapabilityValue*)calloc(1, sizeof(CapabilityValue));
				if (acurrval) {
						acurrval->capabilityId = currValue->capabilityId;
						strncpy(acurrval->capabilityNameValue, currValue->capabilityNameValue, strlen(currValue->capabilityNameValue));
						LL_APPEND(ldevmap->currValues, acurrval);
						APP_LOG("CAPABILITIES", LOG_DEBUG, "First time current values entry capabilityId 0x%x capabilityNameValue %s", \
								acurrval->capabilityId, acurrval->capabilityNameValue);
						retStatus = PLUGIN_SUCCESS;
				}
		}

func_exit:
		if (retStatus < PLUGIN_SUCCESS) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Failed to update current values entry capabilityId 0x%x capabilityNameValue %s", \
								currValue->capabilityId, currValue->capabilityNameValue);
		} else {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Updated current values entry capabilityId 0x%x capabilityNameValue %s", \
								currValue->capabilityId, currValue->capabilityNameValue);
		}
		return retStatus;
}
