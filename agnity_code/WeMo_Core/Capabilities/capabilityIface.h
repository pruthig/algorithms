/***************************************************************************
 *
 *
 * capabilityIface.h
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
#ifndef __CAPABILITYIFACE_H
#define __CAPABILITYIFACE_H

//Interface to get last used capability runtime id
//IN:reserved - Reserved for future use, for now should be NULL
//This interface will return last used run time capability id on success,
//0 if no capability is present or <0 in case of any error.
int getLastRunTimeCapabilityId(void *reserved);

//Interface to get count of available Capabilities profile
//IN:reserved - Reserved for future use, for now should be NULL
//This interface will return number of availabale capability profiles,
//0 if no capability is present or <0 in case of any error.
int getNumOfCapabilityProfiles(void *reserved);

//Interface to get available Capability IDs
//IN:capabilityIds - Intger Array for capability ids which will be filled by Ids from this core,
//size of the array should be same as the one returned from getNumOfCapabilities interface
//IN:NumberOfEntries - Size of array capabilityIds
//OUT:capabilityIds - This array will be filled with capability ids available
//This interface will also return number of capability ids entries filled in capabilityIds,
//0 if no id is present or <0 in case of any error.
//If return number of capability ids is less than the return of getNumOfCapabilities then caller of
//getCapabilityProfileIDs can again call this interface with correct size of capabilityIds
int getCapabilityProfileIDs(unsigned int capabilityIds[], unsigned int numOfEntries);

//Interface to get Capability Profiles Data
//IN:capabilityIds - Intger Array of capability ids for which capability profile is requested
//IN:NumberOfEntries - Number of entries in capabilityIds, if 0 all all available profiles will be returned
//OUT:CapabilityProfiles - List of Capability profiles of requested devices or all profiles
//This interface will also return number of capability entries for which CapabilityProfiles list is returned,
//0 if no profile is present or <0 in case of any error.
int getCapabilityProfiles(unsigned int capabilityIds[], unsigned int numOfEntries, CapabilityProfiles **profileData);

//Interface to add Capability Profiles Data
//IN:CapabilityProfiles - List of Capability profiles of requested device
//This interface will return success(0) if CapabilityProfiles is added successfully or
//<0 in case of any error.
int addCapabilityProfiles(CapabilityProfiles *profileData);

//Interface to add Capability Profiles Data
//IN:capabilityId - Capability id for which adding capability profile is requested,
//usually capabilityId will not be known to the modules using this API, hence this
//should be set to 0 in that case and capability core module will generate id and
//add against this profile
//IN:CapabilityProfiles - List of Capability profiles of requested device
//This interface will return success(0) if CapabilityProfiles is added successfully or
//<0 in case of any error.
int addCapabilityProfilesById(unsigned int capabilityId, CapabilityProfiles *profileData);

//Interface to remove Capability Profiles Data
//IN:capabilityIds - Intger Array of capability ids for which capability profile removal is requested
//IN:NumberOfEntries - Number of entries in capabilityIdList, this must be a value greater than 0
//This interface will also return number of capability entries for which CapabilityProfiles is removed,
//0 if no profile is present or <0 in case of any error.
//Also, use of this API is discouraged since removal of a profile should always be against a
//device not directly with capability Ids
int removeCapabilityProfiles(unsigned int capabilityIds[], unsigned int numOfEntries);

//-----------------------------------------------------

//Interface to get an index for a new device trying to publish its capabilities profile
//IN:reserved - Reserved for future use, for now should be NULL
//This interface will returns a unique index for use in subsequent inetrfaces,
//or <0 in case of any error.
//This interface will return capability core generated device index only if
//capabilityCoreDevIdxFlag returns 1 i.e device index generator flag is set
//while initializing capability core.
unsigned int getNewDeviceIndex(void *reserved);

//Interface to add Capability Profiles of a device
//IN:deviceIdx - Device index for which capability profile as well as its mapping with
//device is requested. This is a mandatory input from the caller of this interface.
//IN:hwAddress - Device hardware address for which capability profile as well as its mapping with
//device is requested. This is a mandatory input from the caller of this interface.
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//This interface will return device index same as the one passed to this interface if successfull
//else or <0 in case of any error.
int addDeviceCapabilities(unsigned int deviceIdx, char *hwAddress, CapabilityProfiles *profileData);

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
int addDeviceCapabilitiesEx(unsigned int deviceIdx, char *hwAddress, CapabilityProfiles *profileData);

//Interface to get end devices Capability
//IN:deviceIdxs - Array of device indices for which capability is required
//IN:numOfDevices - Number of device indices entries in DeviceIdxs
//OUT:EndDevicesCapabilities - List of end devices with their capabilities
//This interface will return number of devices entries in OUT: EndDevicesCapabilities.
//This return will be 0 if capability can't be determined for any devices in deviceIdxs
//or <0 in case of any error.
//If this interface is called with NumberOfDevices set to 0, then OUT:EndDevicesCapabilities
//will be returned for all available sub devices.
int getDevicesCapabilitiesByIdx(unsigned int deviceIdxs[], unsigned int numOfDevices, EndDevicesCapabilities **enddevsCapabilities);

//Interface to get end devices Capability
//IN:devHwAddr - Array of device hardware address for which capability is required
//IN:numOfDevices - Number of device indices entries in devHwAddr
//OUT:EndDevicesCapabilities - List of end devices with their capabilities
//This interface will return number of devices entries in OUT: EndDevicesCapabilities.
//This return will be 0 if capability can't be determined for any devices in devHwAddr
//or <0 in case of any error.
//If this interface is called with NumberOfDevices set to 0, then OUT:EndDevicesCapabilities
//will be returned for all available sub devices.
int getDevicesCapabilitiesByHWAddr(char *devHwAddrs[], unsigned int numOfDevices, EndDevicesCapabilities **enddevsCapabilities);

//Remove capability details corresponding to a end device index
//IN:deviceIdx - index of end devices need to be removed.
//Returns 0 in case of Success or < 0 in case of Failure
int removeDeviceCapabilitiesByIdx(unsigned int deviceIdx);

//Remove capability details corresponding to a end device hardware address
//IN:deviceHwAddress - Hardware address of end devices need to be removed.
//Returns 0 in case of Success or < 0 in case of Failure
int removeDeviceCapabilitiesByHwAddress(char *deviceHwAddr);

//Interface to add Capability Profiles Data
//IN:deviceIdx - Device index for which capability profile as well as its mapping with device
//is requested. This is mandatory for this interface and must exists in the device to capability
//mappining list
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//This interface will return device index if successfull else or <0 in case of any error.
//This interface will return error if this device index is not already present in device
//to capability mapping list.
int addDeviceCapabilitiesByIdx(unsigned int deviceIdx, CapabilityProfiles *profileData);

//Interface to add Capability Profiles Data
//IN:hwAdrress - Device hardware address for which capability profile as well as its mapping with
//device is requested. This is mandatory for this interface and must exists in the device to capability
//mappining list.
//IN:CapabilityProfiles - List of Capability profiles for the requested device
//This interface will return device index if successfull else or <0 in case of any error.
//This interface will return error if this device hardware address is not already present in device
int addDeviceCapabilitiesByHWAddr(char *hwAddress, CapabilityProfiles *profileData);

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
int addDeviceCapabilitiesByIdxEx(unsigned int deviceIdx, CapabilityProfiles *profileData);

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
int addDeviceCapabilitiesByHWAddrEx(char *hwAddress, CapabilityProfiles *profileData);

//Interface to set current capability values of a device
//IN:deviceIdx - index of the device.
//IN:hwAddress - Hardware address of the device.
//IN:CapabilityValue - Current capability value to be set based on the capabilityId.
//All of the above arguements are mandatory for this interface.
//This interfaces returns success (0) if action is successfully set on the requested device else 
//failure (-ve) value is returned
int setDevCapabilityCurrentValue(unsigned int deviceIdx, char *hwAddress, CapabilityValue *pCurrValue);

//Interface to set current capability values of a device
//IN:deviceIdx - index of the device. This is mandatory.
//IN:hwAddress - Hardware address of the device. This is mandatory.
//OUT:CapabilityValue - Current capability value to be set based on the capabilityId.
//This interfaces returns success (0) if action is successfully set on the requested device else 
//failure (-ve) value is returned
//This interface will also return the successfully set value in pCurrValue as output.
int getDevCapabilityCurrentValue(unsigned int deviceIdx, char *hwAddress, CapabilityValue **pCurrValue); 

//Interface to get end device Capability
//IN:deviceIdx - Device Index for which capability is required
//IN:hwAddress - Device hardware address for which capability is required
//OUT:EndDevicesCapabilities - End device with their capabilities
//This interface will return device capabilities in OUT: EndDevicesCapabilities.
//This return will be success (0) if capability of device is determined successfully
//or <0 in case of any error.
int getDevicesCapability(unsigned int deviceIdx, char *hwAddress, EndDevicesCapabilities **enddevsCapabilities);
#endif
