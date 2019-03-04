/***************************************************************************
 *
 *
 * capabilityActions.c
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

//Compare capability Ids of two profiles
static int compareCurrCapabilityIds(CapabilityValue *profile1, CapabilityValue *profile2) {
		if (profile1->capabilityId == profile2->capabilityId) {
				APP_LOG("CAPABILITIES", LOG_DEBUG, "compareCurrCapabilityIds = profile1:0x%x - profile2:0x%x\n", \
						profile1->capabilityId, profile2->capabilityId);
				return PLUGIN_SUCCESS;
		}
		return PLUGIN_FAILURE;
}

int checkDeviceCapabilityValid(unsigned int capabilityId, EndDevicesCapabilities *ldevmap) {
		unsigned int i = 0;
		int retStatus = PLUGIN_FAILURE;

		APP_LOG("CAPABILITIES", LOG_DEBUG, "checkDeviceCapabilityValid = capabilityId:0x%x", capabilityId);
		for (i = 0; i < ldevmap->endDeviceCapabilities.numOfCapabilityId; ++i ) {
				if (capabilityId == ldevmap->endDeviceCapabilities.capabilityIds[i]) {
						APP_LOG("CAPABILITIES", LOG_DEBUG, "checkDeviceCapabilityValid = capabilityId:0x%x - ldevmap->endDeviceCapabilities.capabilityIds[i]:0x%x\n", \
								capabilityId, ldevmap->endDeviceCapabilities.capabilityIds[i]);
						retStatus = PLUGIN_SUCCESS;
						break;
				}
		}

		return retStatus;
}

int setDevCapabilityCurrentValue(unsigned int deviceIdx, char *hwAddress, CapabilityValue *pCurrValue) {
		int retStatus = PLUGIN_FAILURE, retStatus1 = PLUGIN_FAILURE, count = 0, i = 0;
		EndDevicesCapabilities *ldevmap = NULL;
		CapabilityValue *lcurrval = NULL, *tcurrval = NULL, cmpval;
		unsigned int capabilityIds[MAX_CAP_LIMIT];
		CapabilityProfiles *profiles = NULL, *lprofiles = NULL, *tprofiles = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		if ((deviceIdx <= PLUGIN_SUCCESS) && (!hwAddress) && (!pCurrValue)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check deviceIdx %d or hwAddress %s or pCurrValue", \
								deviceIdx, hwAddress);
				goto func_exit;
		}

		LL_COUNT(pCurrValue, tcurrval, count);
		if (count <= PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Count in pCurrValue is zero for deviceIdx %d or hwAddress %s", \
								deviceIdx, hwAddress);
				goto func_exit;
		}

		retStatus = getDevicesCapability(deviceIdx, hwAddress, &ldevmap);
		if (retStatus < PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Couldn't get device capability profile for deviceIdx %d and hwAddress %s", \
								deviceIdx, hwAddress);
				goto func_exit;
		}

		APP_LOG("CAPABILITIES", LOG_DEBUG, "Count in pCurrValue is %d for deviceIdx %d or hwAddress %s", \
						count, deviceIdx, hwAddress);

		LL_FOREACH_SAFE(pCurrValue, lcurrval, tcurrval) {
				retStatus = checkDeviceCapabilityValid(pCurrValue->capabilityId, ldevmap);
				if (retStatus == PLUGIN_SUCCESS) {
						capabilityIds[i] = pCurrValue->capabilityId;
						APP_LOG("CAPABILITIES", LOG_DEBUG, "pCurrValue capabilityId 0x%x at index %d", capabilityIds[i], i);
						++i;
				}
		}

		retStatus = getCapabilityProfiles(capabilityIds, i, &profiles);
		if (retStatus <= PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Couldn't get capability profiles against capabilityIds for deviceIdx %d and hwAddress %s", \
								deviceIdx, hwAddress);
		} else {
				LL_FOREACH_SAFE(profiles, lprofiles, tprofiles) {
						if (lprofiles) {
								cmpval.capabilityId = lprofiles->capabilityData.capabilityId;
								LL_SEARCH(pCurrValue, lcurrval, &cmpval, compareCurrCapabilityIds);
								if ((lcurrval) && (lprofiles->capabilityData.capabilityControl == RW) && (lprofiles->capabilityData.on_set_action)) {
										retStatus = (lprofiles->capabilityData.on_set_action)(ldevmap->endDeviceCapabilities.endDeviceIdx, \
														ldevmap->endDeviceCapabilities.endDeviceIdentifier, lprofiles->capabilityData.capabilityProfileName, lcurrval->capabilityNameValue);
										APP_LOG("CAPABILITIES", LOG_DEBUG, "on_set_action returned %d for deviceIdx %d or hwAddress %s", retStatus, deviceIdx, hwAddress);
										if (retStatus == PLUGIN_SUCCESS) {
												retStatus1 = updateDeviceCapabilityCurrentValues(ldevmap, lcurrval);
												APP_LOG("CAPABILITIES", LOG_DEBUG, "updateDeviceCapabilityCurrentValues returned %d for deviceIdx %d or hwAddress %s", retStatus1, deviceIdx, hwAddress);
										}
								}
						}
				}
		}
		APP_LOG("CAPABILITIES", LOG_DEBUG, "retStatus returned is %d for deviceIdx %d or hwAddress %s", retStatus, deviceIdx, hwAddress);
func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "retStatus returned is %d for deviceIdx %d or hwAddress %s", retStatus, deviceIdx, hwAddress);
		return retStatus;
}

int getDevCapabilityCurrentValue(unsigned int deviceIdx, char *hwAddress, CapabilityValue **pCurrValue) {
		int retStatus = PLUGIN_FAILURE, count = 0, i = 0;
		EndDevicesCapabilities *ldevmap = NULL;
		CapabilityValue *lcurrval = NULL, *tcurrval = NULL, *ocurrval = NULL, *acurrval = NULL;
		CapabilityProfiles *profiles = NULL, *lprofiles = NULL, *tprofiles = NULL;
		char *getcurrval = NULL;

		CHECK_CAPABILITYCORE_STATUS(capabilityCoreStatus());

		if ((deviceIdx <= PLUGIN_SUCCESS) && (!hwAddress)) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Invalid arguments check deviceIdx %d or hwAddress %s", \
								deviceIdx, hwAddress);
				goto func_exit;
		}

		retStatus = getDevicesCapability(deviceIdx, hwAddress, &ldevmap);
		if (retStatus < PLUGIN_SUCCESS) {
				retStatus = PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Couldn't get device capability profile for deviceIdx %d and hwAddress %s", \
								deviceIdx, hwAddress);
				goto func_exit;
		}

		count = 0;
		retStatus = getCapabilityProfiles(ldevmap->endDeviceCapabilities.capabilityIds, ldevmap->endDeviceCapabilities.numOfCapabilityId, &profiles);
		if (retStatus <= PLUGIN_SUCCESS) {
				retStatus= PLUGIN_FAILURE;
				APP_LOG("CAPABILITIES", LOG_ERR, "Couldn't get capability profiles against capabilityIds for deviceIdx %d and hwAddress %s", \
								deviceIdx, hwAddress);
		} else {
				LL_FOREACH_SAFE(profiles, lprofiles, tprofiles) {
						if (lprofiles) {
								if ((lprofiles->capabilityData.on_get_action)) {
										retStatus = (lprofiles->capabilityData.on_get_action)(ldevmap->endDeviceCapabilities.endDeviceIdx, \
														ldevmap->endDeviceCapabilities.endDeviceIdentifier, lprofiles->capabilityData.capabilityProfileName, (void**)&getcurrval);
										if (retStatus <= PLUGIN_SUCCESS) {
												retStatus= PLUGIN_FAILURE;
												APP_LOG("CAPABILITIES", LOG_ERR, "on_get_action failed for deviceIdx %d and hwAddress %s", deviceIdx, hwAddress);
										} else {
												if (getcurrval) {
														tcurrval = (CapabilityValue*)calloc(1, sizeof(CapabilityValue));
														if (tcurrval) {
																tcurrval->capabilityId = lprofiles->capabilityData.capabilityId;
																strncpy(tcurrval->capabilityNameValue, getcurrval, strlen(getcurrval));
																LL_APPEND(lcurrval, tcurrval);
																retStatus = PLUGIN_SUCCESS;
																APP_LOG("CAPABILITIES", LOG_DEBUG, "on_get_action returned %d for deviceIdx %d or hwAddress %s", retStatus, deviceIdx, hwAddress);
																APP_LOG("CAPABILITIES", LOG_DEBUG, "on_get_action returned capabilityNameValue %s for capabilityId %d",tcurrval->capabilityNameValue, \
																				tcurrval->capabilityId);
																++count;
														} else {
																APP_LOG("CAPABILITIES", LOG_ERR, "Memory allocation failed for CapabilityValue for deviceIdx %d and hwAddress %s", \
																				deviceIdx, hwAddress);
																retStatus = PLUGIN_FAILURE;
														}
												} else {
														APP_LOG("CAPABILITIES", LOG_ERR, "on_get_action failed for capabilityNameValue for deviceIdx %d and hwAddress %s", \
																		deviceIdx, hwAddress);
														retStatus = PLUGIN_FAILURE;
												}
										}
								}
						}
				}
				if (count > PLUGIN_SUCCESS) {
						APP_LOG("CAPABILITIES", LOG_DEBUG, "count is gt than PLUGIN_SUCCESS %d for capabilityNameValue for deviceIdx %d and hwAddress %s", \
								count, deviceIdx, hwAddress);
						*pCurrValue = lcurrval;
						retStatus = PLUGIN_SUCCESS;	
				}

				if (count <= PLUGIN_SUCCESS) {
						APP_LOG("CAPABILITIES", LOG_DEBUG, "count is less than PLUGIN_SUCCESS %d for capabilityNameValue for deviceIdx %d and hwAddress %s", \
								count, deviceIdx, hwAddress);
						LL_FOREACH_SAFE(ldevmap->currValues, lcurrval, tcurrval) {
								acurrval = (CapabilityValue*)calloc(1, sizeof(CapabilityValue));
								if (acurrval) {
										acurrval->capabilityId = lcurrval->capabilityId;
										strncpy(acurrval->capabilityNameValue, lcurrval->capabilityNameValue, strlen(lcurrval->capabilityNameValue));
										LL_APPEND(ocurrval, acurrval);
										APP_LOG("CAPABILITIES", LOG_DEBUG, "Current values entry capabilityId 0x%x capabilityNameValue %s from capability core storage", \
												acurrval->capabilityId, acurrval->capabilityNameValue);
										++i;
								}
						}
						if (i > PLUGIN_SUCCESS) {
								APP_LOG("CAPABILITIES", LOG_DEBUG, "count is gt than PLUGIN_SUCCESS %d for capabilityNameValue for deviceIdx %d and hwAddress %s", \
										i, deviceIdx, hwAddress);
								*pCurrValue = ocurrval;
								retStatus = PLUGIN_SUCCESS;
						}
				}
		}
		APP_LOG("CAPABILITIES", LOG_DEBUG, "retStatus returned is %d for deviceIdx %d or hwAddress %s count %d i %d", retStatus, deviceIdx, hwAddress, count, i);
func_exit:
		APP_LOG("CAPABILITIES", LOG_DEBUG, "retStatus returned is %d for deviceIdx %d or hwAddress %s count %d i %d", retStatus, deviceIdx, hwAddress, count, i);
		return retStatus;
}
