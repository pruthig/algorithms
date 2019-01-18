/***************************************************************************
 *
 *
 * remoteUpdate.c
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

#include "types.h"
#include "defines.h"
#include "curl/curl.h"
#include "httpsWrapper.h"
#include "ithread.h"
#include "remoteAccess.h"
#include "logger.h"
#include "controlledevice.h"
#include "sigGen.h"
#include "gpio.h"
#include "osUtils.h"
#include "utils.h"
#include "wifiSetup.h"
#include "wifiHndlr.h"
#include "mxml.h"
#include "rule.h"
#include "plugin_ctrlpoint.h"
#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif
#include "thready_utils.h"

#ifdef __NFTOCLOUD__
# define MAX_NAT_STATUS_COUNT 3
#endif

#define SENDNF_TIMESYNC_AUTH_FAIL_ERR_CODE "ERR_006"
#define MAX_AUTH_FAILURE_RETRY_ATTEMPTS		5

#ifdef PRODUCT_WeMo_Insight
char g_InstantPowerSend = 0;
int g_EventEnable =0;
int g_SendEvent =0;
int g_EventDuration =0;
#endif

#ifdef SIMULATED_OCCUPANCY
extern int gSimulatedRuleRunning;
extern int gSimManualTrigger;
#endif

#ifdef WeMo_SMART_SETUP_V2
pthread_t customizedstate_thread = -1;
pthread_attr_t customizedstate_attr;
extern int g_customizedState;
#endif

extern unsigned short gpluginRemAccessEnable;

extern char g_szFriendlyName[];
extern char g_szHomeId[SIZE_20B];
extern char g_szPluginPrivatekey[MAX_PKEY_LEN];
extern int g_eDeviceType;
extern char  g_szUDN_1[SIZE_128B];
extern char g_szRestoreState[MAX_RES_LEN];
extern char g_szFirmwareVersion[SIZE_64B];
extern char g_szSerialNo[SIZE_64B];
extern char g_szWiFiMacAddress[SIZE_64B];
extern unsigned int g_configChange;

int gpluginPrevStatus = 0;
int gpluginPrevStatusTS = 0;
ithread_mutex_t gRemoteUpdThdMutex;

static int gSendNotifyHealthPunch=0;
extern ithread_t sendremoteupdstatus_thread;
#define SEND_REM_UPD_STAT_TH_MON_TIMEOUT    120	//secs

static int gpluginStatus = 0;
pthread_mutex_t gTsLock;
pthread_mutex_t gRemoteFailCntLock;
pthread_mutex_t gStatusLock;
pthread_mutex_t g_devConfig_mutex1;
pthread_cond_t g_devConfig_cond1;
extern int gpluginStatusTS;
extern int gSignalStrength;
int sendConfigChangeDevStatus();
#if defined(LONG_PRESS_SUPPORTED)
extern int gLongPressTriggered;
#endif
static UserAppSessionData *pUsrAppSsnData = NULL;
static UserAppData *pUsrAppData = NULL;
static authSign *assign = NULL;
extern int gNTPTimeSet;
		
char gNotificationBuffer[MAX_STATUS_BUF] = {'\0',};

void initRemoteUpdSync() {
		ithread_mutex_init(&gRemoteUpdThdMutex, 0);
		APP_LOG("REMOTEACCESS",LOG_ERR, "********** Remote Status Update Notification Lock Init*************");
}

void LockRemoteUpdSync() {
		ithread_mutex_lock(&gRemoteUpdThdMutex);
		APP_LOG("REMOTEACCESS",LOG_ERR, "********** Remote Status Update Notification Lock*************");
}

void UnLockRemoteUpdSync() {
		ithread_mutex_unlock(&gRemoteUpdThdMutex);
		APP_LOG("REMOTEACCESS",LOG_ERR, "********** Remote Status Update Notification UnLock*************");
}

void initRemoteStatusTS()
{
		osUtilsCreateLock(&gTsLock);
		osUtilsCreateLock(&gStatusLock);
		osUtilsCreateLock(&gRemoteFailCntLock);
}

void initdevConfiglock()
{
		pthread_cond_init (&g_devConfig_cond1, NULL);
		pthread_mutex_init(&g_devConfig_mutex1, NULL);
}

void setPluginStatusTS(int ts)
{
		osUtilsGetLock(&gTsLock);
		gpluginStatusTS = ts;
		osUtilsReleaseLock(&gTsLock);
		APP_LOG("REMOTEACCESS",LOG_DEBUG, "gpluginStatusTS updated: %d", gpluginStatusTS);

}

void setPluginStatus(int status)
{
		osUtilsGetLock(&gStatusLock);
		gpluginStatus = status;
		osUtilsReleaseLock(&gStatusLock);
		APP_LOG("REMOTEACCESS",LOG_DEBUG, "gpluginStatus updated: %d", gpluginStatus);
}

int getPluginStatusTS(void)
{
		int ts;
		osUtilsGetLock(&gTsLock);
		ts = gpluginStatusTS;
		osUtilsReleaseLock(&gTsLock);
		return ts;
}

int getPluginStatus(void)
{
		int status;
		osUtilsGetLock(&gStatusLock);
		status = gpluginStatus;
		osUtilsReleaseLock(&gStatusLock);
		return status;
}
void* parseSendNotificationResp(void *sendNfResp, char **errcode) {

		char *snRespXml = NULL;
		mxml_node_t *tree;
		mxml_node_t *find_node;
		int fail_return = 0;
		char *pcode = NULL;

		if(!sendNfResp)
				return NULL;

		snRespXml = (char*)sendNfResp;

		tree = mxmlLoadString(NULL, snRespXml, MXML_OPAQUE_CALLBACK);
		if (tree){
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "XML String Loaded is %s\n", snRespXml);
				find_node = mxmlFindElement(tree, tree, "Error", NULL, NULL, MXML_DESCEND);
				if(find_node){
						find_node = mxmlFindElement(tree, tree, "code", NULL, NULL, MXML_DESCEND);
						if (find_node && find_node->child) {
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "The Error Code set in the node is %s\n", find_node->child->value.opaque);
								fail_return = 1;
								pcode = (char*)calloc(1, SIZE_256B);
								if (fail_return) {
										if (pcode) {
												strncpy(pcode, (find_node->child->value.opaque), SIZE_256B-1);
												*errcode = pcode;
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "The error code set from cloud is %s\n", *errcode);
										}
								}
						}
						if (fail_return) {
								goto on_return;
						}
				}else{
						goto on_return;
				}
		}else {
				goto on_return;
		}
		mxmlDelete(tree);
		return NULL;
on_return:
		mxmlDelete(tree);
		return NULL;
}

#ifdef SIMULATED_OCCUPANCY
void setSimulatedStatus (char *status)
{
		int ts = GetUTCTime();
		snprintf(status, SIZE_256B, "<ruleID>%d</ruleID><ruleMsg><![CDATA[%s]]></ruleMsg><ruleFreq>300</ruleFreq><ruleExecutionTS>%d</ruleExecutionTS><productCode>WeMo</productCode>", atoi(g_szHomeId), "RULECONFIG1", ts);
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "simStatus: %s", status);
} 
#endif

char gFirmwareVersion[SIZE_64B];
void UpdateStatusTSHttpData(char *httpPostStatus)
{
		int time_stamp, status, state=0;
		int s32SendRuleID=0;

		status = getPluginStatus();
		state = getCurrFWUpdateState();

		time_stamp = getPluginStatusTS();

		gpluginPrevStatusTS = time_stamp;

		APP_LOG("REMOTEACCESS", LOG_DEBUG, "state firmwware: %s timestamp: %d, status: %d", gFirmwareVersion, time_stamp, status);   
#ifdef PRODUCT_WeMo_Insight
		char *paramVersion = NULL,*paramPerUnitcost = NULL,*paramCurrency = NULL;
		if(g_InstantPowerSend == 1)	
		{
				LockLED();
				status = GetCurBinaryState();
				UnlockLED();
				if (g_InitialMonthlyEstKWH){
						snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><applianceConsumption><macAddress>%s</macAddress><status>%d</status><instantaneousPower>%u</instantaneousPower><todayTotalTimeOn>%u</todayTotalTimeOn><timeConnected>%u</timeConnected><stateChangeTS>%d</stateChangeTS><lastONFor>%u</lastONFor><avgPowerON>%u</avgPowerON><powerThreshold>%u</powerThreshold><todayTotalKWH>%u</todayTotalKWH><past14DaysKWH>%d</past14DaysKWH><past14TotalTime>%u</past14TotalTime></applianceConsumption>", \
										g_szWiFiMacAddress,status,g_PowerNow,g_TodayONTimeTS,g_HrsConnected,g_StateChangeTS,g_ONFor,g_AvgPowerON,g_PowerThreshold,g_AccumulatedWattMinute,g_InitialMonthlyEstKWH,g_TotalONTime14Days);
				}
				else
				{
						snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><applianceConsumption><macAddress>%s</macAddress><status>%d</status><instantaneousPower>%u</instantaneousPower><todayTotalTimeOn>%u</todayTotalTimeOn><timeConnected>%u</timeConnected><stateChangeTS>%d</stateChangeTS><lastONFor>%u</lastONFor><avgPowerON>%u</avgPowerON><powerThreshold>%u</powerThreshold><todayTotalKWH>%u</todayTotalKWH><past14DaysKWH>%0.f</past14DaysKWH><past14TotalTime>%u</past14TotalTime></applianceConsumption>", \
										g_szWiFiMacAddress,status,g_PowerNow,g_TodayONTimeTS,g_HrsConnected,g_StateChangeTS,g_ONFor,g_AvgPowerON,g_PowerThreshold,g_AccumulatedWattMinute,g_KWH14Days,g_TotalONTime14Days);
				}

		}
		else if(1 == g_InsightCostCur){
				paramVersion = GetBelkinParameter(ENERGYPERUNITCOSTVERSION);
				paramPerUnitcost = GetBelkinParameter(ENERGYPERUNITCOST);
				paramCurrency = GetBelkinParameter(CURRENCY);
				APP_LOG("REMOTEACCESS", LOG_ERR, "Energy cost push: paramVersion: %s, paramPerUnitcost: %s,paramCurrency: %s",paramVersion,paramPerUnitcost,paramCurrency);
				snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><homeSettings><homeId>%s</homeId><homeSettingsVersion>%s</homeSettingsVersion><energyPerUnitCost>%s</energyPerUnitCost><currency>%s</currency></homeSettings>",g_szHomeId,paramVersion,paramPerUnitcost,paramCurrency);

		}
		else
#endif
		{
				if (g_eDeviceType == DEVICE_SOCKET) 
				{
#ifdef PRODUCT_WeMo_Insight

						LockRuleID();
						s32SendRuleID = GetRuleIDFlag();
						UnlockRuleID();
						if(s32SendRuleID == 1)
						{
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Insight</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><ruleID>%d</ruleID><ruleMsg><![CDATA[%s]]></ruleMsg><ruleFreq>%d</ruleFreq><ruleExecutionTS>%d</ruleExecutionTS><productCode>WeMo</productCode></plugin>", g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status, time_stamp, g_szFirmwareVersion, state, gSignalStrength,g_RuleID,g_RuleMSG,g_RuleFreq,g_RuleExecutionTS);
						}
						else if(s32SendRuleID != 1){
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Insight</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status, time_stamp, g_szFirmwareVersion, state, gSignalStrength);
#ifdef SIMULATED_OCCUPANCY
								if(gSimulatedRuleRunning && gSimManualTrigger)
								{
										char simStatus[SIZE_256B];
										memset(simStatus, 0, sizeof(simStatus));
										setSimulatedStatus(simStatus);
										strncat(httpPostStatus, simStatus, MAX_STATUS_BUF-strlen(httpPostStatus)-1);
								}
#endif
								strncat(httpPostStatus, "</plugin>", MAX_STATUS_BUF-strlen(httpPostStatus)-1);
						}
						else

#endif
#if defined PRODUCT_WeMo_Light

						{
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Lightswitch</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status, time_stamp, g_szFirmwareVersion, state, gSignalStrength);
#if defined(LONG_PRESS_SUPPORTED)
								if(gLongPressTriggered)
								{
										char lpStatus[SIZE_256B];
										memset(lpStatus, 0, sizeof(lpStatus));
										snprintf(lpStatus, sizeof(lpStatus), "<longPressStatus>%d</longPressStatus>", gLongPressTriggered);
										strncat(httpPostStatus, lpStatus, MAX_STATUS_BUF-strlen(httpPostStatus)-1);
								}
#endif
#ifdef SIMULATED_OCCUPANCY
								if(gSimulatedRuleRunning && gSimManualTrigger)
								{
										char simStatus[SIZE_256B];
										memset(simStatus, 0, sizeof(simStatus));
										setSimulatedStatus(simStatus);
										strncat(httpPostStatus, simStatus, MAX_STATUS_BUF-strlen(httpPostStatus)-1);
								}

#endif
								strncat(httpPostStatus, "</plugin>", MAX_STATUS_BUF-strlen(httpPostStatus)-1);
						}
#elif defined PRODUCT_WeMo_Baby
						{
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>wemo_baby</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength></plugin>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status, time_stamp, g_szFirmwareVersion, state, gSignalStrength);
						}
#else 

						{
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Switch</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status, time_stamp, g_szFirmwareVersion, state, gSignalStrength);
#ifdef SIMULATED_OCCUPANCY
								if(gSimulatedRuleRunning && gSimManualTrigger)
								{
										char simStatus[SIZE_256B];
										memset(simStatus, 0, sizeof(simStatus));
										setSimulatedStatus(simStatus);
										strncat(httpPostStatus, simStatus, MAX_STATUS_BUF-strlen(httpPostStatus)-1);
								}

#endif

								strncat(httpPostStatus, "</plugin>", MAX_STATUS_BUF-strlen(httpPostStatus)-1);
						}
#endif
						strncpy(gFirmwareVersion, "", sizeof(gFirmwareVersion)-1);
				} 
				else if (g_eDeviceType == DEVICE_SENSOR) 
				{

#if defined (PRODUCT_WeMo_SNS) || defined (PRODUCT_WeMo_NetCam)

						LockRuleID();
						s32SendRuleID = GetRuleIDFlag();
						UnlockRuleID();
						if(s32SendRuleID == 1)
						{
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Sensor</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><ruleID>%d</ruleID><ruleMsg><![CDATA[%s]]></ruleMsg><ruleFreq>%d</ruleFreq><ruleExecutionTS>%d</ruleExecutionTS><productCode>WeMo</productCode></plugin>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status,time_stamp, g_szFirmwareVersion, state, gSignalStrength,g_RuleID,g_RuleMSG,g_RuleFreq,g_RuleExecutionTS);
						}
						else
#endif
						{
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Sensor</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength></plugin>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status,time_stamp, g_szFirmwareVersion, state, gSignalStrength);
						}
						strncpy(gFirmwareVersion, "", sizeof(gFirmwareVersion)-1);

				}
#ifdef PRODUCT_WeMo_CrockPot
				int tempCrockpot=0,ret=0;
				int eventDuration=0;
				else if (g_eDeviceType == DEVICE_CROCKPOT)
				{
						LockCrockpotRemote();
						tempCrockpot = g_remoteNotify;
						UnlockCrockpotRemote();
						if (tempCrockpot == 1) {
								ret = CloudGetCrockPotStatus(&status,&eventDuration);
								snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Switch</deviceType><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion><fwUpgradeStatus>%d</fwUpgradeStatus><signalStrength>%d</signalStrength><eventDuration>%d</eventDuration></plugin>", \
												g_szWiFiMacAddress, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, status, time_stamp, g_szFirmwareVersion, state, gSignalStrength,eventDuration);

								LockCrockpotRemote();
								g_remoteNotify = 0;
								UnlockCrockpotRemote();		
						}

						strncpy(gFirmwareVersion, "", sizeof(gFirmwareVersion)-1);
				}
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
				else 
				{
						APP_LOG("REMOTEACCESS", LOG_ERR, "\n Does not look to be a valid device type\n");
				}
		}
}

#define WDLOGFILE "/var/log/wdLogFile.txt"
#define WDLOGFILE_OLD "/var/log/wdLogFile.txt.old"

int uploadLogFileDevStatus(const char *filename, const char *uploadURL)
{
		int retVal = PLUGIN_SUCCESS;
		UserAppSessionData *pWDSsnData = NULL;
		UserAppData *pWDData = NULL;
		struct stat fileInfo;
		char tmp_url[MAX_FW_URL_LEN];

		if(!filename || !uploadURL)
				return FAILURE;

		pWDData = (UserAppData *)malloc(sizeof(UserAppData));
		if(!pWDData) {
				APP_LOG("WiFiApp", LOG_ERR, "\n Malloc failed, returned NULL, exiting \n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		memset(pWDData, 0x0, sizeof(UserAppData));
		pWDSsnData = webAppCreateSession(0);
		if(!pWDSsnData) {
				APP_LOG("WiFiApp", LOG_ERR, "\n Failed to create session, returned NULL \n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		memset(tmp_url,0,sizeof(tmp_url));
		strncpy(tmp_url, uploadURL, sizeof(tmp_url));

		strncpy(pWDData->keyVal[0].key, "Content-Type", sizeof(pWDData->keyVal[0].key)-1);
		strncpy(pWDData->keyVal[0].value, "multipart/octet-stream", sizeof(pWDData->keyVal[0].value)-1);
		strncpy(pWDData->keyVal[1].key, "Accept", sizeof(pWDData->keyVal[1].key)-1);
		strncpy(pWDData->keyVal[1].value, "application/xml", sizeof(pWDData->keyVal[1].value)-1);
		strncpy(pWDData->keyVal[2].key, "Authorization", sizeof(pWDData->keyVal[2].key)-1);
		strncpy(pWDData->keyVal[3].key, "X-Belkin-Client-Type-Id", sizeof(pWDData->keyVal[3].key)-1);   
		strncpy(pWDData->keyVal[3].value, g_szClientType, sizeof(pWDData->keyVal[3].value)-1);   
		pWDData->keyValLen = 4;

		strncpy(pWDData->mac, g_szWiFiMacAddress, sizeof(pWDData->mac)-1);

		/* Sending the log file
		 * url: upload_url
		 */
		memset(&fileInfo, '\0', sizeof(struct stat));
		stat(filename, &fileInfo);
		if((off_t)fileInfo.st_size != 0) {
				strncpy(pWDData->url, tmp_url, sizeof(pWDData->url)-1);
				strncpy(pWDData->inData, filename, sizeof(pWDData->inData)-1);
				pWDData->inDataLength = 0;

				char *check = strstr(pWDData->url, "https://");
				if (check) {
						pWDData->httpsFlag = 1;
				}

				APP_LOG("WiFiApp",LOG_DEBUG, "Sending... %s\n", filename);
				retVal = webAppSendData(pWDSsnData, pWDData, 2);

				if(retVal == ERROR_INVALID_FILE) {
						APP_LOG("WiFiApp", LOG_ERR, "\n File \'%s\' doesn't exists\n", filename);
				}else if(retVal) {
						APP_LOG("WiFiApp", LOG_ERR, "\n Some error encountered in send status to cloud  , errorCode %d \n", retVal);
						retVal = PLUGIN_FAILURE;
						goto on_return;
				}
		}

on_return:
		if (pWDSsnData) webAppDestroySession (pWDSsnData);
		if (pWDData) {
				if (pWDData->outData) {free(pWDData->outData); pWDData->outData = NULL;}
				free(pWDData); pWDData = NULL;
		}
		return retVal;
}

/* API to parse the deviceConfig return in response of SendNotification */

void* parseConfigNotificationResp(void *sendNfResp) 
{

		char *snRespXml = NULL;
		mxml_node_t *tree;
		mxml_node_t *find_node;
		int ConfigFlag=0;

		if(!sendNfResp)
				return NULL;

		snRespXml = (char*)sendNfResp;
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Entered parseConfigNotificationResp");

		tree = mxmlLoadString(NULL, snRespXml, MXML_OPAQUE_CALLBACK);
		if (tree){
				APP_LOG("REMOTEACCESS", LOG_HIDE, "XML String Loaded is %s\n", snRespXml);
				find_node = mxmlFindElement(tree, tree, "sendNotification", NULL, NULL, MXML_DESCEND);
				if(find_node){
						find_node = mxmlFindElement(tree, tree, "configChange", NULL, NULL, MXML_DESCEND);
						if (find_node && find_node->child) {
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "The ConfigChange Flag is found as %s\n", find_node->child->value.opaque);
								ConfigFlag = atoi(find_node->child->value.opaque);


								if(ConfigFlag == 1)
								{
										g_configChange = 1;		   
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "Config Chage Flag is %d\n", g_configChange);
								}
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)


								else if(ConfigFlag == 2)
								{
										LockRuleID();
										SetRuleIDFlag(RESET_RULE_FLAG);
										UnlockRuleID();
								}
#endif
								else
								{
										/* Do not set variable   g_configChange = 0; */		    
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "Config Change  Flag is found as 0\n");
								}
						}
						else
						{
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "No configChange tag in response");
								goto on_return;
						}
				}else{
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "No sendNotification tag in response");
						goto on_return;
				}
		}else {
				goto on_return;
		}
		mxmlDelete(tree);
		return NULL;
on_return:
		mxmlDelete(tree);
		return NULL;
}

void* sendConfigChangeStatusThread(void *arg)
{
                tu_set_my_thread_name( __FUNCTION__ );

		APP_LOG("REMOTEACCESS", LOG_DEBUG, "sendConfigChangeStatusThread Thread Started");

		while(1){


				/*Wait to get signal that devConfig flag is rcvd as true */
				pthread_mutex_lock(&g_devConfig_mutex1);
				pthread_cond_wait(&g_devConfig_cond1,&g_devConfig_mutex1);
				pthread_mutex_unlock(&g_devConfig_mutex1);
				if (g_configChange == 1) {

						if(sendConfigChangeDevStatus() < PLUGIN_SUCCESS){
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "Send Config Change device status fail");
						}else{
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "Send Config Change device status success...");
						}
				}else {
				}
				sleep(6);
		}
		pthread_exit(0);
		return NULL;
}


void parseMergeHomeResponse(void *sendNfResp)
{
		char *snRespXml = NULL;
		mxml_node_t *tree;
		mxml_node_t *find_node;
		char* paramNames[] = {"HomeId"};
		char *homeid = NULL;

		snRespXml = (char*)sendNfResp;
		APP_LOG("SNS", LOG_HIDE, "XML received in response is %s",snRespXml);
		tree = mxmlLoadString(NULL, snRespXml, MXML_OPAQUE_CALLBACK);
		if (tree)
		{
				find_node = mxmlFindElement(tree, tree, "homeId", NULL, NULL, MXML_DESCEND);
				if(find_node)
				{
						if (find_node && find_node->child) 
						{
								APP_LOG("SNS", LOG_HIDE, "The homeid value is  %s\n", find_node->child->value.opaque);
								/* update self home_id */
								SetBelkinParameter("home_id", find_node->child->value.opaque);
								AsyncSaveData();
								/* update the global variable to contain updated home_id */
								memset(g_szHomeId, 0, sizeof(g_szHomeId));
								strncpy(g_szHomeId, find_node->child->value.opaque, sizeof(g_szHomeId)-1);

								homeid = (char *)malloc(SIZE_64B);
								if(homeid)
								{
										memset(homeid, 0, sizeof(homeid));
										encryptWithMacKeydata(g_szHomeId, homeid);
								}

								PluginCtrlPointSendActionAll(PLUGIN_E_EVENT_SERVICE, "SetHomeId", (const char **)paramNames, (char **)&homeid, 0x01);
								if(homeid)
										free(homeid);
						}
				}
				mxmlDelete(tree);
		}
		else
				APP_LOG("SNS", LOG_ERR, "Could not load tree");

}

/* parse the home id list and prepare the XML in the following format:
 ** <homeIds>
 <homeId>1</homeId>
 <homeId>2</homeId>
 ...
 </homeIds>
 */

void updateMergeHomesRequestXml(char *httpPostStatus, char *homeidlist, char *signaturelist, int len)
{
		char *home_id=NULL;
		char sign[SIZE_256B];
		int cnt=0;
		char tmp[SIZE_256B];
		char *p=NULL;

		APP_LOG("Parse", LOG_HIDE, "Input home id list: %s", homeidlist);
		APP_LOG("Parse", LOG_HIDE, "Input signature list: %s", signaturelist);
		/* prepare the header */
		snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><homes>");

		/* parse list and add to XML string */
		while((home_id = strtok(homeidlist, "-")))
		{
				strncat(httpPostStatus, "<home>", MAX_STATUS_BUF-strlen(httpPostStatus)-1);

				cnt ++;
				APP_LOG("Parse", LOG_HIDE, "Extracted home_id: %s, cnt: %d", home_id, cnt);
				homeidlist = NULL;

				memset(tmp, 0, sizeof(tmp));
				snprintf(tmp,	sizeof(tmp), "<homeId>%s</homeId>", home_id);
				strncat(httpPostStatus, tmp, MAX_STATUS_BUF-strlen(httpPostStatus)-1);

				/* using sscanf to avoid hindering the other list parsing */
				memset(tmp, 0, sizeof(tmp));
				memset(sign, 0, sizeof(sign));

				sscanf(signaturelist, "%[^-]-%s", sign, tmp);
				p = strchr(signaturelist, '-');
				if(p)
						signaturelist  = p+1;


				APP_LOG("Parse", LOG_HIDE, "Extracted sign: %s, updated signature list: %s", sign, signaturelist);

				memset(tmp, 0, sizeof(tmp));
				snprintf(tmp,	sizeof(tmp), "<sign>%s</sign>", sign);
				strncat(httpPostStatus, tmp, MAX_STATUS_BUF-strlen(httpPostStatus)-1);

				strncat(httpPostStatus, "</home>", MAX_STATUS_BUF-strlen(httpPostStatus)-1);
		}

		/* finally, append the trailer to complete the XML */
		strncat(httpPostStatus, "</homes>", MAX_STATUS_BUF-strlen(httpPostStatus)-1);
		APP_LOG("Parse", LOG_HIDE, "XML formed: %s", httpPostStatus);
}


void sendHomeIdListToCloud(char *homeidlist, char *signaturelist)
{
		char httpPostData[MAX_STATUS_BUF];
		int retVal = PLUGIN_SUCCESS;
		UserAppSessionData *pUserAppSsnData = NULL;
		UserAppData *pUserAppData = NULL;
		authSign *sign = NULL;

		/* Drop the response in case there is no Internet connection - Rare case */	
		if (getCurrentClientState() != STATE_CONNECTED) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "NO Internet connection");
				return;
		}

		pUserAppData = (UserAppData *)malloc(sizeof(UserAppData));
		if (!pUserAppData) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "Malloc Failed");
				goto on_return;
		}

		sign = createAuthSignature(g_szWiFiMacAddress, GetSerialNumber(), g_szPluginPrivatekey);
		if (!sign) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Signature Structure returned NULL\n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		memset(httpPostData, 0x0, MAX_STATUS_BUF);
		updateMergeHomesRequestXml(httpPostData, homeidlist, signaturelist, MAX_STATUS_BUF);

		memset( pUserAppData, 0x0, sizeof(UserAppData));

		pUserAppSsnData = webAppCreateSession(0);
		if(!pUserAppSsnData)
		{
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "Websession Creation failed");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}
		else
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "Websession Created\n");


		snprintf( pUserAppData->url, sizeof(pUserAppData->url), "https://%s:8443/apis/http/plugin/mergeHomes", BL_DOMAIN_NM);
		strncpy( pUserAppData->keyVal[0].key, "Content-Type", sizeof(pUserAppData->keyVal[0].key)-1);   
		strncpy( pUserAppData->keyVal[0].value, "application/xml", sizeof(pUserAppData->keyVal[0].value)-1);   
		strncpy( pUserAppData->keyVal[1].key, "Accept", sizeof(pUserAppData->keyVal[1].key)-1);   
		strncpy( pUserAppData->keyVal[1].value, "application/xml", sizeof(pUserAppData->keyVal[1].value)-1);   
		strncpy( pUserAppData->keyVal[2].key, "Authorization", sizeof(pUserAppData->keyVal[2].key)-1);   
		strncpy( pUserAppData->keyVal[2].value, sign->signature, sizeof(pUserAppData->keyVal[2].value)-1);   
		strncpy( pUserAppData->keyVal[3].key, "X-Belkin-Client-Type-Id", sizeof(pUserAppData->keyVal[3].key)-1);   
		strncpy( pUserAppData->keyVal[3].value, g_szClientType, sizeof(pUserAppData->keyVal[3].value)-1);   
		pUserAppData->keyValLen = 4;

		strncpy( pUserAppData->inData, httpPostData, sizeof(pUserAppData->inData)-1);
		pUserAppData->inDataLength = strlen(httpPostData);
		char *check = strstr(pUserAppData->url, "https://");
		if (check) {
				pUserAppData->httpsFlag = 1;
		}
		pUserAppData->disableFlag = 1;    
		APP_LOG("REMOTEACCESS", LOG_DEBUG, " **********Sending Cloud Response XML\n");

		retVal = webAppSendData( pUserAppSsnData, pUserAppData, 1);  
		if (retVal)
		{
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Some error encountered in send status to cloud  , errorCode %d \n", retVal);
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		if( (strstr(pUserAppData->outHeader, "500")) || (strstr(pUserAppData->outHeader, "503")) ){
				APP_LOG("REMOTEACCESS", LOG_ERR, "Cloud is not reachable, error: 500 or 503");
				retVal = PLUGIN_FAILURE;
		}else if(strstr(pUserAppData->outHeader, "200")){
				APP_LOG("REMOTEACESS", LOG_DEBUG, "send status to cloud success");
				parseMergeHomeResponse(pUserAppData->outData);
		}else if(strstr(pUserAppData->outHeader, "403")){
				APP_LOG("REMOTEACCESS", LOG_ERR, "Cloud error: 403");
		}else{
				APP_LOG("REMOTEACESS", LOG_DEBUG, "Some error encountered: Error response from cloud");
				retVal = PLUGIN_FAILURE;
		}

on_return:
		if (pUserAppSsnData) {webAppDestroySession ( pUserAppSsnData ); pUserAppSsnData = NULL;}
		if (sign) {free(sign); sign = NULL;};
		if (pUserAppData) {
				if (pUserAppData->outData) {free(pUserAppData->outData); pUserAppData->outData = NULL;}
				free(pUserAppData); pUserAppData = NULL;
		}

}

int getTimeZone(char *szTimezone)
{
	float localTZ=0.0;
	char *LocalTimeZone = GetBelkinParameter(SYNCTIME_LASTTIMEZONE);
	char *LastDstValue = GetBelkinParameter(LASTDSTENABLE);
    	int DstVal=-1;

	if(!szTimezone)
		return PLUGIN_FAILURE;

	if((LocalTimeZone != NULL) && (strlen(LocalTimeZone) != 0))
	{
		localTZ = atof(LocalTimeZone);
	}
	else
	{
		APP_LOG("REMOTEACCESS",LOG_ERR,"Invalid Timezone stored on flash");
		return PLUGIN_FAILURE;
	}

	if((LastDstValue != NULL) && (strlen(LastDstValue) != 0))
	{
		DstVal = atoi(LastDstValue);
	}
	else
	{
		APP_LOG("REMOTEACCESS",LOG_DEBUG,"Invalid DST value stored on flash");
	}

	APP_LOG("REMOTEACCESS",LOG_DEBUG," ---localTZ: %f,LocalTimeZone:%s, DSTVal: %d",localTZ,LocalTimeZone,!DstVal);
	if(DstVal == 0)
		localTZ = localTZ + 1.0;

	memset(szTimezone, 0, SIZE_16B);

	if (DEVICE_SENSOR == g_eDeviceType) 
		snprintf(szTimezone, SIZE_16B, "TimeZone: %f", localTZ);
	else
		snprintf(szTimezone, SIZE_16B, ";TimeZone: %f", localTZ);

	return PLUGIN_SUCCESS;
}

#ifdef WeMo_SMART_SETUP_V2
void* sendCustomizedStateThread(void *args)
{
    APP_LOG("UPNP",LOG_DEBUG, "send customized state Thread");
    int retVal = PLUGIN_SUCCESS;
    char tmp_url[MAX_FW_URL_LEN];
    UserAppSessionData *pUsrAppSsnData = NULL;
    UserAppData *pUsrAppData = NULL;
    authSign *assign = NULL;
    int customizedState = 0;

    pUsrAppData = (UserAppData *)calloc(1, sizeof(UserAppData));
    if(!pUsrAppData)
    {
	APP_LOG("REMOTEACCESS", LOG_ERR, "Malloc Failed\n");
	resetSystem();
    }

    pUsrAppSsnData = webAppCreateSession(0);
    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Websession Created\n");

    while(1)
    {
	/* loop until device not registered */	
	if ((0x00 == strlen(g_szHomeId) ) || (0x00 == strlen(g_szPluginPrivatekey)) || (atoi(g_szRestoreState) == 0x1))
	{
	    pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);
	    continue;
	}

	/* loop until we find Internet connected */	
	while(1)
	{
	    if (getCurrentClientState() == STATE_CONNECTED) {
		break;
	    }
	    pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
	}

	assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
	if(!assign)
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "Signature returned NULL\n");
	    retVal = PLUGIN_FAILURE;
	    goto on_return;
	}

	if(g_customizedState)
	    customizedState = DEVICE_CUSTOMIZED;
	else
	    customizedState = DEVICE_UNCUSTOMIZED;
	APP_LOG("REMOTEACCESS", LOG_DEBUG, "customized state: %d", customizedState);

	memset(tmp_url,0,sizeof(tmp_url));
	snprintf(tmp_url,sizeof(tmp_url), "https://%s:8443/apis/http/plugin/property/%s/customizedState/%d", BL_DOMAIN_NM, g_szWiFiMacAddress, customizedState);
	strncpy(pUsrAppData->url, tmp_url, sizeof(pUsrAppData->url)-1);
	strncpy( pUsrAppData->keyVal[0].key, "Content-Type", sizeof(pUsrAppData->keyVal[0].key)-1);   
	strncpy( pUsrAppData->keyVal[0].value, "application/xml", sizeof(pUsrAppData->keyVal[0].value)-1);   
	strncpy( pUsrAppData->keyVal[1].key, "Accept", sizeof(pUsrAppData->keyVal[1].key)-1);   
	strncpy( pUsrAppData->keyVal[1].value, "application/xml", sizeof(pUsrAppData->keyVal[1].value)-1);   
	strncpy( pUsrAppData->keyVal[2].key, "Authorization", sizeof(pUsrAppData->keyVal[2].key)-1);   
	strncpy( pUsrAppData->keyVal[2].value, assign->signature, sizeof(pUsrAppData->keyVal[2].value)-1);   
	strncpy( pUsrAppData->keyVal[3].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[3].key)-1);   
	strncpy( pUsrAppData->keyVal[3].value, g_szClientType, sizeof(pUsrAppData->keyVal[3].value)-1);   
	pUsrAppData->keyValLen = 4;

	strncpy( pUsrAppData->inData, " ", sizeof(pUsrAppData->inData)-1);
	pUsrAppData->inDataLength = 1;
	char *check = strstr(pUsrAppData->url, "https://");
	if(check)
	    pUsrAppData->httpsFlag = 1;

	pUsrAppData->disableFlag = 1;    
	APP_LOG("REMOTEACCESS", LOG_DEBUG, " **********Sending customized state notification to Cloud");

	retVal = webAppSendData( pUsrAppSsnData, pUsrAppData, 1);  
	if (retVal)
	{
	    APP_LOG("REMOTEACCESS", LOG_ERR, "\n Curl error encountered in send status to cloud  , errorCode %d \n", retVal);
	    retVal = PLUGIN_FAILURE;
	    break;
	}

	if( (strstr(pUsrAppData->outHeader, "500")) || (strstr(pUsrAppData->outHeader, "503")) )
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Cloud is not reachable, error: 500 or 503");
	    retVal = PLUGIN_FAILURE;
	    pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
	    if (assign) {free(assign); assign = NULL;};
	    continue;
	}
	else if(strstr(pUsrAppData->outHeader, "200"))
	{
	    APP_LOG("REMOTEACESS", LOG_DEBUG, "send customized state notification to cloud success");
	    setCustomizedState(DEVICE_CUSTOMIZED);
	    break;
	}
	else if(strstr(pUsrAppData->outHeader, "403"))
	{
	    APP_LOG("REMOTEACCESS", LOG_DEBUG, "Cloud error: 403");
	    break;
	}
	else
	{
	    APP_LOG("REMOTEACESS", LOG_DEBUG, "Some error encountered: Error response from cloud");
	    retVal = PLUGIN_FAILURE;
	    pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT/3);  //10 sec
	    if (assign) {free(assign); assign = NULL;};
	    continue;
	}

    }

on_return:
    if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
    if (assign) {free(assign); assign = NULL;};
    if (pUsrAppData) {
	if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
	free(pUsrAppData); pUsrAppData = NULL;
    }
    customizedstate_thread = -1;
    APP_LOG("UPNP",LOG_DEBUG, "send customized state Thread done...");
    return NULL;
}

void sendCustomizedStateNotification(void)
{
    if (-1 != customizedstate_thread)
    {
	APP_LOG("UPNP: Device", LOG_DEBUG, "send customized state Thread already created");
	return;
    }
    APP_LOG("UPNP",LOG_DEBUG, "***************Create send customized state Thread ***************");
    int retVal;
    pthread_attr_init(&customizedstate_attr);
    pthread_attr_setdetachstate(&customizedstate_attr, PTHREAD_CREATE_DETACHED);
    retVal = pthread_create(&customizedstate_thread, &customizedstate_attr, &sendCustomizedStateThread, NULL);
    if(retVal < SUCCESS)
    {
	APP_LOG("Remote",LOG_DEBUG, "!! send customized thread not Created !!");
	resetSystem();
    }
    else
    {
	APP_LOG("Remote",LOG_DEBUG, "send customized thread created successfully");
    }
}
#endif

int sendRemoteAccessUpdateDevStatus (void)
{
		int retVal = PLUGIN_SUCCESS;
		char *pcode = NULL;
		char regBuf[MAX_BUF_LEN];
		char tmp_url[MAX_FW_URL_LEN];
		//The flags are used to log WD error message once in WD logfile
		int deferWDLogging = 0, deferWDLogging2 = 0;
		char szTimeZone[SIZE_16B];
		char szLogData[SIZE_256B];

#ifdef PRODUCT_WeMo_Insight
		int InsightParseFlag=0;//Variable taken to handle Insight Parameter 200OK success condition.
#endif
		gSendNotifyHealthPunch++;

		while(1)
		{
				/* loop until we find Internet connected */	
#if 0
				while(1)
				{
						gSendNotifyHealthPunch++;
						if (getCurrentClientState() == STATE_CONNECTED) {
								break;
						}
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
				}
#endif
				while(getCurrentClientState() != STATE_CONNECTED)
				{
						gSendNotifyHealthPunch++;
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
				}
				while(gNTPTimeSet != 1)
				{
						gSendNotifyHealthPunch++;
						pluginUsleep((REMOTE_STATUS_SEND_RETRY_TIMEOUT/2));  //15 sec
				}
				assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
				if (!assign) {
						APP_LOG("REMOTEACCESS", LOG_ERR, "\n Signature Structure returned NULL\n");
						retVal = PLUGIN_FAILURE;
						goto on_return;
				}
				gSendNotifyHealthPunch++;
				memset(gNotificationBuffer, 0x0, MAX_STATUS_BUF);
				UpdateStatusTSHttpData(gNotificationBuffer);

				APP_LOG("REMOTEACCESS", LOG_HIDE, "gNotificationBuffer=%s len=%d\n",gNotificationBuffer, strlen(gNotificationBuffer));
				if (pUsrAppData->outData) {
						free(pUsrAppData->outData);
				}
				memset( pUsrAppData, 0x0, sizeof(UserAppData));

				APP_LOG("REMOTEACCESS", LOG_DEBUG, "Memset Done\n");
				//pUsrAppSsnData = webAppCreateSession(0);
				APP_LOG("REMOTEACCESS", LOG_DEBUG, "Websession Created\n");
				memset(tmp_url,0,sizeof(tmp_url));
#ifdef PRODUCT_WeMo_Insight
				if(g_InstantPowerSend == 1)	
				{
						snprintf(tmp_url, sizeof(tmp_url), "https://%s:8443/apis/http/plugin/insight/insightNotification",BL_DOMAIN_NM);
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "tmp_url =%s \n", tmp_url);
						strncpy(pUsrAppData->url,tmp_url, sizeof(pUsrAppData->url)-1);
						g_InstantPowerSend = 0;
						InsightParseFlag = 1;
						APP_LOG("REMOTEACCESS", LOG_DEBUG, " InsightParseFlag = 1\n");
				}
				else if(1 == g_InsightCostCur){
						snprintf(tmp_url, sizeof(tmp_url), "https://%s:8443/apis/http/plugin/insight/sendInsightParams", BL_DOMAIN_NM);
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "tmp_url =%s \n", tmp_url);
						strncpy(pUsrAppData->url,tmp_url, sizeof(pUsrAppData->url)-1);
						g_InsightCostCur = 0;

				}
				else
#endif 
				{
						snprintf(tmp_url,sizeof(tmp_url), "https://%s:8443/apis/http/plugin/sendNotification", BL_DOMAIN_NM);
						strncpy(pUsrAppData->url, tmp_url, sizeof(pUsrAppData->url)-1);
				}

				memset(szLogData, 0, sizeof(szLogData));

				if(strlen(g_szActuation))
					snprintf(szLogData, sizeof(szLogData), "Actuation: %s", g_szActuation);

				APP_LOG("REMOTEACCESS", LOG_DEBUG, "log data: %s", szLogData);

				if(strlen(g_szRemote))
				{
					strncat(szLogData, ";Remote: ", sizeof(szLogData) - strlen(szLogData) - 1);
					strncat(szLogData, g_szRemote, sizeof(szLogData) - strlen(szLogData) - 1);
					APP_LOG("REMOTEACCESS", LOG_DEBUG, "log data: %s", szLogData);
				}

				if(getTimeZone(szTimeZone) == PLUGIN_SUCCESS)
				{
					strncat(szLogData, szTimeZone, sizeof(szLogData) - strlen(szLogData) - 1);
					APP_LOG("REMOTEACCESS", LOG_DEBUG, "log data: %s", szLogData);
				}

				strncpy( pUsrAppData->keyVal[0].key, "Content-Type", sizeof(pUsrAppData->keyVal[0].key)-1);   
				strncpy( pUsrAppData->keyVal[0].value, "application/xml", sizeof(pUsrAppData->keyVal[0].value)-1);   
				strncpy( pUsrAppData->keyVal[1].key, "Accept", sizeof(pUsrAppData->keyVal[1].key)-1);   
				strncpy( pUsrAppData->keyVal[1].value, "application/xml", sizeof(pUsrAppData->keyVal[1].value)-1);   
				strncpy( pUsrAppData->keyVal[2].key, "Authorization", sizeof(pUsrAppData->keyVal[2].key)-1);   
				strncpy( pUsrAppData->keyVal[2].value, assign->signature, sizeof(pUsrAppData->keyVal[2].value)-1);   
				strncpy( pUsrAppData->keyVal[3].key, "Log-Data", sizeof(pUsrAppData->keyVal[3].key)-1);   
				strncpy( pUsrAppData->keyVal[3].value, szLogData, sizeof(pUsrAppData->keyVal[3].value)-1);   
				strncpy( pUsrAppData->keyVal[4].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[4].key)-1);   
				strncpy( pUsrAppData->keyVal[4].value, g_szClientType, sizeof(pUsrAppData->keyVal[4].value)-1);   
				pUsrAppData->keyValLen = 5;

				strncpy( pUsrAppData->inData, gNotificationBuffer, sizeof(pUsrAppData->inData)-1);
				pUsrAppData->inDataLength = strlen(gNotificationBuffer);
				char *check = strstr(pUsrAppData->url, "https://");
				if (check) {
						pUsrAppData->httpsFlag = 1;
				}
				pUsrAppData->disableFlag = 1;    
				APP_LOG("REMOTEACCESS", LOG_DEBUG, " **********Sending Cloud XML\n");

				retVal = webAppSendData( pUsrAppSsnData, pUsrAppData, 1);  
				if (retVal)
				{
						APP_LOG("REMOTEACCESS", LOG_ERR, "\n Some error encountered in send status to cloud  , errorCode %d \n", retVal);
						APP_LOG("REMOTEACCESS", LOG_ALERT, "Send Notification Status to Cloud failed, CURL error");
						retVal = PLUGIN_FAILURE;
						break;
				}

				if( (strstr(pUsrAppData->outHeader, "500")) || (strstr(pUsrAppData->outHeader, "503")) ){
						APP_LOG("REMOTEACESS", LOG_DEBUG, "Some error encountered: Cloud is not reachable");
						if(!deferWDLogging2) {
								APP_LOG("REMOTEACCESS", LOG_ALERT, "Cloud is not reachable, error: 500 or 503");
						} else {
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "Cloud is not reachable, error: 500 or 503");
						}
						deferWDLogging2++;
						retVal = PLUGIN_FAILURE;
						//if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
						//        if (pUsrAppData) { if(pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}}
#if 0
						if (pUsrAppData) {
								if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
								free(pUsrAppData); pUsrAppData = NULL;
						}
#endif
						enqueueBugsense("REMOTE_NOTIFY_FAIL_5XX");
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
						if (assign) {free(assign); assign = NULL;};
						continue;
				}else if(strstr(pUsrAppData->outHeader, "200")){
#ifdef PRODUCT_WeMo_Insight
						if(InsightParseFlag == 1)
						{
								parseInsightNotificationResp(pUsrAppData->outData, &pcode);
								InsightParseFlag = 0;
						}
#endif

						parseConfigNotificationResp(pUsrAppData->outData);
						APP_LOG("REMOTEACESS", LOG_DEBUG, "send status to cloud success");
						memset(g_szActuation, 0, sizeof(g_szActuation));
						memset(g_szRemote, 0, sizeof(g_szRemote));
#if defined(LONG_PRESS_SUPPORTED)
						LockLongPress();
						gLongPressTriggered = 0x00;
						UnlockLongPress();
#endif
						break;
				}else if(strstr(pUsrAppData->outHeader, "403")){
						parseSendNotificationResp(pUsrAppData->outData, &pcode);
						if ((pcode)){
								memset(regBuf, '\0', MAX_BUF_LEN);
								APP_LOG("REMOTEACESS", LOG_DEBUG, "########AUTH FAILURE (403) : %s: Not sending this Event ########", pcode);
								snprintf(regBuf, sizeof(regBuf), "###AUTH FAILURE(403): Not sending this Event ##, CODE: %s", pcode);
								APP_LOG("REMOTEACCESS", LOG_ALERT, "%s", regBuf);
								if (!strncmp(pcode, "ERR_002", strlen("ERR_002"))) {
										CheckUpdateRemoteFailureCount();
								}
								if (pcode) {free(pcode); pcode= NULL;}
						}
						enqueueBugsense("REMOTE_NOTIFY_FAIL_403");
						//CheckUpdateRemoteFailureCount();
						parseConfigNotificationResp(pUsrAppData->outData);
#if defined(LONG_PRESS_SUPPORTED)
						LockLongPress();
						gLongPressTriggered = 0x00;
						UnlockLongPress();
#endif
						break;
				} else {
						APP_LOG("REMOTEACESS", LOG_DEBUG, "Some error encountered: Error response from cloud");
						if(!deferWDLogging) {
								APP_LOG("REMOTEACCESS", LOG_ALERT, "Cloud not reachable, unknown error");
						} else {
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "Cloud not reachable, unknown error");
						}
						deferWDLogging++;
						retVal = PLUGIN_FAILURE;
						//if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
						//        if (pUsrAppData) { if(pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}}

#if 0
						if (pUsrAppData) {
								if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
								free(pUsrAppData); pUsrAppData = NULL;
						}
#endif
						enqueueBugsense("REMOTE_NOTIFY_FAIL_OTHER");
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT/3);  //10 sec
						if (assign) {free(assign); assign = NULL;};
						continue;
				}

		}
on_return:

		//if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
		if (assign) {free(assign); assign = NULL;};
#if 0
		if (pUsrAppData) {
				if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
				free(pUsrAppData); pUsrAppData = NULL;
		}
#endif
		if (g_configChange) {
				/* Condition variable to trigger 
					 session creation for deviceConfig */
				pthread_mutex_lock(&g_devConfig_mutex1);
				pthread_cond_signal(&g_devConfig_cond1);
				pthread_mutex_unlock(&g_devConfig_mutex1);
		}
		return retVal;
}

void* uploadLogFileThread_6Hours(void *arg)
{
		int oldFileExists = 1, fileExists = 1;
		struct stat fileInfo;
		int ret1 = 0, ret2 = 0;
		char tmp_url[MAX_FW_URL_LEN];
		char tmp_url_old[MAX_FW_URL_LEN];

		char *wdTimeLeft = NULL;
		int wdTime = 0, sleepTime = 0;
		char buf[MAX_KEY_LEN];
		int ts = 0, falseCase = 0;

                tu_set_my_thread_name( __FUNCTION__ );
		while(1) {
				/* Check for the timer value to push the log file to the cloud */
				while(1) {
						if (IsNtpUpdate())
						{
								sleep(60);
								ts = GetUTCTime();

								falseCase = 0;

								wdTimeLeft = GetBelkinParameter(LAST_PUSH_TIME);
								if(!wdTimeLeft || (wdTimeLeft && strlen(wdTimeLeft) == 0)) {
										memset(buf, 0x0, sizeof(buf));
										snprintf(buf, sizeof(buf), "%d", ts);
										SetBelkinParameter(LAST_PUSH_TIME, buf);
										SaveSetting();
										falseCase = 1;
								}

								if(falseCase)
										wdTime = ts;
								else
										wdTime = atoi(wdTimeLeft);

								sleepTime = (WD_DEFAULT_PUSH_TIME - (ts - wdTime));

								if(sleepTime <= 0)
										break;

								APP_LOG("WiFiApp", LOG_DEBUG, "sleepTime = %d\n", sleepTime);
								sleep(sleepTime);
								break;
						}
						sleep(60);
				}

				memset(tmp_url, 0x0, MAX_FW_URL_LEN);
				memset(tmp_url_old, 0x0, MAX_FW_URL_LEN);

				snprintf(tmp_url, sizeof(tmp_url), "http://%s:%s/%s/", BL_DOMAIN_NM_WD, BL_DOMAIN_PORT_WD, BL_DOMAIN_URL_WD);
				strncat(tmp_url, g_szWiFiMacAddress, sizeof(tmp_url)-strlen(tmp_url)-1);

				/* Checking for the existence of the watchdog log files.
				 * If none of the file exists, then exiting the LOG thread
				 */
				if(stat(WDLOGFILE_OLD, &fileInfo) == -1 && errno == ENOENT)
						oldFileExists = 0;

				memset(&fileInfo, 0x0, sizeof(struct stat));

				if(stat(WDLOGFILE, &fileInfo) == -1 && errno == ENOENT)
						fileExists = 0;

				if(oldFileExists == 0 && fileExists == 0) {
						APP_LOG("WiFiApp", LOG_ERR, "\n Watchdog log files doesn't exists\n");

						ts = GetUTCTime();
						memset(buf, 0x0, sizeof(buf));
						snprintf(buf, sizeof(buf), "%d", ts);
						SetBelkinParameter(LAST_PUSH_TIME, buf);
						SaveSetting();
				}

				/* loop until we find Internet connected */	
				while(1)
				{
						if (getCurrentClientState() == STATE_CONNECTED)
								break;
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
				}

				if (oldFileExists) {
						strncpy(tmp_url_old, tmp_url, sizeof(tmp_url_old)-1);
						strncat(tmp_url_old, "_old", sizeof(tmp_url_old)-strlen(tmp_url_old)-1);
						ret1 = uploadLogFileDevStatus(WDLOGFILE_OLD, tmp_url);
				}


				if (fileExists) {
						ret2 = uploadLogFileDevStatus(WDLOGFILE, tmp_url);
				}

				if(ret1 < 0 && ret2 < 0) {
						APP_LOG("WiFiApp", LOG_DEBUG, "Upload Log File status fail");
				} else {
						APP_LOG("WiFiApp", LOG_DEBUG, "Upload Log File status success...");
						//Clearing the log files if Uploaded successfully.
						memset(buf, 0x0, sizeof(buf));
						snprintf(buf, sizeof(buf), "rm %s", WDLOGFILE_OLD);
						system(buf);
						memset(buf, 0x0, sizeof(buf));
						//snprintf(buf, sizeof(buf), "> %s", WDLOGFILE);
						snprintf(buf, sizeof(buf), "rm %s", WDLOGFILE);
						closeWDFile(1);
						pluginUsleep(1000000);  //1 sec
						system(buf);
						openWDFile(1);

						ts = GetUTCTime();
						memset(buf, 0x0, sizeof(buf));
						snprintf(buf, sizeof(buf), "%d", ts);
						SetBelkinParameter(LAST_PUSH_TIME, buf);
						SaveSetting();
				}
		}

		APP_LOG("WiFiApp",LOG_DEBUG, "***************EXITING LOG Thread***************\n");
		pthread_exit(0);
		return NULL;
}

void* uploadLogFileThread(void *arg)
{
		struct stat fileInfo;
		int ret1 = 0, ret2 = 0;
		char tmp_url[MAX_FW_URL_LEN];
		char tmp_url_old[MAX_FW_URL_LEN];
		int oldFileExists = 1, fileExists = 1;
                tu_set_my_thread_name( __FUNCTION__ );

		memset(tmp_url, 0x0, MAX_FW_URL_LEN);
		memset(tmp_url_old, 0x0, MAX_FW_URL_LEN);
		memset(&fileInfo, 0x0, sizeof(struct stat));

		snprintf(tmp_url, sizeof(tmp_url), "http://%s:%s/%s/", BL_DOMAIN_NM_WD, BL_DOMAIN_PORT_WD, BL_DOMAIN_URL_WD);
		strncat(tmp_url, g_szWiFiMacAddress, sizeof(tmp_url)-strlen(tmp_url)-1);

		/* Checking for the existence of the watchdog log files.
		 * If none of the file exists, then exiting the LOG thread
		 */
		if(stat(WDLOGFILE_OLD, &fileInfo) == -1 && errno == ENOENT)
				oldFileExists = 0;

		memset(&fileInfo, 0x0, sizeof(struct stat));

		if(stat(WDLOGFILE, &fileInfo) == -1 && errno == ENOENT)
				fileExists = 0;

		if(oldFileExists == 0 && fileExists == 0) {
				APP_LOG("WiFiApp", LOG_ERR, "\n Watchdog log files doesn't exists\n");
				pthread_exit(0);
				return NULL;
		}

		/* loop until we find Internet connected */	
		while(1)
		{
				if (getCurrentClientState() == STATE_CONNECTED)
						break;
				pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
		}

		if (oldFileExists) {
				strncpy(tmp_url_old, tmp_url, sizeof(tmp_url_old)-1);
				strncat(tmp_url_old, "_old", sizeof(tmp_url_old)-strlen(tmp_url_old)-1);
				ret1 = uploadLogFileDevStatus(WDLOGFILE_OLD, tmp_url_old);
		}

		if (fileExists) {
				ret2 = uploadLogFileDevStatus(WDLOGFILE, tmp_url);
		}

		if (ret1 < 0 && ret2 < 0) {
				APP_LOG("WiFiApp", LOG_DEBUG, "Upload Log File status fail");
		} else {
				APP_LOG("WiFiApp", LOG_DEBUG, "Upload Log File status success...");
		}

		APP_LOG("WiFiApp",LOG_DEBUG, "***************EXITING LOG Thread***************\n");
		pthread_exit(0);
		return NULL;
}

void *sendRemAccessUpdStatusThdMonitor(void *arg)
{
                tu_set_my_thread_name( __FUNCTION__ );
		APP_LOG("REMOTEACCESS",LOG_DEBUG,"sendRemAccessUpdStatusThread Monitor started...");

		while(1)
		{
				pluginUsleep(SEND_REM_UPD_STAT_TH_MON_TIMEOUT * 1000000);
				if(gSendNotifyHealthPunch == 0)
				{	
						APP_LOG("REMOTEACCESS",LOG_CRIT,"sendRemAccessUpdStatusThread monitor detected bad health ...");
						if(sendremoteupdstatus_thread)
								pthread_kill(sendremoteupdstatus_thread, SIGRTMIN);
						sendremoteupdstatus_thread = -1;

						APP_LOG("REMOTEACCESS",LOG_DEBUG,"Removed sendremoteupdstatus thread...");

						//Again create a sendremoteupdstatus thread
						ithread_create(&sendremoteupdstatus_thread, NULL, sendRemoteAccessUpdateStatusThread, NULL);
						ithread_detach (sendremoteupdstatus_thread);

						APP_LOG("REMOTEACCESS",LOG_DEBUG,"sendremoteupdstatus thread created again...");

				}
				else
				{
						APP_LOG("REMOTEACCESS",LOG_DEBUG,"sendremoteupdstatus thread health OK [%d]...", gSendNotifyHealthPunch);
						gSendNotifyHealthPunch = 0;
				}
		}
		return NULL;	
}

void* sendRemoteAccessUpdateStatusThread(void *arg)
{
                tu_set_my_thread_name( __FUNCTION__ );
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "sendRemoteAccessUpdateStatusThread Thread Started");

		pUsrAppData = (UserAppData *)calloc(1, sizeof(UserAppData));
		if (!pUsrAppData) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Malloc Failed\n");
				return NULL;
		}
		pUsrAppSsnData = webAppCreateSession(0);

		while(1){
				gSendNotifyHealthPunch++;
				if ((0x00 == strlen(g_szHomeId) ) || (0x00 == strlen(g_szPluginPrivatekey)) || (atoi(g_szRestoreState) == 0x1)) {
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);
						continue;
				}

				if(!isNotificationEnabled ()) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Notification Sending is disabled");
						pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);
						continue;
				}

				if((gpluginPrevStatusTS == getPluginStatusTS()) && (0 == strlen(gFirmwareVersion))){
#ifndef PRODUCT_WeMo_Insight
						pluginUsleep(5000000);
						continue;
#else
						if((g_InsightHomeSettingsSend == 0)&&(g_SendInsightParams == 0))
						{
								pluginUsleep(5000000);
								continue;
						}
						else{
								if(g_InsightHomeSettingsSend)
								{
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "Sending Insight Home Settings");
										g_InsightHomeSettingsSend=0;
										g_InsightCostCur = 1;

								}
								else if(g_SendInsightParams)
								{
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "Sending Insight Parameters");
										g_SendInsightParams = 0;
										g_InstantPowerSend = 1;
								}
						}
#endif
				}

				if ((0x00 != strlen(g_szHomeId) ) && (0x00 != strlen(g_szPluginPrivatekey)) && (atoi(g_szRestoreState) == 0x0) && \
								(gpluginRemAccessEnable == 1)) {
						if(sendRemoteAccessUpdateDevStatus() < PLUGIN_SUCCESS){
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "Send Remote Update device status fail");
						}else{
								APP_LOG("REMOTEACCESS", LOG_DEBUG, "Send Remote Update device status success...");
						}
				}else {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access does not look to be enabled");
				}

		}
		if (pUsrAppSsnData) {webAppDestroySession ( pUsrAppSsnData ); pUsrAppSsnData = NULL;}
		if (pUsrAppData) {
				if (pUsrAppData->outData) {free(pUsrAppData->outData); pUsrAppData->outData = NULL;}
				free(pUsrAppData); pUsrAppData = NULL;
		}
		pthread_exit(0);
		return NULL;
}

void remoteAccessUpdateStatusTSParams(int status) 
{

		if(0xFF == status) {
				if (DEVICE_SENSOR == g_eDeviceType) 
						status = 1;
				else 
						status = GetDeviceStateIF();

				memset(gFirmwareVersion, 0, SIZE_64B);
				strncpy(gFirmwareVersion, g_szFirmwareVersion, sizeof(gFirmwareVersion)-1);    
		} 
		else {
				if( (DEVICE_SENSOR == g_eDeviceType) && (status) ) { 
						setStatusTS (status);
				}
				else if(DEVICE_SOCKET == g_eDeviceType)  {
						setStatusTS (status);
				}
				else if(DEVICE_CROCKPOT == g_eDeviceType) {	        
						setStatusTS(status);
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "TS status changed to %d",status);
				}

				if(strcmp(gFirmwareVersion, "") == 0)
						memset(gFirmwareVersion, 0, SIZE_64B);

				if( ((DEVICE_SENSOR == g_eDeviceType) && status)  || (DEVICE_SOCKET == g_eDeviceType) )
						setPluginStatus(status);
		}

}

#ifdef __NFTOCLOUD__
int PrepareSendNotificationData(char *sMac_N, char *httpPostStatus, int ts, int natStatus)
{
		int  status = 0;
		int retVal = PLUGIN_SUCCESS;

		status = getPluginStatus();
		APP_LOG("REMOTEACCESS", LOG_DEBUG, "timestamp: %d, status: %d, natStatus: %d", ts, status, natStatus);   
		if (g_eDeviceType == DEVICE_SOCKET) {
				snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Switch</deviceType><networkStatus>%d</networkStatus><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion></plugin>", sMac_N, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, natStatus, status, ts, g_szFirmwareVersion);
				retVal = PLUGIN_SUCCESS;

		} else if (g_eDeviceType == DEVICE_SENSOR) {
				snprintf(httpPostStatus, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><plugin><macAddress>%s</macAddress><serialNumber>%s</serialNumber><friendlyName>%s</friendlyName><udnName>%s</udnName><homeId>%s</homeId><deviceType>Sensor</deviceType><networkStatus>%d</networkStatus><status>%d</status><statusTS>%d</statusTS><firmwareVersion>%s</firmwareVersion></plugin>", sMac_N, g_szSerialNo, g_szFriendlyName, g_szUDN_1, g_szHomeId, natStatus, status, ts, g_szFirmwareVersion);
				retVal = PLUGIN_SUCCESS;
		} else {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Does not look to be a valid device type\n");
				retVal = PLUGIN_FAILURE;
		}
		return retVal;
}

int  sendNATStatusI (int natStatus, int ts) {
		char httpPostStatus[MAX_STATUS_BUF];
		int retVal = PLUGIN_SUCCESS;
		authSign *assign_N = NULL;
		UserAppSessionData *pUsrAppSsnData_N = NULL;
		UserAppData *pUsrAppData_N = NULL;
		char *sMac_N = NULL;
		char tmp_url[MAX_FW_URL_LEN];

		APP_LOG("REMOTEACCESS", LOG_DEBUG, " **********In sendNATStatusI nat init failure.......\n");
		sMac_N = g_szWiFiMacAddress;
		if (!sMac_N) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n delimited sMac_N returned NULL\n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}
		assign_N = createAuthSignature(sMac_N, g_szSerialNo, g_szPluginPrivatekey);
		if (!assign_N) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Signature Structure returned NULL\n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		memset(httpPostStatus, 0x0, MAX_STATUS_BUF);
		retVal = PrepareSendNotificationData(sMac_N, httpPostStatus, ts, natStatus);
		if (retVal != PLUGIN_SUCCESS) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n PrepareSendNotificationData returned failure\n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		APP_LOG("REMOTEACCESS", LOG_HIDE, "httpPostStatus=%s len=%d\n",httpPostStatus, strlen(httpPostStatus));

		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Memset Done\n");
		pUsrAppSsnData_N = webAppCreateSession(0);

		APP_LOG("REMOTEACCESS", LOG_DEBUG, "Websession Created\n");

		pUsrAppData_N = (UserAppData *)malloc(sizeof(UserAppData));
		if (!pUsrAppData_N) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Malloc Failed\n");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}
		memset( pUsrAppData_N, 0x0, sizeof(UserAppData));
		memset( tmp_url, 0x0, sizeof(tmp_url));
		snprintf(tmp_url, sizeof(tmp_url), "https://%s:8443/apis/http/plugin/sendNotification", BL_DOMAIN_NM);
		strncpy(pUsrAppData_N->url, tmp_url, sizeof(pUsrAppData_N->url)-1);
		strncpy( pUsrAppData_N->keyVal[0].key, "Content-Type", sizeof(pUsrAppData_N->keyVal[0].key)-1);   
		strncpy( pUsrAppData_N->keyVal[0].value, "application/xml", sizeof(pUsrAppData_N->keyVal[0].value)-1);   
		strncpy( pUsrAppData_N->keyVal[1].key, "Accept", sizeof(pUsrAppData_N->keyVal[1].key)-1);   
		strncpy( pUsrAppData_N->keyVal[1].value, "application/xml", sizeof(pUsrAppData_N->keyVal[1].value)-1);   
		strncpy( pUsrAppData_N->keyVal[2].key, "Authorization", sizeof(pUsrAppData_N->keyVal[2].key)-1);   
		strncpy( pUsrAppData_N->keyVal[2].value, assign_N->signature, sizeof(pUsrAppData_N->keyVal[2].value)-1);   
		strncpy( pUsrAppData_N->keyVal[3].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData_N->keyVal[3].key)-1);   
		strncpy( pUsrAppData_N->keyVal[3].value, g_szClientType, sizeof(pUsrAppData_N->keyVal[3].value)-1);   
		pUsrAppData_N->keyValLen = 4;

		strncpy( pUsrAppData_N->inData, httpPostStatus, sizeof(pUsrAppData_N->inData)-1);
		pUsrAppData_N->inDataLength = strlen(httpPostStatus);
		char *check = strstr(pUsrAppData_N->url, "https://");
		if (check) {
				pUsrAppData_N->httpsFlag = 1;
		}
		pUsrAppData_N->disableFlag = 1;    
		APP_LOG("REMOTEACCESS", LOG_DEBUG, " **********Sending Cloud XML\n");

		retVal = webAppSendData( pUsrAppSsnData_N, pUsrAppData_N, 1);  
		if (retVal) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Some error encountered in send status to cloud  , errorCode %d \n", retVal);
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		if(strstr(pUsrAppData_N->outHeader, "200")){
				APP_LOG("REMOTEACESS", LOG_DEBUG, "send status to cloud success");
				retVal = PLUGIN_SUCCESS;
		}else {
				APP_LOG("REMOTEACESS", LOG_DEBUG, "send status to cloud failure");
				retVal = PLUGIN_FAILURE;
		}

on_return:
		if (pUsrAppSsnData_N) webAppDestroySession ( pUsrAppSsnData_N );
		if (assign_N) {free(assign_N); assign_N = NULL;};
		if (pUsrAppData_N) {
				if (pUsrAppData_N->outData) {free(pUsrAppData_N->outData); pUsrAppData_N->outData = NULL;}
				free(pUsrAppData_N); pUsrAppData_N = NULL;
		}

		return retVal;
}

int  sendNATStatus (int natStatus) {
		int retry_count=0;
		int ts = 0;
		int retVal = PLUGIN_FAILURE;

		APP_LOG("REMOTEACCESS", LOG_DEBUG, " **********In sendNATStatus nat init failure.......\n");
		ts=GetUTCTime();
		while(retry_count < MAX_NAT_STATUS_COUNT) {
				retry_count++;
				retVal = sendNATStatusI (natStatus, ts);
				if (retVal != PLUGIN_SUCCESS) {
						retVal = PLUGIN_FAILURE;	
				}else {
						retVal = PLUGIN_SUCCESS;
						break;
				}
				pluginUsleep(REMOTE_STATUS_SEND_RETRY_TIMEOUT);  //30 sec
		}
		return retVal;
}
#endif
int sendConfigChangeDevStatus()
{
		UserAppSessionData *pUsrAppSsnData = NULL; 
		UserAppData *pUsrAppData = NULL;
		authSign *assign = NULL;
		int retVal = PLUGIN_SUCCESS;         
		char *ptr = NULL;    
		char *dev_Mac=NULL;
		char tmp_url[MAX_FW_URL_LEN];
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Entered sendConfigChangeDevStatus API\n");
		//Resolve LB domain name to get SIP Server and other server IPs
		dev_Mac = utilsRemDelimitStr(GetMACAddress(), ":");
		assign = createAuthSignature(dev_Mac, g_szSerialNo, g_szPluginPrivatekey);
		if (!assign) {          
				APP_LOG("REMOTEACCESS", LOG_ERR,"\n Signature Structure returned NULL\n");
				retVal = PLUGIN_FAILURE;         
				goto exit_below;                                
		}            
		APP_LOG("deviceconfig",LOG_HIDE, "assign->signature... %s", assign->signature);
		pUsrAppData = (UserAppData *)malloc(sizeof(UserAppData));
		if (!pUsrAppData) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Malloc Failed\n");
				retVal = PLUGIN_FAILURE;         
				goto exit_below;                                
		}
		memset( pUsrAppData, 0x0, sizeof(UserAppData));
		pUsrAppSsnData = webAppCreateSession(0); 
		if (!pUsrAppSsnData) {
				APP_LOG("REMOTEACCESS", LOG_ERR, "\n Malloc Failed\n");
				retVal = PLUGIN_FAILURE;         
				goto exit_below;                                
		}
		APP_LOG("REMOTEACCESS", LOG_DEBUG,"Sending deviceConfig to Cloud  \n");
		memset(tmp_url,0,sizeof(tmp_url));
		/* prepare REST header*/
		{                       
				snprintf(tmp_url, sizeof(tmp_url), "https://%s:8443/apis/http/plugin/ext/deviceConfig",BL_DOMAIN_NM);
				strcpy( pUsrAppData->url, tmp_url);             
				strcpy( pUsrAppData->keyVal[0].key, "Content-Type");
				strcpy( pUsrAppData->keyVal[0].value, "application/xml");
				strcpy( pUsrAppData->keyVal[1].key, "Authorization");
				strcpy( pUsrAppData->keyVal[1].value,  assign->signature);
				strncpy(pUsrAppData->keyVal[2].key, "X-Belkin-Client-Type-Id", sizeof(pUsrAppData->keyVal[2].key)-1);   
				strncpy(pUsrAppData->keyVal[2].value, g_szClientType, sizeof(pUsrAppData->keyVal[2].value)-1);   
				pUsrAppData->keyValLen = 3;
				/* enable SSL if auth URL is on HTTPS*/
				ptr = strstr(tmp_url,"https" );
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
				APP_LOG("REMOTEACCESS", LOG_ERR,"Some error encountered while sending to %s errorCode %d \n", tmp_url, retVal);
				g_configChange=0;/*Resetting the config change status */
				retVal = PLUGIN_FAILURE;         
				goto exit_below;
		}
		/* check response header to see if user is authorized or not*/
		{
				if(strstr(pUsrAppData->outHeader, "200"))
				{
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response 200 OK received from %s\n", tmp_url);
						DespatchSendNotificationResp(pUsrAppData->outData);
						g_configChange=0;/*Resetting the config change status */
						/*Parse the received xml and store in a FIFO */
				}
				else if(strstr(pUsrAppData->outHeader, "403"))
				{
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response 403 received from %s\n", tmp_url);
						DespatchSendNotificationResp(pUsrAppData->outData);
						g_configChange=0;
				}else {
						APP_LOG("REMOTEACCESS", LOG_DEBUG,"Response other than 200 OK received from %s\n", tmp_url);
						g_configChange=0;/*Resetting the config change status */
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
		if (dev_Mac) {free(dev_Mac); dev_Mac = NULL;}
		if (assign) { free(assign); assign = NULL; }

		return retVal;
}

int UploadIcon(char *UploadURL,char *fsFileUrl,char*cookie)
{
		int retVal = PLUGIN_SUCCESS;
		UserAppSessionData *pIconSsnData = NULL;
		UserAppData *pIconData = NULL;
		struct stat fileInfo;
		char pluginKey[MAX_PKEY_LEN];
		authSign *assign = NULL;
		char tmp_url[MAX_FW_URL_LEN];
		char *ptr = NULL;    

		memset(pluginKey, 0x0, MAX_PKEY_LEN);
		strncpy(pluginKey, g_szPluginPrivatekey, sizeof(pluginKey)-1);

		assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, pluginKey);
		if (!assign) {
				APP_LOG("UploadIcon", LOG_DEBUG,"Signature Structure returned NULL");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		pIconData = (UserAppData *)malloc(sizeof(UserAppData));
		if(!pIconData) {
				APP_LOG("UploadIcon", LOG_ERR, "Malloc failed, returned NULL, exiting");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		memset(pIconData, 0, sizeof(UserAppData));
		pIconSsnData = webAppCreateSession(0);
		if(!pIconSsnData) {
				APP_LOG("UploadIcon", LOG_ERR, "Failed to create session, returned NULL");
				retVal = PLUGIN_FAILURE;
				goto on_return;
		}

		strncpy(pIconData->keyVal[0].key, "Content-Type", sizeof(pIconData->keyVal[0].key)-1);
		strncpy(pIconData->keyVal[0].value, "multipart/octet-stream", sizeof(pIconData->keyVal[0].value)-1);
		strncpy(pIconData->keyVal[1].key, "Accept", sizeof(pIconData->keyVal[1].key)-1);
		strncpy(pIconData->keyVal[1].value, "application/xml", sizeof(pIconData->keyVal[1].value)-1);
		strncpy(pIconData->keyVal[2].key, "Authorization", sizeof(pIconData->keyVal[2].key)-1);
		strncpy(pIconData->keyVal[2].value, assign->signature, sizeof(pIconData->keyVal[2].key)-1);
		strncpy(pIconData->keyVal[3].key, "X-Belkin-Client-Type-Id", sizeof(pIconData->keyVal[3].key)-1);   
		strncpy(pIconData->keyVal[3].value, g_szClientType, sizeof(pIconData->keyVal[3].value)-1);   
		pIconData->keyValLen = 4;

		APP_LOG("UploadIcon",LOG_HIDE, "assign->signature... %s", assign->signature);

		strncpy(pIconData->mac, g_szWiFiMacAddress, sizeof(pIconData->mac)-1);

		//Sending the Insight data file 
		memset(&fileInfo, '\0', sizeof(struct stat));
		lstat(ICON_FILE_URL, &fileInfo);
		memset(tmp_url,0,sizeof(tmp_url));
		snprintf(tmp_url, sizeof(tmp_url), "%s", UploadURL);
		if((off_t)fileInfo.st_size != 0) {
				memset(pIconData->url, 0, sizeof(pIconData->url)/sizeof(char));
				memset(pIconData->inData, 0, sizeof(pIconData->inData)/sizeof(char));
				strncpy(pIconData->url, tmp_url, sizeof(pIconData->url)-1);
				strncpy(pIconData->inData, ICON_FILE_URL, sizeof(pIconData->inData)-1);
				pIconData->inDataLength = 0;

				char *check = strstr(pIconData->url, "https://");
				if (check) {
						pIconData->httpsFlag = 1;
				}

				APP_LOG("UploadIcon",LOG_DEBUG, "Sending... %s", ICON_FILE_URL);
				retVal = webAppSendData(pIconSsnData, pIconData, 2);

				if(retVal == ERROR_INVALID_FILE) {
						APP_LOG("UploadIcon", LOG_ERR, "File \'%s\' doesn't exists", ICON_FILE_URL);
				}
				ptr = strstr(pIconData->outHeader,"200 OK" );
				if(ptr != NULL)
				{
						APP_LOG("UploadIcon", LOG_DEBUG,"Response 200 OK received from %s\n", UploadURL);
						memset(cookie,0,SIZE_512B);
						strncpy(cookie,pIconData->cookie_data,sizeof(cookie)-1);
						APP_LOG("REMOTEACCESS", LOG_HIDE,"Cookie is =%s\n", cookie);
						APP_LOG("UPLOAD ICON", LOG_HIDE," Response data received from %s\n", pIconData->outData);
						if(NULL !=  pIconData->outData){
								strcpy(fsFileUrl, pIconData->outData);
								APP_LOG("UPLOAD ICON", LOG_HIDE,"NC: fsFileUrl=%s\n", fsFileUrl);
						}
						else{
								APP_LOG("UPLOAD ICON", LOG_DEBUG,"ERROR: #####Unable to Upload icon\n");
						}
						retVal = PLUGIN_SUCCESS;
				}else {
						APP_LOG("UploadIcon", LOG_DEBUG,"Response other than 200 OK received from %s\n", UploadURL);
						retVal = PLUGIN_FAILURE;
						goto on_return;
				}
				if(retVal) {
						APP_LOG("UploadIcon", LOG_ERR, "Some error encountered in send status to cloud  , errorCode %d", retVal);
						retVal = PLUGIN_FAILURE;
						goto on_return;
				}
		}

on_return:
		if (pIconSsnData) webAppDestroySession (pIconSsnData);
		if (pIconData) {
				if (pIconData->outData) {free(pIconData->outData); pIconData->outData = NULL;}
				free(pIconData); pIconData = NULL;
		}
		if (assign) {free(assign); assign = NULL;}

		return retVal;	
}
