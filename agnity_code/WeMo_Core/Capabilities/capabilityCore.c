/***************************************************************************
 *
 *
 * capabilitycore.c
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

CapabilityCore *gCapabilityCoreIns = NULL;
int gCapabilityCoreInitialized = 0;
int gDeviceIndexGenerator = 0;

//Interface to get capability core intialization status
//This interface will return 1 if capability core is initialized and 0 if not
int capabilityCoreStatus() {
		return gCapabilityCoreInitialized;
}

//Interface to get capability core device index generator. If this interface returns
//1 then capability core will generate device index if a capability is being added by
//a device first time. If 0 then capability core interfaces must always be called with
int capabilityCoreDevIdxFlag() {
		return gDeviceIndexGenerator;
}

//Initialize capability core module. This must be called once to intialize the capability
//core module. No exposed external interfaces of capability core module will be allowed
//to success if capability core is not intialized
//IN:init - Reserved for future use
//IN:flag - If flag is set to 0 then capability core will not generate device indices on
//its own and the caller of the capability core interfaces must provide correct device
//index wherever needed else if flag is set to 1 then capability core will generate device
//index when a device tries to add capability first time
//This will return success (0) if initialized successfully or failure (-ve)
int capabilityCoreInit(void *init, int flag) {
		gCapabilityCoreInitialized = 1;
		
		if (flag) {
				gDeviceIndexGenerator = 1;
		}

		//Need to register callback with lower layer to know the
		//capability of devices joining the network

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capabilities core intialized gCapabilityCoreInitialized:%d gDeviceIndexGenerator:%d", \
				gCapabilityCoreInitialized, gDeviceIndexGenerator);

		return PLUGIN_SUCCESS;
}

//De-initialize capability core
//IN:deinit - Reserved for future use
//IN:flag - Reserved for future use
//This will return success (0) if initialized successfully or failure (-ve)
int capabilityCoreDeInit(void *init, int flag) {
		int retStatus = PLUGIN_SUCCESS;
		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		gCapabilityCoreInitialized = 0;
		gDeviceIndexGenerator = 0;
		
		//TODO, not important right now
		//Need to de-register callback with lower layer to know the
		//capability of devices joining the network
		
		APP_LOG("CAPABILITIES", LOG_DEBUG, "Capabilities core de-intialized gCapabilityCoreInitialized:%d gDeviceIndexGenerator:%d", \
				gCapabilityCoreInitialized, gDeviceIndexGenerator);

		return PLUGIN_SUCCESS;
}

//comapare component id
static int compareComponentIds(CapabilityCore *core1, CapabilityCore *core2) {
		if (core1->coreEx.componentId == core2->coreEx.componentId) {
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

//Creation of a Capability Core instance requested by an external
//component
//External component trying to create a capability core instance must provide
//component id and capability core extended structure filled with callback handlers
//IN:compId - component id of the module creating a instance
//IN:CapabilityCoreEx - capability core structure with filled callbacks
//IN:flag - Reserved for future use
//This will return success (0) if instance created successfully or failure (-ve)
int capabilityCoreCreate(int compId, CapabilityCoreEx *core, int flag) {
		CapabilityCore *mcore = NULL, tmpcore;
		int retStatus = PLUGIN_FAILURE;
	
		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		if (!core) {
				retStatus = PLUGIN_FAILURE; 
				APP_LOG("CAPABILITIES", LOG_ERR, "capability core instance of componentId:%d is NULL", compId);
				goto func_exit;
		}

		if (compId <= PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE; 
				APP_LOG("CAPABILITIES", LOG_ERR, "capability core instance of componentId:%d is not valid", compId);
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to create capability core instance by componentId:%d", compId);
		tmpcore.coreEx.componentId = compId;
		LL_SEARCH(gCapabilityCoreIns, mcore, &tmpcore, compareComponentIds);
		if (mcore) {
				retStatus = PLUGIN_FAILURE; 
				APP_LOG("CAPABILITIES", LOG_ERR, "capability core instance of componentId:%d already exists", compId);
				goto func_exit;
		} else {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Creating capability core instance of componentId:%d", compId);
				mcore = (CapabilityCore *)calloc(1, sizeof(CapabilityCore));
				if (mcore) {
						memcpy(&mcore->coreEx, core, sizeof(CapabilityCoreEx));
						LL_APPEND(gCapabilityCoreIns, mcore);
						retStatus = PLUGIN_SUCCESS;
						APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability core instance of componentId:%d created successfull", compId);
				}
		}

func_exit:
		return retStatus;
}

//Destroy capability core instance corresponding to the component id of the 
//external module
//IN:compId -  component id of the module destroying the instance
//IN:flag - Reserved for future use
//This will return success (0) if instance created successfully or failure (-ve)
int capabilityCoreDestroy(unsigned int compId, int flag) {
		CapabilityCore *mcore = NULL, tmpcore;
		int retStatus = PLUGIN_FAILURE;
	
		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		if (compId <= PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE; 
				APP_LOG("CAPABILITIES", LOG_ERR, "capability core instance of componentId:%d is not valid", compId);
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Request to destroy capability core instance by componentId:%d", compId);
		tmpcore.coreEx.componentId = compId;
		LL_SEARCH(gCapabilityCoreIns, mcore, &tmpcore, compareComponentIds);
		if (mcore) {
				LL_DELETE(gCapabilityCoreIns, mcore);
				free(mcore);
				mcore = 0;
				retStatus = PLUGIN_SUCCESS;
				APP_LOG("CAPABILITIES", LOG_DEBUG, "Capability core instance of componentId:%d deleted successfull", compId);
		} else {
				APP_LOG("CAPABILITIES", LOG_ERR, "Capability core instance of componentId:%d deletion failed", compId);
				retStatus = PLUGIN_FAILURE; 
		}

func_exit:
		return retStatus;
}

//Internal interface to invoke callbacks registered with core by upper layer modules
//interacting wit capability core. Whenever a device adds a capability the below interface
//should be called with flag set 1 and in case of delete this should be called with 0
//IN:EndDeviceCapabilities - Device to capability mapping node whenever a device added or 
//remove a capability corresponding to it
void invokeCoreCallbacks(EndDeviceCapabilities *devCaps, int flag) {
		CapabilityCore *lcoreIns = NULL, *tcoreIns = NULL;
		
		if (!devCaps) {
				APP_LOG("CAPABILITIES", LOG_ERR, "Argument devCaps is NULL, not valid");
				return;
		}

		LL_FOREACH_SAFE(gCapabilityCoreIns, lcoreIns, tcoreIns)
		{
				if (lcoreIns) {
						if ((lcoreIns->coreEx.on_add_dev_capability) && (flag == 1)) {
								APP_LOG("CAPABILITIES", LOG_DEBUG, "Invoking on_add_dev_capability handler for Capability core instance of componentId:%d ", \
										lcoreIns->coreEx.componentId);
								(lcoreIns->coreEx.on_add_dev_capability)(devCaps->endDeviceIdx, devCaps->endDeviceIdentifier, devCaps);
						}


						if ((lcoreIns->coreEx.on_del_dev_capability) && (flag == 0)) {
								APP_LOG("CAPABILITIES", LOG_DEBUG, "Invoking on_del_dev_capability handler for Capability core instance of componentId:%d ", \
										lcoreIns->coreEx.componentId);
								(lcoreIns->coreEx.on_del_dev_capability)(devCaps->endDeviceIdx, devCaps->endDeviceIdentifier, devCaps);
						}
				}
		}
		return;
}
