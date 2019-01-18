/***************************************************************************
*
*
* plugin_ctrlpoint.c
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
#include "defines.h"
#include "controlledevice.h"
#include "plugin_device.h"
#include "plugin_ctrlpoint.h"
#include "plugin_cmd.h"
#include "rule.h"
#include "gpio.h"
#include "remoteAccess.h"

#include "utils.h"
#include "osUtils.h"
#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#include "upnpCommon.h"
#include "wifiHndlr.h"
#include "xgethostbyname.h"
#include "getRemoteMac.h"
#include <sys/syscall.h>

#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif
#include "thready_utils.h"

#ifdef SIMULATED_OCCUPANCY
extern SimulatedDevData *pgGlobalSimDevList;
extern int gSimulatedRuleRunning;
extern int gSimDevUpnpRespCount;
#endif

extern int g_eDeviceType;
extern char	g_szUDN[SIZE_128B];
extern void UnlockDeviceSync(void);
ithread_mutex_t DeviceListMutex;
ithread_mutex_t SignNotifyMutex;
pthread_mutex_t HomeidListMutex;
int gProcessData = TRUE;

#define STATE_RELAY_OVERRIDE_OFF 0x02

#define STATE_RELAY_OVERRIDE_ON 0x03

char gMacAddr[MAX_MAC_LEN];
char gSerialNo[MAX_MAC_LEN];
char gPluginUDN[MAX_DEVICE_UDN_SIZE];
char g_homeid[SIZE_64B];
char g_deviceid[SIZE_64B];
char g_signature[SIGNATURE_LEN];
int ctrlpt_handle = -1;
extern char g_routerMac[MAX_MAC_LEN];
extern char g_routerSsid[MAX_ESSID_LEN];
extern char g_szRestoreState[MAX_RES_LEN];
char PluginDeviceType[] = "urn:Belkin:device:";
char gHomeIdList[SIZE_256B];
char gSignatureList[SIZE_2048B];

#ifdef WeMo_INSTACONNECT
extern int g_usualSetup;
extern char gRouterSSID[SSID_LEN];
extern int g_connectInstaStatus;
#endif

#ifdef PRODUCT_WeMo_Insight
char* g_szSensorServiceList = "urn:Belkin:service:basicevent:1;urn:Belkin:service:remoteaccess:1;urn:Belkin:service:insight:1";

char *PluginDeviceServiceType[] = { "urn:Belkin:service:WiFiSetup:1",
                "urn:Belkin:service:timesync:1",
                "urn:Belkin:service:basicevent:1",
                "urn:Belkin:service:firmwareupdate:1",
                "urn:Belkin:service:rules:1",
                "urn:Belkin:service:metainfo:1",
		"urn:Belkin:service:remoteaccess:1"
		"urn:Belkin:service:insight:1"
                };
#else
char* g_szSensorServiceList = "urn:Belkin:service:basicevent:1;urn:Belkin:service:remoteaccess:1";

char *PluginDeviceServiceType[] = { "urn:Belkin:service:WiFiSetup:1",
                "urn:Belkin:service:timesync:1",
                "urn:Belkin:service:basicevent:1",
                "urn:Belkin:service:firmwareupdate:1",
                "urn:Belkin:service:rules:1",
                "urn:Belkin:service:metainfo:1",
				"urn:Belkin:service:remoteaccess:1"
                };

#endif


int default_timeout = 1801;
int ctrlptstatusid;

int g_iRediscoverHandle = 0x00;

extern char	g_szUDN[SIZE_128B];

int g_deviceindex = 0;
static int nRA = 0x0;
static int g_homeiddeviceidchk = 1;

pCtrlPluginDeviceNode g_pGlobalPluginDeviceList = NULL;

char	g_szOverrideTable[SIZE_512B];	

extern char g_szHomeId[SIZE_20B];
extern char g_szPluginPrivatekey[SIZE_50B];
extern char g_szSmartDeviceId[SIZE_256B];
extern char g_szSmartPrivateKey[SIZE_50B];

#define MAX_SERVICE_UPDATE_TIME			120	//10 hours?
#define MAX_SERVICE_UPDATE_TIME_NEW		10
extern unsigned short gpluginRemAccessEnable;
ithread_t s_RediscoverHandle 	= -1;
static pthread_attr_t tmp_hmattr;
static ithread_t s_GetHomeIdHandle 		= -1;
static pthread_attr_t tmp_reremattr;
ithread_t s_ResendRemoteRequestHandle 		= -1;
static pthread_attr_t tmp_attr;
static pthread_t tmpthread = -1;

static int	s_cntRediscoveryCounter = 0x00;
#define		MAX_REDISCOVERY_TIMES	10		//10 * 120 s = 1200 seconds 20 minutes

inline void initHomeIdListLock()
{
	osUtilsCreateLock(&HomeidListMutex);
}

inline void  AcquireHomeIdListLock()
{
	osUtilsGetLock(&HomeidListMutex);
}

inline void  ReleaseHomeIdListLock()
{
	osUtilsReleaseLock(&HomeidListMutex);
}

void *CtrlPtRediscoverTask(void *args)
{
    	tu_set_my_thread_name( __FUNCTION__ );
	s_cntRediscoveryCounter = 0x00;

	while (1)
	{
		int rnd = rand() % 60;
		char homeIdList[SIZE_256B];
		char signaturelist[SIZE_2048B];

		memset(homeIdList, 0, sizeof(homeIdList));

		if (0x00 == rnd)
			rnd= 10;
		
		pluginUsleep(rnd*1000000);

		if(gProcessData)
		{
			/* send data gathered from devices in last iteration */
			AcquireHomeIdListLock();
			if(strlen(g_szHomeId) && strlen(gHomeIdList) && !strcmp(g_szRestoreState, "0"))
			{
				authSign *assign = NULL;

				/* create our signature */
				assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
				if (!assign) {
					APP_LOG("UPNP: DEVICE", LOG_ERR, "\n Signature Structure returned NULL\n");
				}


				/* Append our home id and signature in the lists before sending */
				snprintf(homeIdList, sizeof(homeIdList)-1, "%s-%s", gHomeIdList, g_szHomeId);
				if(assign)
				{
					APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################Self Signature: <%s>", assign->signature);

					snprintf(signaturelist, sizeof(signaturelist)-1, "%s-%s", gSignatureList, assign->signature);

					free(assign);
				}
				else
				{
					/* Insert dummy signature to keep syntax sane */
					snprintf(signaturelist, sizeof(signaturelist)-1, "%s-%s", gSignatureList, "DUMMY");
				}

				/* reset the home id  and signature list */
				memset(gHomeIdList, 0, sizeof(gHomeIdList));
				memset(gSignatureList, 0, sizeof(gSignatureList));
			}
			ReleaseHomeIdListLock();

			/* send the list to cloud */
			if(strlen(homeIdList) && !strcmp(g_szRestoreState, "0"))
				sendHomeIdListToCloud(homeIdList, signaturelist);
		}
		
		pluginUsleep(MAX_SERVICE_UPDATE_TIME*1000000);

		if (s_cntRediscoveryCounter >= MAX_REDISCOVERY_TIMES)
		{
			//- Reset
			APP_LOG("UPNP: Device", LOG_DEBUG, "##################### to clean up device list ###################");
			s_cntRediscoveryCounter = 0x00;
			nRA = 0x0;
			CtrlPointRemoveAll();
			/* Range Extender fix: We have to stop discovering and sending information to cloud now */
			gProcessData = FALSE;
			APP_LOG("UPNP: Device", LOG_DEBUG, "Stop home merge processing");
			if ((DEVICE_SOCKET == g_eDeviceType) && (g_eDeviceTypeTemp != DEVICE_INSIGHT))
			{
				if((0x00 == atoi(g_szRestoreState)) && (0x00 != strlen(g_szHomeId))  && (0x00 != strlen(g_szPluginPrivatekey)))
				{
#ifdef SIMULATED_OCCUPANCY
					if(!gSimulatedRuleRunning)
#endif
					{
						/* stop control point too for a switch which is registered */
						APP_LOG("UPNP: Device", LOG_DEBUG, "Stop plugin ctrlpoint for SWITCH");
						StopPluginCtrlPoint();
					}
					s_RediscoverHandle = -1;
					return NULL;
				}
			}
		}

		s_cntRediscoveryCounter++;
		APP_LOG("UPNP: Device", LOG_DEBUG, "##################### Control point to re-discover: %d/%d ###################", 
			s_cntRediscoveryCounter, MAX_REDISCOVERY_TIMES);
		CtrlPointDiscoverDevices();

		if(gProcessData == TRUE)
		{
			/* sleep for a while to allow device discovery and creation of device list */
			pluginUsleep((MAX_DISCOVER_TIMEOUT + 1)*1000000);
			/* send action to the device list */
			PluginCtrlPointSendActionAll(PLUGIN_E_EVENT_SERVICE, "GetHomeInfo", 0x00, 0x00, 0x00);
			/* Response will be processed in CallBack later */
		}

		
	}
	return NULL;
}

/**
 *
 *
 *
 *
 *
 *
 *
 */
void RunDiscoverTask()
{
	if (-1 != s_RediscoverHandle)
	{
		APP_LOG("UPNP: Device", LOG_ERR, "############Rediscover handle already created################");
		return;
	}

	ithread_create(&s_RediscoverHandle, NULL, CtrlPtRediscoverTask, NULL);
	if (-1 != s_RediscoverHandle)
	{
		APP_LOG("UPNP: Device", LOG_DEBUG, "Rediscover thread created successfully");
		ithread_detach(s_RediscoverHandle);
	}
	else
	{
			APP_LOG("UPNP: Device", LOG_ERR, "#######Rediscover thread can not be created#########");
	}
}

void StopDiscoverTask()
{
	if (-1 != s_RediscoverHandle)
	{
		ithread_cancel(s_RediscoverHandle);
		s_RediscoverHandle = -1;
	}
}


/**
 *	Get device index in the device list for send command
 *
 *
 *
 *
 *
 *
 *
 ******************************************************************/
int GetDeviceIndexByUDN(const char* udn)
{

    pCtrlPluginDeviceNode tmpdevnode = g_pGlobalPluginDeviceList;
	int isFound = 0x00;

	int index = 0x00;
	if (tmpdevnode)
	{
		while(tmpdevnode)
		{
			if (0x00 == strcmp(udn, tmpdevnode->device.UDN))
			{
				APP_LOG("UPnPCtrPt", LOG_DEBUG, "Device %s found", udn);
				isFound = 0x01;
				index++;
				break;
			}
			else
			{					
				tmpdevnode = tmpdevnode->next;
				index++;
			}
		}
	}
	else
	{
		 APP_LOG("UPnPCtrPt", LOG_DEBUG, "No living devices found");
	}
	
	if (!isFound)
		index = 0x00;

	return index;
	
}

char* GetUDNByDeviceIndex(int nIndex)
{

    pCtrlPluginDeviceNode tmpdevnode = g_pGlobalPluginDeviceList;

	int index = 0x01;
	if (tmpdevnode)
	{
		while(tmpdevnode)
		{
			if (index == nIndex)
			{
				APP_LOG("UPnPCtrPt", LOG_DEBUG, "Device %s found",  tmpdevnode->device.UDN);
				return (tmpdevnode->device.UDN);
			}
			else
			{					
				tmpdevnode = tmpdevnode->next;
				index++;
			}
		}
	}
	else
	{
		 APP_LOG("UPnPCtrPt", LOG_DEBUG, "No living devices found");
	}
	
	return NULL;
	
}

int GetDeviceIndexNumber(void* arg)
{

    	pCtrlPluginDeviceNode tmpdevnode = g_pGlobalPluginDeviceList;
	int index = 0x00;

	if (tmpdevnode)
	{
		while(tmpdevnode)
		{
			index++;
			tmpdevnode = tmpdevnode->next;
		}
	}
	else
	{
		 APP_LOG("UPnPCtrPt", LOG_DEBUG, "No devices found");
	}
	
	return index;
	
}


/**
 * 	Function:
 		SendOverriddPushNotification
 *	Description:
 		Send overridding push notification immediately by UDN
 *
 *
 *
 *
 *
 *
 **************************************************************/
void SendOverriddPushNotification(char* szUDN)
{
	LockDeviceSync();

	if (!g_pGlobalPluginDeviceList)
	{
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "No living device, senor detection not applied");
		UnlockDeviceSync();
		return;
	}

	UnlockDeviceSync();

	lockRule();
	if (g_pCurTimerControlEntry)
	{
		if ((g_pCurTimerControlEntry->count > 0x00) && (g_pCurTimerControlEntry->pSocketList))
		{
			if (IsDeviceInOverrideTable(szUDN))
			{
				char tmp[SIZE_256B];
				memset(tmp, 0x00, sizeof(tmp));
				snprintf(tmp, sizeof(tmp), "%s;%s", szUDN, g_szUDN);
				APP_LOG("Sensor: rule", LOG_DEBUG, "Device is in override table: %s", szUDN);
				
				if (0x00 == g_pCurTimerControlEntry->isExtraRule)
				{
					RuleOverrideNotify(ACTION_SENSOR_RULE, szUDN, g_pCurTimerControlEntry->time, g_pCurTimerControlEntry->action);
				}
				else
				{
					RuleOverrideNotify(ACTION_SENSOR_RULE, szUDN, g_pCurTimerControlEntry->isExtraRule, g_pCurTimerControlEntry->action);
				}
			}
			else
			{
				APP_LOG("Sensor: rule", LOG_DEBUG, "overridding table not built, should not occur");			
			}
		}
		else
		{
			APP_LOG("Sensor: rule", LOG_DEBUG, "Sensor rule, but no managed device in list, should not occur");			
		}
	}
	else
	{
		APP_LOG("Sensor: rule", LOG_DEBUG, "No rule activated, no process");
	}

	unlockRule();

}
void SendEvent2ControlledDevices()
{
	int hasTimerNow= 0x00;
	LockDeviceSync();
	int iloop = 0x00;
	int isNotificationRule = FALSE;
	char szUDN[SIZE_256B] = {'\0',};

	APP_LOG("UPnPCtrPt", LOG_DEBUG, "#### sensor to control: %d device", g_pCurTimerControlEntry->count);
	for (; iloop < g_pCurTimerControlEntry->count; iloop++)
	{
		 if(g_pCurTimerControlEntry->pSocketList[iloop].triggerType == 9)
	  {
		  isNotificationRule = TRUE;
	  }
	}
	if (!g_pGlobalPluginDeviceList && !isNotificationRule)
	{
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "No living device, senor detection not applied, searching devices now");
		UnlockDeviceSync();
		lockRule();
		if (0x00 == g_pCurTimerControlEntry)
			hasTimerNow = 0x01;
		unlockRule();
		
		if (hasTimerNow)
		{
			CtrlPointDiscoverDevices();
		}
		
		return;
	}

	UnlockDeviceSync();

	lockRule();

	if (g_pCurTimerControlEntry)
	{
		if ((g_pCurTimerControlEntry->count > 0x00) && (g_pCurTimerControlEntry->pSocketList))
		{
			int loop = 0x00;

			APP_LOG("UPnPCtrPt", LOG_DEBUG, "#### sensor to control: %d device", g_pCurTimerControlEntry->count);
			for (; loop < g_pCurTimerControlEntry->count; loop++)
			{
				char* udn = g_pCurTimerControlEntry->pSocketList[loop].UND;

				if ((0x00 == udn) || 0x00 == strlen(udn))
				{
					APP_LOG("UPnPCtrPt", LOG_DEBUG, "Device UDN NULL, should not happen");
					continue;
				}
			  if(g_pCurTimerControlEntry->pSocketList[loop].triggerType != 9)
			  {

				//- Override
				if (IsDeviceInOverrideTable(udn))
				{
					char tmp[SIZE_256B];
					memset(tmp, 0x00, sizeof(tmp));
					snprintf(tmp, sizeof(tmp), "%s;%s", udn, g_szUDN);
					APP_LOG("UPnPCtrPt", LOG_DEBUG, "Device is in override table: %s", udn);
					if (0x00 == g_pCurTimerControlEntry->isExtraRule)
					{
						RuleOverrideNotify(ACTION_SENSOR_RULE, udn, g_pCurTimerControlEntry->time, g_pCurTimerControlEntry->action);
					}
					else
					{
						RuleOverrideNotify(ACTION_SENSOR_RULE, udn, g_pCurTimerControlEntry->isExtraRule, g_pCurTimerControlEntry->action);
					}

					continue;
				}

				memset(szUDN, 0, sizeof(szUDN));
				if(strstr(udn, "uuid:Bridge") != NULL)
				{
				    strncpy(szUDN, udn, BRIDGE_UDN_LEN);
				}
				else if(strstr(udn, "uuid:Maker") != NULL)
				{
					strncpy(szUDN, udn, MAKER_UDN_LEN);
					strncat(szUDN, ":sensor:switch", sizeof(szUDN)-strlen(szUDN)-1);
				}
				else
				    strncpy(szUDN, udn, sizeof(szUDN)-1);

				APP_LOG("UPNP: Device", LOG_DEBUG, "Input UDN: %s converted UDN: %s", udn, szUDN);

				LockDeviceSync();
				int deviceIndex = GetDeviceIndexByUDN(szUDN);
				UnlockDeviceSync();
				
				if (0x00 != deviceIndex)
				{
					int ret = PluginCtrlPointSensorEventInd(deviceIndex, 
						g_pCurTimerControlEntry->pSocketList[loop].triggerType,
						g_pCurTimerControlEntry->pSocketList[loop].cntDuration, 
						g_pCurTimerControlEntry->pSocketList[loop].endAction,
						udn
						);
					
					if (ret != UPNP_E_SUCCESS)
					{
						APP_LOG("UPnPCtrPt", LOG_DEBUG, "Senoring command sent unsuccess: %s", szUDN);						
					}
				}
				else
				{
					APP_LOG("UPnPCtrPt", LOG_DEBUG, "Device:%s  not online, command not sent", szUDN);	
					CtrlPointDiscoverDevices();
				}
			  }
#if defined (PRODUCT_WeMo_SNS) || defined (PRODUCT_WeMo_NetCam)
			  else
			  {
			      SendRuleNotification(g_pCurTimerControlEntry->pSocketList[loop].cntDuration);   
			  }
#endif
			}
		}
		else
		{
			APP_LOG("UPnPCtrPt", LOG_DEBUG, "######### No device in device table");
		}

	}
	else
	{
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "###### No timer rule now running ############");
	}

	unlockRule();
}


int CtrlPointDiscoverDevices(void)
{
    int rect = -1;

    if (-1 == ctrlpt_handle)
    {
	APP_LOG("UPnPCtrPt",LOG_ERR, "###### control point not created");
	return 0x01;
    }

     rect = UpnpSearchAsync(ctrlpt_handle, MAX_DISCOVER_TIMEOUT, PluginDeviceType, NULL);
     if( UPNP_E_SUCCESS != rect )
     {
         APP_LOG("UPnPCtrPt",LOG_ERR, "Error sending search request%d", rect );
         return UPNP_E_INVALID_DEVICE;
     }

  return UPNP_E_SUCCESS;
}

void EnableContrlPointRediscover(int isEnable)
{
	if (isEnable)
		RunDiscoverTask();
}

int StartPluginCtrlPoint(char* szAddress, unsigned short port)
{
     int rect;

	 initDeviceSync();
	 initSignNotify();
	
     rect = UpnpInit(szAddress, port, g_szUDN_1);
     if((UPNP_E_SUCCESS != rect) && UPNP_E_INIT != rect)
     {
         APP_LOG("UPnPCtrPt",LOG_ERR, "StartCtrlPoint: UpnpInit() Error: %d", rect);
         UpnpFinish();
         return UPNP_E_INIT_FAILED;
     }

     if( NULL == szAddress )
     {
         szAddress = UpnpGetServerIpAddress();
     }
     if( 0 == port )
     {
         port = UpnpGetServerPort();
     }

     APP_LOG("UPnPCtrPt",LOG_DEBUG, "UPnP Initialized ipaddress= %s port = %u",
         szAddress, port);

     APP_LOG("UPnPCtrPt",LOG_DEBUG, "Registering Control Point: sensor" );

     rect = UpnpRegisterClient(CtrlPointCallbackEventHandler,
                              &ctrlpt_handle, &ctrlpt_handle );

     if( UPNP_E_SUCCESS != rect )
     {
         APP_LOG("UPnPCtrPt",LOG_ERR, "Error registering callback: %d", rect);
         UpnpFinish();
         return UPNP_E_INIT_FAILED;
     }

     APP_LOG("UPnPCtrPt",LOG_ERR, "Register sensor client success");
     if ((g_eDeviceType == DEVICE_SENSOR) || (g_eDeviceTypeTemp == DEVICE_INSIGHT))
     {
	     //- Only send out when device is sensor, socket will be via adv
	     CtrlPointDiscoverDevices();
     }

     system("route del -net 127.0.0.0 netmask 255.0.0.0");

     return UPNP_E_SUCCESS;
}


void CtrlPointPrintDevice()
{
    if (0x00 == g_pGlobalPluginDeviceList)
        {

            APP_LOG("UPnPCtrPt",LOG_DEBUG, "No device in discovery list");
            return;
        }

        pCtrlPluginDeviceNode tmpNode =  g_pGlobalPluginDeviceList;
        int index = 0x00;
        while (0x00 != tmpNode)
        {
            index++;
            APP_LOG("UPnPCtrPt",LOG_DEBUG, "Device %d: %s\n", index, tmpNode->device.UDN);
            tmpNode = tmpNode->next;
        }

}


void *ResendRemoteEnableRequestThreadProc(void *args)
{
	int iVal = 60, exp = 1, retry_iVal = 0, iCnt = 1;
    	tu_set_my_thread_name( __FUNCTION__ );
	while(1)
	{
		retry_iVal = iVal * exp;
		/* re-try time is 60 60 60 60 60 120 180 240 300 300 300 */
		if(retry_iVal < MAX_RETRY_INTERVAL && iCnt > INIT_RETRY_INTERVAL){			
			exp++;
		}else{
		    iCnt++;
		}
		APP_LOG("UPnPCtrPt",LOG_ERR, "RE-TRYING in <%d> seconds..", retry_iVal);
		pluginUsleep(retry_iVal * 1000000);
		char *pluginKey = GetBelkinParameter (DEFAULT_PLUGIN_PRIVATE_KEY);
       		if ((pluginKey && (0x00 == strlen(pluginKey)) && (0x00 == atoi(g_szRestoreState)))
			||(pluginKey && (0x00 != strlen(pluginKey)) && (0x01 == atoi(g_szRestoreState))))
		{
			PluginCtrlPointShareHWInfo(g_deviceindex);
			PluginCtrlPointRemoteAccess(g_deviceindex, g_deviceid, 0x01, g_homeid, PLUGIN_REMOTE_REQ);
			APP_LOG("UPNP: Device", LOG_DEBUG, "ResendRemoteEnableRequestThreadProc thread: PluginCtrlPointRemoteAccess request send successfully");
		}

	#ifdef PRODUCT_WeMo_NetCam
  	if (pluginKey)
  		free(pluginKey);
  		pluginKey = 0x00;
  	#endif
	}
	return NULL;
}

void *GetHomeIdThreadProc(void *args)
{
    	tu_set_my_thread_name( __FUNCTION__ );
	while(1)
	{
		pluginUsleep(300000000);
		char *homeId = GetBelkinParameter (DEFAULT_HOME_ID);
                if (0x00 != homeId && (0x00 == strlen(homeId))){
			PluginCtrlPointGetHomeId(g_deviceindex, 0x00);	
			APP_LOG("UPNP: Device", LOG_DEBUG, "GetHomeIdThreadProc thread: PluginCtrlPointGetHomeId request send successfully");
		}
	#ifdef PRODUCT_WeMo_NetCam
  	if (homeId)
  		free(homeId);
  	#endif
	}
	return NULL;
}

void GetHomeIdTaskThread(void)
{
	if (-1 != s_GetHomeIdHandle)
	{
		APP_LOG("UPNP: Device", LOG_DEBUG, "############GetHomeIdTaskThread handle already created################");
		return;
	}

	pthread_attr_init(&tmp_hmattr);
	pthread_attr_setdetachstate(&tmp_hmattr,PTHREAD_CREATE_DETACHED);
	pthread_create(&s_GetHomeIdHandle, &tmp_hmattr, GetHomeIdThreadProc, NULL);
	if (-1 != s_GetHomeIdHandle)
	{
		APP_LOG("UPNP: Device", LOG_DEBUG, "GetHomeIdTaskThread thread created successfully");
	}
	else
	{
			APP_LOG("UPNP: Device", LOG_DEBUG, "#######GetHomeIdTaskThread thread can not be created#########");
	}
}

void ResendRemoteEnableRequestTaskThread(void)
{
	if (-1 != s_ResendRemoteRequestHandle)
	{
		APP_LOG("UPNP: Device", LOG_DEBUG, "############s_ResendRemoteRequestHandleTaskThread handle already created################");
		return;
	}

	pthread_attr_init(&tmp_reremattr);
	pthread_attr_setdetachstate(&tmp_reremattr,PTHREAD_CREATE_DETACHED);
	pthread_create(&s_ResendRemoteRequestHandle, &tmp_reremattr, ResendRemoteEnableRequestThreadProc, NULL);
	if (-1 != s_ResendRemoteRequestHandle)
	{
		APP_LOG("UPNP: Device", LOG_DEBUG, "ResendRemoteEnableRequestTaskThread thread created successfully");
	}
	else
	{
			APP_LOG("UPNP: Device", LOG_DEBUG, "#######ResendRemoteEnableRequestTaskThread thread can not be created#########");
	}
}

void RemoteAccessResponseTask(struct Upnp_Action_Complete *args)
{
	if(args == 0x00)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "RemoteAccessResponseTask: args is NULL");
		return;
	}
   	APP_LOG("UPnPCtrPt", LOG_ERR, "\n******************** Remote Access Response received ****************\n");
	struct Upnp_Action_Complete *a_event = ( struct Upnp_Action_Complete * )args;

	char* paramValue1 = Util_GetFirstDocumentItem(a_event->ActionResult, "homeId");
	if (0x00 != paramValue1 && (0x00 != strlen(paramValue1)))
	{
		APP_LOG("UPnPCtrPt",LOG_HIDE, "homeId%s\n", paramValue1);
		if(0x0 != strcmp(paramValue1, "FAIL")){
#if 0
			SetBelkinParameter (DEFAULT_HOME_ID, paramValue1);
#else
		  char cmdbuf[SIZE_64B];
		  memset(cmdbuf, 0x0, sizeof(cmdbuf));
		  snprintf(cmdbuf, sizeof(cmdbuf), "nvram set %s=\"%s\"", DEFAULT_HOME_ID, paramValue1);
		  system(cmdbuf);
		  //AsyncSaveData();
#endif
			memset(g_szHomeId, 0x00, sizeof(g_szHomeId));
			strncpy(g_szHomeId, paramValue1, sizeof(g_szHomeId)-1);
		}
	}
	char* paramValue2 = Util_GetFirstDocumentItem(a_event->ActionResult, "pluginprivateKey");
	if (0x00 != paramValue2 && (0x00 != strlen(paramValue2)))
	{
		APP_LOG("UPnPCtrPt",LOG_HIDE, "pluginprivateKey%s\n", paramValue2);
		if(0x0 != strcmp(paramValue2, "FAIL")){
#if 0
			SetBelkinParameter (DEFAULT_PLUGIN_PRIVATE_KEY, paramValue2);
#else
		  char cmdbuf[SIZE_64B];
		  memset(cmdbuf, 0x0, sizeof(cmdbuf));
		  snprintf(cmdbuf, sizeof(cmdbuf), "nvram set %s=\"%s\"", DEFAULT_PLUGIN_PRIVATE_KEY, paramValue2);
		  system(cmdbuf);
		  //AsyncSaveData();
#endif
			memset(g_szPluginPrivatekey, 0x00, sizeof(g_szPluginPrivatekey));
			strncpy(g_szPluginPrivatekey, paramValue2, sizeof(g_szPluginPrivatekey)-1);
		}
	}
	char* paramValue3 = Util_GetFirstDocumentItem(a_event->ActionResult, "smartprivateKey");
	if (0x00 != paramValue3 && (0x00 != strlen(paramValue3)))
	{
		APP_LOG("UPnPCtrPt",LOG_HIDE, "smartprivateKey%s\n", paramValue3);
		if(0x0 != strcmp(paramValue3, "FAIL")){
#if 0
			SetBelkinParameter (DEFAULT_SMART_PRIVATE_KEY, paramValue3);
#else
		  char cmdbuf[SIZE_64B];
		  memset(cmdbuf, 0x0, sizeof(cmdbuf));
		  snprintf(cmdbuf, sizeof(cmdbuf), "nvram set %s=\"%s\"", DEFAULT_SMART_PRIVATE_KEY, paramValue3);
		  system(cmdbuf);
		  //AsyncSaveData();
#endif
			memset(g_szSmartPrivateKey, 0x00, sizeof(g_szSmartPrivateKey));
			strncpy(g_szSmartPrivateKey, paramValue3, sizeof(g_szSmartPrivateKey)-1);
		}
	}
	char* paramValue4 = Util_GetFirstDocumentItem(a_event->ActionResult, "statusCode");
	if (0x00 != paramValue4 && (0x00 != strlen(paramValue4)))
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "statusCode%s\n", paramValue4);
		if(0x0 == strcmp(paramValue4, "F")){
			APP_LOG("UPnPCtrPt",LOG_ERR, "My last remote access attempt failed...");
			ResendRemoteEnableRequestTaskThread();
		}
	}
	char* paramValue5 = Util_GetFirstDocumentItem(a_event->ActionResult, "resultCode");
	if (0x00 != paramValue5 && (0x00 != strlen(paramValue5)))
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "resultCode%s\n", paramValue5);
	}
	char* paramValue6 = Util_GetFirstDocumentItem(a_event->ActionResult, "description");
	if (0x00 != paramValue6 && (0x00 != strlen(paramValue6)))
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "description%s\n", paramValue6);
	}
	char* paramValue7 = Util_GetFirstDocumentItem(a_event->ActionResult, "smartUniqueId");
	if (0x00 != paramValue7 && (0x00 != strlen(paramValue7)))
	{
		APP_LOG("UPnPCtrPt",LOG_HIDE, "SmartUniqueId%s\n", paramValue7);
		SetBelkinParameter (DEFAULT_SMART_DEVICE_ID, paramValue7);
		memset(g_szSmartDeviceId, 0x00, sizeof(g_szSmartDeviceId));
		strncpy(g_szSmartDeviceId, paramValue7, sizeof(g_szSmartDeviceId)-1);
	}
	char* paramValue8 = Util_GetFirstDocumentItem(a_event->ActionResult, "numSmartDev");
	if (0x00 != paramValue8 && (0x00 != strlen(paramValue8)))
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "numSmartDev:%s", paramValue8);
		if(atoi(paramValue8) >= 0x1)
		{
			PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, g_deviceindex, "GetSmartDevInfo", 0x00, 0x00, 0x00);
		}
	}
	else
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "numSmartDev none");
	}
	if(0x00 != paramValue4 && 0x0 == strcmp(paramValue4, "S")){
		if (-1 != s_ResendRemoteRequestHandle)
		{
			pthread_kill(s_ResendRemoteRequestHandle, SIGRTMIN);
			s_ResendRemoteRequestHandle = -1;
		}
		char restoreStatus[2];
		int i=0;
		if((0x00 != paramValue1 && (0x00 != strlen(paramValue1))) && (0x00 != paramValue2 && (0x00 != strlen(paramValue2)))){
			memset(restoreStatus,0,2);
			snprintf(restoreStatus, sizeof(restoreStatus), "%d", i);
			SetBelkinParameter (RESTORE_PARAM, restoreStatus);
			memset(g_szRestoreState, 0x0, sizeof(g_szRestoreState));
			strncpy(g_szRestoreState, restoreStatus, sizeof(g_szRestoreState)-1);
			char routerMac[MAX_MAC_LEN];
			char routerssid[MAX_ESSID_LEN];
			memset(routerMac, 0, sizeof(routerMac));
			memset(routerssid, 0, sizeof(routerssid));
			getRouterEssidMac (routerssid, routerMac, INTERFACE_CLIENT);
		#ifdef WeMo_INSTACONNECT
			/* in case of insta connect mode, replace Essid with router ssid */
			if(!g_usualSetup)
			{
			    APP_LOG("WiFiApp", LOG_DEBUG,"Insta connect setup mode...");
			    if (strlen(gRouterSSID) > 0x0)
			    {
				APP_LOG("WiFiApp", LOG_DEBUG,"replace essid with router ssid for insta");
				memset(routerssid, 0, sizeof(routerssid));
				strncpy(routerssid, gRouterSSID, sizeof(routerssid)-1);
			    }
			}
		#endif
			if(strlen(gGatewayMAC) > 0x0)
			{
			    memset(routerMac, 0, sizeof(routerMac));
			    strncpy(routerMac, gGatewayMAC, sizeof(routerMac)-1);
			}
			SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
			SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
			memset(g_routerMac, 0x0, sizeof(g_routerMac));
			strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
			memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
			strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
			memset(g_homeid, 0x0, sizeof(g_homeid));
			memset(g_deviceid, 0x0, sizeof(g_deviceid));
			AsyncSaveData();
			g_homeiddeviceidchk = 0;
		}
		ithread_t remoteinit_thread;
		if ((0x00 != strlen(g_szHomeId) ) && (0x00 != strlen(g_szPluginPrivatekey)) && \
				(atoi(g_szRestoreState) == 0x0) &&(gpluginRemAccessEnable == 0)) {
			gpluginRemAccessEnable = 1;
			ithread_create(&remoteinit_thread, NULL, remoteAccessInitThd, NULL);
		}

	}
	FreeXmlSource(paramValue1);
	FreeXmlSource(paramValue2);
	FreeXmlSource(paramValue3);
	FreeXmlSource(paramValue4);
	FreeXmlSource(paramValue5);
	FreeXmlSource(paramValue6);
	FreeXmlSource(paramValue7);
	FreeXmlSource(paramValue8);

}

void GetSmartDevInfoResponseTask(struct Upnp_Action_Complete *g_args)
{
	if(g_args == 0x00)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "GetSmartDevInfoResponseTask: g_args is NULL");
		return;
	}
	struct Upnp_Action_Complete *a_event = ( struct Upnp_Action_Complete * )g_args;

	char* paramValue_SR = Util_GetFirstDocumentItem(a_event->ActionResult, "SmartDevInfo");
	if (0x00 != paramValue_SR && (0x00 != strlen(paramValue_SR)))
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "SmartDevInfo: %s\n", paramValue_SR);
		char buf[SIZE_256B] = {'\0'};
		int retVal = 0;
		snprintf(buf, sizeof(buf), "wget %s -O %s", paramValue_SR, SMARTDEVXML);
		APP_LOG("UPnPCtrPt", LOG_DEBUG,"Going to download smart device info xml with command %s\n", buf);
		retVal = system(buf);
		if (retVal != 0) {
			APP_LOG("UPnPCtrPt", LOG_ERR,"Download smart device info xml %s failed\n", buf);
		}else {
			APP_LOG("UPnPCtrPt", LOG_NOTICE,"Downloaded smart device info xml from location %s successfully\n", buf);
		}
	}	
	FreeXmlSource(paramValue_SR);
}

void GetMacAddrResponseTask(struct Upnp_Action_Complete *gm_args)
{
	if(gm_args == 0x00)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "GetMacAddrResponseTask: gm_args is NULL");
		return;
	}
	struct Upnp_Action_Complete *a_event = ( struct Upnp_Action_Complete * )gm_args;

	char* paramValue = Util_GetFirstDocumentItem(a_event->ActionResult, "MacAddr");
	if (0x00 != paramValue && (0x00 != strlen(paramValue)))
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "MacAddr:%s\n", paramValue);
		memset(gMacAddr, 0, sizeof(gMacAddr));
	}
	char* paramValue1 = Util_GetFirstDocumentItem(a_event->ActionResult, "SerialNo");
	if (0x00 != paramValue1 && (0x00 != strlen(paramValue1)))
	{
		APP_LOG("UPnPCtrPt", LOG_ERR, "SerialNo:%s\n", paramValue1);
		memset(gSerialNo, 0, sizeof(gSerialNo));
	}
	char* paramValue2 = Util_GetFirstDocumentItem(a_event->ActionResult, "PluginUDN");
	if (0x00 != paramValue2 && (0x00 != strlen(paramValue2)))
	{
		APP_LOG("UPnPCtrPt", LOG_ERR, "PluginUDN:%s\n", paramValue2);
		memset(gPluginUDN, 0, sizeof(gPluginUDN));
	}

	FreeXmlSource(paramValue);
	FreeXmlSource(paramValue1);
	FreeXmlSource(paramValue2);
}

#ifdef SIMULATED_OCCUPANCY
void ProcessGetSimulatedRuleDataResponse(struct Upnp_Action_Complete *args)
{
	char *index = NULL;
	char *state = NULL;
	char *remtimetotoggle = NULL;
	char *udn = NULL;
	SimulatedDevData *devicedata=NULL;
	SimulatedDevData *tmpdevdata=NULL;	
	int 	found = 0;

	APP_LOG("UPnPCtrPt", LOG_DEBUG, "ProcessGetSimulatedRuleDataResponse...");
	if(args == 0x00)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "Argument is NULL");
		return;
	}
	struct Upnp_Action_Complete *a_event = ( struct Upnp_Action_Complete * )args;

	char* paramValue = Util_GetFirstDocumentItem(a_event->ActionResult, "RuleData");
	if (0x00 != paramValue && (0x00 != strlen(paramValue)))
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "RuleData received:%s", paramValue);

		/* Process response - format: index|binarystate|remtimetotoggle| */

		index = strtok(paramValue, "|");
		state = strtok(NULL, "|");
		remtimetotoggle = strtok(NULL, "|");
		udn = strtok(NULL, "|");

		APP_LOG("UPnPCtrPt",LOG_DEBUG, "Parsed deviceindex:%s, binarystate: %s, remaing time to toggle: %s and udn: %s", index, state, remtimetotoggle, udn);
		LockSimulatedOccupancy();
		tmpdevdata = pgGlobalSimDevList;
		while (tmpdevdata)
		{
		    if (strcmp(tmpdevdata->sDevInfo.UDN, udn) == 0)
		    {
			found = 1;
			break;
		    }

		    if (!tmpdevdata->next)
			break;

		    tmpdevdata = tmpdevdata->next;
		}

		if (!found)
		{
		    devicedata = (SimulatedDevData*)malloc(sizeof(SimulatedDevData));
		    if (0x00 == devicedata)
		    {
			APP_LOG("UPnPCtrPt", LOG_ERR, "Error: Can not allocate device memory\n");
			FreeXmlSource(paramValue);
			UnlockSimulatedOccupancy();
			system("reboot");	
		    }
		    else
		    {
			APP_LOG("UPnPCtrPt", LOG_ERR, "Allocated %d bytes for dev UDN: %s", sizeof(SimulatedDevData), udn);
		    }

		    memset(devicedata, 0, sizeof(SimulatedDevData));
		    strncpy(devicedata->sDevInfo.UDN, udn, sizeof(devicedata->sDevInfo.UDN)-1);
		    devicedata->sDevInfo.devIndex = atoi(index);
		    devicedata->binaryState = atoi(state);
		    devicedata->remTimeToToggle = atoi(remtimetotoggle);
		    devicedata->next = NULL;

		    if (!tmpdevdata)
		    {
			pgGlobalSimDevList = devicedata;

		    }
		    else
		    {
			tmpdevdata->next = devicedata;
		    }

		    APP_LOG("UPnPCtrPt", LOG_ERR, " ############### Simulated device data added:%s #################", devicedata->sDevInfo.UDN);
		    APP_LOG("UPnPCtrPt", LOG_ERR, "Device data index: %d", devicedata->sDevInfo.devIndex);
		    APP_LOG("UPnPCtrPt", LOG_ERR, "Device data binary state: %d", devicedata->binaryState);
		    APP_LOG("UPnPCtrPt", LOG_ERR, "Device data remaining time to toggle: %d", devicedata->remTimeToToggle);
		}
		else
		{
		    APP_LOG("UPnPCtrPt", LOG_ERR, " ############### Simulated device data updated: %s #################", tmpdevdata->sDevInfo.UDN);
		    tmpdevdata->sDevInfo.devIndex = atoi(index);
		    tmpdevdata->binaryState = atoi(state);
		    tmpdevdata->remTimeToToggle = atoi(remtimetotoggle);
		    APP_LOG("UPnPCtrPt", LOG_ERR, "Device data index: %d", tmpdevdata->sDevInfo.devIndex);
		    APP_LOG("UPnPCtrPt", LOG_ERR, "Device data binary state: %d", tmpdevdata->binaryState);
		    APP_LOG("UPnPCtrPt", LOG_ERR, "Device data remaining time to toggle: %d", tmpdevdata->remTimeToToggle);
		}

		UnlockSimulatedOccupancy();		
		
		gSimDevUpnpRespCount++;	//upnp action response count
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "******** SIMULATED DEVICE UPNP RESPONSE COUNT is: %d ********", gSimDevUpnpRespCount);
	
	}
	else
	{
		APP_LOG("UPnPCtrPt", LOG_ERR, "Invalid response");
	}

	FreeXmlSource(paramValue);
	APP_LOG("UPnPCtrPt", LOG_DEBUG, "ProcessGetSimulatedRuleDataResponse done...");
}
#endif

void ProcessGetHomeInfoResponse(struct Upnp_Action_Complete *args)
{
	char *home_id = NULL;
	char *restore_state = NULL;
	char *signature = NULL;
	char sign[SIZE_256B];

	if(args == 0x00)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "Argument is NULL");
		return;
	}
	struct Upnp_Action_Complete *a_event = ( struct Upnp_Action_Complete * )args;

	char* paramValue = Util_GetFirstDocumentItem(a_event->ActionResult, "HomeInfo");
	if (0x00 != paramValue && (0x00 != strlen(paramValue)))
	{
		if(!strcmp(paramValue, "failure"))
		{
		    APP_LOG("UPnPCtrPt", LOG_ERR, "failure response received");
		    goto on_return;
		}

		APP_LOG("UPnPCtrPt",LOG_HIDE, "HomeInfo received:%s", paramValue);

		/* Process response - format: restore_state |home_id | signature | */
		//sscanf(paramValue, "%[^|]|%[^|]|%s", restore_state, home_id, signature);

		restore_state = strtok(paramValue, "|");
		home_id = strtok(NULL, "|");
		signature = strtok(NULL, "|");

		APP_LOG("UPnPCtrPt",LOG_HIDE, "Parsed Home id:%s, restore state: %s, signature: %s", home_id, restore_state, signature);

		/* Process this ONLY If restore state is 0 and home id is valid */
		if(strlen(g_szHomeId) && strcmp(g_szHomeId, home_id) && !strcmp(restore_state, "0") && strcmp(home_id, "NO_HOME"))
		{
			AcquireHomeIdListLock();
			/* check if this home id is present in our list, update list if absent */
			if(!strstr(gHomeIdList, home_id))
			{
				/* Decrypt the signature */
				memset(sign, 0, sizeof(sign));
				decryptWithMacKeydata(signature, sign);
				if (!strlen(sign))
				{
				    APP_LOG("UPNPCtrPt", LOG_ERR, "\n Signature Decrypt returned NULL\n");
				    ReleaseHomeIdListLock();
				    goto on_return;
				}

				/* Not in the list, add this home_id */
				if(strlen(gHomeIdList))
				{
					strncat(gHomeIdList, "-", sizeof(gHomeIdList)-strlen(gHomeIdList)-1);
				}
				strncat(gHomeIdList, home_id, sizeof(gHomeIdList)-strlen(gHomeIdList)-1);

				/* update the signature list too */
				if(strlen(gSignatureList))
				{
					strncat(gSignatureList, "-", sizeof(gSignatureList)-strlen(gSignatureList)-1);
				}
				strncat(gSignatureList, sign, sizeof(gSignatureList)-strlen(gSignatureList)-1);

				APP_LOG("UPnPCtrPt",LOG_HIDE, "Updated home id list: %s", gHomeIdList);
				APP_LOG("UPnPCtrPt",LOG_HIDE, "Updated signature list: %s", gSignatureList);
			}
			ReleaseHomeIdListLock();
		}
		else
		{
			APP_LOG("UPnPCtrPt",LOG_HIDE, "Not updating home id list, self home_id: %s", g_szHomeId);
		}
	}
	else
	{
		APP_LOG("UPnPCtrPt", LOG_ERR, "Invalid response");
	}

on_return:
	FreeXmlSource(paramValue);
	return;
}



void GetHomeIdResponseTask(struct Upnp_Action_Complete *a_event)
{
	char* paramValue = Util_GetFirstDocumentItem(a_event->ActionResult, "HomeId");
	if (0x00 != paramValue && (0x00 != strlen(paramValue)))
	{
		APP_LOG("UPnPCtrPt",LOG_HIDE, "HomeId:%s\n", paramValue);
		if(0x0 == strcmp(paramValue, "failure"))
		{
			char *homeId = GetBelkinParameter (DEFAULT_HOME_ID);
			if (0x00 != homeId && (0x00 == strlen(homeId))){
				if (-1 == s_GetHomeIdHandle){
					GetHomeIdTaskThread();
				}
			}
		}
		else
		{
			if (-1 != s_GetHomeIdHandle)
			{
				pthread_kill(s_GetHomeIdHandle, SIGRTMIN);
				s_GetHomeIdHandle = -1;
			}
			char* paramValue1 = Util_GetFirstDocumentItem(a_event->ActionResult, "DeviceId");
			if (0x00 != paramValue1 && (0x00 != strlen(paramValue1)))
			{
				APP_LOG("UPnPCtrPt", LOG_HIDE, "DeviceId:%s\n", paramValue1);
				SetBelkinParameter (DEFAULT_SMART_DEVICE_ID,paramValue1);
				SaveSetting();
			}
			if(!nRA)
			{
				char *szHomeId = paramValue;
				memset(g_homeid, 0x0, sizeof(g_homeid));
				strncpy(g_homeid, szHomeId, sizeof(g_homeid)-1);
				char *szDeviceId = paramValue1;
				memset(g_deviceid, 0x0, sizeof(g_deviceid));
				strncpy(g_deviceid, szDeviceId, sizeof(g_deviceid)-1);
				PluginCtrlPointShareHWInfo(g_deviceindex);
				PluginCtrlPointRemoteAccess(g_deviceindex, szDeviceId, 0x01, szHomeId, PLUGIN_REMOTE_REQ);
				++nRA;
			}

			FreeXmlSource(paramValue1);
		}
		FreeXmlSource(paramValue);
	}	

}

void CtrlPointProcessControlAction(struct Upnp_Action_Complete *a_event)
{
	char *localName = a_event->ActionResult->n.firstChild->localName;
	
	if(localName)
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "Node value: %s", localName);
	
	if (0x00 == strcmp(localName, "RemoteAccessResponse"))
	{
		RemoteAccessResponseTask(a_event);
	}

	else  if (0x00 == strcmp(localName, "GetSmartDevInfoResponse"))
	{
	}

	else  if (0x00 == strcmp(localName, "GetMacAddrResponse"))
	{
		GetMacAddrResponseTask(a_event);
	}
	else  if (0x00 == strcmp(localName, "GetHomeInfoResponse"))
	{
		ProcessGetHomeInfoResponse(a_event);
	}
#ifdef SIMULATED_OCCUPANCY
	else if (gSimulatedRuleRunning && (0x00 == strcmp(localName, "GetSimulatedRuleDataResponse")))
	{
	    APP_LOG("UPnPCtrPt", LOG_DEBUG, "Process Get Simulated Rule Data Response case...");
	    ProcessGetSimulatedRuleDataResponse(a_event);
	}
#endif
}


void subscribeDeviceServices(IXML_Document *DescDoc, const char *location, pCtrlPluginDeviceNode deviceNode)
{
	int 	service;
	int 	ret = 1;
	char *serviceId[PLUGIN_SERVICE_COUNT]      = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	char *eventURL[PLUGIN_SERVICE_COUNT]        = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	char *controlURL[PLUGIN_SERVICE_COUNT]      = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	Upnp_SID eventSID[PLUGIN_SERVICE_COUNT];
	int TimeOut[PLUGIN_SERVICE_COUNT]         = {default_timeout, default_timeout, default_timeout, default_timeout, default_timeout, default_timeout, default_timeout};

	memset(eventSID, 0, sizeof(eventSID));

	for (service = 0; service < PLUGIN_SERVICE_COUNT; service++)
	{
		if (0x00 != strstr(g_szSensorServiceList,  PluginDeviceServiceType[service]))
		{
			if( (0x00 == strcmp("urn:Belkin:service:remoteaccess:1", PluginDeviceServiceType[service])) &&
					(0x00 != strlen(g_szHomeId)) && (0x00 == atoi(g_szRestoreState)) )
			{
				APP_LOG("UPnPCtrPt", LOG_DEBUG, "Remote already enabled, not Subscribing to RemoteAccess service!!!");
			}
			else
			{
				if (Util_FindAndParseService(DescDoc, location, PluginDeviceServiceType[service], &serviceId[service], &eventURL[service], &controlURL[service]))
				{
					ret = UpnpSubscribe(ctrlpt_handle, eventURL[service], &TimeOut[service], eventSID[service]);
					if (ret == UPNP_E_SUCCESS)
					{
						if (NULL != serviceId[service])
							strncpy(deviceNode->device.services[service].ServiceId, serviceId[service], SIZE_256B - 1);
						if (NULL != PluginDeviceServiceType[service])
							strncpy(deviceNode->device.services[service].ServiceType, PluginDeviceServiceType[service], SIZE_256B - 1);
						if (NULL != controlURL[service])
							strncpy(deviceNode->device.services[service].ControlURL, controlURL[service], SIZE_256B - 1);
						if (NULL != eventURL[service])
							strncpy(deviceNode->device.services[service].EventURL, eventURL[service], SIZE_256B - 1);
						if (NULL != eventSID[service])
						{
							strncpy(deviceNode->device.services[service].SID, eventSID[service], SIZE_256B - 1);
							APP_LOG("UPnPCtrPt",LOG_ERR, "Subscribed to service:%s, sid: %s", PluginDeviceServiceType[service], deviceNode->device.services[service].SID);
						}
					}
					else
					{
						APP_LOG("UPnPCtrPt", LOG_ERR, "####### Error Subscribing to EventURL -- %s\n", PluginDeviceServiceType[service]);
					}
				}
			}
		}

	}


	for (service = 0; service < PLUGIN_SERVICE_COUNT; service++) 
	{
		FreeXmlSource(serviceId[service]);
		FreeXmlSource(controlURL[service]);
		FreeXmlSource(eventURL[service]);
	}
}

/**
 *	Function:
 *		CtrlPointProcessDeviceDiscoveryEx
 *	
 * Description:
 *		Extension of function "CtrlPointProcessDeviceDiscovery" adding the advertisement process
 *		The original one still kept but not called any more in case need to revert to original one
 *
 **/
void CtrlPointProcessDeviceDiscoveryEx(IXML_Document *DescDoc, const char *location, int expires, int isAdv)
{
	//- Device information
	char *friendlyName = NULL;
	char presURL[SIZE_256B];
	char *baseURL = NULL;
	char *relURL = NULL;
	char *UDN = NULL;

	pCtrlPluginDeviceNode deviceNode;
	pCtrlPluginDeviceNode tmpdevnode;

	int 	ret = 1;
	int 	found = 0;

	UDN                 = Util_GetFirstDocumentItem(DescDoc, "UDN");
	friendlyName        = Util_GetFirstDocumentItem(DescDoc, "friendlyName");
	baseURL             = Util_GetFirstDocumentItem(DescDoc, "URLBase");
	relURL              = Util_GetFirstDocumentItem(DescDoc, "presentationURL");

	ret = UpnpResolveURL((baseURL ? baseURL : location), relURL, presURL);

	if (UPNP_E_SUCCESS != ret)
	{
		//- Comment out junk message
	}


	if (!UDN || !strlen(UDN))
	{
		APP_LOG("UPnPCtrPt:discover",LOG_ERR, "UDN not found");
		goto exit_func;
	}

	if (!strcmp(UDN, g_szUDN))
	{
		goto exit_func;
	}

	//- Access start here
	LockDeviceSync();
	tmpdevnode = g_pGlobalPluginDeviceList;

	while (tmpdevnode)
	{
		if (strcmp(tmpdevnode->device.UDN, UDN) == 0)
		{
			found = 1;
			break;
		}

		if (!tmpdevnode->next)
			break;	//- The latest one

		tmpdevnode = tmpdevnode->next;
	}

	if (!found)
	{
		//search and subscrible service

		deviceNode = (CtrlPluginDeviceNode *)malloc(sizeof(CtrlPluginDeviceNode));
		memset(deviceNode, 0, sizeof(CtrlPluginDeviceNode));
		if (0x00 == deviceNode)
		{
			APP_LOG("UPnPCtrPt", LOG_ERR, "Error: Can not allocate device memory\n");
			UnlockDeviceSync();
			goto exit_func;
		}
		else
		{
			APP_LOG("UPnPCtrPt", LOG_ERR, "Allocated %d bytes for dev UDN: %s", sizeof(CtrlPluginDeviceNode), UDN);
		}

		//- Compiler behavior not so sure, so force reset to 0x00;
		deviceNode->IsDeviceRequestUpdate = 0x00;
#ifdef SIMULATED_OCCUPANCY
		deviceNode->Skip = 0x00;
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "device Skip: %d", deviceNode->Skip);
#endif

		strncpy(deviceNode->device.UDN, UDN, sizeof(deviceNode->device.UDN)-1);

		subscribeDeviceServices(DescDoc, location, deviceNode);
		

		//- Device Management
		strncpy(deviceNode->device.DescDocURL, location, SIZE_256B - 1);
		strncpy(deviceNode->device.FriendlyName, friendlyName, SIZE_256B - 1);
		strncpy(deviceNode->device.PresURL, presURL, SIZE_256B - 1);
		deviceNode->device.AdvrTimeOut = expires;

		deviceNode->next = NULL;

		if (!tmpdevnode)
		{
			g_pGlobalPluginDeviceList = deviceNode;

		}
		else
		{
			tmpdevnode->next = deviceNode;
		}

		APP_LOG("UPnPCtrPt", LOG_ERR, " ##################### New device added:%s ######################", deviceNode->device.UDN);
		APP_LOG("UPnPCtrPt", LOG_ERR, "DescDocURL:%s", deviceNode->device.DescDocURL);
		APP_LOG("UPnPCtrPt", LOG_ERR, "FriendlyName:%s", deviceNode->device.FriendlyName);
		APP_LOG("UPnPCtrPt", LOG_ERR, "PresURL:%s", deviceNode->device.PresURL);
		APP_LOG("UPnPCtrPt", LOG_ERR, "AdvrTimeOut:%d", deviceNode->device.AdvrTimeOut);
		int deviceIndex = GetDeviceIndexByUDN(UDN);
		g_deviceindex = deviceIndex;
	}
	else
	{
		tmpdevnode->device.AdvrTimeOut = expires;
		if (isAdv)
		{
			CtrlPointDeleteNode(tmpdevnode);
			tmpdevnode->IsDeviceRequestUpdate = 0x01;
		}

		if (tmpdevnode->IsDeviceRequestUpdate)
		{
			subscribeDeviceServices(DescDoc, location, tmpdevnode);

			//- Device Management
			strncpy(tmpdevnode->device.DescDocURL, location, SIZE_256B - 1);
			strncpy(tmpdevnode->device.FriendlyName, friendlyName, SIZE_256B - 1);
			strncpy(tmpdevnode->device.PresURL, presURL, SIZE_256B - 1);
			tmpdevnode->device.AdvrTimeOut = expires;
			tmpdevnode->IsDeviceRequestUpdate = 0x00;

			int deviceIndex = GetDeviceIndexByUDN(UDN);
			g_deviceindex = deviceIndex;

			APP_LOG("Device Management", LOG_ERR, "Device: %s updated", tmpdevnode->device.UDN);
		}
	}

	UnlockDeviceSync();

exit_func:
	FreeXmlSource(UDN);
	FreeXmlSource(friendlyName);
	FreeXmlSource(baseURL);
	FreeXmlSource(relURL);
}

void RemoveDeviceByUDN(const char* szUDN)
{
	LockDeviceSync();
	
	pCtrlPluginDeviceNode curdevnode 	= g_pGlobalPluginDeviceList;
	pCtrlPluginDeviceNode prevdevnode 	= 0x00;
	if (0x00 == curdevnode)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "No device in the list, can not delete");
		UnlockDeviceSync();
		return;
	}

	//- First device
	if(0x00 == strcmp(curdevnode->device.UDN, szUDN)) 
	{
		g_pGlobalPluginDeviceList = curdevnode->next;
		CtrlPointDeleteNode(curdevnode);

	} 
	else
	{
		prevdevnode = curdevnode;
		curdevnode  = curdevnode->next;
		while(curdevnode) 
		{
			if(0x00 == strcmp(curdevnode->device.UDN, szUDN)) 
			{
                prevdevnode->next = curdevnode->next;
                CtrlPointDeleteNode(curdevnode);
				APP_LOG("UPnPCtrPt",LOG_DEBUG, "Find and remove device");
                break;
			}

			prevdevnode = curdevnode;
			curdevnode = curdevnode->next;
		}
	}

	UnlockDeviceSync();
	
}

void CtrlPointProcessDeviceByebye(IXML_Document *DescDoc)
{
	if (0x00 == DescDoc)
	{
		APP_LOG("UPnPCtrPt",LOG_ERR, "NO xml file found");	
		return;
	}

	 char* szUDN = Util_GetFirstDocumentItem(DescDoc, "UDN");
	 if ((0x00 == szUDN) || 0x00 == strlen(szUDN))
	 {
		APP_LOG("UPnPCtrPt",LOG_ERR, "Can not find device UDN");
		return;
	 }

	RemoveDeviceByUDN(szUDN);
	 
}



pCtrlPluginDeviceNode GetDeviceNodeBySID(const char* SID)
{
	pCtrlPluginDeviceNode node = 0x00;

	node = g_pGlobalPluginDeviceList;
	while(node)
	{
	if ((0x00==strcmp(SID, node->device.services[PLUGIN_E_EVENT_SERVICE].SID))||(0x00==strcmp(SID, node->device.services[PLUGIN_E_REMOTE_ACCESS_SERVICE].SID)))
		{
			//APP_LOG("UPnPCtrPt", LOG_DEBUG, "device found: %04X:%s", node, node->device.UDN);
			break;
		}
		else
		{
			node = node->next;
		}
	}

	return node;
}

int IsDeviceInOverrideTable(const char* szUDN)
{
	int ret = 0x00;
	
	if ((0x00 != strlen(g_szOverrideTable)) && 
        (0x00 != strstr(g_szOverrideTable, szUDN))
       )
		ret = 0x01;
	
	return ret;
}


void CleanupOverrideTable()
{
	//- Reset
	memset(g_szOverrideTable, 0x00, sizeof(g_szOverrideTable));
}


void UpdateOverrideTable(const char* szUDN)
{
   int isRuleActive = 0x00;
   lockRule();
#ifdef PRODUCT_WeMo_Insight
   isRuleActive = g_isInsightRuleActivated;
#else
   isRuleActive = g_isSensorRuleActivated;
#endif
   unlockRule();

   int isNotified = 0x00;

   APP_LOG("UPnPCtrPt", LOG_ERR, "Not updating override table: %s", szUDN);
   return;

   if (!isRuleActive)
   {
		APP_LOG("UPnPCtrPt", LOG_ERR, "Rule not active, would not build overridden table");
		return;
   }	

	lockRule();

	int loop = 0x00;
	if (0x00 != g_pCurTimerControlEntry)
	{
		for (; loop < g_pCurTimerControlEntry->count; loop++)
		{
			if (0x00 == strcmp(g_pCurTimerControlEntry->pSocketList[loop].UND, szUDN))
			{
				//- Something should be true here
				isNotified = 0x01;
				break;
			}
		}

		if (loop != g_pCurTimerControlEntry->count)
		{
			//-in controlled device list, process it

			if ( (RULE_DO_NOTHING == g_pCurTimerControlEntry->pSocketList[loop].triggerType) || 
				(RULE_DO_NOTHING == g_pCurTimerControlEntry->pSocketList[loop].endAction) )
			{
				APP_LOG("UPnPCtrPt", LOG_ERR, "Do-nothing included time event, not overidable: %s", 
					g_pCurTimerControlEntry->pSocketList[loop].UND);
				unlockRule();
				return;
							
			}
		}
		else
		{
			//- Not in controlled device list, ignore it, in case it is other device
		}
	}
	else
	{		
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "Rule not active, not to create overidding table");
		unlockRule();
		return;
	}

	if (isNotified)
	{
		if (0x00 != strstr(g_szOverrideTable, szUDN))
		{
			//- Already in the table
			APP_LOG("UPnPCtrPt", LOG_ERR, "device already in override table: %s", g_szOverrideTable);
		}
		else
		{
			strncat(g_szOverrideTable, szUDN, sizeof(g_szOverrideTable)-strlen(g_szOverrideTable)-1);
			strncat(g_szOverrideTable, ";", sizeof(g_szOverrideTable)-strlen(g_szOverrideTable)-1);
			APP_LOG("UPnPCtrPt", LOG_ERR, "override table updated: %s", g_szOverrideTable);
		}
	}

	unlockRule();

	if (isNotified)
	{
		SendOverriddPushNotification((char *)szUDN);
	}

}

void* sendRemoteRegisterRequestThread(void *arg) 
{
    	tu_set_my_thread_name( __FUNCTION__ );
	pluginUsleep(50);
	APP_LOG("UPnP: Device",LOG_DEBUG,
			"sendRemoteRegisterRequestThread running");
#ifdef WeMo_INSTACONNECT
	if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
	    createAutoRegThread();
	return NULL;
}

void* sendRemoteRequestThread(void *arg) 
{
	pluginUsleep(50);
	APP_LOG("UPnP: Device",LOG_DEBUG,
			"SendRemoteRequest Thread running");
	PluginCtrlPointShareHWInfo(g_deviceindex);
	PluginCtrlPointRemoteAccess(g_deviceindex, g_deviceid, 0x01, g_homeid, PLUGIN_REMOTE_REQ);
	return NULL;
}

void ProcessUpNpNotify(struct Upnp_Event *event)
{
    	char routerMac[MAX_MAC_LEN];
    	char routerssid[MAX_ESSID_LEN];
	char szLocalUDN[SIZE_256B];
	memset(szLocalUDN, 0x00, sizeof(szLocalUDN));
    	memset(routerMac, 0, sizeof(routerMac));
    	memset(routerssid, 0, sizeof(routerssid));
	if (!event)
		return;

	LockDeviceSync();
		
	pCtrlPluginDeviceNode deviceNode = GetDeviceNodeBySID(event->Sid);

	if (0x00 == deviceNode)
	{
		APP_LOG("UPnPCtrPt", LOG_ERR, "Can not find device of SID: %s", event->Sid);
		UnlockDeviceSync();
		return;
	}

	strncpy(szLocalUDN, deviceNode->device.UDN, sizeof(szLocalUDN)-1);
	UnlockDeviceSync();	

	char* state = Util_GetFirstDocumentItem(event->ChangedVariables, "UserAction");

	if ((0x00 != state) && (0x00 != strlen(state)))
	{
		int iState = atoi(state);

		APP_LOG("UPnPCtrPt", LOG_ERR, "state change: %d", iState);
		
		if (STATE_RELAY_OVERRIDE_OFF == iState)
		{
			UpdateOverrideTable(szLocalUDN);
			APP_LOG("UPnPCtrPt", LOG_ERR, "socket OFF: %s", szLocalUDN);
		}
		else if (STATE_RELAY_OVERRIDE_ON == iState)
		{		
			UpdateOverrideTable(szLocalUDN);
			APP_LOG("UPnPCtrPt", LOG_ERR, "socket ON: %s", szLocalUDN);
		}
		else
		{
			APP_LOG("UPnPCtrPt", LOG_ERR, "unknown status change: %d", iState);	
		}
			
	}

	char *pSignature = Util_GetFirstDocumentItem(event->ChangedVariables, "PluginParam");
	if ((0x00 != pSignature) && (0x00 != strlen(pSignature)))
	{
		memset(g_signature, 0x0, SIGNATURE_LEN);
		decryptSignature(pSignature, g_signature);
		APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################After decryption: Signature: <%s>", g_signature);
		APP_LOG("UPnPCtrPt", LOG_HIDE, "GOT SIGNATURE NOTIFY: <%s>", g_signature);

		/* If we couldn't decrypt signature, create auto-registration thread */
		if(strlen(g_signature) == 0)
		{
			APP_LOG("UPnPCtrPt", LOG_ERR, "Decryption failed, creating auto registration thread");
			createAutoRegThread();
		}

	}
	FreeXmlSource(pSignature);
	
	char *nhomeid = Util_GetFirstDocumentItem(event->ChangedVariables, "HomeIdRequest");
	if ((0x00 != nhomeid) && (0x00 != strlen(nhomeid)))
	{
		memset(g_homeid, 0x0, sizeof(g_homeid));
		strncpy(g_homeid, nhomeid, sizeof(g_homeid)-1);
		APP_LOG("UPnPCtrPt", LOG_HIDE, "GOT HOME ID NOTIFY: <%s>", nhomeid);
	}

	char *ndeviceid = Util_GetFirstDocumentItem(event->ChangedVariables, "DeviceIdRequest");
	if ((0x00 != ndeviceid) && (0x00 != strlen(ndeviceid)))
	{
		memset(g_deviceid, 0x0, sizeof(g_deviceid));
		strncpy(g_deviceid, ndeviceid, sizeof(g_deviceid)-1);
		APP_LOG("UPnPCtrPt", LOG_ERR, "GOT DEVICE ID NOTIFY: <%s>", ndeviceid);
	}
#ifdef PRODUCT_WeMo_Insight
	char *nEnergyPerUnitCost = Util_GetFirstDocumentItem(event->ChangedVariables, "EnergyPerUnitCost");
	if ((0x00 != nEnergyPerUnitCost) && (0x00 != strlen(nEnergyPerUnitCost)))
	{
		APP_LOG("UPnPCtrPt", LOG_DEBUG, "GOT ENERGY PER UNIT COST  NOTIFY: <%s>", nEnergyPerUnitCost);
		ProcessEnergyPerunitCostNotify(nEnergyPerUnitCost);
	}
#endif

	//- Clean up
	FreeXmlSource(state);	
	FreeXmlSource(nhomeid);
	FreeXmlSource(ndeviceid);
	
	if ( (0x00 != strlen(g_signature) && (0x00 == strlen(g_szHomeId) && 0x00 == atoi(g_szRestoreState))))	//remote access not enabled case
	{
	    if(!nRA)
	    {
		    int retVal;
		    ++nRA;
		    APP_LOG("UPnPCtrPt", LOG_ERR, "########################## SENDING REMOTE ACCESS ENABLE REQUEST...");
		    pthread_attr_init(&tmp_attr);
		    pthread_attr_setdetachstate(&tmp_attr,PTHREAD_CREATE_DETACHED);
		    retVal = pthread_create(&tmpthread,&tmp_attr,
			    (void*)&sendRemoteRegisterRequestThread, NULL);
		    if(retVal < SUCCESS) {
			APP_LOG("UPnP: Device",LOG_ERR,
				"sendRemoteRegisterRequestThread not created");
		    } 
	    }
	}
}

int CtrlPointCallbackEventHandler(Upnp_EventType EventType,
                                 void *Event,
                                 void *Cookie )
{
	int sourceID = 0x00;

	int isAdv = 0x00;
	
    switch ( EventType )
        {
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
			//- PLEASE do NOT break here, let it fall through
			//- Adv and discovery response using the same API, but taking a bit difference
	{
		struct Upnp_Discovery *d_event = ( struct Upnp_Discovery * )Event;

		/* Set the isAdv flag for any 1 advertisement message only */
		if(strstr(d_event->ServiceType, "basicevent"))
		{
			int rnd = (30 + (rand() % 30));
                	APP_LOG("UPnPCtrPt", LOG_DEBUG, "Turning on adv flag on devid: %s, sleep: %d\n", d_event->DeviceId, rnd);
			isAdv=0x1;
			/* delay the advertisement processing */
			sleep(rnd);
		}
	}

        case UPNP_DISCOVERY_SEARCH_RESULT:
        {
			struct Upnp_Discovery *d_event = ( struct Upnp_Discovery * )Event;
            IXML_Document *DescDoc = NULL;
            int ret;

            if( d_event->ErrCode != UPNP_E_SUCCESS )
            {
                APP_LOG("UPnPCtrPt", LOG_ERR, "Error in Discovery Callback -- %d\n", d_event->ErrCode);
            }

            if( ( ret = UpnpDownloadXmlDoc( d_event->Location, &DescDoc ) ) != UPNP_E_SUCCESS)
            {
            }
            else
            {
                CtrlPointProcessDeviceDiscoveryEx(DescDoc, d_event->Location, d_event->Expires, isAdv);
            }

            if( DescDoc )
            {
                ixmlDocument_free(DescDoc);
            }

			sourceID = 0x00;
			
            break;
        }

        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            /*
               Nothing to do here...
             */
            break;

        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
	    {
		APP_LOG("UPnPCtrPt", LOG_ERR, "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE received");

		struct Upnp_Discovery *d_event = ( struct Upnp_Discovery * )Event;
                IXML_Document *DescDoc = NULL;
                int ret;

                if( d_event->ErrCode != UPNP_E_SUCCESS )
                {
                    APP_LOG("UPnPCtrPt", LOG_ERR, "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE: -- %d\n", d_event->ErrCode);
					return d_event->ErrCode;
                }

                if( ( ret = UpnpDownloadXmlDoc( d_event->Location, &DescDoc ) ) != UPNP_E_SUCCESS)
                {
					APP_LOG("UPnPCtrPt", LOG_ERR, "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE: -- %d\n", d_event->ErrCode);
					return d_event->ErrCode;                    
                }
                else
                {
                    CtrlPointProcessDeviceByebye(DescDoc);
                }

                if( DescDoc )
                {
                    ixmlDocument_free(DescDoc);
                }

                break;

            break;
        }

        case UPNP_CONTROL_ACTION_COMPLETE:
            {
                struct Upnp_Action_Complete *a_event =
                    ( struct Upnp_Action_Complete * )Event;

				if (0x00 == a_event)
				{
					break;
				}

                if( a_event->ErrCode != UPNP_E_SUCCESS )
                {
                    APP_LOG("UPnPCtrPt",LOG_ERR, "UPNP_CONTROL_ACTION_COMPLETE, failure [%d]\n", a_event->ErrCode);
                }
                else
                {
					
					APP_LOG("UPnPCtrPt",LOG_ERR, "UPNP_CONTROL_ACTION_COMPLETE, success");
                    CtrlPointProcessControlAction(a_event);
                }
                break;

        case UPNP_CONTROL_GET_VAR_COMPLETE:
            {
                break;
            }


        case UPNP_EVENT_RECEIVED:
            {

              	struct Upnp_Event *e_event = ( struct Upnp_Event * )Event;

				ProcessUpNpNotify(e_event);

                break;
            }

        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
            {

                APP_LOG("UPnPCtrPt",LOG_DEBUG, "UPNP_EVENT_SUBSCRIBE_COMPLETE: ");

                break;
            }  

		
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
		{
			break;	
		}
        case UPNP_EVENT_RENEWAL_COMPLETE:
        {
       
            break;
        }

        case UPNP_EVENT_AUTORENEWAL_FAILED:
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            {
                break;
            }

            /*
               ignore these cases, since this is not a device
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
        case UPNP_CONTROL_GET_VAR_REQUEST:
        case UPNP_CONTROL_ACTION_REQUEST:
            break;
    }
}
	
    return 0;
}

/***********************************************************
 *
 *
 *
 *
 *
 *
 *
 *
 *************************************************************/
int PluginCtrlPointSetSensorEvent(int deviceIndex, const char* UDN, int eventStatus, const char* eventFriendlyName, int triggerDuration /*minutes*/)
{
    PluginCtrlPointSendAction(PLUGIN_E_SETUP_SERVICE, deviceIndex, "SetSensorEvent", 0x00, 0x00, 0x00);
    return 0x00;
}

#if 1
int PluginCtrlPointAddRule(int deviceIndex)
{
    PluginCtrlPointSendAction(PLUGIN_E_RULES_SERVICE, deviceIndex, "EditRule", 0x00, 0x00, 0x00);
    return 0;
}
#endif
int PluginCtrlPointGetFirmware(int deviceIndex)
{
        PluginCtrlPointSendAction(PLUGIN_E_FIRMWARE_SERVICE, deviceIndex, "GetFirmwareVersion", 0x00, 0x00, 0x00);
	return 0;
}

int PluginCtrlPointUpdateFirmware(int deviceIndex, const char* URL)
{
    char* paramNames[] = {"URL"};
    APP_LOG("UPnPCtrPt",LOG_DEBUG, "URL:%s\n", URL);
    PluginCtrlPointSendAction(PLUGIN_E_FIRMWARE_SERVICE, deviceIndex, "UpdateFirmware", (const char **)paramNames, (char **)&URL, 0x01);
    return 0;
}

/***
 *
 *
 */
int PluginCtrlPointSyncTime(int deviceIndex, int utc, int TimeZone, int dstEnable)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "calling PluginCtrlPointGetApList: %d device", deviceIndex);

    char* paramsList[] = {"utc", "TimeZone", "dst"};

    char*  values[3];
    values[0] = (char*)malloc(SIZE_16B);
    values[1] = (char*)malloc(SIZE_16B);
    values[2] = (char*)malloc(SIZE_16B);

    snprintf(values[0], SIZE_16B, "%d", utc);
    snprintf(values[1], SIZE_16B, "%d", TimeZone);
    snprintf(values[2], SIZE_16B, "%d", dstEnable);

    PluginCtrlPointSendAction(PLUGIN_E_TIME_SYNC_SERVICE, deviceIndex, "SyncTime", (const char **)paramsList, values, 3);

    free(values[0]);
    free(values[1]);
    free(values[2]);

    return 0x00;
}

int PluginCtrlPointRemoteAccess(int deviceIndex, const char* devId, int dstEnable, const char *hmId, const char* devName)
{
    char *paramNames[] = {"DeviceId", "dst", "HomeId", "DeviceName", "MacAddr"};
    char*  values[5];
    values[0] = (char*)malloc(SIZE_128B);
    values[1] = (char*)malloc(SIZE_128B);	
    values[2] = (char*)malloc(SIZE_128B);
    values[3] = (char*)malloc(SIZE_128B);
    values[4] = (char*)malloc(SIZE_128B);
    memset(values[0], 0, SIZE_128B);
    memset(values[1], 0, SIZE_128B);
    memset(values[2], 0, SIZE_128B);
    memset(values[3], 0, SIZE_128B);
    memset(values[4], 0, SIZE_128B);

    snprintf(values[0], SIZE_128B, "%s", devId);
    snprintf(values[1], SIZE_128B, "%d", dstEnable);
    if(hmId == 0x00)
    {
	snprintf(values[2], SIZE_128B, "%s", "");
    }
    else
    {
	snprintf(values[2], SIZE_128B, "%s", hmId);
    }
    snprintf(values[3], SIZE_128B, "%s", devName);
    snprintf(values[4], SIZE_128B, "%s", g_szWiFiMacAddress);

    APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointRemoteAccess:values[0]=%s\n",values[0]);
    APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointRemoteAccess:values[1]=%s\n",values[1]);
    APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointRemoteAccess:values[2]=%s\n",values[2]);
    APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointRemoteAccess:values[3]=%s\n",values[3]);
    APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointRemoteAccess:values[4]=%s\n",values[4]);
    PluginCtrlPointSendAction(PLUGIN_E_REMOTE_ACCESS_SERVICE, deviceIndex, "RemoteAccess", (const char **)paramNames, values, 5);
    free(values[0]);
    free(values[1]);
    free(values[2]);
    free(values[3]);
    free(values[4]);

    return 0x00;	
}

int PluginCtrlPointSetSmartDevUrl(int deviceIndex)
{
    char *paramNames[] = {"SmartDevURL"};
    char*  values[1];
    char szSmartDevInfoURL[SIZE_128B];

    values[0] = (char*)malloc(SIZE_128B);
    memset(values[0], 0, SIZE_128B);
    memset(szSmartDevInfoURL, 0x00, sizeof(szSmartDevInfoURL));

    if (strlen(g_server_ip) > 0x00)
    {
	snprintf(szSmartDevInfoURL, sizeof(szSmartDevInfoURL), "http://%s:%d/smartDev.txt", g_server_ip, g_server_port);
	snprintf(values[0], SIZE_128B, "%s", szSmartDevInfoURL);
	APP_LOG("UPnPCtrPt",LOG_DEBUG, "PluginCtrlPointSetSmartDevUrl:values[0]=%s\n",values[0]);

	PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "SetSmartDevInfo", (const char **)paramNames, values, 0x01);
    }

    free(values[0]);
    return 0x00;	
}

int PluginCtrlPointShareHWInfo(int deviceIndex)
{
	char *paramNames[] = {"Mac", "Serial", "Udn","RestoreState","HomeId","PluginKey"};
	char*  values[6];
    	values[0] = (char*)malloc(SIZE_128B);
    	values[1] = (char*)malloc(SIZE_128B);	
	values[2] = (char*)malloc(SIZE_128B);
    	values[3] = (char*)malloc(SIZE_128B);
    	values[4] = (char*)malloc(SIZE_128B);	
	values[5] = (char*)malloc(SIZE_128B);

	memset(values[0], 0, SIZE_128B);
	memset(values[1], 0, SIZE_128B);
	memset(values[2], 0, SIZE_128B);
	memset(values[3], 0, SIZE_128B);
	memset(values[4], 0, SIZE_128B);
	memset(values[5], 0, SIZE_128B);
	
	snprintf(values[0], SIZE_128B, "%s", g_szWiFiMacAddress);
	snprintf(values[1], SIZE_128B, "%s", g_szSerialNo);	
	snprintf(values[2], SIZE_128B, "%s", g_szUDN);
	snprintf(values[3], SIZE_128B, "%s", g_szRestoreState);
	snprintf(values[4], SIZE_128B, "%s", g_szHomeId);
	snprintf(values[5], SIZE_128B, "%s", g_szPluginPrivatekey);
	
	APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointShareHWInfo:values[0]=%s\n",values[0]);
	APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointShareHWInfo:values[1]=%s\n",values[1]);
	APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointShareHWInfo:values[2]=%s\n",values[2]);
	APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointShareHWInfo:values[3]=%s\n",values[3]);
	APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointShareHWInfo:values[4]=%s\n",values[4]);
	APP_LOG("UPnPCtrPt",LOG_HIDE, "PluginCtrlPointShareHWInfo:values[5]=%s\n",values[5]);

	PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "ShareHWInfo", (const char **)paramNames, values, 0x06);

	free(values[0]);
    	free(values[1]);
	free(values[2]);
	free(values[3]);
    	free(values[4]);
	free(values[5]);
	
	return 0x00;	
}

int PluginCtrlPointCloseAp(int devnum)
{
    APP_LOG("UPnPCtrPt",LOG_DEBUG, "calling PluginCtrlPointCloseAp: %d device", devnum);
    PluginCtrlPointSendAction(PLUGIN_E_SETUP_SERVICE, devnum, "CloseSetup", 0x00, 0x00, 0x00);

    return UPNP_E_SUCCESS;
}

int PluginCtrlPointGetMetaInfo(int devnum)
{
    PluginCtrlPointSendAction(PLUGIN_E_METAINFO_SERVICE, devnum, "GetMetaInfo", 0x00, 0x00, 0x00);
    return 0;
}

int PluginGetNetworkStatus(int devnum)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "calling PluginGetNetworkStatus: %d device", devnum);
    PluginCtrlPointSendAction(PLUGIN_E_SETUP_SERVICE, devnum, "GetNetworkStatus", 0x00, 0x00, 0x00);

    return UPNP_E_SUCCESS;
}

int PluginCtrlPointGetApList(int devnum)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "calling PluginCtrlPointGetApList: %d device", devnum);

    PluginCtrlPointSendAction(PLUGIN_E_SETUP_SERVICE, devnum, "GetApList", 0x00, 0x00, 0x00);

    return UPNP_E_SUCCESS;
}

#define MAX_NETWORK_PARAMS 5
int  PluginCtrlPointConnectHomeNetwork(int deviceIndex, int channel, const char* ssid, const char* Auth, const char* Encrypt, const char* password)
{
    APP_LOG("UPnPCtrPt",LOG_HIDE, "channel: %d, ssid: %s, auth: %s, encrypto: %s, password: %s\n", channel, ssid, Auth, Encrypt, password);

    char *paramNames[] = {"ssid", "auth", "encrypt", "password", "channel"};
    char* values[MAX_NETWORK_PARAMS];
    values[0x00] = (char*)malloc(SIZE_32B);
    values[0x01] = (char*)malloc(SIZE_32B);
    values[0x02] = (char*)malloc(SIZE_32B);
    values[0x03] = (char*)malloc(SIZE_32B);
    values[0x04] = (char*)malloc(SIZE_32B);

    memset(values[0], 0x00, SIZE_32B);
    strncpy(values[0], ssid, SIZE_32B-1);

    memset(values[1], 0x00, SIZE_32B);
    strncpy(values[1], Auth, SIZE_32B-1);

    memset(values[2], 0x00, SIZE_32B);
    strncpy(values[2], Encrypt, SIZE_32B-1);

    memset(values[3], 0x00, SIZE_32B);
    strncpy(values[3], password, SIZE_32B-1);

    snprintf(values[4], SIZE_32B, "%d", channel);

    PluginCtrlPointSendAction(PLUGIN_E_SETUP_SERVICE, deviceIndex, "ConnectHomeNetwork", (const char **)paramNames, values, MAX_NETWORK_PARAMS);

    free(values[0x00]);
    free(values[0x01]);
    free(values[0x02]);
    free(values[0x03]);
    free(values[0x04]);

    APP_LOG("UPnPCtrPt",LOG_ERR, "PluginCtrlPointConnectHomeNetwork: command request\n");

    return UPNP_E_SUCCESS;
}


int PluginCtrlPointGetDevice(int devnum, pCtrlPluginDeviceNode *devnode)
{
    int count = devnum;
    pCtrlPluginDeviceNode tmpdevnode = NULL;

    if (count)
    {
	tmpdevnode = g_pGlobalPluginDeviceList;
    }
    else
    {
	return PLUGIN_ERROR;
    }

    while (--count && tmpdevnode)
    {
	tmpdevnode = tmpdevnode->next;
    }

    if (tmpdevnode)
	*devnode = tmpdevnode;
    else
	APP_LOG("UPnPCtrPt", LOG_ERR, "PluginCtrlPointGetDevice: can not find device");

    return PLUGIN_SUCCESS;
}

int PluginCtrlPointSendAction(int service, int devnum, const char *actionname, const char **param_name, char **param_val, int param_count)
{
    pCtrlPluginDeviceNode 	devnode = NULL;
    IXML_Document 			*actionNode = NULL;

    int rect = UPNP_E_SUCCESS;
    int param;

    char szTmpEventURL[SIZE_256B];
    memset(szTmpEventURL, 0x00, sizeof(szTmpEventURL));

    //-Lock and unlock quickly so that no delay 
    LockDeviceSync();

    rect = PluginCtrlPointGetDevice(devnum, &devnode);
    if (PLUGIN_SUCCESS == rect)
    {
	if (0x00 != devnode)
	    strncpy(szTmpEventURL, devnode->device.services[service].ControlURL, sizeof(szTmpEventURL)-1);
    }

    UnlockDeviceSync();

    if (0x00 != strlen(szTmpEventURL))
    {
	if (0 == param_count)
	{
	    actionNode = UpnpMakeAction(actionname, PluginDeviceServiceType[service], 0, NULL);
	}
	else
	{
	    for (param = 0; param < param_count; param++)
	    {
		rect = UpnpAddToAction(&actionNode, actionname, 
			PluginDeviceServiceType[service], param_name[param], param_val[param]);

		if (0x00 != rect)
		{
		    APP_LOG("UPnPCtrPt", LOG_ERR, "UpnpAddToAction: can not add action to list");
		}
	    }
	}

	if(PluginDeviceServiceType[service] != NULL)
	{
	    rect = UpnpSendActionAsync(ctrlpt_handle, szTmpEventURL, PluginDeviceServiceType[service], NULL, actionNode,
		    CtrlPointCallbackEventHandler, NULL);	

	    if (rect != UPNP_E_SUCCESS)
	    {
		APP_LOG("UPnPCtrPt",LOG_ERR, "Error in UpnpSendActionAsync -- %d\n", rect);
		rect = 0x01;
	    }

	    //TODO: This call can crash if service on which action it trying to send is not subscribed by any control point
	}

    }
    else
    {
	APP_LOG("UPnPCtrPt",LOG_ERR, "PluginCtrlPointSendAction: can not find device in device table");
    }

    if (actionNode)
	ixmlDocument_free(actionNode);

    return rect;
}

int PluginCtrlPointSendActionAll(int service, const char *actionname, const char **param_name, char **param_val, int param_count)
{
    IXML_Document 			*actionNode = NULL;
    pCtrlPluginDeviceNode tmpdevnode = NULL;

    int rect = UPNP_E_SUCCESS;
    int param;

    char szTmpEventURL[SIZE_256B];

    //-Lock and unlock quickly so that no delay 
    LockDeviceSync();

    tmpdevnode = g_pGlobalPluginDeviceList;

    while (tmpdevnode)
    {
#ifdef SIMULATED_OCCUPANCY
	if(!(tmpdevnode->Skip))
#endif
	{
	    memset(szTmpEventURL, 0x00, sizeof(szTmpEventURL));
	    strncpy(szTmpEventURL, tmpdevnode->device.services[service].ControlURL, sizeof(szTmpEventURL)-1);

	    if (0x00 != strlen(szTmpEventURL))
	    {
		if (0 == param_count)
		{
		    actionNode = UpnpMakeAction(actionname, PluginDeviceServiceType[service], 0, NULL);
		}
		else
		{
		    for (param = 0; param < param_count; param++)
		    {
			rect = UpnpAddToAction(&actionNode, actionname, 
				PluginDeviceServiceType[service], param_name[param], param_val[param]);

			if (0x00 != rect)
			{
			    APP_LOG("UPnPCtrPt", LOG_ERR, "UpnpAddToAction: can not add action to list");
			}
		    }
		}

		if(PluginDeviceServiceType[service] != NULL)
		{
		    rect = UpnpSendActionAsync(ctrlpt_handle, szTmpEventURL, PluginDeviceServiceType[service], NULL, actionNode,
			    CtrlPointCallbackEventHandler, NULL);	

		    if (rect != UPNP_E_SUCCESS)
		    {
			APP_LOG("UPnPCtrPt",LOG_ERR, "Error in UpnpSendActionAsync -- %d\n", rect);
			rect = 0x01;
		    }
		    else
		    {
			APP_LOG("UPnPCtrPt",LOG_ERR, "UpnpSendActionAsync %s successful for %s", actionname, tmpdevnode->device.UDN);
		    }
		}

	    }
	    else
	    {
		APP_LOG("UPnPCtrPt",LOG_ERR, "PluginCtrlPointSendAction: can not find device in device table");
	    }

	    if (actionNode)
	    {
		ixmlDocument_free(actionNode);
		actionNode = NULL;
	    }

	}
	tmpdevnode = tmpdevnode->next;
    }

    UnlockDeviceSync();

    return rect;
}

#ifdef SIMULATED_OCCUPANCY
int PluginCtrlPointSendActionSimulated(int service, const char *actionname)
{
	IXML_Document 	*actionNode = NULL;
	pCtrlPluginDeviceNode tmpdevnode = NULL;
	SimulatedDevInfo *simdeviceinf;
	int simdevicecnt = -1, i = 0; 
	int rect = UPNP_E_SUCCESS;
	char szTmpEventURL[SIZE_256B] = {'\0',};
	char szUDN[SIZE_256B] = {'\0',};


	LockSimulatedOccupancy();
	simdeviceinf = pgSimdevList;
	simdevicecnt = gSimulatedDeviceCount;
	UnlockSimulatedOccupancy();

	APP_LOG("Rule", LOG_DEBUG, "action: %s simdevicecnt: %d", actionname, simdevicecnt);

	if(PluginDeviceServiceType[service] == NULL)
	{
		APP_LOG("UPnPCtrPt", LOG_ERR, "Undefined service type: %d", service);
		return 0x1;
	}

	for(i = 0; i < simdevicecnt; i++)
	{
		memset(szUDN, 0, sizeof(szUDN));

		if(strstr(simdeviceinf[i].UDN, "uuid:Bridge") != NULL)
		{
			strncpy(szUDN, simdeviceinf[i].UDN, BRIDGE_UDN_LEN);
		}
		else if(strstr(simdeviceinf[i].UDN, "uuid:Maker") != NULL)
		{
			strncpy(szUDN, simdeviceinf[i].UDN, MAKER_UDN_LEN);
			strncat(szUDN, ":sensor:switch", sizeof(szUDN)-strlen(szUDN)-1);
		}
		else
			strncpy(szUDN, simdeviceinf[i].UDN, sizeof(szUDN)-1);

		APP_LOG("UPNP: Device", LOG_DEBUG, "Input UDN: %s converted UDN: %s", simdeviceinf[i].UDN, szUDN);

		LockDeviceSync();
		tmpdevnode = g_pGlobalPluginDeviceList;

		while (tmpdevnode)
		{
			if(strcmp(tmpdevnode->device.UDN, szUDN) == 0)  //a simulated device
			{
				APP_LOG("UPNP: Device", LOG_DEBUG, "device: %s in global list matched with device: %s in simulated list", tmpdevnode->device.UDN, szUDN);

				memset(szTmpEventURL, 0x00, sizeof(szTmpEventURL));
				strncpy(szTmpEventURL, tmpdevnode->device.services[service].ControlURL, sizeof(szTmpEventURL)-1);

				if (0x00 != strlen(szTmpEventURL))
				{
					rect = UpnpAddToAction(&actionNode, actionname, 
							PluginDeviceServiceType[service], (const char *)"UDN", (const char *)simdeviceinf[i].UDN);

					if (0x00 != rect)
					{
						APP_LOG("UPnPCtrPt", LOG_ERR, "UpnpAddToAction: can not add action to list");
					}
					else
					{
						rect = UpnpSendActionAsync(ctrlpt_handle, szTmpEventURL, PluginDeviceServiceType[service], NULL, actionNode,
								CtrlPointCallbackEventHandler, NULL);	

						if (rect != UPNP_E_SUCCESS)
						{
							APP_LOG("UPnPCtrPt",LOG_ERR, "Error in UpnpSendActionAsync -- %d\n", rect);
							rect = 0x01;
						}
						else
						{
							APP_LOG("UPnPCtrPt",LOG_ERR, "UpnpSendActionAsync %s successful for %s", actionname, szUDN);
						}
					}
				}
				else
				{
					APP_LOG("UPnPCtrPt",LOG_ERR, "PluginCtrlPointSendAction: can not find device in device table");
				}

				if (actionNode)
				{
					ixmlDocument_free(actionNode);
					actionNode = NULL;
				}


			}
			tmpdevnode = tmpdevnode->next;
		}
		UnlockDeviceSync();
	}

	return rect;
}
#endif

//------------------------------------------------------Now for new service ---------------------------------

int PluginCtrlPointSetFriendlyName(int deviceIndex, const char* name)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "name: %s", name);
    char *paramNames[] = {"FriendlyName"};

    PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "SetFriendlyName", (const char **)&paramNames, (char **)&name, 1);
    return 0;
}

int PluginCtrlPointSensorEventInd(int deviceIndex, int nowAction, int duration, int endAction, const char* udn)
{

    APP_LOG("UPnPCtrPt:Sensor", LOG_DEBUG, "######## %s called", __FUNCTION__);


    char *paramNames[] = {"BinaryState", "Duration", "EndAction", "UDN"};
    char *values[4];
    values[0x00] = (char*)malloc(SIZE_8B);
    values[0x01] = (char*)malloc(SIZE_8B);		
    values[0x02] = (char*)malloc(SIZE_8B);				
    values[0x03] = (char*)calloc(1, SIZE_256B);				
    snprintf(values[0x00], SIZE_8B, "%d", nowAction);
    snprintf(values[0x01], SIZE_8B, "%d", duration);
    snprintf(values[0x02], SIZE_8B, "%d", endAction);
    snprintf(values[0x03], SIZE_256B, "%s", udn);

    APP_LOG("UPnPCtrPt:Sensor", LOG_DEBUG, "start action: %d, command duration: %d, stop action: %d, udn: %s", 
	    nowAction, duration, endAction, udn);

    int rect = PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, 
	    "SetBinaryState", 
	    (const char **)&paramNames, (char **)&values, 4);

    free(values[0x00]);
    free(values[0x01]);
    free(values[0x02]);
    free(values[0x03]);

    return rect;

}


int PluginCtrlPointChangeBinaryState (int deviceIndex, int newValue)
{
    char values[8];
    memset(values, 0x00, 8);
    snprintf(values, 8, "%d", newValue);

    PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "SetBinaryState", 0x00, 0x00, 0x00);
    return 0;
}

/***
 *
 ***********************************************************************************************/
int PluginCtrlPointSetBinaryState (int deviceIndex, const char** name)
{

    //- TODO: ##################################
    int hasDevice = 0x00;
    LockDeviceSync();
    if (g_pGlobalPluginDeviceList)
	hasDevice = 0x01;
    UnlockDeviceSync();

    if (hasDevice)
    {
	char *paramNames[] = {"BinaryState"};
	PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "SetBinaryState", (const char **)&paramNames, (char **)name, 1);
    }
    else
    {
	APP_LOG("UPnPCtrPt:Sensor", LOG_ERR, "No device is discovery list");
    }
    return 0;
}


int PluginCtrlPointGetFriendlyName(int deviceIndex, const char* name)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "Get friendly name ......");

    PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "GetFriendlyName", 0x00, 0x00, 0x00);
    return 0;
}
int PluginCtrlPointGetBinary (int deviceIndex, const char* name)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "name: %s", name);

    PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "GetBinaryState", 0x00, 0x00, 0x00);
    return 0;
}
int PluginCtrlPointGetHomeId(int deviceIndex, const char* name)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "Get Home Id ......");

    PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "GetHomeId", 0x00, 0x00, 0x00);
    return 0;
}
int PluginCtrlPointGetDeviceId(int deviceIndex, const char* name)
{
    APP_LOG("UPnPCtrPt",LOG_ERR, "Get Device Id ......");

    PluginCtrlPointSendAction(PLUGIN_E_EVENT_SERVICE, deviceIndex, "GetDeviceId", 0x00, 0x00, 0x00);
    return 0;
}


void *AutoCtrlPointTestLoop(void *args)
{
    static int k = 0x01;
    pluginUsleep(30000000);
    while (1)
    {
	pluginUsleep(10000000);
	APP_LOG("UPnPCtrPt:AutoCtrlPointTestLoop", LOG_ERR, "###### Auto test command send ####");

	PluginCtrlPointChangeBinaryState(1, k % 2);
	k++;
    }
    return NULL;
}

void initDeviceSync()
{
    ithread_mutex_init(&DeviceListMutex, 0);
}


void LockDeviceSync()
{
    ithread_mutex_lock(&DeviceListMutex);
}

void UnlockDeviceSync()
{
    ithread_mutex_unlock(&DeviceListMutex);
}

void initSignNotify()
{
    ithread_mutex_init(&SignNotifyMutex, 0);
}

void LockSignNotify()
{
    ithread_mutex_lock(&SignNotifyMutex);
}

void UnlockSignNotify()
{
    ithread_mutex_unlock(&SignNotifyMutex);
}

int StopPluginCtrlPoint(void)
{
    if (-1 == ctrlpt_handle)
	return 0x00;

    StopDiscoverTask();

    APP_LOG("UPnPCtrPt", LOG_DEBUG, "StopPluginCtrlPoint Called");
    CtrlPointRemoveAll();
    UpnpUnRegisterClient( ctrlpt_handle );
    ctrlpt_handle = -1;
    APP_LOG("UPnPCtrPt", LOG_ERR, "StopPluginCtrlPoint: UpnpUnRegisterClient DONE");
    APP_LOG("UPnPCtrPt", LOG_ERR, "StopPluginCtrlPoint: UpnpFinish DONE");

    return PLUGIN_SUCCESS;
}

int CtrlPointRemoveAll(void)
{
    LockDeviceSync();

    pCtrlPluginDeviceNode curdevnode;
    pCtrlPluginDeviceNode next;

    curdevnode = g_pGlobalPluginDeviceList;

    while (curdevnode) 
    {
	curdevnode->IsDeviceRequestUpdate = 0x01;	//- Reset, requested update when next discovery respone comes
	CtrlPointDeleteNode(curdevnode);
	next = curdevnode->next;
	curdevnode = next;
    }

    APP_LOG("UPnPCtrPt", LOG_DEBUG, "CtrlPointRemoveAll done");
    UnlockDeviceSync();

    return PLUGIN_SUCCESS;
}
int CtrlPointDeleteNode( CtrlPluginDeviceNode *node )
{
    APP_LOG("UPnPCtrPt", LOG_DEBUG, "CtrlPointDeleteNode Called");
    int rc, service;

    if (NULL == node) 
    {
	APP_LOG("UPnPCtrPt", LOG_ERR, "ERROR: CtrlPointDeleteNode: Node is empty");
	return FAILURE;	
    }

    for (service = 0; service < PLUGIN_SERVICE_COUNT; service++)
    {
	if ((PLUGIN_E_EVENT_SERVICE == service) || (PLUGIN_E_REMOTE_ACCESS_SERVICE == service))
	{
	    if (strcmp(node->device.services[service].SID, "") != 0)
	    {
		rc = UpnpUnSubscribe(ctrlpt_handle, node->device.services[service].SID);
		if (UPNP_E_SUCCESS == rc) 
		{
		    APP_LOG("UPnPCtrPt", LOG_NOTICE, "SUCCESS: Unsubscribed: service: %s :SID=%s",
			    node->device.services[service].ServiceType, 
			    node->device.services[service].SID);
		}
		else 
		{
		    APP_LOG("UPnPCtrPt", LOG_ERR, "####### ERROR: unsubscribed: service: %s SID=%s, RC=%d", 
			    node->device.services[service].ServiceType, node->device.services[service].SID, rc);
		}
	    }
	}
    }

    APP_LOG("UPnPCtrPt", LOG_DEBUG, "CtrlPointDeleteNode done\n");

    return PLUGIN_SUCCESS;
}

int CtrlPointRediscoverCallBack(void)
{
    APP_LOG("UPNP: Device", LOG_DEBUG, "##################### Control point to re-discover ###################");


    CtrlPointDiscoverDevices();

    return 0x00;
}
