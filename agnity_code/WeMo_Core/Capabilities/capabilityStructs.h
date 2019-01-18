/***************************************************************************
 *
 *
 * capabilityStructs.h
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
#ifndef __CAPABILITYSTRUCTS_H
#define __CAPABILITYSTRUCTS_H

//Capability Profile Name and IDs
//The below are global capability profile type identifiers
//First two MSB octets will represent well known global profiles
//and rest of the 6 octets will represent the sub profiles which
//will be learnt dynamically by underlying modules.
//Must be same accross all entities using or
//interacting with this layer.
#define	BinaryCtl		0x01000000
#define LinearCtl		0x02000000
#define ToggleCtl		0x03000000
#define DiscreteCtl	0x04000000
#define	MessageCtl	0x05000000

//String representation of the above global profiles
//Must be same accross all entities using or
//interacting with this layer.
#define	BinaryCtlS		"BinaryCtl"
#define LinearCtlS		"LinearCtl"
#define ToggleCtlS		"ToggleCtl"
#define DiscreteCtlS	"DiscreteCtl"
#define	MessageCtlS		"MessageCtl"

//Capability Specification defines
//This defines the underlying protocol supported by
//the devices or end devices which is publishing
//its capability.
//This will be useful if both the mediums are supported and
//applications using this layer wants to perform some specific
//operation based on the suppoted protocol
#define WiFi		0x00000001
#define ZigBee	0x00000002

//Capability Data Types
#define Boolean					0		//binary controllable values such as ON(1) and OFF(0) or
														//TURE(1) or FALSE(0)
#define Integer					1		//AutoToggle control where only next supported value
														//such as toggle to OFF(0) or ON(1) 
#define String					2 
#define IntegerRange		3		//For variable range of values such as min 0 to max 100
#define IntegerSet			4		//Discrete range of controllable values

//Control Access Modes define
#define RO	1		//Capabilities are read i.e get only
#define RW	2		//Capabilities can be read (get) as well as write (set)

//Capability Profile Characteristics Structure
typedef struct CapabilityProfileData {
		unsigned int capabilityId;	//Global capability Id such as 0x01000000, 0x02000000 ORed with
																//Sub profile id
		char capabilityProfileName[SIZE_20B];		//Global capability name such as BinaryCtl, LinearCtl
		unsigned int capabilitySpec;		//Protocol Specification WiFi or ZigBee etc...
		unsigned int capabilityDataType;		//Capability data type such as Boolean, IntegerRange, String etc...
		char capabilityNameValue[SIZE_128B];	//Capability data name and value pair such as "ON:1,OFF:0" or 
																//"LOW:0,HIGH:100,TIME:120" or "VAL1:2,VAL2:5,VAL3:7,VAL4:9"
		unsigned int capabilityControl;	//Control is only readable only 1 or writable 2 
		char capabilitySubProfileName[SIZE_20B];	//Cability sub profile name such as "SimpleBulb", "DimmingBulb" etc...
																						  //This should be pre-defined and fixed name corresponding to a matching
																							//capability of devices
		int (*on_set_action)(unsigned int, char*, char*, void*);	//Interface exposed by underlying module for setting current value
		int (*on_get_action)(unsigned int, char*, char*, void**);	//Interface exposed by underlying module for getting current value
}CapabilityProfileData, *pCapabilityProfileData;

//List of learnt capability profiles
typedef struct CapabilityProfiles {
		CapabilityProfileData capabilityData;		//Capability profile data attributes
		unsigned int refCount;									//This profile usage reference count to indicate multiple devices
																						//having this same profile		
		struct CapabilityProfiles *next;		//Pointer to the next Capability profile data
}CapabilityProfiles, *pCapabilityProfiles;

//Device to their Capabilties Mappings
typedef struct EndDeviceCapabilities {
		unsigned int endDeviceIdx;	//Integer index of end device
		char endDeviceIdentifier[SIZE_20B];	//Hardware Address of end device
#if 0
		unsigned int endDeviceStatus;		//Current status of end device such as paired, unpaired, invalid etc...
#endif
		unsigned int numOfCapabilityId;	//Number of supported capability id, maximum 8
		unsigned int capabilityIds[SIZE_8B];		//Integer array of capabilities ID supported by this end device
																		//These capability ID will be uniquely identified by entry in
																		//CapabilityProfileData in CapabilityProfiles list, Max 8 capabilities
																		//can be supported by device
}EndDeviceCapabilities, *pEndDeviceCapabilities;

//Structure to set or get device capabilities current values
typedef struct CapabilityValue {
		unsigned int capabilityId;	//Capability Id
		char capabilityNameValue[SIZE_128B]; //Current value to be set or checked if get
		struct CapabilityValue *next;		//Pointer to next CapabilityValue
}CapabilityValue, *pCapabilityValue;

//List of end devices with their capability
typedef struct EndDevicesCapabilities {
		EndDeviceCapabilities endDeviceCapabilities;		//Device to their Capabilties Mappings
		CapabilityValue *currValues;		//Current Capability values of this device
		struct EndDevicesCapabilities *next;	//Pointer to the next DeviceCapability
}EndDevicesCapabilities, *pEndDevicesCapabilities;

//Top level bridge device structure with end devices and 
//their capability mappings
typedef struct DeviceCapabilities {
		char thisDeviceId[SIZE_20B];
		EndDevicesCapabilities *endDevicesCapabilities;
}DeviceCapabilities, *pDeviceCapabilities;

#endif
