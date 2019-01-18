/***************************************************************************
 *
 *
 * capabilityCore.h
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
#ifndef __CAPABILITYCORE_H
#define __CAPABILITYCORE_H

//Capability core extended structure.
//Any external module which needs a realtime indication when a device adds or removes a 
//capability, must register the callback handlers on_add_dev_capability and on_del_dev_capability
//along with a componentId (this is the modules identifier) using capabilityCoreCreate interface
typedef struct CapabilityCoreEx {
		unsigned int componentId;		//Module Identifier of the external component which is registering
																//the callback handlers
		//callback handler being invoked whenever a device adds a capability
		int (*on_add_dev_capability)(unsigned int deviceIdx, char *hwAddr, EndDeviceCapabilities *devCapability);
		//callback handler being invoked whenever a device removes a capability
		int(*on_del_dev_capability)(unsigned int deviceIdx, char *hwAddr, EndDeviceCapabilities *devCapability);
}CapabilityCoreEx, *pCapabilityCoreEx;

//List of capability core extended structure, each node corresponds to a component which has registered
//callbacks using CapabilityCoreEx and capabilityCoreCreate
typedef struct CapabilityCore {
		CapabilityCoreEx coreEx;		//Component node of component id and callback handlers
		struct CapabilityCore *next;		//Pointer to next node
}CapabilityCore, *pCapabilityCore;

//Interface to get capability core intialization status
//This interface will return 1 if capability core is initialized and 0 if not
int capabilityCoreStatus();

//Interface to get capability core device index generator. If this interface returns
//1 then capability core will generate device index if a capability is being added by
//a device first time. If 0 then capability core interfaces must always be called with
//correct device index.
int capabilityCoreDevIdxFlag();

//Initialize capability core module. This must be called once to intialize the capability
//core module. No exposed external interfaces of capability core module will be allowed
//to success if capability core is not intialized
//IN:init - Reserved for future use
//IN:flag - If flag is set to 0 then capability core will not generate device indices on
//its own and the caller of the capability core interfaces must provide correct device
//index wherever needed else if flag is set to 1 then capability core will generate device
//index when a device tries to add capability first time
//This will return success (0) if initialized successfully or failure (-ve)
int capabilityCoreInit(void *init, int flag);

//De-initialize capability core
//IN:deinit - Reserved for future use
//IN:flag - Reserved for future use
//This will return success (0) if initialized successfully or failure (-ve)
int capabilityCoreDeInit(void *deinit, int flag);

//Creation of a Capability Core instance requested by an external
//component
//External component trying to create a capability core instance must provide
//component id and capability core extended structure filled with callback handlers
//IN:compId - component id of the module creating a instance
//IN:CapabilityCoreEx - capability core structure with filled callbacks
//IN:flag - Reserved for future use
//This will return success (0) if instance created successfully or failure (-ve)
int capabilityCoreCreate(int compId, CapabilityCoreEx *core, int flag);

//Destroy capability core instance corresponding to the component id of the 
//external module
//IN:compId -  component id of the module destroying the instance
//IN:flag - Reserved for future use
//This will return success (0) if instance created successfully or failure (-ve)
int capabilityCoreDestroy(unsigned int compId, int flag);

#endif
