/***************************************************************************
 *
 *
 * capabilityDefs.h
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
#ifndef __CAPABILITYDEFS_H
#define __CAPABILITYDEFS_H

//Maximum number of global profiles entries in gCapabilityGlobalProfiles
//If any new entry is added to the gCapabilityGlobalProfiles then this macro value
//must be changed to correct value
#define MAX_GLOBAL_PROFILES	5

//Maximum number of runtime profiles entries for retrieving capanility id array
#define MAX_CAP_LIMIT		10

//Macro to check whether expr returned 1 or any other value, 1 is success else failure
//Currently this is used to check return of capabilityCoreStatus which return 1 if capability
//core is initialized or else 0. This macro should be called at entry of all exposed interfaces
//from capability core module. No exposed interfaces should be allowed to be success if capability
//core is not initialized successfully
#define CHECK_CAPABILITYCORE_STATUS(expr) retStatus=expr; \
													 if (retStatus!=1) { \
																APP_LOG("CAPABILITIES", LOG_ERR, "%s returned retStatus %d", #expr, retStatus); \
															 return -1; \
													 }
//Macro to check whether expr returned 1 or any other value, 1 is success else failure
//Currently this is used to check return of capabilityCoreStatus which return 1 if capability
//core is initialized or else 0. This macro should be called at entry of all exposed interfaces
//from capability core module. No exposed interfaces should be allowed to be success if capability
//core is not initialized successfully
#define CHECK_CAPABILITYCORE_STATUS_P(expr) retStatus=expr; \
													 if (retStatus!=1) { \
																APP_LOG("CAPABILITIES", LOG_ERR, "%s returned retStatus %d", #expr, retStatus); \
															 return NULL; \
													 }

//Internal interfaces to capability core module, should not be called from external
//modules interacting with capability core
//Internal interface to all capability profile
//IN:CapabilityProfiles - capability profile needs to be added, this must not be NULL
//This will return capability id after successfull addition of capability profile or else
//FAILURE (-ve)
int addCapabilityProfilesI(CapabilityProfiles *profileData);

//Internal interface to invoke callbacks registered with core by upper layer modules
//interacting wit capability core. Whenever a device adds a capability the below interface
//should be called with flag set 1 and in case of delete this should be called with 0
//IN:EndDeviceCapabilities - Device to capability mapping node whenever a device added or 
//remove a capability corresponding to it
void invokeCoreCallbacks(EndDeviceCapabilities *devCaps, int flag);

//Internal interface to get all available capability profiles,
//internal to capability core module
//This interface on successfull call returns all available capability profiles pointed by
//profileData along with number of profile counts else failure (-ve) will be returned.
int getAllCapabilityProfilesI(CapabilityProfiles **profileData);

//Get global capability id corresponding to the name
//IN:profileName - Name of the global profile for which profile attributes are needed.
//On success this interface returns CapabilityProfileData or else NULL is returned.
CapabilityProfileData* getGlobalCapbilityIdByName(char *profileName);

//update current capability values under end devices capabilities struct once a set current capability
//value action is successfull
int updateDeviceCapabilityCurrentValues(EndDevicesCapabilities *endDevs, CapabilityValue *currValue);
#endif
