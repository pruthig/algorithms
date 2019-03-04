/***************************************************************************
*
*
* remoteHandlers.c
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
#include <sys/stat.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "types.h"
#include "defines.h"
#include "remoteAccess.h"
#include "mxml.h"
#include "logger.h"

#include "pjnath.h"
#include "pjlib-util.h"
#include "pjlib.h"
#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#include "gpio.h"
#include "cloudIF.h"
#include "rule.h"
#include "utils.h"
#include "aes_inc.h"
#include "httpsWrapper.h"
#include "controlledevice.h"
#include "natTravIceIntDef.h"

#include "controlledevice.h"
#include "insightRemoteAccess.h"
#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif

#include "thready_utils.h"


#ifdef WeMo_SMART_SETUP_V2
extern int g_customizedState;
#endif

#ifdef SIMULATED_OCCUPANCY
extern int gSimulatedDeviceCount;
#endif
//#define MAX_RULE_ENTRY_SIZE SIZE_1024B

extern char g_szFriendlyName[SIZE_256B];
extern char g_szHomeId[SIZE_20B];
extern char g_szSmartDeviceId[SIZE_256B];
extern char g_szSmartPrivateKey[MAX_PKEY_LEN];
extern char g_szPluginPrivatekey[MAX_PKEY_LEN];
extern char g_szPluginCloudId[SIZE_16B];
extern char g_szRestoreState[MAX_RES_LEN];
extern char g_NotificationStatus[MAX_RES_LEN];
extern char g_szFirmwareURLSignature[MAX_FW_URL_SIGN_LEN];
extern char g_szFirmwareVersion[SIZE_64B];
extern char g_szSerialNo[SIZE_64B];
extern char g_szWiFiMacAddress[SIZE_64B];
extern int gSignalStrength;
extern char gIcePrevTransactionId[PLUGIN_MAX_REQUESTS][TRANSACTION_ID_LEN];
extern int gIceRunning;
extern int gDataInProgress;
extern nattrav_cand_offer_Info gRemoteInfo;
static int gEncDecEnabled = 1;
static char* szWeekDayShortname[WEEK_DAYS_NO] = {"mon", "tues", "wed", "thurs", "fri", "sat", "sun"};
static int  getDBFile(unsigned long int length,char *url,char*cookie);
int  sendDBFile(unsigned long int length,char *url, char *fsFileUrl ,char* cookie);
char *remotePktDecrypt(void *pkt, unsigned pkt_len, int *declen);


#ifdef PRODUCT_WeMo_NetCam
extern void AsyncRequestNetCamFriendlyName();
#endif

int remoteAccessupdateweeklyTrigger(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id);

int GetDevStatusHandle(void *hndl, char *xmlBuf, unsigned int buflen, void* data_sock , void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    pluginDeviceStatus *pdevStatus = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;
    char statusResp[MAX_DATA_LEN];
    char *dVer = NULL, *dbVer = NULL;
    char *override = NULL, *ovrd = NULL;
    int state=0;
#ifdef PRODUCT_WeMo_CrockPot
    int eventDuration=0,crockpotRet=1;
#endif
#ifdef WeMo_SMART_SETUP_V2
    char customizedState[SIZE_64B] = {'\0'};
    if(g_customizedState)
	snprintf(customizedState, sizeof(customizedState), "<customizedState>%d</customizedState>", DEVICE_CUSTOMIZED);
    else
	snprintf(customizedState, sizeof(customizedState), "<customizedState>%d</customizedState>", DEVICE_UNCUSTOMIZED);
    APP_LOG("REMOTEACCESS", LOG_DEBUG, "customized state: %s", customizedState);
#endif
    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
		CHECK_PREV_TSX_ID(E_PLUGINDEVICESTATUS,transaction_id,retVal);

    pdevStatus = (pluginDeviceStatus*)calloc(1, sizeof(pluginDeviceStatus));
    if (!pdevStatus) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }

    APP_LOG("REMOTEACCESS", LOG_DEBUG, "transaction_id ===%s", transaction_id);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	dVer = (char*)calloc(1, MAX_RVAL_LEN);
	if (!dVer) {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "dVer malloc failed");
	    retVal = PLUGIN_FAILURE;
	    mxmlDelete(tree);
	    goto handler_exit1;
	}
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    strcpy(pdevStatus->macAddress, (find_node->child->value.opaque));
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp(pdevStatus->macAddress, g_szWiFiMacAddress)) {
		//Get plug-in details as well as device status
		//use gemtek apis to get plugin details
		memset(dVer, 0x0, MAX_RVAL_LEN);
		dbVer = CloudGetRuleDBVersion(dVer);
		override = (char*)calloc(1, MAX_FILE_LINE*2);
		ovrd = GetOverriddenStatus(override); 
		pdevStatus->pluginId = strtol(g_szPluginCloudId, NULL, 0);
		strcpy(pdevStatus->macAddress, g_szWiFiMacAddress);
		strcpy(pdevStatus->friendlyName, g_szFriendlyName);
		//use gpio interfaces for device status
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Current Binary state of plug-in %d", GetCurBinaryState());
		pdevStatus->status = CloudGetBinaryState();
		pdevStatus->statusTS = getPluginStatusTS();
		state = getCurrFWUpdateState();
		#ifdef PRODUCT_WeMo_NetCam
		AsyncRequestNetCamFriendlyName();
		#endif
#ifdef PRODUCT_WeMo_CrockPot
		/* Get the current crockpot status */
		crockpotRet = CloudGetCrockPotStatus(&(pdevStatus->status),&eventDuration);				
#endif /* #ifdef PRODUCT_WeMo_CrockPot */
		//Create xml response
		memset(statusResp, 0x0, MAX_DATA_LEN);
		if (dbVer && ovrd) {
#ifdef PRODUCT_WeMo_CrockPot
		    /* Event duration is the extra parameter */
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden></rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><eventDuration>%d</eventDuration></plugin>\n", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,eventDuration);
#else /*#ifdef PRODUCT_WeMo_CrockPot*/
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>%s</dbVersion><rOverriden>%s</rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><iconVersion>%d</iconVersion>",  pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName, dbVer, ovrd,g_NotificationStatus, pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,gWebIconVersion);
#ifdef WeMo_SMART_SETUP_V2
		    strncat(statusResp, customizedState, sizeof(statusResp)-strlen(statusResp)-1);
#endif
		    strncat(statusResp, "</plugin>\n", sizeof(statusResp)-strlen(statusResp)-1);
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
		}else if ((!(dbVer)) && (ovrd)){
#ifdef PRODUCT_WeMo_CrockPot
		    /* Event duration is the extra parameter */
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden></rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><eventDuration>%d</eventDuration></plugin>\n", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,eventDuration);
#else /*#ifdef PRODUCT_WeMo_CrockPot*/
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden>%s</rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><iconVersion>%d</iconVersion>",  pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName, ovrd,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,gWebIconVersion);
#ifdef WeMo_SMART_SETUP_V2
		    strncat(statusResp, customizedState, sizeof(statusResp)-strlen(statusResp)-1);
#endif
		    strncat(statusResp, "</plugin>\n", sizeof(statusResp)-strlen(statusResp)-1);
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
		}else if (dbVer && (!(ovrd))) {
#ifdef PRODUCT_WeMo_CrockPot
		    /* Event duration is the extra parameter */
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden></rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><eventDuration>%d</eventDuration></plugin>\n", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,eventDuration);
#else /*#ifdef PRODUCT_WeMo_CrockPot*/
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>%s</dbVersion><rOverriden></rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><iconVersion>%d</iconVersion>",  pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName, dbVer,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,gWebIconVersion);
#ifdef WeMo_SMART_SETUP_V2
		    strncat(statusResp, customizedState, sizeof(statusResp)-strlen(statusResp)-1);
#endif
		    strncat(statusResp, "</plugin>\n", sizeof(statusResp)-strlen(statusResp)-1);
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
		}else {
#ifdef PRODUCT_WeMo_CrockPot
		    /* Event duration is the extra parameter */
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden></rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><eventDuration>%d</eventDuration></plugin>\n",  pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,eventDuration);
#else /*#ifdef PRODUCT_WeMo_CrockPot*/
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden></rOverriden><notificationType>%s</notificationType><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><iconVersion>%d</iconVersion>", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,g_NotificationStatus,pdevStatus->statusTS, g_szFirmwareVersion, state, gSignalStrength,gWebIconVersion);
#ifdef WeMo_SMART_SETUP_V2
		    strncat(statusResp, customizedState, sizeof(statusResp)-strlen(statusResp)-1);
#endif
		    strncat(statusResp, "</plugin>\n", sizeof(statusResp)-strlen(statusResp)-1);
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_PLUGINDEVICESTATUS);
	    } else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", pdevStatus->macAddress, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (pdevStatus) {free(pdevStatus); pdevStatus = NULL;}
    if (dVer) {free(dVer);dVer = NULL;}
    if (ovrd) {free(ovrd);ovrd = NULL;}
    return retVal;
}

#ifdef SIMULATED_OCCUPANCY
int saveSimDevXml(void *xmlstr)
{
    FILE *fps = NULL;
    mxml_node_t *simtree = NULL;
    int ret = PLUGIN_FAILURE;

    simtree = (mxml_node_t*)xmlstr;
    if (simtree) {
	fps = fopen(SIMULATED_RULE_FILE_PATH, "w");
	if (!fps) {
	    APP_LOG("REMOTEACCESS", LOG_ERR,"file error write");
	    ret = PLUGIN_FAILURE;
	    return ret;
	}
	ret = mxmlSaveFile(simtree, fps, MXML_NO_CALLBACK);
	fclose(fps);
    }else {
	ret = PLUGIN_FAILURE;
    }
    return ret;
}
int SetSimulatedRuleData(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *list_node = NULL, *count_node = NULL, *find_node = NULL;
    char statusResp[MAX_FILE_LINE];
    int ret = PLUGIN_FAILURE;
    char *tempTree = NULL;
    mxml_node_t *xml=NULL;
    mxml_node_t *device_node=NULL;
    mxml_node_t *top_node=NULL,*add_node=NULL, *top_new_node=NULL;
    mxml_index_t *node_index=NULL;
    mxml_node_t *first_node=NULL, *chNode=NULL;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_SETSIMULATEDRULEDATA,transaction_id,retVal);

    APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree)
    {
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque))
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    
	    //Create xml response
	    memset(statusResp, 0x0, MAX_FILE_LINE);
	    snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	    retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_SETSIMULATEDRULEDATA);
	    if(PLUGIN_FAILURE == retVal)
		    goto handler_exit1;

	    count_node= mxmlFindElement(tree, tree, "DeviceCount", NULL, NULL, MXML_DESCEND);
	    if ((count_node) && (count_node->child) && (count_node->child->value.opaque)) {
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Device Count value is %s", count_node->child->value.opaque);
		LockSimulatedOccupancy();
		gSimulatedDeviceCount = atoi(count_node->child->value.opaque);
		UnlockSimulatedOccupancy();
		APP_LOG("UPNPDevice", LOG_DEBUG, "Simulated Rule Data: gSimulatedDeviceCount: %d", gSimulatedDeviceCount);
		mxmlDelete(count_node);
		count_node= NULL;
	    }else{
		APP_LOG("REMOTEACCESS", LOG_ERR, "Device Count tag not present in xml payload");
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	    }

	    list_node = mxmlFindElement(tree, tree, "DeviceList", NULL, NULL, MXML_DESCEND);
	    if ((list_node) && (list_node->child) && (list_node->child->value.opaque))
	    {
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Device List value is %s", list_node->child->value.opaque);
		xml = mxmlNewXML("1.0");
		top_node = mxmlFindElement(tree, tree, "SimulatedRuleData", NULL, NULL, MXML_DESCEND);
		node_index = mxmlIndexNew(top_node,"Device", NULL);
		if (!node_index)
		{
		    APP_LOG("RULE", LOG_DEBUG, "node index error");
		    retVal = PLUGIN_FAILURE;
		    goto handler_exit1;
		}

		first_node = mxmlIndexReset(node_index);
		if (!first_node)
		{
		    APP_LOG("RULE", LOG_DEBUG, "first node error");
		    retVal = PLUGIN_FAILURE;
		    goto handler_exit1;
		}

		if(xml)
		{
		    top_new_node = mxmlNewElement(xml, "SimulatedRuleData");
		    if (top_new_node)
		    {
			while (first_node != NULL)
			{
			    APP_LOG("RULE", LOG_DEBUG, "first node");
			    first_node = mxmlIndexFind(node_index,"Device", NULL);
			    if (first_node)
			    {
				APP_LOG("RULE", LOG_DEBUG, "first child node");
				device_node = mxmlNewElement(top_new_node, "Device");
				if(device_node)
				{
				    chNode = mxmlFindElement(first_node, tree, "UDN", NULL, NULL, MXML_DESCEND);
				    if (chNode && chNode->child)
				    {
					APP_LOG("RULE", LOG_DEBUG,"The node %s with value is %s\n", chNode->value.element.name, chNode->child->value.opaque);
					add_node = mxmlNewElement(device_node, "UDN");
					mxmlNewOpaque(add_node, chNode->child->value.opaque);
				    }

				    chNode = mxmlFindElement(first_node, tree, "index", NULL, NULL, MXML_DESCEND);
				    if (chNode && chNode->child)
				    {
					APP_LOG("RULE", LOG_DEBUG,"The node %s with value is %s\n", chNode->value.element.name, chNode->child->value.opaque);
					add_node = mxmlNewElement(device_node, "index");
					mxmlNewOpaque(add_node, chNode->child->value.opaque);
				    }
				}
				else
				{
				    retVal = PLUGIN_FAILURE;
				    goto handler_exit1;
				}
			    }
			}   //end of while
		    }
		    else
		    {
			retVal = PLUGIN_FAILURE;
			goto handler_exit1;
		    }

		    tempTree = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG,"!SimulatedDevInfo alloc string is %s\n", tempTree);
		    free(tempTree);
		    tempTree = NULL;
		    ret = saveSimDevXml(xml);
		    if (ret == PLUGIN_SUCCESS) {
			retVal = PLUGIN_SUCCESS;
		    }else{
			retVal = PLUGIN_FAILURE;
			goto handler_exit1;
		    }
		}
		else
		{
		    retVal = PLUGIN_FAILURE;
		    goto handler_exit1;
		}	

		mxmlDelete(list_node);
		list_node= NULL;
	    }else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Device list tag not present in xml payload");
		LockSimulatedOccupancy();
		gSimulatedDeviceCount = 0; 
		UnlockSimulatedOccupancy();
		APP_LOG("UPNPDevice", LOG_DEBUG, "unset gSimulatedDeviceCount: %d", gSimulatedDeviceCount);
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	    }
	}
	else
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    //Create xml response
	    memset(statusResp, 0x0, MAX_FILE_LINE);
	    snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	    retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_SETSIMULATEDRULEDATA);
	    retVal = PLUGIN_FAILURE;
	}

    }
    else
    {
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	//Create xml response
	memset(statusResp, 0x0, MAX_FILE_LINE);
	snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_SETSIMULATEDRULEDATA);
	retVal = PLUGIN_FAILURE;
    }

handler_exit1:
    if(list_node) mxmlDelete(list_node);
    if(count_node) mxmlDelete(count_node);
    if(find_node) mxmlDelete(find_node);
    if (xml) mxmlDelete(xml);
    if(tree) mxmlDelete(tree);
    return retVal;
}

#endif
#ifdef WeMo_SMART_SETUP_V2
int remoteSetCustomizedState(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;
    char statusResp[MAX_FILE_LINE];
    int ret = PLUGIN_FAILURE;
    char *tempTree = NULL;
    mxml_node_t *xml=NULL;
    mxml_node_t *device_node=NULL;
    mxml_node_t *top_node=NULL,*add_node=NULL, *top_new_node=NULL;
    mxml_index_t *node_index=NULL;
    mxml_node_t *first_node=NULL, *chNode=NULL;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_RESETCUSTOMIZEDSTATE,transaction_id,retVal);

    APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree)
    {
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque))
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);

	    if (!strcmp(find_node->child->value.opaque, g_szWiFiMacAddress))
	    {
		setCustomizedState(DEVICE_CUSTOMIZED_NOTIFY);
		sendCustomizedStateNotification();

		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><customizedState>1</customizedState></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
	    }
	    else
	    {
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "macAddress not matching");
		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	else
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    //Create xml response
	    memset(statusResp, 0x0, MAX_FILE_LINE);
	    snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	    retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
	    retVal = PLUGIN_FAILURE;
	}

    }
    else
    {
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	//Create xml response
	memset(statusResp, 0x0, MAX_FILE_LINE);
	snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
	retVal = PLUGIN_FAILURE;
    }

handler_exit1:
    if(find_node) mxmlDelete(find_node);
    if (xml) mxmlDelete(xml);
    if(tree) mxmlDelete(tree);
    return retVal;
}
#endif
#ifdef WeMo_SMART_SETUP_V2
int remoteSetCustomizedState(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;
    char statusResp[MAX_FILE_LINE];
    int ret = PLUGIN_FAILURE;
    char *tempTree = NULL;
    mxml_node_t *xml=NULL;
    mxml_node_t *device_node=NULL;
    mxml_node_t *top_node=NULL,*add_node=NULL, *top_new_node=NULL;
    mxml_index_t *node_index=NULL;
    mxml_node_t *first_node=NULL, *chNode=NULL;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_RESETCUSTOMIZEDSTATE,transaction_id,retVal);

    APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree)
    {
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque))
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);

	    if (!strcmp(find_node->child->value.opaque, g_szWiFiMacAddress))
	    {
		setCustomizedState(DEVICE_CUSTOMIZED_NOTIFY);
		sendCustomizedStateNotification();

		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><customizedState>1</customizedState></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
	    }
	    else
	    {
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "macAddress not matching");
		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	else
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    //Create xml response
	    memset(statusResp, 0x0, MAX_FILE_LINE);
	    snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	    retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
	    retVal = PLUGIN_FAILURE;
	}

    }
    else
    {
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	//Create xml response
	memset(statusResp, 0x0, MAX_FILE_LINE);
	snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETCUSTOMIZEDSTATE);
	retVal = PLUGIN_FAILURE;
    }

handler_exit1:
    if(find_node) mxmlDelete(find_node);
    if (xml) mxmlDelete(xml);
    if(tree) mxmlDelete(tree);
    return retVal;
}
#endif

#ifdef PRODUCT_WeMo_Light
int SetNightLightStatusValue(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL, *status_node = NULL;
    char statusResp[MAX_FILE_LINE];

    if (!xmlBuf)
    {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_SETNIGHTLIGHTSTATUSVALUE,transaction_id,retVal);

    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree)
    {
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque))
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp(find_node->child->value.opaque, g_szWiFiMacAddress))
	    {
		//Set LS night light status value as requested
		status_node = mxmlFindElement(tree, tree, "status", NULL, NULL, MXML_DESCEND);
		if((status_node) && (status_node->child))
		{
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Status of plug-in set in XML is %s", status_node->child->value.opaque);
		    CloudSetNightLightStatus(status_node->child->value.opaque);
		    mxmlDelete(status_node);
		}
		else
		{
		    APP_LOG("REMOTEACCESS", LOG_ERR, "status tag not present in xml payload");
		    retVal = PLUGIN_FAILURE;
		    mxmlDelete(find_node);
		    mxmlDelete(tree);
		    goto handler_exit1;
		}
		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp, sizeof(statusResp),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_SETNIGHTLIGHTSTATUSVALUE);
	    }
	    else
	    {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received doesn't match with plugin Mac %s", g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	else
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	mxmlDelete(tree);
    }
    else
    {
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    return retVal;
}

int GetNightLightStatusValue(void *hndl, char *xmlBuf, unsigned int buflen, void* data_sock , void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;
    char statusResp[MAX_DATA_LEN];
    int state=-1;

    if (!xmlBuf)
    {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_GETNIGHTLIGHTSTATUSVALUE,transaction_id,retVal);

    APP_LOG("REMOTEACCESS", LOG_DEBUG, "transaction_id ===%s", transaction_id);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree)
    {
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque))
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp(find_node->child->value.opaque, g_szWiFiMacAddress))
	    {
		char *dimVal = GetBelkinParameter (DIMVALUE);
		state = atoi(dimVal);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Current DimValue in Flash: %d", state);
		//Create xml response
		memset(statusResp, 0x0, MAX_DATA_LEN);
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status></plugin>\n",  strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, state);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		//Send this response towards cloud synchronously using same data socket
		retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_GETNIGHTLIGHTSTATUSVALUE);
	    }
	    else
	    {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received doesn't match with plugin Mac %s", g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	else
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	mxmlDelete(tree);
    }
    else
    {
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    return retVal;
}

#endif

int SetDevStatusHandle(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    pluginDeviceStatus *pdevStatus = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL, *status_node = NULL;
    char statusResp[MAX_FILE_LINE];
#ifdef PRODUCT_WeMo_CrockPot
    int eventDuration=0,status,ret,waitCnt=10;
    int tempVal=0,tempTime,tempStatus;
#endif

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
		CHECK_PREV_TSX_ID(E_PLUGINSETDEVICESTATUS,transaction_id,retVal);

    pdevStatus = (pluginDeviceStatus*)calloc(1, sizeof(pluginDeviceStatus));
    if (!pdevStatus) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    strcpy(pdevStatus->macAddress, (find_node->child->value.opaque));
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp(pdevStatus->macAddress, g_szWiFiMacAddress)) {
#ifdef PRODUCT_WeMo_CrockPot
		//Set plugin device status as requested
		status_node = mxmlFindElement(tree, tree, "status", NULL, NULL, MXML_DESCEND);
		if ((status_node) && (status_node->child)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Status of plug-in set in XML is %s", status_node->child->value.opaque);
		    status = atoi(status_node->child->value.opaque);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Status of plug-in to set is %d", status);					
		    mxmlDelete(status_node);
		}
		else {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "status tag not present in xml payload");
		    retVal = PLUGIN_FAILURE;
		    mxmlDelete(find_node);
		    mxmlDelete(tree);
		    goto handler_exit1;
		}
		//Set plugin device duration as requested
		status_node = mxmlFindElement(tree, tree, "eventDuration", NULL, NULL, MXML_DESCEND);
		if ((status_node) && (status_node->child)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "eventDuration of plug-in set in XML is %s", status_node->child->value.opaque);
		    eventDuration = atoi(status_node->child->value.opaque);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "eventDuration of plug-in to set is %d", eventDuration);					
		    mxmlDelete(status_node);
		}
		else {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "eventDuration tag not present in xml payload. Setting to default=0");
		    eventDuration = 0;
		}

		CloudSetCrockPotStatus(status, eventDuration);

		pdevStatus->pluginId = strtol(g_szPluginCloudId, NULL, 0);
		strcpy(pdevStatus->macAddress, g_szWiFiMacAddress);
		strcpy(pdevStatus->friendlyName, g_szFriendlyName);

		/**
		 * Wait for max 1 sec to complete the set operation by the crockpotGPIOThread 
		 */
		waitCnt = 30;
		while(waitCnt) {
		    if(0 == g_remoteSetStatus) {
			if(eventDuration < 0) {
			    usleep(1000*1000);
			}
			else {
			    usleep(11*1000*1000);
			}
			break;
		    }
		    usleep(100*1000); /* 100 msec once? */
		    waitCnt--;
		}
		ret = GetCrockpotStatus(&(pdevStatus->status),&eventDuration);
		pdevStatus->statusTS = getPluginStatusTS();

#else /*PRODUCT_WeMo_CrockPot*/		
		//Set plugin device status as requested
		status_node = mxmlFindElement(tree, tree, "status", NULL, NULL, MXML_DESCEND);
		if ((status_node) && (status_node->child)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Status of plug-in set in XML is %s", status_node->child->value.opaque);
		    int status = 	atoi(status_node->child->value.opaque);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Status of plug-in to set is %d", status);
		    CloudSetBinaryState(status);
#ifdef PRODUCT_WeMo_Insight
		    if(status == 1)
			g_PowerStatus = POWER_SBY;
#endif
		    mxmlDelete(status_node);
		}else {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "status tag not present in xml payload");
		    retVal = PLUGIN_FAILURE;
		    mxmlDelete(find_node);
		    mxmlDelete(tree);
		    goto handler_exit1;
		}
		//Get plug-in details as well as device status
		//use gemtek apis to get plugin details
		pdevStatus->pluginId = strtol(g_szPluginCloudId, NULL, 0);
		strcpy(pdevStatus->macAddress, g_szWiFiMacAddress);
		strcpy(pdevStatus->friendlyName, g_szFriendlyName);
		//use gpio interfaces for device status
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Current Binary state of plug-in %d", GetCurBinaryState());
		pdevStatus->status = CloudGetBinaryState();
		pdevStatus->statusTS = getPluginStatusTS();
#endif /*PRODUCT_WeMo_CrockPot*/
		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
#ifdef PRODUCT_WeMo_CrockPot
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><statusTS>%d</statusTS><eventDuration>%d</eventDuration></plugin>\n", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,pdevStatus->statusTS,eventDuration);
#else /*#ifdef PRODUCT_WeMo_CrockPot*/
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName><statusTS>%d</statusTS></plugin>\n", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName,pdevStatus->statusTS);
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_PLUGINSETDEVICESTATUS);
	    } else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", pdevStatus->macAddress, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (pdevStatus) {free(pdevStatus); pdevStatus = NULL;}
    return retVal;
}

int UpgradeFwVersion(void *hndl, char *xmlBuf, unsigned int buflen,void *data_sock, void* remoteInfo,char* transaction_id ) 
{
    int retVal = PLUGIN_SUCCESS, ret = PLUGIN_SUCCESS;
    pluginUpgradeFwStatus *pUpdFwStatus = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL, *signature_node = NULL, *fwDwldUrl_node = NULL, *dwldStartTime_node = NULL;
    char statusResp[MAX_FILE_LINE];
    int dwldStartTime = 0, fwUnSignMode = 0;
    char FirmwareURL[MAX_FW_URL_LEN];

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_UPGRADEFWVERSION,transaction_id,retVal);
    pUpdFwStatus = (pluginUpgradeFwStatus*)calloc(1, sizeof(pluginUpgradeFwStatus));
    if (!pUpdFwStatus) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    strcpy(pUpdFwStatus->macAddress, (find_node->child->value.opaque));
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp(pUpdFwStatus->macAddress, g_szWiFiMacAddress)) {

		//Get FW upgrade url signature
		signature_node = mxmlFindElement(tree, tree, "signature", NULL, NULL, MXML_DESCEND);
		if ((signature_node) && (signature_node->child)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Signature of plug-in upgrade firmware req in XML is %s", signature_node->child->value.opaque);
		    memset(g_szFirmwareURLSignature, 0x0, sizeof(g_szFirmwareURLSignature));
		    strcpy(g_szFirmwareURLSignature, (signature_node->child->value.opaque));
		    APP_LOG("REMOTEACCESS", LOG_HIDE, "Signature of plug-in upgrade firmware req is %s", g_szFirmwareURLSignature);
		    mxmlDelete(signature_node);
		}

		//Get FW upgrade download Start Time 
		dwldStartTime_node = mxmlFindElement(tree, tree, "downloadStartTime", NULL, NULL, MXML_DESCEND);
		if ((dwldStartTime_node) && (dwldStartTime_node->child)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "download Start Time of plug-in upgrade firmware req in XML is %s", dwldStartTime_node->child->value.opaque);
		    dwldStartTime = atoi(dwldStartTime_node->child->value.opaque);
		    dwldStartTime = dwldStartTime*60; //Seconds
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "download Start Time of plug-in upgrade firmware req is %d", dwldStartTime);
		    mxmlDelete(dwldStartTime_node);
		}

		//Get FW upgrade url
		fwDwldUrl_node = mxmlFindElement(tree, tree, "firmwareDownloadUrl", NULL, NULL, MXML_DESCEND);
		if ((fwDwldUrl_node) && (fwDwldUrl_node->child) && (fwDwldUrl_node->child->value.opaque)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "URL of plug-in upgrade firmware req in XML is %s", fwDwldUrl_node->child->value.opaque);
		    memset(FirmwareURL, 0x0, sizeof(FirmwareURL));
		    strcpy(FirmwareURL, (fwDwldUrl_node->child->value.opaque));
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "URL of plug-in upgrade firmware req is %s", FirmwareURL);
		    mxmlDelete(fwDwldUrl_node);
		}else {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "firmwareDownloadUrl Complete Information  not present in xml payload");
		    retVal = PLUGIN_FAILURE;
		    mxmlDelete(find_node);
		    mxmlDelete(tree);
		    goto handler_exit1;
		}
#if 0	
		//Get FW upgrade url
    mxml_node_t *fwUnSignMode_node = NULL;
		fwUnSignMode_node = mxmlFindElement(tree, tree, "WithUnsignedImage", NULL, NULL, MXML_DESCEND);
		if ((fwUnSignMode_node) && (fwUnSignMode_node->child) && (fwUnSignMode_node->child->value.opaque)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Sign Mode of plug-in upgrade firmware req in XML is %s", fwUnSignMode_node->child->value.opaque);
				fwUnSignMode = atoi(fwUnSignMode_node->child->value.opaque);
		    mxmlDelete(fwDwldUrl_node);
		} else {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Sign Mode of plug-in upgrade firmware req in XML is not present");
		}
#endif
		fwUnSignMode = PLUGIN_SUCCESS;
		/* Start upgrade firmware as requested, Call firmware update cloudIF routine */
		ret = CloudUpgradeFwVersion(dwldStartTime, FirmwareURL, fwUnSignMode);
		if(retVal < PLUGIN_SUCCESS)
		{
		    APP_LOG("REMOTEACCESS", LOG_ERR, "FirmwareUpdateStart thread create failed...");
		    retVal = PLUGIN_FAILURE;
		    mxmlDelete(find_node);
		    mxmlDelete(tree);
		    goto handler_exit1;
		}

		//Get plug-in details as well as firmware upgrade status and version
		pUpdFwStatus->pluginId = strtol(g_szPluginCloudId, NULL, 0);
		strcpy(pUpdFwStatus->macAddress, g_szWiFiMacAddress);
		pUpdFwStatus->fwUpgradeStatus = getCurrFWUpdateState(); 
		strcpy(pUpdFwStatus->fwVersion, g_szFirmwareVersion);

		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><fwUpgradeStatus>%d</fwUpgradeStatus><firmwareVersion>%s</firmwareVersion></plugin>\n", pUpdFwStatus->pluginId, pUpdFwStatus->macAddress, pUpdFwStatus->fwUpgradeStatus, pUpdFwStatus->fwVersion);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in Firmware upgrade status response created is %s", statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_UPGRADEFWVERSION);
	    } else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", pUpdFwStatus->macAddress, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (pUpdFwStatus) {free(pUpdFwStatus); pUpdFwStatus = NULL;}
    return retVal;
}

//Create and edit rules interface
int RemoteUpdateWeeklyCalendar(void *hndl, mxml_node_t *tree, void *data_sock, void* remoteInfo,char* transaction_id)
{
    char* szWeekList[WEEK_DAYS_NO] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int loop = 0x00;
    int resp = 0x00;
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *find_node = NULL;
    char resp_data[MAX_RESP_LEN];

    if (tree){
	for (loop = 0x00; loop < WEEK_DAYS_NO; loop++)
	{	
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "processing rule list for day:%s", szWeekDayShortname[loop]);
	    find_node = mxmlFindElement(tree, tree, szWeekDayShortname[loop], NULL, NULL, MXML_DESCEND);
	    if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "weekday value is %s", find_node->child->value.opaque);
		szWeekList[loop]=strdup(find_node->child->value.opaque);
		mxmlDelete(find_node);
		find_node = NULL;
	    }
	}
    }	//if tree
    resp = CloudUpdateRule((char**)szWeekList);
    APP_LOG("REMOTEACCESS", LOG_DEBUG, "update rule resp=%d", resp);
    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);

		memset(resp_data,0,sizeof(resp_data));
    if (0x00 == resp)
    {
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "update rule resp=%d", resp);
	snprintf(resp_data, sizeof(resp_data),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	retVal = PLUGIN_SUCCESS;
    }
    else
    {
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "update rule resp=%d", resp);
	snprintf(resp_data, sizeof(resp_data),"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
	retVal = PLUGIN_FAILURE;
    }
	retVal=SendNatPkt(hndl,resp_data,remoteInfo,transaction_id,data_sock,E_UPDATEWEEKLYCALENDAR);
loop = 0x00;
    for (loop = 0x00; loop < WEEK_DAYS_NO; loop++) {
	if (szWeekList[loop]) {free(szWeekList[loop]); szWeekList[loop]=NULL;}
    }
    return retVal;
}

#define MAX_BBUF_LEN	1025
#define MAX_BBUF1_LEN	1050
#define BIN_START_LEN 12
#define BIN_END_LEN 10

int GetRulesHandle(void *hndl, char *xmlBuf, unsigned int buflen,void*data_sock,  void* remoteInfo,char* transaction_id) 
{

    int retVal = PLUGIN_SUCCESS;
    int dsock = PLUGIN_SUCCESS, i = PLUGIN_SUCCESS, len=PLUGIN_SUCCESS, nBlocks, lastChunk;
    unsigned long fileLen=PLUGIN_SUCCESS;
    char buffer[MAX_BBUF_LEN], buffer1[MAX_BBUF1_LEN];
    dsock = *(int *)data_sock;


    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    CHECK_PREV_TSX_ID(E_PLUGINGETDBFILE,transaction_id,retVal);
    /*Opening file for reading in binary mode*/
    FILE *handleRead= NULL;

    handleRead = fopen(RULE_DB_PATH, "rb");
    if(handleRead == NULL) {
	APP_LOG("REMOTEACCESS", LOG_ERR, "failed to open rules file for reading");
	return PLUGIN_FAILURE;
    }
    //Get file length
    fseek(handleRead, 0, SEEK_END);
    fileLen=ftell(handleRead);
    fseek(handleRead, 0, SEEK_SET);

    /* sending first chunk of pkt append "binstartEnd" */
    memset(buffer1, 0, MAX_BBUF1_LEN);
    memset(buffer, 0, MAX_BBUF_LEN);
    strcpy(buffer, "binStartData");
    APP_LOG("REMOTEACCESS", LOG_ERR, "strlen buffer =%d ", strlen(buffer));
    retVal = send(dsock, buffer, BIN_START_LEN, 0);
    if (retVal < PLUGIN_SUCCESS) {
	APP_LOG("REMOTEACCESS", LOG_ERR, "getDB response data sock send is failed %d", retVal);
	retVal = PLUGIN_FAILURE;
    }else {
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "getDB status response data sock total bytes send is %d", retVal);
	retVal = PLUGIN_SUCCESS;
    }
    /* sending remaining chunks of pkt */
    nBlocks = (fileLen)/MAX_BUF_LEN;
    lastChunk = (fileLen)%MAX_BUF_LEN;
    for(i=1; i<=nBlocks; i++)
    {
	memset(buffer, 0, MAX_BUF_LEN);
	len = fread(buffer, 1, MAX_BUF_LEN, handleRead);
	APP_LOG("REMOTEACCESS", LOG_ERR, "read %d bytes of data from file", len);
	retVal = send(dsock, buffer, len, 0);
	if (retVal < PLUGIN_SUCCESS) {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "getDB response data sock send is failed %d", retVal);
	    retVal = PLUGIN_FAILURE;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "getDB status response data sock total bytes send is %d", retVal);
if(gIceRunning)
		memcpy(gIcePrevTransactionId[E_PLUGINGETDBFILE],transaction_id,TRANSACTION_ID_LEN);
retVal = PLUGIN_SUCCESS;
	}		 
    }
    /* sending last chunk of pkt */
    if(lastChunk > 1)
    {
	memset(buffer,0,MAX_BUF_LEN);
	len = fread(buffer, 1, lastChunk, handleRead);
	APP_LOG("REMOTEACCESS", LOG_ERR, "read last chunk of %d bytes of data from file", len);
	retVal = send(dsock, buffer, lastChunk, 0);
	if (retVal < PLUGIN_SUCCESS) {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "getDB response data sock send is failed %d", retVal);
	    retVal = PLUGIN_FAILURE;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "getDB status response data sock total bytes send is %d", retVal);
	    retVal = PLUGIN_SUCCESS;
	}
    }

    memset(buffer1, 0, MAX_BBUF1_LEN);
    strcpy(buffer1, "binEndData");
    retVal = send(dsock, buffer1, BIN_END_LEN, 0);
    if (retVal < PLUGIN_SUCCESS) {
	APP_LOG("REMOTEACCESS", LOG_ERR, "getDB response data sock send is failed %d", retVal);
	retVal = PLUGIN_FAILURE;
    }else {
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "getDB status response data sock total bytes send is %d", retVal);
	retVal = PLUGIN_SUCCESS;
    }
    /*Closing File*/
    fclose(handleRead);
handler_exit1:
    return retVal;

}

int GetLogHandle(void *hndl, char *xmlBuf, unsigned int buflen,void *data_sock,  void* remoteInfo,char* transaction_id) 
{
    int retVal = PLUGIN_SUCCESS;
    pluginDeviceStatus *pdevStatus = NULL;
    mxml_node_t *tree = NULL;
    char statusResp[MAX_FILE_LINE];

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	return retVal;
    }
    CHECK_PREV_TSX_ID(E_WDLOGFILE,transaction_id,retVal);
    pdevStatus = (pluginDeviceStatus*)calloc(1, sizeof(pluginDeviceStatus));
    if (!pdevStatus) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }

    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);

	strcpy(pdevStatus->macAddress, g_szWiFiMacAddress);
	strcpy(pdevStatus->friendlyName, g_szFriendlyName);
	pdevStatus->status = 1;

	//Create xml response
	memset(statusResp, 0x0, MAX_FILE_LINE);

	snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><pluginId>1</pluginId><macAddress>%s</macAddress><status>%d</status><friendlyName>%s</friendlyName></plugin>\n", pdevStatus->macAddress, pdevStatus->status, pdevStatus->friendlyName);
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_WDLOGFILE);
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (pdevStatus) {free(pdevStatus); pdevStatus = NULL;}
    return retVal;
}

int SetRulesHandle(void *hndl, char *xmlBuf, unsigned int buflen, void*data_sock, void* remoteInfo,char* transaction_id) 
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	return retVal;
    }
    CHECK_PREV_TSX_ID(E_UPDATEWEEKLYCALENDAR,transaction_id,retVal);
    APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	find_node = mxmlFindElement(tree, tree, "updateweeklycalendar", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "updateweeklycalendar loaded is %s", find_node->child->value.opaque);
	    retVal = RemoteUpdateWeeklyCalendar(hndl, find_node,data_sock, remoteInfo, transaction_id);
	    if(retVal != PLUGIN_SUCCESS)
		APP_LOG("REMOTEACCESS", LOG_ERR, "set rule failed");
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	mxmlDelete(tree);
    }
handler_exit1:
    return retVal;
}

int RemoteSetRulesDBVersion(void *hndl, char *xmlBuf, unsigned int buflen, void*data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree;
    mxml_node_t *find_node, *status_node;
    char *dVer = NULL, *dbVer = NULL;
    char statusResp[MAX_FILE_LINE];

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
		CHECK_PREV_TSX_ID(E_SETDBVERSION,transaction_id,retVal);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	dVer = (char*)calloc(1, MAX_RVAL_LEN);
	if (!dVer) {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "dVer malloc failed");
	    retVal = PLUGIN_FAILURE;
	    mxmlDelete(tree);
	    goto handler_exit1;
	}
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp((find_node->child->value.opaque), g_szWiFiMacAddress)) {
		status_node = mxmlFindElement(tree, tree, "dbVersion", NULL, NULL, MXML_DESCEND);
		if ((status_node) && (status_node->child)) {
		    if (status_node->child->value.opaque) {
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "dbVersion of plug-in set in XML is %s", status_node->child->value.opaque);
			memset(dVer, 0x0, MAX_RVAL_LEN);
			strcpy(dVer, status_node->child->value.opaque);
			//Save Rules db version Asynchronously.
			SetBelkinParameter (RULE_DB_VERSION_KEY, dVer);
			AsyncSaveData ();
			mxmlDelete(status_node);
			status_node = NULL;
		    }else {
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "dbVersion of plug-in value not set in XML");
			if (status_node) {mxmlDelete(status_node); status_node = NULL;}
		    }
		}else {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "dbVersion tag not present in xml payload");
		    if (status_node) {mxmlDelete(status_node); status_node = NULL;}
		    mxmlDelete(tree);
		    retVal = PLUGIN_FAILURE;
		    goto handler_exit1;
		}
		//Create xml response
		memset(dVer, 0x0, MAX_RVAL_LEN);
		dbVer = CloudGetRuleDBVersion(dVer);
		memset(statusResp, 0x0, MAX_FILE_LINE);
		if (dbVer) {
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><dbVersion>%s</dbVersion></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName, dbVer);
		}else{
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev set rule db version response created is %s", statusResp);
		//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_SETDBVERSION);
	    } else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    if (find_node) {
		mxmlDelete(find_node);
		find_node = NULL;
	    }
	    retVal = PLUGIN_FAILURE;
	}
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (dVer) {free(dVer); dVer = NULL;}
    return retVal;
}

int RemoteGetRulesDBVersion(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree;
    mxml_node_t *find_node;
    char statusResp[MAX_DATA_LEN];
    char *dVer = NULL, *dbVer = NULL;
    char *override = NULL, *ovrd = NULL;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
		CHECK_PREV_TSX_ID(E_GETDBVERSION,transaction_id,retVal);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	dVer = (char*)calloc(1, MAX_RVAL_LEN);
	if (!dVer) {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "dVer malloc failed");
	    retVal = PLUGIN_FAILURE;
	    mxmlDelete(tree);
	    goto handler_exit1;
	}
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp((find_node->child->value.opaque), g_szWiFiMacAddress)) {
		//Create xml response
		memset(dVer, 0x0, MAX_RVAL_LEN);
		dbVer = CloudGetRuleDBVersion(dVer);
		memset(statusResp, 0x0, MAX_DATA_LEN);
		override = (char*)calloc(1, MAX_FILE_LINE*2);
		ovrd = GetOverriddenStatus(override); 
		if (dbVer && ovrd) {
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><dbVersion>%s</dbVersion><rOverriden>%s</rOverriden></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName, dbVer, ovrd);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev get rule db version response created is %s", statusResp);
		}else if ((!(dbVer)) && (ovrd)){
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden>%s</rOverriden></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName, ovrd);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev get rule db version response created is %s", statusResp);
		}else if (dbVer && (!(ovrd))) {
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><dbVersion>%s</dbVersion><rOverriden></rOverriden></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName, dbVer);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev get rule db version response created is %s", statusResp);
		}else {
		    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>0</status><friendlyName>%s</friendlyName><dbVersion>-1</dbVersion><rOverriden></rOverriden></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev get rule db version response created is %s", statusResp);
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev get rule db version response created is %s", statusResp);
		//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_GETDBVERSION);
	    } else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (dVer) {free(dVer);dVer = NULL;}
    if (ovrd) {free(ovrd);ovrd = NULL;}
    return retVal;
}

int RemoteAccessDevConfig(void *hndl, char *xmlBuf, unsigned int buflen,void *data_sock, void* remoteInfo,char* transaction_id ) 
{
	int retVal = PLUGIN_SUCCESS;
	mxml_node_t *tree = NULL;
	mxml_node_t *find_node = NULL;
	char statusResp[MAX_FILE_LINE];
	char *sMac = NULL;

	if (!xmlBuf) 
	{
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	}
	CHECK_PREV_TSX_ID(E_DEVCONFIG,transaction_id,retVal);
	tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
	if (tree){
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
		find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
		if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
			//Get mac address of the device
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", GetMACAddress());
			sMac = utilsRemDelimitStr(GetMACAddress(), ":");
			if (!strcmp(find_node->child->value.opaque, sMac)) 
			{
				/* parse and enqueue command */
				DespatchSendNotificationResp((void *) xmlBuf);

				//Create xml response
				memset(statusResp, 0x0, MAX_FILE_LINE);
				snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>0</status></plugin>",sMac);
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in Firmware upgrade status response created is %s", statusResp);
				retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_DEVCONFIG);
			} else {
				APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, sMac);
				retVal = PLUGIN_FAILURE;
			}
			mxmlDelete(find_node);
			find_node = NULL;
		}else {
			APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
			retVal = PLUGIN_FAILURE;
		}
		if (find_node) {mxmlDelete(find_node); find_node = NULL;}
		mxmlDelete(tree);
	}else{
  	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
		retVal = PLUGIN_FAILURE;
	}
handler_exit1:
	if (sMac) {free(sMac); sMac = NULL;}
	return retVal;
}




int remoteAccessChangeFriendlyName(void* hndl,char *xmlBuf, unsigned int buflen, void *data_sock,void*remoteInfo,char*transaction_id)
{
	int retVal = PLUGIN_SUCCESS;
	mxml_node_t *tree;
	mxml_node_t *find_node, *status_node;
	char statusResp[SIZE_256B];

	if (!xmlBuf) {
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	}
		CHECK_PREV_TSX_ID(E_CHANGEFRIENDLYNAME,transaction_id,retVal);//TBD
	tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
	if (tree){
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
		find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
		if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
			//Get mac address of the device
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
			if (!strcmp((find_node->child->value.opaque), g_szWiFiMacAddress)) {
				status_node = mxmlFindElement(tree, tree, "friendlyName", NULL, NULL, MXML_DESCEND);
				if ((status_node) && (status_node->child)) {
					if (status_node->child->value.opaque) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "friendlyName of plug-in set in XML is %s", status_node->child->value.opaque);
						memset(g_szFriendlyName, 0x0, sizeof(g_szFriendlyName));
						strncpy(g_szFriendlyName, status_node->child->value.opaque, sizeof(g_szFriendlyName)-1);
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "friendlyName of plug-in set in  g_szFriendlyName: %s", g_szFriendlyName);
						SetBelkinParameter("FriendlyName", g_szFriendlyName);
						UpdateXML2Factory();
						AsyncSaveData();
						mxmlDelete(status_node);
					}else {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "friendlyName of plug-in value not set in XML");
					}
				}else {
					APP_LOG("REMOTEACCESS", LOG_ERR, "friendlyName tag not present in xml payload");
					retVal = PLUGIN_FAILURE;
					goto handler_exit1;
				}
				//Create xml response
				memset(statusResp, 0x0, SIZE_256B);

				snprintf(statusResp, sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);

				APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in friendlyName response created is %s", statusResp);
				//Send this response towards cloud synchronously using same data socket
				retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_CHANGEFRIENDLYNAME);//TBD
			} else {
				APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, g_szWiFiMacAddress);
				retVal = PLUGIN_FAILURE;
			}
			mxmlDelete(find_node);
		}else {
			APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
			retVal = PLUGIN_FAILURE;

		}
		mxmlDelete(tree);
	}else{
		APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
		retVal = PLUGIN_FAILURE;
	}
handler_exit1:
	remoteAccessUpdateStatusTSParams (0xFF);
	return retVal;
}

int remoteAccessResetNameRulesData( void * hndl,char *xmlBuf, unsigned int buflen, void *data_sock,void* remoteInfo,char*transaction_id) {
	int retVal = PLUGIN_SUCCESS;
	mxml_node_t *tree=NULL;
	mxml_node_t *find_node=NULL;
	char statusResp[SIZE_1024B];
	char macAddress[MAX_MAC_LEN];
	int ResetType=0x00;

	if (!xmlBuf) {
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	}
		CHECK_PREV_TSX_ID(E_RESETNAMERULESDATA,transaction_id,retVal);//TBD
	tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
	
if (tree){
  	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
		find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
		if (find_node) {
			strcpy(macAddress, (find_node->child->value.opaque));
			//Get mac address of the device
			if (!strcmp(macAddress, g_szWiFiMacAddress)) {
				//Get plug-in details as well as device status
				//use gemtek apis to get plugin details
				//Create xml response
			    find_node = mxmlFindElement(tree, tree, "resetType", NULL, NULL, MXML_DESCEND);
			    if (find_node) {
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "reset Type  value is %s", find_node->child->value.opaque);
				ResetType = atoi(find_node->child->value.opaque);
				resetFNGlobalsToDefaults(ResetType);
				ExecuteReset(ResetType);
#if defined(PRODUCT_WeMo_Insight)
				snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><iconVersion>0</iconVersion></plugin>", g_szWiFiMacAddress,DEFAULT_INSIGHT_FRIENDLY_NAME);
#else
				snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>1</status><friendlyName>%s</friendlyName><iconVersion>%d</iconVersion></plugin>", g_szWiFiMacAddress,g_szFriendlyName,gWebIconVersion);
#endif
				APP_LOG("REMOTEACCESS", LOG_ERR, "Sending ResetNameRulesData Response: %s ", statusResp);
				//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_RESETNAMERULESDATA);//TBD
				}else {
				    APP_LOG("REMOTEACCESS", LOG_ERR, "resetType tag not present in xml payload");
				    retVal = PLUGIN_FAILURE;
				}
			} else {
  			APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with Device Mac %s", macAddress, g_szWiFiMacAddress);
				retVal = PLUGIN_FAILURE;
			}
			mxmlDelete(find_node);
		}else {
			APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
			retVal = PLUGIN_FAILURE;
		}
		mxmlDelete(tree);
	}else{
  	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
		retVal = PLUGIN_FAILURE;
	}
handler_exit1:
	return retVal;
}

int remoteAccessGetIcon(void *hndl,char *xmlBuf, unsigned int buflen, void *data_sock,void *remoteInfo,char*transaction_id) {
	int retVal = PLUGIN_SUCCESS;
	mxml_node_t *tree=NULL;
	mxml_node_t *find_node=NULL;
	char statusResp[SIZE_1024B];
	char macAddress[MAX_MAC_LEN];
	char *iconVer;
	int WebIconVersion=0;
	char *fsFileUrl = NULL;
  char cookie[SIZE_512B];
	if (!xmlBuf) {
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	}
		CHECK_PREV_TSX_ID(E_GETICON,transaction_id,retVal);//TBD
	tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
	
if (tree){
  	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
		find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
		if (find_node) {
			strcpy(macAddress, (find_node->child->value.opaque));
			//Get mac address of the device
			if (!strcmp(macAddress, g_szWiFiMacAddress)) {
				//Get plug-in details as well as device status
				//use gemtek apis to get plugin details
				//Create xml response
			    find_node = mxmlFindElement(tree, tree, "uri", NULL, NULL, MXML_DESCEND);
			    if (find_node) {
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "icon URI value is %s", find_node->child->value.opaque);
				/* load icon version */
				iconVer = GetBelkinParameter(ICON_VERSION_KEY);
				if(iconVer && strlen(iconVer))
				{
				    WebIconVersion = atoi(iconVer);
				}

				#ifdef PRODUCT_WeMo_NetCam
				if (iconVer)
					free(iconVer);
				#endif
				
				fsFileUrl = (char *)malloc((sizeof(char)*256));
				if(fsFileUrl == NULL)
				{
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"fsFileUrl malloc failed\n");
						mxmlDelete(tree);
						goto handler_exit1;

				}

				
				retVal = UploadIcon(find_node->child->value.opaque,fsFileUrl,cookie);
				if(!retVal)
				{
				    sprintf(statusResp, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>1</status><iconVersion>%d</iconVersion><uri>%s</uri><cookie>%s</cookie></plugin>",g_szWiFiMacAddress,WebIconVersion,fsFileUrl,cookie);
				}else{
				    APP_LOG("REMOTEACCESS", LOG_ERR, "Icon Upload on URL: %s  failed", find_node->child->value.opaque);
				    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>0</status><iconVersion>%d</iconVersion><uri>%s<uri></plugin>",g_szWiFiMacAddress,WebIconVersion,fsFileUrl);
				}
				    APP_LOG("REMOTEACCESS", LOG_ERR, "Sending Icon Upload Response: %s ", statusResp);
				//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_GETICON);//TBD
				}else {
				    APP_LOG("REMOTEACCESS", LOG_ERR, "uri tag not present in xml payload");
				    retVal = PLUGIN_FAILURE;
				}
			} else {
  			APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with Device Mac %s", macAddress, g_szWiFiMacAddress);
				retVal = PLUGIN_FAILURE;
			}
			mxmlDelete(find_node);
		}else {
			APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
			retVal = PLUGIN_FAILURE;
		}
		mxmlDelete(tree);
	}else{
  	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
		retVal = PLUGIN_FAILURE;
	}
handler_exit1:
	if (fsFileUrl) {free(fsFileUrl); fsFileUrl = NULL;}
	return retVal;
}

#define ICON_VERSION_KEY    "IconVersion"
int remoteAccessSetIcon(void *hndl,char *xmlBuf, unsigned int buflen, void *data_sock,void *remoteInfo,char *transaction_id ) {
	int retVal = PLUGIN_SUCCESS;
	mxml_node_t *tree=NULL;
	mxml_node_t *find_node=NULL;
	char statusResp[SIZE_1024B];
	char macAddress[MAX_MAC_LEN];
	char buff[SIZE_64B];

	if (!xmlBuf) {
		retVal = PLUGIN_FAILURE;
		goto handler_exit1;
	}
		CHECK_PREV_TSX_ID(E_SETICON,transaction_id,retVal);//TBD
	tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
	
if (tree){
  	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
		find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
		if (find_node) {
			strcpy(macAddress, (find_node->child->value.opaque));
			//Get mac address of the device
			if (!strcmp(macAddress, g_szWiFiMacAddress)) {
				//Get plug-in details as well as device status
				//use gemtek apis to get plugin details
				//Create xml response
    char cookie[MAX_FILE_LINE];
				memset(cookie,0,sizeof(cookie));
				find_node = mxmlFindElement(tree, tree, "cookie", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						memset(cookie,0,sizeof(cookie));
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The cookie is %s\n", find_node->child->value.opaque);
						strcpy(cookie,find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
				}											
			    find_node = mxmlFindElement(tree, tree, "uri", NULL, NULL, MXML_DESCEND);
			    if (find_node && (find_node->child) && (find_node->child->value.opaque)) {
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "icon URI value is %s", find_node->child->value.opaque);
				retVal = DownloadIcon(find_node->child->value.opaque ,cookie);
				if(retVal == PLUGIN_SUCCESS)
				{
				    gWebIconVersion++;
				    memset(buff, 0x0, SIZE_64B);
				    snprintf(buff, SIZE_64B, "%d", gWebIconVersion);
				    SetBelkinParameter(ICON_VERSION_KEY, buff);
				    AsyncSaveData();
				    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>1</status><iconVersion>%d</iconVersion></plugin>",g_szWiFiMacAddress,gWebIconVersion);
				}else{
				    APP_LOG("REMOTEACCESS", LOG_ERR, "CURL Download failed");
				    snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>0</status><iconVersion>%d</iconVersion></plugin>",g_szWiFiMacAddress,gWebIconVersion);
				}
				//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_SETICON);//TBD
				}else {
				    APP_LOG("REMOTEACCESS", LOG_ERR, "uri tag not present in xml payload");
				    retVal = PLUGIN_FAILURE;
				}
			} else {
  			APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with Device Mac %s", macAddress, g_szWiFiMacAddress);
				retVal = PLUGIN_FAILURE;
			}
			mxmlDelete(find_node);
		}else {
			APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
			retVal = PLUGIN_FAILURE;
		}
		mxmlDelete(tree);
	}else{
  	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
		retVal = PLUGIN_FAILURE;
	}
handler_exit1:
	return retVal;
}

int icondl_progress(void *clientp,double dltotal,double dlnow,double ultotal,double ulnow)
{
    if (dlnow && dltotal)
	APP_LOG("DOWNLOAD ICON",LOG_DEBUG, "downloading Icon:%3.0f%%\r",100*dlnow/dltotal);
    fflush(stdout);    
    return CURLE_OK;
}
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    APP_LOG("DOWNLOAD ICON", LOG_DEBUG, "%d written to to icon.jpg",written);
    return written;
}

int DownloadIcon(char *url ,char* cookie)
{
		UserAppSessionData *pUsrAppSsnData = NULL; 
		UserAppData *pUsrAppData = NULL;
		authSign *assign = NULL;
		int status = 1;         
		int retVal = 0;         
		char *ptr = NULL;    
		FILE *fptr=NULL;				

		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered DownloadIcon API\n");
		//Resolve LB domain name to get SIP Server and other server IPs
		assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
		if (!assign) {          
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Signature Structure returned NULL\n");
				goto exit_below;                                
		}            
		pUsrAppData = (UserAppData *)malloc(sizeof(UserAppData));
		if (!pUsrAppData) {
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Malloc Structure returned NULL\n");
				goto exit_below;                                
		}
		memset( pUsrAppData, 0x0, sizeof(UserAppData));
		pUsrAppSsnData = webAppCreateSession(0);
		if (!pUsrAppSsnData) {
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Session Structure returned NULL\n");
				goto exit_below;                                
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Getting ICON   from Cloud  \n");
		/* prepare REST header*/
		{                       
				strcpy( pUsrAppData->url, url);             
				strcpy( pUsrAppData->keyVal[0].key, "Content-Type");
				strcpy( pUsrAppData->keyVal[0].value, "multipart/octet-stream");
				strcpy( pUsrAppData->keyVal[1].key, "Authorization");
				strcpy( pUsrAppData->keyVal[1].value,  assign->signature);
				strncpy( pUsrAppData->keyVal[2].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[2].key)-1);   
				strncpy( pUsrAppData->keyVal[2].value, g_szClientType, sizeof(pUsrAppData->keyVal[2].value)-1);   
				pUsrAppData->keyValLen = 3;
				if(cookie)
				{
						strcpy( pUsrAppData->keyVal[3].key, "cookie");
						strcpy( pUsrAppData->keyVal[3].value, cookie);
						pUsrAppData->keyValLen = 4;
				}
				/* enable SSL if auth URL is on HTTPS*/
				ptr = strstr(url,"https" );
				if(ptr != NULL)
						pUsrAppData->httpsFlag =  1;
				else
						pUsrAppData->httpsFlag =  0;
		}
		pUsrAppData->disableFlag = 1;
		pUsrAppData->inDataLength = 0;
		retVal = webAppSendData( pUsrAppSsnData, pUsrAppData, 0);
		if (retVal)
		{
				APP_LOG("REMOTEACCESS", LOG_ERR,"Some error encountered while sending to %s errorCode %d \n", url, retVal);
				goto exit_below;
		}
		/* check response header to see if user is authorized or not*/
		{
				ptr = strstr(pUsrAppData->outHeader,"200 OK" );
				if(ptr != NULL)
				{
						retVal = PLUGIN_SUCCESS;
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response 200 OK received from %s\n", url);
						char cmdbuf[MAX_LVALUE_LEN];
						memset(cmdbuf, 0x0, MAX_LVALUE_LEN);
						sprintf(cmdbuf, "rm -rf %s",ICON_FILE_URL);
						status = system(cmdbuf);
						if(status != PLUGIN_SUCCESS)
								APP_LOG("REMOTEACCESS", LOG_ERR, "failed to remove existing rule db file");

						fptr= fopen(ICON_FILE_URL, "ab");
						if(fptr == NULL) {
								APP_LOG("REMOTEACCESS", LOG_ERR, "failed to open rules file for writing");
								retVal=PLUGIN_FAILURE;
								goto exit_below;
						}

						fwrite(pUsrAppData->outData,1,pUsrAppData->outDataLength,fptr);
						fclose(fptr);
						fptr = NULL;

				}else {
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response other than 200 OK received from %s\n", url);
						retVal=PLUGIN_FAILURE;
						goto exit_below;
				}
		}
exit_below:
		if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
		if (pUsrAppData) { 
				if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
				free(pUsrAppData); pUsrAppData = NULL; 
		}
		if (assign) { free(assign); assign = NULL; }
		return retVal;
}

char *getNotificationStatus () {
    return g_NotificationStatus;
}

int setNotificationStatus (int state) {
    char szNotPrm[MAX_RES_LEN];

    memset (szNotPrm, 0, MAX_RES_LEN);
    snprintf(szNotPrm, sizeof(szNotPrm), "%d", state);

    SetBelkinParameter (NOTIFICATION_VALUE, szNotPrm);
    SaveSetting ();

    memset (g_NotificationStatus,0x0, sizeof(g_NotificationStatus));
    strncpy(g_NotificationStatus,szNotPrm,sizeof(g_NotificationStatus)-1);
    return SUCCESS;
}

int isNotificationEnabled () {
    return atoi(getNotificationStatus());
}

int remoteAccessDevConfig(char *xmlBuf, unsigned int buflen, void *data_sock)
{
	int retVal = PLUGIN_SUCCESS;
	mxml_node_t *tree;
	mxml_node_t *find_node;
	char statusResp[MAX_FILE_LINE];
	int dsock = PLUGIN_SUCCESS;
	char *sMac = NULL;

	memset(statusResp, 0x0, MAX_FILE_LINE);
	if (!xmlBuf) 
	{
		retVal = PLUGIN_FAILURE;
		goto handler_exit;
	}
	tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
	if (tree)
	{
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);

		find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
		if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) 
		{
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
			//Get mac address of the device
			APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", GetMACAddress());
			sMac = utilsRemDelimitStr(GetMACAddress(), ":");

			if (!strcmp((find_node->child->value.opaque), sMac)) 
			{
				/* parse and enqueue command */
				DespatchSendNotificationResp((void *) xmlBuf);
				retVal = PLUGIN_SUCCESS;
			} 
			else 
			{
				APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, sMac);
				retVal = PLUGIN_FAILURE;
			}
			mxmlDelete(find_node);
		}
		else 
		{
			APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
			retVal = PLUGIN_FAILURE;

		}
		mxmlDelete(tree);
	}
	else
	{
		APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
		retVal = PLUGIN_FAILURE;
	}

handler_exit:

	//Create xml response
	if(retVal)
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>1</status></plugin>",sMac);
	else
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>0</status></plugin>",sMac);

	APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in devConfig response created is %s", statusResp);
	//Send this response towards cloud synchronously using same data socket
	dsock = *(int*)data_sock;
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in devConfig response data sock used is %d", dsock);
	if (gEncDecEnabled) 
	{
		retVal = remoteSendEncryptPkt(dsock, statusResp, strlen(statusResp));
	}
	else 
	{
		retVal = send(dsock, statusResp, strlen(statusResp), 0);
	}
	if (retVal < PLUGIN_SUCCESS) {
		APP_LOG("REMOTEACCESS", LOG_ERR, "plug-in devConfig response data sock send is failed %d", retVal);
		retVal = PLUGIN_FAILURE;
	}
	else 
	{
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in devConfig response data sock total bytes send is %d", retVal);
		retVal = PLUGIN_SUCCESS;
	}

	if (sMac) free(sMac);
	return retVal;
}

int setEventPush (void *hndl, char *xmlBuf, unsigned int buflen,void*data_sock, void* remoteInfo, char* transaction_id )
{
    int retVal = PLUGIN_SUCCESS,statusNotify=0;
    pluginDeviceStatus *pdevStatus = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL, *status_node = NULL;
    char statusResp[MAX_FILE_LINE];
    int dsock = PLUGIN_SUCCESS;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    pdevStatus = (pluginDeviceStatus*)calloc(1, sizeof(pluginDeviceStatus));
    if (!pdevStatus) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }

    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Set Event Push Command Received from Cloud.....");

    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    strcpy(pdevStatus->macAddress, (find_node->child->value.opaque));
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (!strcmp(pdevStatus->macAddress, g_szWiFiMacAddress)) {
		//Set plugin device status as requested
		status_node = mxmlFindElement(tree, tree, "notificationType", NULL, NULL, MXML_DESCEND);
		if ((status_node) && (status_node->child)) {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Notifation Status in XML is %s", status_node->child->value.opaque);
		    statusNotify = 	atoi(status_node->child->value.opaque);
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Notification Status to set is %d", statusNotify);
		    //setPower(status); SetCurBinaryState(status);
		    setNotificationStatus (statusNotify);
#if defined(PRODUCT_WeMo_Insight)
		    SetBelkinParameter (INSIGHT_EVENT_ENABLE,status_node->child->value.opaque);
		    if(g_EventEnable && (statusNotify != 0)) //Send Event if thread is sleeping.
			g_SendEvent = 1;
		    g_EventEnable=statusNotify;
#endif
		    SaveSetting ();
		    mxmlDelete(status_node);
		}else {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "status tag not present in xml payload");
		    retVal = PLUGIN_FAILURE;
		    mxmlDelete(tree);
		    goto handler_exit1;
		}
		//Create xml response
		pdevStatus->pluginId = strtol(g_szPluginCloudId, NULL, 0);
		strcpy(pdevStatus->macAddress, g_szWiFiMacAddress);
		strcpy(pdevStatus->friendlyName, g_szFriendlyName);
		//use gpio interfaces for device status
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Current Binary state of plug-in %d", GetCurBinaryState());
		//pdevStatus->status = GetCurBinaryState();
		pdevStatus->status = CloudGetBinaryState();
		#ifdef PRODUCT_WeMo_NetCam
		AsyncRequestNetCamFriendlyName();
		#endif
		//Create xml response
		memset(statusResp, 0x0, MAX_FILE_LINE);
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><notificationType>%d</notificationType></plugin>\n", pdevStatus->pluginId, pdevStatus->macAddress, pdevStatus->status, statusNotify);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in dev status response created is %s", statusResp);
		//Send this response towards cloud synchronously using same data socket
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_GETPLUGINDBFILE);
		if(gIceRunning)
		{
		    retVal=SendIcePkt(hndl,statusResp,remoteInfo,transaction_id);
		}
		else
		{
		    dsock = *(int*)data_sock;
		    retVal=SendTurnPkt( dsock,statusResp);
		}
		if (retVal < PLUGIN_SUCCESS) {
		    APP_LOG("REMOTEACCESS", LOG_ERR, "plug-in Notification sock send is failed %d", retVal);
		    retVal = PLUGIN_FAILURE;
		}else {
		    APP_LOG("REMOTEACCESS", LOG_DEBUG, "plug-in Notification sock send is %d", retVal);
		    retVal = PLUGIN_SUCCESS;
		}

	    } else {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", pdevStatus->macAddress, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
	    }
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "macAddress tag not present in xml payload");
	    retVal = PLUGIN_FAILURE;
	}
	if (find_node) {
	    mxmlDelete(find_node);
	    find_node = NULL;
	}
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (pdevStatus) { free(pdevStatus); pdevStatus = NULL; }
    return retVal;
}

int sendDbFileHandle(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
		int retVal = PLUGIN_SUCCESS;
		mxml_node_t *tree = NULL;
		mxml_node_t *find_node = NULL;
		char statusResp[MAX_DATA_LEN];
		unsigned long int DbfileLen=0;
		char DbFileURL[MAX_FILE_LINE];
		char cookie[MAX_FILE_LINE];
		if (!xmlBuf) {
				retVal = PLUGIN_FAILURE;
				goto handler_exit1;
		}
		CHECK_PREV_TSX_ID(E_SENDPLUGINDBFILE,transaction_id,retVal);
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered icesendDbFileHandle\n");
		tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
		if (tree){
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
				find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
						//Get mac address of the device
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
						if (strcmp((find_node->child->value.opaque), g_szWiFiMacAddress)) {
								APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, g_szWiFiMacAddress);
								retVal = PLUGIN_FAILURE;
								mxmlDelete(find_node);
								find_node = NULL;
								goto handler_exit1;
						}
				}
				if (find_node) {mxmlDelete(find_node); find_node = NULL;}

				find_node = mxmlFindElement(tree, tree, "length", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The DbFile length rcvd is %s\n", find_node->child->value.opaque);
						DbfileLen=atol(find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
						goto handler_exit1;
				}	

				find_node = mxmlFindElement(tree, tree, "uri", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						memset(DbFileURL,0,sizeof(DbFileURL));
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The uri is %s\n", find_node->child->value.opaque);
						strcpy(DbFileURL,find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
						goto handler_exit1;
				}											
				memset(cookie,0,sizeof(cookie));
				find_node = mxmlFindElement(tree, tree, "cookie", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						memset(cookie,0,sizeof(cookie));
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The cookie is %s\n", find_node->child->value.opaque);
						strcpy(cookie,find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
				}											
				retVal=getDBFile(DbfileLen,DbFileURL,cookie);
				if(retVal) {
						retVal=3;
				}
				else
				{
						retVal=1;
				}
				memset(statusResp,0,sizeof(statusResp));
				snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status></plugin>\n",strtol(g_szPluginCloudId, NULL, 0),g_szWiFiMacAddress,retVal );
				APP_LOG("REMOTEACCESS", LOG_DEBUG,"statusResp =========%s ",statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_GETPLUGINDBFILE);
		}else{
				APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
				retVal = PLUGIN_FAILURE;
		}
handler_exit1:
		if(tree){mxmlDelete(tree); tree=NULL;}
		return retVal;
}

int  getDBFile(unsigned long int length,char *url,char*cookie)
{
		UserAppSessionData *pUsrAppSsnData = NULL; 
		UserAppData *pUsrAppData = NULL;
		authSign *assign = NULL;
		int status = 1;         
		int retVal = 0;         
		char *ptr = NULL;    
		FILE *fptr=NULL;				

		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered getDBFile API\n");
		//Resolve LB domain name to get SIP Server and other server IPs
		assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
		if (!assign) {          
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Signature Structure returned NULL\n");
				goto exit_below;                                
		}            
		pUsrAppData = (UserAppData *)malloc(sizeof(UserAppData));
		if (!pUsrAppData) {
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Malloc Structure returned NULL\n");
				goto exit_below;                                
		}
		memset( pUsrAppData, 0x0, sizeof(UserAppData));
		pUsrAppSsnData = webAppCreateSession(0);
		if (!pUsrAppSsnData) {
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Session Structure returned NULL\n");
				goto exit_below;                                
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Getting DbFile  from Cloud  \n");
		/* prepare REST header*/
		{                       
				strcpy( pUsrAppData->url, url);             
				strcpy( pUsrAppData->keyVal[0].key, "Content-Type");
				strcpy( pUsrAppData->keyVal[0].value, "multipart/octet-stream");
				strcpy( pUsrAppData->keyVal[1].key, "Authorization");
				strcpy( pUsrAppData->keyVal[1].value,  assign->signature);
				strncpy( pUsrAppData->keyVal[2].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[2].key)-1);   
				strncpy( pUsrAppData->keyVal[2].value, g_szClientType, sizeof(pUsrAppData->keyVal[2].value)-1);   
				pUsrAppData->keyValLen = 3;
				if(cookie)
				{
						strcpy( pUsrAppData->keyVal[3].key, "cookie");
						strcpy( pUsrAppData->keyVal[3].value, cookie);
						pUsrAppData->keyValLen = 4;
				}
				/* enable SSL if auth URL is on HTTPS*/
				ptr = strstr(url,"https" );
				if(ptr != NULL)
						pUsrAppData->httpsFlag =  1;
				else
						pUsrAppData->httpsFlag =  0;
		}
		pUsrAppData->disableFlag = 1;
		pUsrAppData->inDataLength = 0;
		retVal = webAppSendData( pUsrAppSsnData, pUsrAppData, 0);
		if (retVal)
		{
				APP_LOG("REMOTEACCESS", LOG_ERR,"Some error encountered while sending to %s errorCode %d \n", url, retVal);
				goto exit_below;
		}
		/* check response header to see if user is authorized or not*/
		{
				ptr = strstr(pUsrAppData->outHeader,"200 OK" );
				if(ptr != NULL)
				{
						retVal = PLUGIN_SUCCESS;
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response 200 OK received from %s\n", url);
						if(pUsrAppData->outDataLength !=length)
						{
								APP_LOG("REMOTEACCESS", LOG_DEBUG,"Received File length %d and actual file length %d \n",pUsrAppData->outDataLength,length );
								retVal=PLUGIN_FAILURE;
								goto exit_below;
						}
						char cmdbuf[MAX_LVALUE_LEN];
						memset(cmdbuf, 0x0, MAX_LVALUE_LEN);
						snprintf(cmdbuf,sizeof(cmdbuf), "rm -rf %s", RULE_DB_PATH);
						status = system(cmdbuf);
						if(status != PLUGIN_SUCCESS)
								APP_LOG("REMOTEACCESS", LOG_ERR, "failed to remove existing rule db file");

						fptr= fopen(RULE_DB_PATH, "ab");
						if(fptr == NULL) {
								APP_LOG("REMOTEACCESS", LOG_ERR, "failed to open rules file for writing");
								retVal=PLUGIN_FAILURE;
								goto exit_below;
						}

						fwrite(pUsrAppData->outData,1,length,fptr);
						fclose(fptr);
						fptr = NULL;

				}else {
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response other than 200 OK received from %s\n", url);
						retVal=PLUGIN_FAILURE;
						goto exit_below;
				}
		}
exit_below:
		if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
		if (pUsrAppData) { 
				if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
				free(pUsrAppData); pUsrAppData = NULL; 
		}
		if (assign) { free(assign); assign = NULL; }
		return retVal;
}

int getDbFileHandle(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
    int retVal = PLUGIN_SUCCESS;
    mxml_node_t *tree = NULL;
    mxml_node_t *find_node = NULL;
    char statusResp[MAX_DATA_LEN];
    unsigned long int DbfileLen=0;
    char DbFileURL[MAX_FILE_LINE];
    char cookie[SIZE_512B];
    int fileLen=0;
    char *fsFileUrl = NULL;
    char *URL=NULL;

    if (!xmlBuf) {
	retVal = PLUGIN_FAILURE;
	goto handler_exit1;
    }
    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Entered icegetDbFileHandle %s", xmlBuf);
		CHECK_PREV_TSX_ID(E_GETPLUGINDBFILE,transaction_id,retVal);
    tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
    if (tree){
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
	find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
	    //Get mac address of the device
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
	    if (strcmp((find_node->child->value.opaque), g_szWiFiMacAddress)) {
		APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, g_szWiFiMacAddress);
		retVal = PLUGIN_FAILURE;
		mxmlDelete(find_node);
		find_node = NULL;
		mxmlDelete(tree);
		goto handler_exit1;
	    }
	}
	if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	find_node = mxmlFindElement(tree, tree, "uri", NULL, NULL, MXML_DESCEND);
	if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
	    memset(DbFileURL,0,sizeof(DbFileURL));
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "The uri is %s\n", find_node->child->value.opaque);
	    strcpy(DbFileURL,find_node->child->value.opaque);
	    mxmlDelete(find_node);
	    find_node = NULL;
	}else{
	    if (find_node) {mxmlDelete(find_node); find_node = NULL;}
	    mxmlDelete(tree);
	    goto handler_exit1;
	}											
	fsFileUrl = (char *)malloc((sizeof(char)*256));
	if(fsFileUrl == NULL)
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG,"fsFileUrl malloc failed\n");
	    mxmlDelete(tree);
	    goto handler_exit1;

	}
	retVal=sendDBFile(DbfileLen,DbFileURL, fsFileUrl,cookie);
	if(fsFileUrl) {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG,"fsFileUrl=%s \n", fsFileUrl);
	} else {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG,"fsFileUrl=NULLLL \n");
	}
	if(!retVal)
	{
	    retVal=1;
	}
	else
	{
	    retVal=3;
	}
	/*Opening file for reading in binary mode*/
	FILE *hndlRead= NULL;

	hndlRead = fopen(RULE_DB_PATH, "rb");
	if(hndlRead == NULL) {
	    APP_LOG("REMOTEACCESS", LOG_ERR, "failed to open rules file for reading");
	    mxmlDelete(tree);
	    goto handler_exit1;
	}
	//Get file length
	fseek(hndlRead, 0, SEEK_END);
	fileLen=ftell(hndlRead);
	fseek(hndlRead, 0, SEEK_SET);
	fclose(hndlRead);
	hndlRead = NULL;

	snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status><length>%d</length><uri>%s</uri><cookie>%s</cookie></plugin>\n",strtol(g_szPluginCloudId, NULL, 0),g_szWiFiMacAddress,retVal,fileLen,fsFileUrl,cookie );
	APP_LOG("REMOTEACCESS", LOG_DEBUG,"statusResp ======%s",statusResp);
	retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_GETPLUGINDBFILE);
	mxmlDelete(tree);
    }else{
	APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
	retVal = PLUGIN_FAILURE;
    }
handler_exit1:
    if (fsFileUrl) {free(fsFileUrl); fsFileUrl = NULL;}
    if (URL) {free(URL); URL = NULL;}
    return retVal;
}

int  sendDBFile(unsigned long int length,char *url, char *fsFileUrl ,char* cookie)
{
    UserAppSessionData *pUsrAppSsnData = NULL; 
    UserAppData *pUsrAppData = NULL;
    authSign *assign = NULL;
    int retVal = 0;         
    char *ptr = NULL;    

    APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered sendDBFile API\n");
    //Resolve LB domain name to get SIP Server and other server IPs
    assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
    if (!assign) {          
	APP_LOG("REMOTEACCESS", LOG_ERR,"\n Signature Structure returned NULL\n");
	retVal = PLUGIN_FAILURE;
	goto exit_below;                                
    }            
    pUsrAppData = (UserAppData *)malloc(sizeof(UserAppData));
    if (!pUsrAppData) {
	APP_LOG("REMOTEACCESS", LOG_ERR,"\n Malloc Structure returned NULL\n");
	retVal = PLUGIN_FAILURE;
	goto exit_below;                                
    }
    memset( pUsrAppData, 0x0, sizeof(UserAppData));
    pUsrAppSsnData = webAppCreateSession(0);
    if (!pUsrAppSsnData) {
	APP_LOG("REMOTEACCESS", LOG_ERR,"\n Session Structure returned NULL\n");
	retVal = PLUGIN_FAILURE;
	goto exit_below;                                
    }

    APP_LOG("REMOTEACCESS", LOG_DEBUG,"Sending DBFile to Cloud  \n");

    /* prepare REST header*/
    {                       
	strcpy( pUsrAppData->url, url);             
	strcpy( pUsrAppData->keyVal[0].key, "Content-Type");
	strcpy( pUsrAppData->keyVal[0].value, "multipart/octet-stream");
	strcpy( pUsrAppData->keyVal[1].key, "Authorization");
	strcpy( pUsrAppData->keyVal[1].value,  assign->signature);
	strncpy( pUsrAppData->keyVal[2].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[2].key)-1);   
	strncpy( pUsrAppData->keyVal[2].value, g_szClientType, sizeof(pUsrAppData->keyVal[2].value)-1);   
	pUsrAppData->keyValLen = 3;
	/* enable SSL if auth URL is on HTTPS*/
	ptr = strstr(url,"https" );
	if(ptr != NULL)
	    pUsrAppData->httpsFlag =  1;
	else
	    pUsrAppData->httpsFlag =  0;
    }


    pUsrAppData->disableFlag = 1;
    pUsrAppData->inDataLength = 0;
    strcpy(pUsrAppData->inData,RULE_DB_PATH);
    retVal = webAppSendData( pUsrAppSsnData, pUsrAppData, 2);
    if (retVal)
    {
	APP_LOG("REMOTEACCESS", LOG_ERR,"Some error encountered while sending to %s errorCode %d \n", url, retVal);
	retVal = PLUGIN_FAILURE;
	goto exit_below;
    }
    /* check response header to see if user is authorized or not*/
    {
	APP_LOG("REMOTEACCESS", LOG_HIDE,"NC: Response data received from %s\n", pUsrAppData->outData);

	ptr = strstr(pUsrAppData->outHeader,"200 OK" );
	if(ptr != NULL)
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response 200 OK received from %s\n", url);
	    strcpy(fsFileUrl, pUsrAppData->outData);
	    memset(cookie,0,SIZE_512B);
	    strncpy(cookie,pUsrAppData->cookie_data,sizeof(cookie)-1);
	    APP_LOG("REMOTEACCESS", LOG_HIDE,"Cookie is =%s\n", cookie);
	    APP_LOG("REMOTEACCESS", LOG_DEBUG,"fsFileUrl=%s\n", fsFileUrl);
	    retVal = PLUGIN_SUCCESS;
	}else {
	    APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response other than 200 OK received from %s\n", url);
	    retVal = PLUGIN_FAILURE;
	    goto exit_below;
	}
    }
exit_below:
    if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
    if (pUsrAppData) { 
	if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
	free(pUsrAppData); pUsrAppData = NULL; 
    }
    if (assign) { free(assign); assign = NULL; }
    return retVal;
}



#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

int remoteAccessEditPNRuleMessage(void *hndl,char *xmlBuf, unsigned int buflen,void *data_sock, void *remoteInfo,char* transaction_id)
{
		int retVal = PLUGIN_SUCCESS;
		mxml_node_t *tree=NULL;
		mxml_node_t *find_node=NULL;
		char statusResp[SIZE_1024B];
		char macAddress[MAX_MAC_LEN];
		char RuleFreq[MAX_RULEFREQ_LEN];
		char RuleID[MAX_RULEID_LEN];
		char RuleMSG[MAX_RULEMSG_LEN];
		int  PNAction=0x00;

		if (!xmlBuf) {
				retVal = PLUGIN_FAILURE;
				goto handler_exit1;
		}
		CHECK_PREV_TSX_ID(E_EDITPNRULEMESSAGE,transaction_id,retVal);
		tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);

		if (tree){
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
				find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
				if (find_node) {
						strcpy(macAddress, (find_node->child->value.opaque));
						//Get mac address of the device
						if (!strcmp(macAddress, g_szWiFiMacAddress)) {
								//Create xml response
								find_node = mxmlFindElement(tree, tree, "action", NULL, NULL, MXML_DESCEND);
								if (find_node) {
										PNAction = atoi(find_node->child->value.opaque);
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "RuleAction value is %d", PNAction);
								}
								else {
										APP_LOG("REMOTEACCESS", LOG_ERR, "action tag not present in xml payload");
										retVal = PLUGIN_FAILURE;
								}
								find_node = mxmlFindElement(tree, tree, "ruleId", NULL, NULL, MXML_DESCEND);
								if (find_node) {
										memset(RuleID, 0x00, sizeof(RuleID));
										strcpy(RuleID, (find_node->child->value.opaque));
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "RuleID value is %s", RuleID);
								}
								else {
										APP_LOG("REMOTEACCESS", LOG_ERR, "RuleID tag not present in xml payload");
										retVal = PLUGIN_FAILURE;
								}
								find_node = mxmlFindElement(tree, tree, "frequency", NULL, NULL, MXML_DESCEND);
								if (find_node) {
										memset(RuleFreq, 0x00, sizeof(RuleFreq));
										strcpy(RuleFreq, (find_node->child->value.opaque));
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "RuleFreq value is %s", RuleFreq);
								}
								else {
										APP_LOG("REMOTEACCESS", LOG_ERR, "frequency tag not present in xml payload");
										retVal = PLUGIN_FAILURE;
								}
								find_node = mxmlFindElement(tree, tree, "messageText", NULL, NULL, MXML_DESCEND);
								if (find_node) {
										memset(RuleMSG, 0x00, sizeof(RuleMSG));
										strcpy(RuleMSG, (find_node->child->value.opaque));
										if(RuleMSG[0] == '\''){
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "RuleMSG value is %s", RuleMSG);
										}
										else {
												APP_LOG("REMOTEACCESS", LOG_ERR, "message in wrong format in xml payload");
												retVal = PLUGIN_FAILURE;	
										}
								}
								else {
										APP_LOG("REMOTEACCESS", LOG_ERR, "messageText tag not present in xml payload");
										retVal = PLUGIN_FAILURE;
								}
								if(retVal != PLUGIN_FAILURE){
										if(0 == PNAction){
												lockRule();
												retVal = UpdateAPNSDB(RuleID,RuleMSG,RuleFreq,0);
												unlockRule();
										}
										else if (1 == PNAction){ 
												lockRule();
												retVal = UpdateAPNSDB(RuleID,RuleMSG,RuleFreq,1);
												unlockRule();
										}
										else{
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "Unknow Action Type");
												retVal = PLUGIN_FAILURE;
										}
										if(retVal != PLUGIN_FAILURE){
												snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>1</status></plugin>",  g_szWiFiMacAddress);
										}
										else{
												snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><status>0</status></plugin>",  g_szWiFiMacAddress);
										}


										APP_LOG("REMOTEACCESS", LOG_DEBUG, "EditPNRuleMessage response created is %s", statusResp);
										//Send this response towards cloud synchronously using same data socket
										retVal=SendNatPkt(hndl,statusResp,remoteInfo,transaction_id,data_sock,E_EDITPNRULEMESSAGE);

								}
						} else {
								APP_LOG("REMOTEACCESS", LOG_ERR, "EditPNRuleMessage Mac received %s doesn't match with Mac %s", macAddress, g_szWiFiMacAddress);
								retVal = PLUGIN_FAILURE;
						}
						mxmlDelete(find_node);
				}else {
						APP_LOG("REMOTEACCESS", LOG_ERR, "EditPNRuleMessage macAddress tag not present in xml payload");
						retVal = PLUGIN_FAILURE;
				}
				mxmlDelete(tree);
		}else{
				APP_LOG("REMOTEACCESS", LOG_ERR, "EditPNRuleMessage XML String %s couldn't be loaded", xmlBuf);
				retVal = PLUGIN_FAILURE;
		}
handler_exit1:
		return retVal;
}
#endif

int SendNatPkt(void *hndl,char* statusResp,void* remoteInfo,char*transaction_id,void*data_sock,int index)
{
 int retVal=PLUGIN_FAILURE;
 int dsock=PLUGIN_SUCCESS;
		if(gIceRunning)
		{
				retVal=SendIcePkt(hndl,statusResp,remoteInfo,transaction_id);
		}
		else
		{
				dsock = *(int*)data_sock;
				retVal=SendTurnPkt( dsock,statusResp);
		}
		if (retVal < PLUGIN_SUCCESS) {
				APP_LOG("REMOTEACCESS", LOG_ERR, " response data sock send is failed %d", retVal);
				retVal = PLUGIN_FAILURE;
		}else {
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "response data sock total bytes send is %d", retVal);
				retVal = PLUGIN_SUCCESS;
				if (transaction_id) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Index is %d ", index);
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "transaction_id is %s ", transaction_id);
				}
				if(gIceRunning && (index != E_PLUGINERRORRSP))	memcpy(gIcePrevTransactionId[index],transaction_id,TRANSACTION_ID_LEN);

		}
		return retVal;
}

char *remotePktDecrypt(void *pkt, unsigned pkt_len, int *declen) 
{
	char publickey[MAX_PKEY_LEN];
	unsigned char *key_data;
	int key_data_len;
	unsigned char *ciphertext;
	int olen;
	char *plaintext;
	char *salt = NULL;
	unsigned char *iv;
	int saltlen = MAX_OCT_LEN;
	int ivlen = SIZE_16B;

	APP_LOG("REMOTEACCESS", LOG_HIDE, "\n Key for plug-in device is %s", g_szPluginPrivatekey);
	memset(publickey, 0x0, MAX_PKEY_LEN);
	strncpy(publickey, g_szPluginPrivatekey, sizeof(publickey)-1);
	
	key_data = (unsigned char *)publickey;
	key_data_len = strlen((char *)key_data);
	key_data[key_data_len] = '\0';

	salt = (char*)calloc(1, saltlen);
	strncpy(salt, (char *)key_data, saltlen);
	salt[saltlen] = '\0';
	
	iv = (unsigned char*)calloc(1, ivlen);
	memcpy(iv, key_data, ivlen);
	iv[ivlen] = '\0';

	APP_LOG("REMOTEACCESS", LOG_HIDE, "Key is %s\n", (char*)key_data);
	APP_LOG("REMOTEACCESS", LOG_HIDE, "Key data length 1 is %d\n", key_data_len);
	APP_LOG("REMOTEACCESS", LOG_HIDE, "Raw Salt is %s - length %d\n", (char*)salt, saltlen);
	APP_LOG("REMOTEACCESS", LOG_HIDE, "Raw IV is %s - length %d\n", (char*)iv, ivlen);
	
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "Packet Length Received from remote is %d\n", pkt_len);
	if (pkt_len <= PLUGIN_SUCCESS) {
		return NULL;	
	}
	ciphertext = (unsigned char*)pkt;
	olen = pkt_len;
	plaintext = (char*)pluginAESDecrypt(key_data, key_data_len, (unsigned char *)salt, saltlen, (unsigned char*)iv, ivlen, (unsigned char*)ciphertext, &olen);
	if(plaintext){plaintext[olen] = '\0';}
	*declen = olen;  
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "\n....after decrypt input length is %d\n", olen);
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "\nDecrypted data in string %s\n", plaintext);
	free(salt);
	salt = NULL;
	free(iv);
	iv = NULL;
	return plaintext;
}
#define MAX_PAYLOAD_LEN   SIZE_8192B
int remoteAccessXmlHandler(void *hndl, void *pkt, unsigned pkt_len,void* remoteInfo, char* transaction_id,void* data_sock)
{

		int retVal = PLUGIN_SUCCESS;
		char *decPkt = NULL, *xmltBuf = NULL;
		mxml_node_t *tree = NULL;
		mxml_node_t *find_node = NULL;
		int declen = PLUGIN_SUCCESS;
		char writeLogsBuf[MAX_BUF_LEN];

		APP_LOG("REMOTEACCESS", LOG_ERR, "-----Invoked %d", pkt_len);
		if (pkt_len <= 0 && pkt_len >=MAX_PAYLOAD_LEN ) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "Packet Length %d  received is Invalid", pkt_len);
				memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
				snprintf(writeLogsBuf, MAX_BUF_LEN, "Packet Length received is %d", pkt_len);
				retVal = PLUGIN_FAILURE;
				goto handler_exit;
		}
		if (!pkt) {
				memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
				snprintf(writeLogsBuf, MAX_BUF_LEN, "Packet received is NULL");
				APP_LOG("REMOTEACCESS", LOG_ERR, "Packet received is NULL");
				retVal = PLUGIN_FAILURE;
				goto handler_exit;
		}

		if(gIceRunning)
		{
				pj_thread_desc iceReq_thread_desc;
				pj_thread_t         *iceReq_thread;
				pj_thread_register("iceAccessServiceHandler", iceReq_thread_desc, &iceReq_thread);
				data_sock=NULL;
				pkt=(unsigned char*)pkt+30;
				pkt_len=pkt_len-30;
		}
		else
		{
				remoteInfo=NULL;
				transaction_id=NULL;
		}

		if (gEncDecEnabled) {
				decPkt = remotePktDecrypt((void*)pkt, pkt_len, &declen);
				if (!decPkt) {
						memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
						snprintf(writeLogsBuf, MAX_BUF_LEN, "remotePktDecrypt failed");
						retVal = PLUGIN_FAILURE;
						goto handler_exit;
				}
		}else {
				declen = pkt_len;
				decPkt = (char*)calloc(1, declen);
				strncpy(decPkt, (char *)pkt, declen);
				if (!decPkt) {
						retVal = PLUGIN_FAILURE;
						goto handler_exit;
				}
		}

		if (declen < 1) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "declen received is less than 1");
				retVal = PLUGIN_FAILURE;
				goto handler_exit;
		}

		xmltBuf = (char*)calloc(1, (declen+MAX_LVALUE_LEN));
		if (!xmltBuf) {
				retVal = PLUGIN_FAILURE;
				goto handler_exit;
		}
		memset(xmltBuf, 0x0, declen+MAX_LVALUE_LEN);
		memcpy(xmltBuf, (char*)decPkt, declen);//TODO
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Modified Pkt to be parsed is %s", xmltBuf);
		tree = mxmlLoadString(NULL, xmltBuf, MXML_OPAQUE_CALLBACK);
		if (tree) {
				if ((find_node = mxmlFindElement(tree, tree, "pluginDeviceStatus", NULL, NULL, MXML_DESCEND))) {
						retVal =GetDevStatusHandle(hndl, xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "GetDevStatusHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
#ifdef PRODUCT_WeMo_Insight
				}else if ((find_node = mxmlFindElement(tree, tree, "getPluginDetails", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessGetPluginDetails(hndl,xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "setPowerThreshold", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessSetPowerThreshold(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "getPowerThreshold", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessGetPowerThreshold(hndl,xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "setClearDataUsage", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessSetClearDataUsage(hndl,xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "getDataExportInfo", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessGetDataExportInfo(hndl,xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "scheduleDataExport", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessScheduleDataExport(hndl,xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "setInsightHomeSettings", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessSetInsightHomeSettings(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
#endif
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)


				}else if ((find_node = mxmlFindElement(tree, tree, "editPNRuleMessage", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessEditPNRuleMessage(hndl,xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
#endif
				}else if ((find_node = mxmlFindElement(tree, tree, "pluginSetDeviceStatus", NULL, NULL, MXML_DESCEND))) {
						retVal = SetDevStatusHandle(hndl, xmltBuf, strlen(xmltBuf),data_sock,remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "SetDevStatusHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}

				}else if ((find_node = mxmlFindElement(tree, tree, "PluginGetDBFile", NULL, NULL, MXML_DESCEND))) {
						retVal = GetRulesHandle(hndl, xmltBuf, strlen(xmltBuf), data_sock,remoteInfo, transaction_id );
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "GetRulesHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "wdLogFile", NULL, NULL, MXML_DESCEND))) {
						retVal = GetLogHandle(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "GetLogHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}

						ithread_t logFile_thread;
						APP_LOG("REMOTEACCESS",LOG_DEBUG, "*********LOG Thread created 2*********** \n");
						ithread_create(&logFile_thread, NULL, uploadLogFileThread, NULL);
						ithread_detach (logFile_thread);
				}
#ifdef SIMULATED_OCCUPANCY
				else if ((find_node = mxmlFindElement(tree, tree, "SimulatedRuleData", NULL, NULL, MXML_DESCEND))) {
				    APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with SimulatedRuleData");
				    retVal = SetSimulatedRuleData(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo, transaction_id);
				    mxmlDelete(find_node);
				    if (retVal != PLUGIN_SUCCESS) {
					memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
					snprintf(writeLogsBuf, MAX_BUF_LEN, "SetSimulatedRuleData");
					retVal = PLUGIN_FAILURE;
					goto handler_exit;
				    }
				}
#endif
#ifdef WeMo_SMART_SETUP_V2 
				else if ((find_node = mxmlFindElement(tree, tree, "SetCustomizedStatus", NULL, NULL, MXML_DESCEND))) {
				    APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with resetCustomizedStatus");
				    retVal = remoteSetCustomizedState(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo, transaction_id);
				    mxmlDelete(find_node);
				    if (retVal != PLUGIN_SUCCESS) {
					memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
					snprintf(writeLogsBuf, MAX_BUF_LEN, "remoteSetCustomizedState");
					retVal = PLUGIN_FAILURE;
					goto handler_exit;
				    }
				}
#endif
#ifdef PRODUCT_WeMo_Light
				else if ((find_node = mxmlFindElement(tree, tree, "pluginSetNightLightStatus", NULL, NULL, MXML_DESCEND)))
				{
				    retVal = SetNightLightStatusValue(hndl, xmltBuf, strlen(xmltBuf),data_sock,remoteInfo, transaction_id);
				    mxmlDelete(find_node);
				    if (retVal != PLUGIN_SUCCESS)
				    {
					memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
					snprintf(writeLogsBuf, MAX_BUF_LEN, "SetNightLightStatusValue");
					retVal = PLUGIN_FAILURE;
					goto handler_exit;
				    }
				}
				else if ((find_node = mxmlFindElement(tree, tree, "pluginGetNightLightStatus", NULL, NULL, MXML_DESCEND)))
				{
				    retVal = GetNightLightStatusValue(hndl, xmltBuf, strlen(xmltBuf),data_sock,remoteInfo, transaction_id);
				    mxmlDelete(find_node);
				    if (retVal != PLUGIN_SUCCESS)
				    {
					memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
					snprintf(writeLogsBuf, MAX_BUF_LEN, "GetNightLightStatusValue");
					retVal = PLUGIN_FAILURE;
					goto handler_exit;
				    }
				}
#endif
				else if ((find_node = mxmlFindElement(tree, tree, "updateweeklycalendar", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with updateweeklycalender");
						retVal = SetRulesHandle(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "SetRulesHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "setDbVersion", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with set setDbVersion");
						retVal = RemoteSetRulesDBVersion(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "RemoteSetRulesDBVersion");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "getDbVersion", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with getDbVersion");
						retVal = RemoteGetRulesDBVersion(hndl, xmltBuf, strlen(xmltBuf), data_sock,remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "RemoteGetRulesDBVersion");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "pluginSetNotificationType", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with getActivationStatus");
						retVal = setEventPush(hndl, xmltBuf, strlen(xmltBuf), data_sock,remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "setEventPush");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "sendPluginDbFile", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with sendPluginDbFile");
						retVal = sendDbFileHandle(hndl, xmltBuf, strlen(xmltBuf), data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "sendDbFileHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "getPluginDbFile", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with getPluginDbFile");
						retVal = getDbFileHandle(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "getDbFileHandle");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "upgradeFwVersion", NULL, NULL, MXML_DESCEND))) {
						retVal = UpgradeFwVersion(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo, transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "UpgradeFwVersion");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "deviceConfiguration", NULL, NULL, MXML_DESCEND))) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "node found with deviceConfiguration");
						retVal = RemoteAccessDevConfig(hndl, xmltBuf, strlen(xmltBuf),data_sock, remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								memset(writeLogsBuf, 0x0, MAX_BUF_LEN);
								snprintf(writeLogsBuf, MAX_BUF_LEN, "RemoteAccessDevConfig");
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "setIcon", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessSetIcon(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "getIcon", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessGetIcon(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "resetNameRulesData", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessResetNameRulesData(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id );
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "changeFriendlyName", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessChangeFriendlyName(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else if ((find_node = mxmlFindElement(tree, tree, "updateweeklyTrigger", NULL, NULL, MXML_DESCEND))) {
						retVal = remoteAccessupdateweeklyTrigger(hndl,xmltBuf, strlen(xmltBuf),data_sock,remoteInfo,transaction_id);
						mxmlDelete(find_node);
						if (retVal != PLUGIN_SUCCESS) {
								retVal = PLUGIN_FAILURE;
								goto handler_exit;
						}
				}else{
						APP_LOG("REMOTEACCESS", LOG_ERR, "No Valid XML request payload received");
						retVal = PLUGIN_FAILURE;
						mxmlDelete(tree);
						tree=NULL;
						goto handler_exit;
				}
		}else {
				APP_LOG("REMOTEACCESS", LOG_ERR, "No Valid XML TREE in request payload received NULL");
				retVal = PLUGIN_FAILURE;
		}
handler_exit:
		if(retVal != PLUGIN_SUCCESS)
		{
				char *resp_data=NULL;
        
				resp_data=(char*)malloc(SIZE_256B * sizeof(char));
				if(resp_data)
				{
						memset(resp_data,0,sizeof(resp_data));
						snprintf(resp_data,sizeof(resp_data), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>3</status><friendlyName>%s</friendlyName></plugin>\n", strtol(g_szPluginCloudId, NULL, 0), g_szWiFiMacAddress, g_szFriendlyName);

						retVal=SendNatPkt(hndl,resp_data,remoteInfo,transaction_id,data_sock,E_PLUGINERRORRSP);
						free(resp_data);
				}
		}
		if(tree)	mxmlDelete(tree);
		if (decPkt) free(decPkt);
		if (xmltBuf) free(xmltBuf);
		return retVal;
}

int  getWeeklyCallendar(void*hndl,void* data_sock,void* remoteInfo,char* transaction_id,unsigned long int length,char *url,char*cookie)
{
		UserAppSessionData *pUsrAppSsnData = NULL; 
		UserAppData *pUsrAppData = NULL;
		authSign *assign = NULL;
		int retVal = 0;         
		char *ptr = NULL;    

		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered getWeeklyCallendar API\n");
		//Resolve LB domain name to get SIP Server and other server IPs
		assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
		if (!assign) {          
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Signature Structure returned NULL\n");
				goto exit_below;                                
		}            
		pUsrAppData = (UserAppData *)malloc(sizeof(UserAppData));
		if (!pUsrAppData) {
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Malloc Structure returned NULL\n");
				goto exit_below;                                
		}
		memset( pUsrAppData, 0x0, sizeof(UserAppData));
		pUsrAppSsnData = webAppCreateSession(0);
		if (!pUsrAppSsnData) {
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Session Structure returned NULL\n");
				goto exit_below;                                
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Getting WeeklyCallendar  from Cloud  \n");
		/* prepare REST header*/
		{                       
				strcpy( pUsrAppData->url, url);             
				strcpy( pUsrAppData->keyVal[0].key, "Content-Type");
				strcpy( pUsrAppData->keyVal[0].value, "multipart/octet-stream");
				strcpy( pUsrAppData->keyVal[1].key, "Authorization");
				strcpy( pUsrAppData->keyVal[1].value,  assign->signature);
				strncpy( pUsrAppData->keyVal[2].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[2].key)-1);   
				strncpy( pUsrAppData->keyVal[2].value, g_szClientType, sizeof(pUsrAppData->keyVal[2].value)-1);   
				pUsrAppData->keyValLen = 3;
				if(cookie)
				{
						strcpy( pUsrAppData->keyVal[3].key, "cookie");
						strcpy( pUsrAppData->keyVal[3].value, cookie);
						pUsrAppData->keyValLen = 4;
				}
				/* enable SSL if auth URL is on HTTPS*/
				ptr = strstr(url,"https" );
				if(ptr != NULL)
						pUsrAppData->httpsFlag =  1;
				else
						pUsrAppData->httpsFlag =  0;
		}
		pUsrAppData->disableFlag = 1;
		pUsrAppData->inDataLength = 0;
		retVal = webAppSendData( pUsrAppSsnData, pUsrAppData, 0);
		if (retVal)
		{
				APP_LOG("REMOTEACCESS", LOG_ERR,"Some error encountered while sending to %s errorCode %d \n", url, retVal);
				goto exit_below;
		}
		/* check response header to see if user is authorized or not*/
		{
				ptr = strstr(pUsrAppData->outHeader,"200 OK" );
				if(ptr != NULL)
				{
						retVal = PLUGIN_SUCCESS;
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response 200 OK received from %s\n", url);
						if(pUsrAppData->outDataLength !=length)
						{
								APP_LOG("REMOTEACCESS", LOG_HIDE,"Received Callendar length %d and actual Callendar length %d \n",pUsrAppData->outDataLength,length );
								retVal=PLUGIN_FAILURE;
								goto exit_below;
						}
				   retVal= SetRulesHandle(hndl, pUsrAppData->outData, pUsrAppData->outDataLength,data_sock, remoteInfo, transaction_id);		
				}else {
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response other than 200 OK received from %s\n", url);
						retVal=PLUGIN_FAILURE;
						goto exit_below;
				}
		}
exit_below:
		if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
		if (pUsrAppData) { 
				if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
				free(pUsrAppData); pUsrAppData = NULL; 
		}
		if (assign) { free(assign); assign = NULL; }
		return retVal;
}
struct callendar_arg_struct{
		void					*hndl;
		void					*data_sock;
		char				  *transaction_id;
		unsigned long int length;
		char						url[MAX_FILE_LINE];
		char						cookie[MAX_FILE_LINE];
};
void* WeeklyCallendarthread(void *arg)
{
		tu_set_my_thread_name( __FUNCTION__ );

		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered in WeeklyCallendarthread\n");
		int retVal=PLUGIN_SUCCESS;
		struct callendar_arg_struct *temp=((struct callendar_arg_struct*)arg);
		if(!pj_thread_is_registered())
		{
				pj_thread_desc WeeklyCallendarthread_desc;
				pj_thread_t         *WeeklyCallendar_thread;
				pj_thread_register("WeeklyCallendarthread",WeeklyCallendarthread_desc, &WeeklyCallendar_thread);
		}	
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered in WeeklyCallendarthread  temp= %d \n",temp);


		APP_LOG("REMOTEACCESS", LOG_DEBUG," transaction id is %s\n",temp->transaction_id);
		gDataInProgress=1;
		retVal=getWeeklyCallendar( temp->hndl,temp->data_sock,&gRemoteInfo,temp->transaction_id,temp->length,temp->url,temp->cookie);
		if(retVal) {
				retVal=3;
		}
		else
		{
				retVal=1;
		}
		char statusResp[MAX_DATA_LEN];
		memset(statusResp,0,sizeof(statusResp));
		snprintf(statusResp,sizeof(statusResp), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><recipientId>%lu</recipientId><macAddress>%s</macAddress><status>%d</status></plugin>\n",strtol(g_szPluginCloudId, NULL, 0),g_szWiFiMacAddress,retVal );
		APP_LOG("REMOTEACCESS", LOG_DEBUG," sizeof( statusResp) =========%d ",sizeof(statusResp));
		retVal=SendNatPkt(temp->hndl,statusResp,&gRemoteInfo,temp->transaction_id,temp->data_sock,E_UPDATEWEEKLYCALENDAR);//TODO
		gDataInProgress=0;
		if(arg){free(arg);}
		return NULL;
}
int remoteAccessupdateweeklyTrigger(void *hndl, char *xmlBuf, unsigned int buflen, void *data_sock, void* remoteInfo,char* transaction_id)
{
		int retVal = PLUGIN_SUCCESS;
		mxml_node_t *tree = NULL;
		mxml_node_t *find_node = NULL;
		struct callendar_arg_struct *arguments=NULL;
		
		if (!xmlBuf) {
				retVal = PLUGIN_FAILURE;
				goto handler_exit1;
		}
		CHECK_PREV_TSX_ID(E_UPDATEWEEKLYCALENDAR,transaction_id,retVal);//TODO
		arguments=(struct callendar_arg_struct *)malloc( sizeof(struct callendar_arg_struct));
		if(!arguments) {
				retVal = PLUGIN_FAILURE;
				goto handler_exit1;

		}

		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered remoteAccessupdateweeklyTrigger\n");
		tree = mxmlLoadString(NULL, xmlBuf, MXML_OPAQUE_CALLBACK);
		if (tree){
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String loaded is %s", xmlBuf);
				find_node = mxmlFindElement(tree, tree, "macAddress", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address value is %s", find_node->child->value.opaque);
						//Get mac address of the device
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Mac Address of plug-in is %s", g_szWiFiMacAddress);
						if (strcmp((find_node->child->value.opaque), g_szWiFiMacAddress)) {
								APP_LOG("REMOTEACCESS", LOG_ERR, "Mac received %s doesn't match with plugin Mac %s", find_node->child->value.opaque, g_szWiFiMacAddress);
								retVal = PLUGIN_FAILURE;
								mxmlDelete(find_node);
								find_node = NULL;
								goto handler_exit1;
						}
				}
				if (find_node) {mxmlDelete(find_node); find_node = NULL;}

				find_node = mxmlFindElement(tree, tree, "length", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The Callendar length rcvd is %s\n", find_node->child->value.opaque);
						arguments->length=atol(find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
						goto handler_exit1;
				}	

				find_node = mxmlFindElement(tree, tree, "uri", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						memset(arguments->url,0,sizeof(arguments->url));
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The Callendar url is %s\n", find_node->child->value.opaque);
						strcpy(arguments->url,find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
						goto handler_exit1;
				}											
				memset(arguments->cookie,0,sizeof(arguments->cookie));
				find_node = mxmlFindElement(tree, tree, "cookie", NULL, NULL, MXML_DESCEND);
				if ((find_node) && (find_node->child) && (find_node->child->value.opaque)) {
						memset(arguments->cookie,0,sizeof(arguments->cookie));
						APP_LOG("REMOTEACCESS", LOG_HIDE, "The cookie is %s\n", find_node->child->value.opaque);
						strcpy(arguments->cookie,find_node->child->value.opaque);
						mxmlDelete(find_node);
						find_node = NULL;
				}else{
						if (find_node) {mxmlDelete(find_node); find_node = NULL;}
				}
		arguments->data_sock=data_sock;
		arguments->hndl=hndl;
		arguments->transaction_id=transaction_id;
		pthread_t WeeklyCallendar_thread;
		retVal=pthread_create(&WeeklyCallendar_thread, NULL,WeeklyCallendarthread ,(void*) (arguments));
		pthread_join(WeeklyCallendar_thread,NULL);
		}else{
				APP_LOG("REMOTEACCESS", LOG_ERR, "XML String %s couldn't be loaded", xmlBuf);
				retVal = PLUGIN_FAILURE;
		}
handler_exit1:
		if(tree){mxmlDelete(tree); tree=NULL;}
		return retVal;
}
