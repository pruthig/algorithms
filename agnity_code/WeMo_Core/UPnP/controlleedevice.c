/***************************************************************************
 *
 *
 * controlleedevice.c
 *
 * Created by Belkin International, Software Engineering on May 27, 2011
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
#include <ithread.h>
#include <upnp.h>
#include <sys/time.h>
#include <math.h>

#include "global.h"
#include "defines.h"
#include "fw_rev.h"
#include "logger.h"
#include "wifiHndlr.h"
#include "controlledevice.h"
#include "gpio.h"
#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#include "new_upgrade.h"
#include "itc.h"
#include "remoteAccess.h"
#include "pktStructs.h"
#include "rule.h"
#include "plugin_ctrlpoint.h"
#include "utils.h"
#include "mxml.h"
#include "sigGen.h"
#include "watchDog.h"
#include "upnpCommon.h"
#include "osUtils.h"
#include "xgethostbyname.h"
#include "gpioApi.h"
#ifdef PRODUCT_WeMo_Insight
#include "insight.h"
#endif
#include "getRemoteMac.h"

#ifdef WeMo_SMART_SETUP
#include "smartSetupUPnPHandler.h"
#endif

#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif

#include "thready_utils.h"

/* serial number length*/
#define MAX_SERIAL_LEN  14
#define SERIAL_TYPE_INDEX 6 //- The seventh digital indicating device type
#define SUB_DEVICE_TYPE_INDEX 8 //- The ninth digital indicating sub device type

#ifdef PRODUCT_WeMo_CrockPot
#include "crockpot.h"
#endif

extern int webAppFileDownload(char *url, char *outfilename);
extern void StopDownloadRequest(void);
extern int pj_dst_data_os(int, char *, int);
extern void nat_trav_destroy(void *);
extern void initBugsense(void);

#ifdef PRODUCT_WeMo_Insight
extern void Update30MinDataOnFlash();
#endif
int g_isRemoteAccessByApp = 0x00;
pthread_mutex_t g_remoteAccess_mutex;
pthread_cond_t g_remoteAccess_cond;
pthread_t remoteRegthread=-1;
pthread_attr_t remoteReg_attr;
ProxyRemoteAccessInfo *g_pxRemRegInf = NULL;
extern pluginRegisterInfo *pgPluginReRegInf;

pthread_attr_t updateFw_attr;
pthread_t fwUpMonitorthread=-1;
pthread_attr_t firmwareUp_attr;
pthread_t firmwareUpThread=-1;
pthread_attr_t wdLog_attr;
ithread_t logFile_thread=0;
int currFWUpdateState=0;
int gTimeZoneUpdated=0;

//Increase this time Download time to 10 minutes for now until new strategy/design is created.
#define MAX_FW_DL_TIME_OUT  10*60
char* ip_address 	= NULL;
char* desc_doc_name 	= NULL;
char* web_dir_path 	= NULL;

int g_lastTimeSync = 0x00;
int gWebIconVersion=0;
extern int gLastAuthVal;

//- It is a absolute time zone, should not be relative one from mobile app
float g_lastTimeZone = 0.0;
int   g_lastDstStatus = 0x00;

char  g_server_ip[SIZE_32B];
unsigned short g_server_port;
char gUserKey[PASSWORD_MAX_LEN];

extern char g_szApSSID[MAX_APSSID_LEN];
extern char g_routerMac[MAX_MAC_LEN];
extern char g_routerSsid[MAX_ESSID_LEN];
extern int gNTPTimeSet;
extern char g_szRestoreState[SIZE_2B];
extern int ctrlpt_handle;
extern int gBootReconnect; 
extern int gStopDownloadFW;
extern int gSignalStrength;
extern int gpluginNatInitialized;
extern int gNatClReInit24;


char g_szDeviceUDN[MAX_DEVICE_UDN_SIZE];

static char* DEFAULT_SERIAL_NO = "0123456789";
static int   DEFAULT_SERIAL_TAILER_SIZE = 3;
static pthread_attr_t reset_attr;
static pthread_t reset_thread = -1;

int g_eDeviceType = DEVICE_UNKNOWN;	//- Device type indentifier
int g_eDeviceTypeTemp = DEVICE_UNKNOWN;	//- Device type indentifier
int g_ra0DownFlag = 0;
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)
sqlite3 *g_APNSRuleDB=NULL;
char g_SendInsightParams =0;
int g_isExecuteTimerToo = 0;
int g_isDSTApplied = 0;
int g_overlappedInsightAction = 0;
int g_StateLog = 0;
#endif
int gSetupRequested = 0; 
int gAppCalledCloseAp = 0; 

char g_szWiFiMacAddress[SIZE_64B];
char g_szFriendlyName[SIZE_256B];

extern char g_szSerialNo[SIZE_64B];
extern char g_szProdVarType[SIZE_16B];
char g_szUDN[SIZE_128B];
char g_szUDN_1[SIZE_128B];
char g_szFirmwareVersion[SIZE_64B];
char g_szSkuNo[SIZE_64B];


char g_szFirmwareURLSignature[MAX_FW_URL_SIGN_LEN];

static  ithread_t ithPowerSensorMonitorTask = -1;
static  int sPowerDuration 	= 0x00;
static  int sPowerEndAction = -1;


char* g_szBuiltFirmwareVersion = 0x00;
char* g_szBuiltTime = 0x00;
pthread_attr_t dst_main_attr;
pthread_t dstmainthread = -1;
int gDstEnable=0;
extern int gDstSupported;


int g_iDstNowTimeStatus	= 0x00;
int gpluginStatusTS = 0;
pthread_mutex_t gFWUpdateStateLock;
pthread_mutex_t gSiteSurveyStateLock;

char g_szBootArgs[SIZE_128B];

extern char g_serverEnvIPaddr[SIZE_32B];
extern char g_turnServerEnvIPaddr[SIZE_32B];
extern SERVERENV g_ServerEnvType;

#ifdef WeMo_SMART_SETUP
extern int gSmartSetup;
#endif

#ifdef SIMULATED_OCCUPANCY
extern int gSimulatedDeviceCount;
extern SimulatedDevInfo *pgSimdevList;
extern int gSimulatedRuleRunning;
extern int gSelfIndex;
extern int gSimManualTrigger;
extern int gRemTimeToToggle;
#endif

char g_szActuation[SIZE_128B];
char g_szClientType[SIZE_128B];
char g_szRemote[SIZE_8B];

#define	    CONTROLLEE_DEVICE_STOP_WAIT_TIME	   5*1000000 

//--------------- Global Definition ------------ //- WiFi setup callback list
PluginDeviceUpnpAction g_Wifi_Setup_Actions[] = {
		{"GetApList", GetApList},
		{"GetNetworkList", GetNetworkList},
		{"ConnectHomeNetwork", ConnectHomeNetwork},
		{"GetNetworkStatus", GetNetworkStatus},
		{"SetSensorEvent", SetBinaryState},
		{"TimeSync", SyncTime},
		{"CloseSetup", CloseSetup},
		{"StopPair", StopPair},
};

PluginDeviceUpnpAction g_time_sync_Actions[] = {
		{"TimeSync", SyncTime}, 
		{"GetTime", 0x00}
};

//- Basic event callback list
PluginDeviceUpnpAction g_basic_event_Actions[] = {
#ifdef PRODUCT_WeMo_Insight
		{"SetInsightHomeSettings", SetInsightHomeSettings},
		{"GetInsightHomeSettings", GetInsightHomeSettings},
#endif
		{"SetBinaryState", SetBinaryState},
		{"SetMultiState", 0x00},
		{"GetBinaryState", GetBinaryState},
#ifdef PRODUCT_WeMo_CrockPot
		{"SetCrockpotState", SetCrockpotState}, /* Crockpot specific command */
		{"GetCrockpotState", GetCrockpotState}, /* Crockpot specific command */
#endif
		{"GetFriendlyName", GetFriendlyName},
		{"ChangeFriendlyName", SetFriendlyName},
		{"GetHomeId", GetHomeId},
		{"GetHomeInfo", GetHomeInfo},
		{"SetHomeId", SetHomeId},
		{"GetDeviceId", GetDeviceId},
		{"GetMacAddr", GetMacAddr},
		{"GetSerialNo", GetSerialNo},
		{"GetPluginUDN", GetPluginUDN},
		{"GetSmartDevInfo", GetSmartDevInfo},
		{"SetSmartDevInfo", SetSmartDevInfo},
		{"ShareHWInfo", GetShareHWInfo},
		{"SetDeviceId", SetDeviceId},
		{"GetIconURL", GetIcon},
		{"ReSetup", ReSetup},
		{"SetLogLevelOption", SetLogLevelOption},
		{"GetLogFileURL", GetLogFilePath},
		{"GetWatchdogFile", GetWatchdogFile},
		{"GetSignalStrength", SignalStrengthGet},
		{"SetServerEnvironment", SetServerEnvironment},
		{"GetServerEnvironment", GetServerEnvironment},
		{"GetIconVersion", GetIconVersion},
#ifdef PRODUCT_WeMo_Light
		{"SetNightLightStatus", SetNightLightStatus},
		{"GetNightLightStatus", GetNightLightStatus},
#endif
#ifdef SIMULATED_OCCUPANCY
		{"GetSimulatedRuleData", GetSimulatedRuleData},
		{"NotifyManualToggle", NotifyManualToggle},
#endif
};

PluginDeviceUpnpAction g_Rules_Actions[] = {
		{"UpdateWeeklyCalendar", UpdateWeeklyCalendar},
		{"EditWeeklycalendar", EditWeeklycalendar},
		{"GetRulesDBPath", GetRulesDBPath},
		{"SetRulesDBVersion", SetRulesDBVersion},
		{"GetRulesDBVersion", GetRulesDBVersion},
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)	
		{"SetRuleID", SetRuleID},
		{"DeleteRuleID", DeleteRuleID},
#endif
#ifdef SIMULATED_OCCUPANCY
		{"SimulatedRuleData", SimulatedRuleData},
#endif 
};	 



//- Firmware update callback list

PluginDeviceUpnpAction g_firmware_event_Actions[] = {
		{"UpdateFirmware", UpdateFirmware},
		{"GetFirmwareVersion", GetFirmwareVersion},
};

// - Remote access callback
PluginDeviceUpnpAction g_remote_access_Actions[] = {
		{"RemoteAccess", RemoteAccess},
};

PluginDeviceUpnpAction g_metaInfo_Actions[] = {
		{"GetMetaInfo", GetMetaInfo},
		{"GetExtMetaInfo", GetExtMetaInfo},
};

PluginDeviceUpnpAction g_deviceInfo_Actions[] = {
		{"GetDeviceInformation", GetDeviceInformation},
#ifdef WeMo_SMART_SETUP_V2
		{"GetInformation", GetInformation},
#endif
#ifdef WeMo_INSTACONNECT
		{"OpenInstaAP", OpenInstaAP},
		{"CloseInstaAP", CloseInstaAP},
		{"GetConfigureState", GetConfigureState},
		{"InstaConnectHomeNetwork", InstaConnectHomeNetwork},
		{"UpdateBridgeList", UpdateBridgeList},
		{"InstaRemoteAccess", InstaRemoteAccess},
		{"GetRouterInformation", GetRouterInformation}
#endif
};

#ifdef PRODUCT_WeMo_Insight
PluginDeviceUpnpAction g_insight_Actions[] = {
		{"GetInsightParams",GetInsightParams},
		{"GetPowerThreshold",GetPowerThreshold},
		{"SetPowerThreshold",SetPowerThreshold},
		{"SetAutoPowerThreshold",SetAutoPowerThreshold},
		{"ResetPowerThreshold",ResetPowerThreshold},
		{"ScheduleDataExport", ScheduleDataExport},
		{"GetDataExportInfo", GetDataExportInfo},
};
#endif

#ifdef WeMo_SMART_SETUP
PluginDeviceUpnpAction g_smart_setup_Actions[] = {
		{"PairAndRegister",PairAndRegister},
		{"GetRegistrationData",GetRegistrationData},
		{"GetRegistrationStatus",GetRegistrationStatus},
#ifdef WeMo_SMART_SETUP_V2
		{"NewPairAndRegister",PairAndRegister},
		{"NewGetRegistrationData",GetRegistrationData},
		{"SetCustomizedState",SetCustomizedState},
		{"GetCustomizedState",GetCustomizedState},
#endif
};
#endif
PluginDeviceUpnpAction g_manufacture_Actions[] = {
	{"GetManufactureData", GetManufactureData},
};



char *CtrleeDeviceServiceType[] = {"urn:Belkin:service:WiFiSetup:1",
		"urn:Belkin:service:timesync:1",
		"urn:Belkin:service:basicevent:1",
		"urn:Belkin:service:firmwareupdate:1",
		"urn:Belkin:service:rules:1",
		"urn:Belkin:service:metainfo:1",
#ifdef PRODUCT_WeMo_Insight
		"urn:Belkin:service:remoteaccess:1",
		"urn:Belkin:service:insight:1",
#else
		"urn:Belkin:service:remoteaccess:1",
#endif
		"urn:Belkin:service:deviceinfo:1",
#ifdef WeMo_SMART_SETUP
		"urn:Belkin:service:smartsetup:1",
#endif
		"urn:Belkin:service:manufacture:1"

};

char* szServiceTypeInfo[] = {"PLUGIN_E_SETUP_SERVICE",
		"PLUGIN_E_TIME_SYNC_SERVICE",
		"PLUGIN_E_EVENT_SERVICE",
		"PLUGIN_E_FIRMWARE_SERVICE",
		"PLUGIN_E_RULES_SERVICE",
		"PLUGIN_E_METAINFO_SERVICE",
#ifdef PRODUCT_WeMo_Insight
		"PLUGIN_E_REMOTE_ACCESS_SERVICE",
		"PLUGIN_E_INSIGHT_SERVICE",
#else
		"PLUGIN_E_REMOTE_ACCESS_SERVICE",
#endif
		"PLUGIN_E_DEVICEINFO_SERVICE",
#ifdef WeMo_SMART_SETUP
		"PLUGIN_E_SMART_SETUP_SERVICE",
#endif
		"PLUGIN_E_MANUFACTURE_SERVICE"

};

PluginDevice SocketDevice = {-1, PLUGIN_MAX_SERVICES};


char* s_szNtpServer = "192.43.244.18";	//in Default, use North America
#define DEFAULT_REGION_INDEX 5
#ifndef PRODUCT_WeMo_NetCam
tTimeZone g_tTimeZoneList[] = 
{
		{-12.0, 	1, "(GMT-12:00) Enewetak, Kwajalein"},
		{-11.0, 	2, "(GMT-11:00) Midway Island, Samoa"},
		{-10.0, 	3, "(GMT-10:00) Hawaii"},
		{-9.5, 		4, "(GMT-09:30) Marquesas Islands"},
		{-9.0, 		5, "(GMT-09:00) Alaska"},
		{-8.0, 		6, "(GMT-08:00) Pacific Time (US & Canada); Tijuana"},
		{-7.0, 		8, "(GMT-07:00) Mountain Time (US & Canada)"},
		{-6.0, 		9, "(GMT-06:00) Central Time (US & Canada)"},
		{-5.0, 		13, "(GMT-05:00) Eastern Time (US & Canada)"},		
		{-4.5, 		15, "(GMT-04:30) Venezuela, Caracas"},	
		{-4.0, 		16, "(GMT-04:00) Atlantic Time (Canada)"},	
		{-3.5, 		19, "(GMT-03:30) Newfoundland"},			
		{-3.0, 		20, "(GMT-03:00) Brasilia"},					
		{-2.0, 		22, "(GMT-02:00) Mid-Atlantic"},	
		{-1.0, 		23, "(GMT-01:00) Azores"},	
		{0.0, 		26, "(GMT) Greenwich Mean Time: Lisbon, London,Dublin, Edinburgh"},	
		{1.0, 		31, "(GMT+01:00) Paris, Sarajevo, Skopje"},	
		{2.0, 		35, "(GMT+02:00) Cairo"},			
		{3.0, 		40, "(GMT+03:00) Moscow, St. Petersburg,Volgograd, Kazan"},			
		{3.5, 		42, "(GMT+03:30) Iran"},
		{4.0, 		43, "(GMT+04:00) Abu Dhabi, Muscat, Tbilisi"},
		{4.5, 		44, "(GMT+04:30) Kabul, Afghanistan"},
		{5.0, 		46, "(GMT+05:00) Islamabad, Karachi"},
		{5.5, 		47, "(GMT+05:30) India, Sri Lanka"},
		{5.75, 		48, "(GMT+05:45) Nepal"},
		{6.0, 		49, "(GMT+06:00) Almaty, Dhaka"},
		{6.5, 		50, "(GMT+06:30) Cocos Islands, Myanmar"},
		{7.0, 		51, "(GMT+07:00) Bangkok, Jakarta, Hanoi"},
		{8.0, 		52, "(GMT+08:00) Beijing, Chongqing, Urumqi, Hong Kong, Perth, Singapore, Taipei"},
		{9.0, 		54, "(GMT+09:00) Toyko, Osaka, Sapporo"},
		{9.5, 		55, "(GMT+09:30) Northern Territory, South Australia"},
		{10.0, 		56, "(GMT+10:00) Brisbane"},
		{10.5, 		60, "(GMT+10:30) Lord Howe Island"},
		{11.0, 		62, "(GMT+11:00) Magada"},
		{11.5, 		63, "(GMT+11:30) Norfolk Island"},
		{12.0, 		64, "(GMT+12:00) Fiji, Kamchatka, Marshall Is."},
		{12.75, 	66, "(GMT+12:45) Chatham Islands"},
		{13.0, 		67, "(GMT+13:00) Tonga"},
		{14.0, 		68, "(GMT+14:00) Line Islands"}	
};
#else
tTimeZone g_tTimeZoneList[] = 
{
		{-12.0, 	1, "(GMT-12:00) Enewetak, Kwajalein", "Pacific/Kwajalein"},
		{-11.0, 	2, "(GMT-11:00) Midway Island, Samoa", "Pacific/Midway"},
		{-10.0, 	3, "(GMT-10:00) Hawaii", "US/Hawaii"},
		{-9.5, 		4, "(GMT-09:30) Marquesas Islands", "Pacific/Marquesas"},
		{-9.0, 		5, "(GMT-09:00) Alaska", "US/Alaska"},
		{-8.0, 		6, "(GMT-08:00) Pacific Time (US & Canada); Tijuana", "US/Pacific"},
		{-7.0, 		8, "(GMT-07:00) Mountain Time (US & Canada)", "US/Mountain"},
		{-6.0, 		9, "(GMT-06:00) Central Time (US & Canada)", "US/Central"},
		{-5.0, 		13, "(GMT-05:00) Eastern Time (US & Canada)", "US/Eastern"},		
		{-4.5, 		15, "(GMT-04:30) Venezuela, Caracas", "America/Caracas"},	
		{-4.0, 		16, "(GMT-04:00) Atlantic Time (Canada)", "Canada/Atlantic"},	
		{-3.5, 		19, "(GMT-03:30) Newfoundland", "Canada/Newfoundland"},			
		{-3.0, 		20, "(GMT-03:00) Brasilia", "Brazil/Brasilia"},					
		{-2.0, 		22, "(GMT-02:00) Mid-Atlantic", "Atlantic/South_Georgia"},	
		{-1.0, 		23, "(GMT-01:00) Azores", "Atlantic/Azores"},	
		{0.0, 		26, "(GMT) Greenwich Mean Time: Lisbon, London,Dublin, Edinburgh", "Europe/London"},	
		{1.0, 		31, "(GMT+01:00) Paris, Sarajevo, Skopje", "Europe/Paris"},	
		{2.0, 		35, "(GMT+02:00) Cairo", "Africa/Cairo"},			
		{3.0, 		40, "(GMT+03:00) Moscow, St. Petersburg,Volgograd, Kazan", "Europe/Moscow"},			
		{3.5, 		42, "(GMT+03:30) Iran", "Asia/Tehran", "Asia/Tehran"},
		{4.0, 		43, "(GMT+04:00) Abu Dhabi, Muscat, Tbilisi", "Asia/Muscat"},
		{4.5, 		44, "(GMT+04:30) Kabul, Afghanistan", "Asia/Kabul"},
		{5.0, 		46, "(GMT+05:00) Islamabad, Karachi", "Asia/Karachi"},
		{5.5, 		47, "(GMT+05:30) India, Sri Lanka", "Asia/Calcutta"},
		{5.75, 		48, "(GMT+05:45) Nepal", "Asia/Katmandu"},
		{6.0, 		49, "(GMT+06:00) Almaty, Dhaka", "Asia/Almaty"},
		{6.5, 		50, "(GMT+06:30) Cocos Islands, Myanmar", "Asia/Rangoon"},
		{7.0, 		51, "(GMT+07:00) Bangkok, Jakarta, Hanoi", "Asia/Bangkok"},
		{8.0, 		52, "(GMT+08:00) Beijing, Chongqing, Urumqi, Hong Kong, Perth, Singapore, Taipei", "Asia/Chongqing"},
		{9.0, 		54, "(GMT+09:00) Toyko, Osaka, Sapporo", "Asia/Toyko"},
		{9.5, 		55, "(GMT+09:30) Northern Territory, South Australia", "Australia/South"},
		{10.0, 		56, "(GMT+10:00) Brisbane", "Australia/Brisbane"},
		{10.5, 		60, "(GMT+10:30) Lord Howe Island", "Australia/Lord_Howe"},
		{11.0, 		62, "(GMT+11:00) Magada", "Asia/Magadan"},
		{11.5, 		63, "(GMT+11:30) Norfolk Island", "Pacific/Norfolk"},
		{12.0, 		64, "(GMT+12:00) Fiji, Kamchatka, Marshall Is.", "Pacific/Fiji"},
		{12.75, 	66, "(GMT+12:45) Chatham Islands", "Pacific/Chatham"},
		{13.0, 		67, "(GMT+13:00) Tonga", "Pacific/Tongatapu"},
		{14.0, 		68, "(GMT+14:00) Line Islands", "Pacific/Kiritimati"}	
};

#endif

#ifndef _OPENWRT_
/* nvram setting restore related definitions */
#define NVRAM_FILE_NAME "/tmp/Belkin_settings/nvram_settings.sh"
#define NVRAM_SETTING_BUF_SIZE	SIZE_2048B


char* g_apu8NvramVars[] = {
		"timezone_index",
		"restore_state",
		"home_id",
		"SmartDeviceId",
		"SmartPrivatekey",
		"token_id",
		"PluginCloudId",
		"plugin_key",
		"wl0_currentChannel",
		"cwf_serial_number",
		"ClientSSID",
		"ClientPass",
		"ClientAuth",
		"ClientEncryp",
		"APChannel",
		"RouterMac",
		"RouterSsid",
		"LastTimeZone",
		"DstSupportFlag",
		"LastDstEnable",
		"NotificationFlag",
		"FirmwareVersion",
		"SkuNo",
		"DeviceType",
		"StatusTS",
		"PVT_LOG_ENABLE",
		"FirmwareUpURL",
		"FriendlyName",
		"RuleDbVersion",
		"ntp_server1"
};
#endif

char* gDevTypeStringArr[] = { 
		"controllee", /* Default - unknown case */
		"controllee", /* DEVICE_SOCKET */
		"sensor", /* DEVICE_SENSOR */
		"wemo_baby", /* DEVICE_BABYMON */
		"stream", /* DEVICE_STREAMING - NOT USED */
		"bridge", /* DEVICE_BRIDGE - NOT USED */
		"insight", /* DEVICE_INSIGHT */
		"wemo_crockpot", /* DEVICE_CROCKPOT */
		"lightswitch", /* DEVICE_LIGHTSWITCH */
		"NetCamSensor", /* DEVICE_NETCAM */
		"sbiron", /* DEVICE_SBIRON */
		"mrcoffee", /* DEVICE_MRCOFFEE */
		"petfeeder", /* DEVICE_PETFEEDER */
		"smart", /* DEVICE_SMART */
		"maker" /* DEVICE_MAKER */
		"EchoWater", /* DEVICE_ECHO */
}; 

char* gDevUDNStringArr[] = { 
		"Socket", /* Default - unknown case */
		"Socket", /* DEVICE_SOCKET */
		"Sensor", /* DEVICE_SENSOR */
		"wemo_baby", /* DEVICE_BABYMON */
		"stream", /* DEVICE_STREAMING - NOT USED */
		"bridge", /* DEVICE_BRIDGE - NOT USED */
		"Insight", /* DEVICE_INSIGHT */
		"wemo_crockpot", /* DEVICE_CROCKPOT */
		"Lightswitch", /* DEVICE_LIGHTSWITCH */
		"NetCamSensor", /* DEVICE_NETCAM */
		"Sbiron",       /* DEVICE_SBIRON */
		"Mrcoffee",     /* DEVICE_MRCOFFEE */
		"Petfeeder", /* DEVICE_PETFEEDER */
		"Smart",        /* DEVICE_SMART */
		"Maker", /* DEVICE_MAKER */
		"EchoWater", /* DEVICE_ECHO */
}; 

char* gDefFriendlyName[] = { 
		DEFAULT_SOCKET_FRIENDLY_NAME, /* Default - unknown case */
		DEFAULT_SOCKET_FRIENDLY_NAME, /* DEVICE_SOCKET */
		DEFAULT_SENSOR_FRIENDLY_NAME, /* DEVICE_SENSOR */
		DEFAULT_BABY_FRIENDLY_NAME, /* DEVICE_BABYMON */
		DEFAULT_STREAMING_FRIENDLY_NAME, /* DEVICE_STREAMING - NOT USED */
		DEFAULT_BRIDGE_FRIENDLY_NAME, /* DEVICE_BRIDGE - NOT USED */
		DEFAULT_INSIGHT_FRIENDLY_NAME, /* DEVICE_INSIGHT */
		DEFAULT_CROCKPOT_FRIENDLY_NAME, /* DEVICE_CROCKPOT */
		DEFAULT_LIGHTSWITCH_FRIENDLY_NAME, /* DEVICE_LIGHTSWITCH */
		DEFAULT_NETCAM_FRIENDLY_NAME, /* DEVICE_NETCAM */
		DEFAULT_SBIRON_FRIENDLY_NAME, /* DEVICE_SBIRON */
		DEFAULT_MRCOFFEE_FRIENDLY_NAME, /* DEVICE_MRCOFFEE */
		DEFAULT_PETFEEDER_FRIENDLY_NAME,/* DEVICE_PETFEEDER */
		DEFAULT_SMART_FRIENDLY_NAME,/* DEVICE_SMART */
		DEFAULT_MAKER_FRIENDLY_NAME,/* DEVICE_MAKER */
		DEFAULT_ECHO_FRIENDLY_NAME,/* DEVICE_ECHO*/
}; 	


char* gDeviceClientType[] = { 
		":a3b6-41e8-afb5-a3430cea2dcd", /* Default - unknown case */
		":a3b6-41e8-afb5-a3430cea2dcd", /* DEVICE_SOCKET */
		":1a08-463e-8cbb-e4ea74e427ed", /* DEVICE_SENSOR */
		":", /* DEVICE_BABYMON */
		":", /* DEVICE_STREAMING : NOT USED */
		":52f8-406e-a5a6-5c5a2254868c", /* DEVICE_BRIDGE - NOT USED */
		":5e7e-40a7-8bf4-539d1dc2ce42", /* DEVICE_INSIGHT */
		":b136dbb7:5880-48f1-a347-084347ad22d9", /* DEVICE_CROCKPOT */
		":ccbb-4346-a240-70bcf8f53632", /* DEVICE_LIGHTSWITCH */
		":4f76-477f-a7ca-fd56a8f85390", /* DEVICE_NETCAM */
		":", /* DEVICE_SBIRON */
		":66bf-445d-8404-994bc95055bf", /* DEVICE_MRCOFFEE */
		":", /* DEVICE_PETFEEDER */
		":", /* DEVICE_SMART */
		"3310c367:c39b-4966-8b0e-07cbd3cbdde7", /* DEVICE_MAKER */
		":e19a9d16-1136-4519-a1ce-0732e2b2d848", /* DEVICE_ECHO*/
}; 




//--------------- Local Definition -------------

UpnpDevice_Handle device_handle = -1;
/*
	 The amount of time (in seconds) before advertisements
	 will expire
 */
int default_advr_expire = 86400;//24 * 60 *60;

//- In default, in local network
int g_IsUPnPOnInternet = FALSE;
//- In dedault, not in setup mode
int g_IsDeviceInSetupMode = FALSE;

extern char g_szHomeId[SIZE_20B];
extern char g_szSmartDeviceId[SIZE_256B];
extern char g_szSmartPrivateKey[MAX_PKEY_LEN];
extern char g_szPluginPrivatekey[MAX_PKEY_LEN];
extern char g_szPluginCloudId[SIZE_16B];
static pthread_mutex_t s_upnp_param_mutex;

//- store socket override status
#define	MAX_OVERRIDEN_STATUS_LEN	512
char szOverridenStatus[MAX_OVERRIDEN_STATUS_LEN];


void initUPnPThreadParam()
{
		ithread_mutexattr_t attr;
		ithread_mutexattr_init(&attr);
		ithread_mutexattr_setkind_np( &attr, ITHREAD_MUTEX_RECURSIVE_NP );
		ithread_mutex_init(&s_upnp_param_mutex, &attr);
		ithread_mutexattr_destroy(&attr);
}

void initFWUpdateStateLock()
{
		osUtilsCreateLock(&gFWUpdateStateLock);
}

void initSiteSurveyStateLock()
{
		osUtilsCreateLock(&gSiteSurveyStateLock);
}


void UPnPTimeSyncStatusNotify()
{
		char* szParamNames[] = {"TimeSyncRequest"};
		char* szParamValues[] = {"0"};
		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)szParamNames, (const char **)szParamValues, 0x01);

		APP_LOG("UPNP: DEVICE", LOG_DEBUG, "###############################Notification: time sync request sent");

		//- add the time zone and dst push notification to sensor, short term solution
		//- Below for NetCam, but should in SNS to support the timezone push notification
		char* tzIndex = GetBelkinParameter("timezone_index");
		if ((0x00 != tzIndex) && (0x00 != strlen(tzIndex)))
		{
				char* szTimeZone[] = {"TimeZoneNotification"};
				char* szTimeZoneValue[1] = {0x00};
				szTimeZoneValue[0] = (char*)malloc(SIZE_4B);
				memset(szTimeZoneValue[0], 0x00, SIZE_4B);
				snprintf(szTimeZoneValue[0], SIZE_4B, "%s", tzIndex);

				UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
								SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)szTimeZone, (const char **)szTimeZoneValue, 0x01);

				free(szTimeZoneValue[0]);
		}

#ifdef PRODUCT_WeMo_NetCam
		if (0x00 != tzIndex)
				free(tzIndex);
#endif
}

void UPnPSetHomeDeviceIdNotify()
{
		char* szParamValues[SIZE_2B];

		char* szParamNames[] = {"HomeIdRequest", "DeviceIdRequest"};
		szParamValues[0] = (char*)malloc(SIZE_20B);
		szParamValues[1] = (char*)malloc(SIZE_256B);

		memset(szParamValues[0], 0x0, SIZE_20B);
		memset(szParamValues[1], 0x0, SIZE_256B);

		snprintf(szParamValues[0], SIZE_20B, "%s", g_szHomeId);
		snprintf(szParamValues[1], SIZE_256B, "%s", g_szSmartDeviceId);	

		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)szParamNames, (const char **)szParamValues, 0x02);

		APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################Notification: HomeId - szParamValues[0]: <%s> and DeviceId - szParamValues[1]: <%s> sent", szParamValues[0], szParamValues[1]);

		free(szParamValues[0]);
		free(szParamValues[1]);
}

void UPnPSendSignatureNotify()
{
		char* szParamValues[1];
		authSign *assign = NULL;
		char sKey[MAX_PKEY_LEN];
		memset(sKey, 0x0, MAX_PKEY_LEN);
		strncpy(sKey, g_szPluginPrivatekey, sizeof(sKey)-1);

		char* szParamNames[] = {"PluginParam"};
		szParamValues[0] = (char*)malloc(SIZE_256B);
		memset(szParamValues[0], 0x0, SIZE_256B);

		assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, sKey);
		if (!assign) {
				APP_LOG("UPNP: DEVICE", LOG_ERR, "\n Signature Structure returned NULL\n");
				return;
		}

		APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################Before encryption: Signature: <%s>", assign->signature);
		encryptSignature(assign->signature, szParamValues[0]);

		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)szParamNames, (const char **)szParamValues, 0x01);

		APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################Notification: Signature- szParamValues[0]: <%s> sent", szParamValues[0]);

		free(szParamValues[0]);
		if (assign) {free(assign); assign = NULL;};
}

char* getDeviceTypeString(void)
{
		if(g_eDeviceTypeTemp)
		{
				APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device Type: %s", gDevTypeStringArr[g_eDeviceTypeTemp]);
				return gDevTypeStringArr[g_eDeviceTypeTemp];
		}
		else
		{
				APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device Type: %s", gDevTypeStringArr[g_eDeviceType]);
				return gDevTypeStringArr[g_eDeviceType];
		}

}

char* getDeviceUDNString(void)
{
		if(g_eDeviceTypeTemp)
		{
				APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device UDN: %s", gDevUDNStringArr[g_eDeviceTypeTemp]);
				return gDevUDNStringArr[g_eDeviceTypeTemp];
		}
		else
		{
				APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device UDN: %s", gDevUDNStringArr[g_eDeviceType]);
				return gDevUDNStringArr[g_eDeviceType];
		}

}

int getClientType(void)
{
	int index=0;

        if(g_eDeviceTypeTemp)
        {
		index = g_eDeviceTypeTemp;
        }
        else
        {
		index = g_eDeviceType;
        }

	memset(g_szClientType, 0, sizeof(g_szClientType));

	strncpy(g_szClientType, g_szFirmwareVersion,sizeof(g_szClientType) - 1);
        strncat(g_szClientType, gDeviceClientType[index], sizeof(g_szClientType) - strlen(g_szClientType) - 1);
	
        APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Idx: %d, Client Type: %s, len: %d", index, g_szClientType, strlen(g_szClientType));

	return SUCCESS;
}

void updateXmlDeviceTag(char *szBuff)
{

		char *pDevString;

		pDevString = getDeviceTypeString();

		if(0 == strcmp(pDevString, "wemo_baby"))
		{
				if(strlen(g_szProdVarType) > 0)
						pDevString = g_szProdVarType;
		}

		sprintf(szBuff, "<deviceType>urn:Belkin:device:%s:1</deviceType>\n", pDevString);
		APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device type tag: %s", szBuff);
}

void updateXmlUDNTag(char *szBuff, char *szBuff1)
{

		char *pDevString;

		pDevString = getDeviceUDNString();

		if(0 == strcmp(pDevString, "wemo_baby"))
		{
				if(strlen(g_szProdVarType) > 0)
						pDevString = g_szProdVarType;
		}

		sprintf(szBuff1, "uuid:%s-1_0-%s", pDevString, g_szSerialNo);
		sprintf(szBuff, "<UDN>%s</UDN>\n", szBuff1);
		APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device UDN tag: %s", szBuff);
}


int UpdateXML2Factory()
{
		FILE* pfReadStream  = 0x00;
		FILE* pfWriteStream = 0x00;
		char szBuff[SIZE_256B];
		char szBuff1[SIZE_256B];

		//- Open file to write
		//gautam: update the Insight and LS Makefile to copy Insightsetup.xml and Lightsetup.xml in /sbin/web/ as setup.xml 
		pfReadStream 	= fopen("/sbin/web/setup.xml", "r");
		pfWriteStream = fopen("/tmp/Belkin_settings/setup.xml", "w");

		if (0x00 == pfReadStream || 0x00 == pfWriteStream)
		{
				APP_LOG("UPNP: DEVICE", LOG_ERR, "UpdateXML2Factory: open files handles failure");

				if(pfReadStream)
						fclose(pfReadStream);

				if(pfWriteStream)
						fclose(pfWriteStream);

				return PLUGIN_UNSUCCESS;
		}

		while (!feof(pfReadStream))
		{
				memset(szBuff, 0x00, sizeof(szBuff));

				fgets(szBuff, SIZE_256B, pfReadStream);

				if (strstr(szBuff, "<deviceType>"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						updateXmlDeviceTag(szBuff);
				}
				else if (strstr(szBuff, "<UDN>"))
				{
						//- reset it again
						memset(szBuff, 0x00, sizeof(szBuff));
						memset(szBuff1, 0x00, sizeof(szBuff1));
						updateXmlUDNTag(szBuff, szBuff1);

						memset(g_szUDN, 0x00, sizeof(g_szUDN));
						strncpy(g_szUDN, szBuff1, sizeof(g_szUDN)-1);
						strncpy(g_szUDN_1, szBuff1, sizeof(g_szUDN_1)-1);

						APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Device UDN: %s", g_szUDN_1);
				}
				else if (strstr(szBuff, "<serialNumber>"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						snprintf(szBuff, SIZE_256B, "<serialNumber>%s</serialNumber>\n", g_szSerialNo);
				}
				else if (strstr(szBuff, "friendlyName"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						snprintf(szBuff, SIZE_256B, "<friendlyName>%s</friendlyName>\n", g_szFriendlyName);
				}
				else if (strstr(szBuff, "firmwareVersion"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						snprintf(szBuff, SIZE_256B, "<firmwareVersion>%s</firmwareVersion>\n", g_szFirmwareVersion);
				}
				else if (strstr(szBuff, "macAddress"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						snprintf(szBuff, SIZE_256B, "<macAddress>%s</macAddress>\n", g_szWiFiMacAddress);
				}
				else if (strstr(szBuff, "iconVersion"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						int port = 0;

						port = UpnpGetServerPort();
						snprintf(szBuff, SIZE_256B, "<iconVersion>%d|%d</iconVersion>\n", gWebIconVersion, port);
				}
				else if (strstr(szBuff, "binaryState"))
				{
						memset(szBuff, 0x00, sizeof(szBuff));
						int state = 0;

						state = GetCurBinaryState();
						snprintf(szBuff, SIZE_256B, "<binaryState>%d</binaryState>\n", state);
				}

				fwrite(szBuff, 1, strlen(szBuff), pfWriteStream);

		}

		fclose(pfReadStream);
		fclose(pfWriteStream);

		APP_LOG("UPNP", LOG_DEBUG, "Replace XML successfully\n");

		return 0x00;

}

void GetMacAddress()
{
		char* szMac = utilsRemDelimitStr(GetMACAddress(),":");
		memset(g_szWiFiMacAddress, 0x00, sizeof(g_szWiFiMacAddress));

		strncpy(g_szWiFiMacAddress, szMac, sizeof(g_szWiFiMacAddress)-1);
		free(szMac);
		APP_LOG("STARTUP", LOG_DEBUG, "MAC:%s", g_szWiFiMacAddress);
}


void GetFirmware()
{
		//char* szPreviousVserion = GetBelkinParameter("FirmwareVersion");

		char* szPreviousVserion = g_szBuiltFirmwareVersion;

		memset(g_szFirmwareVersion, 0x00, sizeof(g_szFirmwareVersion));
		if (0x00 == szPreviousVserion || 0x00 == strlen(szPreviousVserion))
		{
				snprintf(g_szFirmwareVersion, sizeof(g_szFirmwareVersion), "%s", DEFAULT_FIRMWARE_VERSION);
		}
		else
		{
				snprintf(g_szFirmwareVersion, sizeof(g_szFirmwareVersion), "%s", szPreviousVserion);
		}

		APP_LOG("Bootup", LOG_DEBUG, "Firmware:%s, built time: %s", g_szFirmwareVersion, g_szBuiltTime?g_szBuiltTime:"Unknown");

}
void GetSkuNo()
{
		char* szPreviousSkuNo   = GetBelkinParameter("SkuNo");
		memset(g_szSkuNo, 0x00, sizeof(g_szSkuNo));
		if (0x00 == szPreviousSkuNo || 0x00 == strlen(szPreviousSkuNo))
		{
				snprintf(g_szSkuNo, sizeof(g_szSkuNo), "%s", DEFAULT_SKU_NO);
		}
		else
		{
				snprintf(g_szSkuNo, sizeof(g_szSkuNo), "%s", szPreviousSkuNo);
		}


		APP_LOG("STARTUP", LOG_DEBUG, "SKU:%s", g_szSkuNo);
#ifdef PRODUCT_WeMo_NetCam
		if (szPreviousSkuNo)
				free(szPreviousSkuNo);
#endif

}

char* getDefaultFriendlyName()
{
		if(g_eDeviceTypeTemp)
		{
				APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Name : %s", gDefFriendlyName[g_eDeviceTypeTemp]);
				return gDefFriendlyName[g_eDeviceTypeTemp];
		}
		else
		{
				APP_LOG("UPNP: DEVICE", LOG_DEBUG, "Name: %s", gDefFriendlyName[g_eDeviceType]);
				return gDefFriendlyName[g_eDeviceType];
		}


}

void GetDeviceFriendlyName()
{
		char *pszFriendlyName = NULL;

		memset(g_szFriendlyName, 0x00, sizeof(g_szFriendlyName));
		char *szFriendlyName = GetDeviceConfig("FriendlyName");

		if ((0x00 != szFriendlyName) && (0x00 != strlen(szFriendlyName)))
		{
				strncpy(g_szFriendlyName, szFriendlyName, sizeof(g_szFriendlyName)-1);
		}
		else
		{
				pszFriendlyName = getDefaultFriendlyName();

				if(0 == strcmp(pszFriendlyName, DEFAULT_BABY_FRIENDLY_NAME))
				{
						char *pProdVar = GetBelkinParameter (PRODUCT_VARIANT_NAME);

						if(pProdVar!= NULL && (strlen(pProdVar) > 0)) 
								snprintf(g_szFriendlyName, sizeof(g_szFriendlyName), "%sBaby", pProdVar);
						else
								strncpy(g_szFriendlyName, pszFriendlyName, sizeof(g_szFriendlyName)-1);
				}
				else
						strncpy(g_szFriendlyName, pszFriendlyName, sizeof(g_szFriendlyName)-1);
		}

		APP_LOG("Startup", LOG_DEBUG, "Friendly Name: %s", g_szFriendlyName);
}

/**
 * GetDeviceType: This function identifies the device type 
 *                based on the device serial number 
 *
 * Following is the description of the serial no schema
 *  Supplier ID | Yr of Mfg | Wk of Mfg | Product | Unique Seq Id 
 *            22|12|38|K01|FFFFF
 * K = Relay based products
 *      K01- Switch 1.0US; K11-Switch 1.0 WW; K12-Insight; K13 - Light Switch; K14 - Crockpot
 * L = Sensors
 *      L01 - Motion Sensor 1.0 US; L11 - Motion Sensor 1.0 WW;
 * B = Bridges
 * M = Monitors
 * V = Video
 */

void GetDeviceType()
{
		char DevSerial[SIZE_64B]={'\0'};

		strncpy(DevSerial, g_szSerialNo, sizeof(DevSerial)-1);

		if (0x00 == strlen(g_szSerialNo))
		{
				APP_LOG("UPNP", LOG_ERR, "Device type unknow, please see the manufacture reference");
				g_eDeviceType = DEVICE_SOCKET;
				g_eDeviceTypeTemp = DEVICE_UNKNOWN;
		}
		else
		{
				switch(DevSerial[SERIAL_TYPE_INDEX])
				{
						case 'K':
								{
										//- Switch
										if ('1' == DevSerial[SERIAL_TYPE_INDEX+2])
										{
												APP_LOG("UPNP", LOG_DEBUG, "DEVICE: SOCKET");
												g_eDeviceType = DEVICE_SOCKET;
												g_eDeviceTypeTemp = DEVICE_UNKNOWN;
										}
										else if ('2' == DevSerial[SERIAL_TYPE_INDEX+2])
										{
												APP_LOG("UPNP", LOG_DEBUG, "DEVICE: INSIGHT");
												g_eDeviceType = DEVICE_SOCKET;
												g_eDeviceTypeTemp = DEVICE_INSIGHT;
										}
										else if ('3' == DevSerial[SERIAL_TYPE_INDEX+2])
										{
												APP_LOG("UPNP", LOG_DEBUG, "DEVICE: LIGHTSWITCH");
												g_eDeviceType = DEVICE_SOCKET;
												g_eDeviceTypeTemp = DEVICE_LIGHTSWITCH;
										}
										else if ('4' == DevSerial[SERIAL_TYPE_INDEX+2])
										{
												APP_LOG("UPNP", LOG_DEBUG, "DEVICE: CROCKPOT");
												g_eDeviceType = DEVICE_CROCKPOT;
												g_eDeviceTypeTemp = DEVICE_UNKNOWN;
										}
								}
								break;

						case 'L':
								{
										APP_LOG("UPNP", LOG_DEBUG, "DEVICE: SENSOR");
										g_eDeviceType = DEVICE_SENSOR;
										g_eDeviceTypeTemp = DEVICE_UNKNOWN;
								}
								break;

						case 'M':
								{
									APP_LOG("UPNP", LOG_DEBUG, "DEVICE: BABYMON");
									g_eDeviceType = DEVICE_SOCKET;
									g_eDeviceTypeTemp = DEVICE_BABYMON;
								}
								break;

						case 'B':
								{
									APP_LOG("UPNP", LOG_DEBUG, "DEVICE: BRIDGE");
									g_eDeviceType = DEVICE_SOCKET;
									g_eDeviceTypeTemp = DEVICE_BRIDGE;
								}
								break;



						case 'V':
								{
										//- NetCam sensor, still as a sensor
										APP_LOG("UPNP", LOG_DEBUG, "DEVICE: NetCam SENSOR");
										g_eDeviceType = DEVICE_SENSOR;
										g_eDeviceTypeTemp = DEVICE_NETCAM;
								}
								break;

						default:
								{
										/* Default device type is Socket/Switch */
										APP_LOG("UPNP", LOG_DEBUG, "DEVICE: Default SOCKET");
										g_eDeviceType = DEVICE_SOCKET;
										g_eDeviceTypeTemp = DEVICE_UNKNOWN;
								}
								break;
				}
		}
}

void serverEnvIPaddrInit(void)
{
		memset(g_serverEnvIPaddr, 0x0, sizeof(g_serverEnvIPaddr));
		memset(g_turnServerEnvIPaddr, 0x0, sizeof(g_turnServerEnvIPaddr));
		g_ServerEnvType = E_SERVERENV_PROD;
}

void initDeviceUPnP()
{
		char *iconVer;

		//- Get device type   
		/** Move serial request to top since it is the king element*/
		initSerialRequest();
		GetDeviceType();

		GetMacAddress();
		GetSkuNo();
		GetFirmware();
		GetDeviceFriendlyName();

		/* load icon version */
		iconVer = GetBelkinParameter(ICON_VERSION_KEY);
		if(iconVer && strlen(iconVer))
		{
				gWebIconVersion = atoi(iconVer);
		}
		APP_LOG("UPNP", LOG_DEBUG, "saved icon version: %d", gWebIconVersion);


		//Copy all xml file from etc to MTD
		system("rm -rf /tmp/Belkin_settings/*.xml");

		system("cp -f /sbin/web/* /tmp/Belkin_settings");

		//gautam: update the Insight and LS Makefile to copy Insightsetup.xml and Lightsetup.xml in /sbin/web/ as setup.xml 

		//Change some tags of setup.xml binding to device related 

		//-Check existence of icon file
		FILE* pFileIcon = fopen("/tmp/Belkin_settings/icon.jpg", "r");
		if (0x00 == pFileIcon)
		{
				//-Icon not existing, copy the factory one
				APP_LOG("UPNP", LOG_DEBUG, "icon not found, using the default one");
				if ((DEVICE_SOCKET == g_eDeviceType) || (DEVICE_CROCKPOT == g_eDeviceType))
				{   
						//gautam: update the Insight and LS Makefile to copy Insight.png and Light.png in /etc/ as icon.jpg
						system("cp /etc/icon.jpg /tmp/Belkin_settings");
				}
				else
				{
						system("cp /etc/sensor.jpg /tmp/Belkin_settings/icon.jpg");
				}
		}
		else
		{
				fclose(pFileIcon);
				pFileIcon = 0x00;
		}

		UpdateXML2Factory();

		initUPnPThreadParam();
		serverEnvIPaddrInit();

		getClientType();

		/* not the ideal place but to make integration transparent for all Applications */
		//initBugsense();
}


int ControlleeDeviceCallbackEventHandler( Upnp_EventType EventType,
				void *Event,
				void *Cookie)
{

		switch (EventType)
		{

				case UPNP_EVENT_SUBSCRIPTION_REQUEST:

						PluginDeviceHandleSubscriptionRequest( ( struct
												Upnp_Subscription_Request
												* )Event );
						break;

				case UPNP_CONTROL_GET_VAR_REQUEST:
						break;

				case UPNP_CONTROL_ACTION_REQUEST:
						{
								struct Upnp_Action_Request* pEvent = (struct Upnp_Action_Request*)Event;
								CtrleeDeviceHandleActionRequest(pEvent);
						}

						break;

				case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
				case UPNP_DISCOVERY_SEARCH_RESULT:
				case UPNP_DISCOVERY_SEARCH_TIMEOUT:
				case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
				case UPNP_CONTROL_ACTION_COMPLETE:
				case UPNP_CONTROL_GET_VAR_COMPLETE:
				case UPNP_EVENT_RECEIVED:
				case UPNP_EVENT_RENEWAL_COMPLETE:
				case UPNP_EVENT_SUBSCRIBE_COMPLETE:
				case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
						break;

				default:
						APP_LOG("UPNP",LOG_ERR, "Error in ControlleeDeviceCallbackEventHandler: unknown event type %d\n",
										EventType );
		}

		return (0);
}

void FreeActionRequestSource(struct Upnp_Action_Request* pEvent)
{
		if (0x00 == pEvent)
				return;

		if (pEvent->SoapHeader)
				ixmlDocument_free(pEvent->SoapHeader);
		if (pEvent->ActionRequest)
				ixmlDocument_free(pEvent->ActionRequest);
		if (pEvent->ActionResult)
				ixmlDocument_free(pEvent->ActionResult);

}

int CtrleeDeviceHandleActionRequest(struct Upnp_Action_Request *pActionRequest)
{
		int rect = UPNP_E_SUCCESS;
		int loop = 0x00;
		char *errorString = NULL;

		if (0x00 == pActionRequest || !pActionRequest->DevUDN || 0x00 == strlen(pActionRequest->DevUDN))
		{
				APP_LOG("UPNP", LOG_ERR, "Parameters error");
				return 0x01;
		}

		for (loop = 0x00; loop < PLUGIN_MAX_SERVICES; loop++)
		{
				//- to locate service containing this command
				if (0x00 == strcmp(pActionRequest->DevUDN, SocketDevice.service_table[loop].UDN) &&
								0x00 == strcmp(pActionRequest->ServiceID, SocketDevice.service_table[loop].ServiceId))
				{
						break;
				}
		}

		if (PLUGIN_MAX_SERVICES == loop)
		{
				APP_LOG("UPNP",LOG_ERR, "Action service not found: %s", pActionRequest->ServiceID);
				rect = UPNP_E_INVALID_SERVICE;
				return rect;
		}

		//- Service found, and to locate the action name and callback function
		{
				int cntActionNo = SocketDevice.service_table[loop].cntTableSize;
				int index = 0x00;

				if (0x00 == cntActionNo)
				{
						APP_LOG("UPNP",LOG_ERR, "No device action found in actions table");
						return rect;
				}


				for (index = 0x00; index < cntActionNo; index++)
				{
						PluginDeviceUpnpAction* pTable = SocketDevice.service_table[loop].ActionTable;
						if (0x00 != pTable)
						{
								if (0x00 == strcmp(pActionRequest->ActionName, (pTable + index)->actionName))
								{
										APP_LOG("UPNP",LOG_DEBUG, "Action found: %s", pActionRequest->ActionName);
										break;
								}
						}
						else
						{	
								APP_LOG("UPNP",LOG_ERR, "Action table not set");		
								return rect;
						}
				}

				if (cntActionNo == index)
				{
						APP_LOG("UPNP",LOG_ERR, "Action not found: %s", pActionRequest->ActionName);		
						return 0x01;
				}

				{
						PluginDeviceUpnpAction* pAction = SocketDevice.service_table[loop].ActionTable + index;
						if (0x00 == pAction)
						{
								APP_LOG("UPNP",LOG_ERR, "Action entry empty");	
								return 0x01;
						}

						if (pAction->pUpnpAction)
								pAction->pUpnpAction(pActionRequest, pActionRequest->ActionRequest, &pActionRequest->ActionResult, (const char **)&errorString);
						else
								APP_LOG("UPNP", LOG_WARNING, "Action name found: %s, but callback entry not set", pActionRequest->ActionName);
				}


		}

		//- Get service type

		return rect;

}


int ControlleeDeviceStop()
{

		if (-1 == device_handle)
				return 0x00;

		UpnpUnRegisterRootDevice(device_handle);
		device_handle = -1;
		UpnpFinish();

		APP_LOG("UPNP", LOG_DEBUG, "UPNP is to stop for setup");

		return 0x00;

}

static struct UpnpVirtualDirCallbacks VirtualCallBack;

int ControlleeDeviceStart(char *ip_address,
				unsigned short port,
				char *desc_doc_name,
				char *web_dir_path)
{
		int ret = UPNP_E_SUCCESS;
		char desc_doc_url[MAX_FW_URL_LEN];

		//shutdown the previous instance of UPNP, if any
		ControlleeDeviceStop();


		memset(g_server_ip, 0x00, sizeof(g_server_ip));
		g_server_port = 0x00;

		VirtualCallBack.open 		= (void *)&OpenWebFile;
		VirtualCallBack.get_info 	= &GetFileInfo;
		VirtualCallBack.write	  	= (void *)&PostFile;
		VirtualCallBack.read	 	= 0x00;
		VirtualCallBack.seek	 	= 0x00;
		VirtualCallBack.close	 	= 0x00;

		if(strcmp(ip_address, WAN_IP_ADR) == 0)
				port = 49152;
		else
				port = 49153;

		APP_LOG("UPNP",LOG_DEBUG, "Initializing UPnP Sdk with ipaddress = %s port = %u", ip_address, port);

		if( ( ret = UpnpInit( ip_address, port, g_szUDN_1) ) != UPNP_E_SUCCESS )
		{
				APP_LOG("UPNP",LOG_CRIT, "Error with UpnpInit -- %d\n", ret );
				UpnpFinish();
				return ret;
		}

		if( ip_address == NULL ) 
		{ 
				ip_address = UpnpGetServerIpAddress();
		}

		strncpy(g_server_ip, ip_address, sizeof(g_server_ip)-1);

		port = g_server_port = UpnpGetServerPort();

		APP_LOG("UPNP",LOG_CRIT, "UPnP Initialized ipaddress= %s port = %u",
						ip_address, port );

		if( desc_doc_name == NULL ) {
				//gautam: update the Insight and LS Makefile to copy Insightsetup.xml and Lightsetup.xml in /sbin/web/ as setup.xml 
				desc_doc_name = "setup.xml";

		}

		if( web_dir_path == NULL ) {
				web_dir_path = DEFAULT_WEB_DIR;
		}

		snprintf( desc_doc_url, MAX_FW_URL_LEN, "http://%s:%d/%s", ip_address, port, desc_doc_name );

		UpdateXML2Factory();
		AsyncSaveData();

		APP_LOG("UPNP",LOG_DEBUG, "Specifying the webserver root directory -- %s\n",
						web_dir_path );
		if( ( ret =
								UpnpSetWebServerRootDir( web_dir_path ) ) != UPNP_E_SUCCESS ) {
				APP_LOG("UPNP",LOG_ERR, "Error specifying webserver root directory -- %s: %d\n",
								web_dir_path, ret );
				UpnpFinish();
				return ret;
		}

		APP_LOG("UPNP",LOG_DEBUG,
						"Registering the RootDevice\n"
						"\t with desc_doc_url: %s\n",
						desc_doc_url );


		UpnpEnableWebserver(TRUE);
		ret = UpnpSetVirtualDirCallbacks(&VirtualCallBack);

		if (ret == UPNP_E_SUCCESS)
		{
				APP_LOG("UPNP", LOG_DEBUG, "UpnpSetVirtualDirCallbacks: success");
		}
		else
		{
				APP_LOG("UPNP", LOG_ERR, "UpnpSetVirtualDirCallbacks failure");
		}

		ret = UpnpAddVirtualDir("./");
		if (UPNP_E_SUCCESS != ret)
		{
				APP_LOG("UPNP", LOG_ERR, "Add virtual directory failure");
		}
		else
		{
				APP_LOG("UPNP", LOG_DEBUG, "Add virtual directory success");
		}


		if( ( ret = UpnpRegisterRootDevice( desc_doc_url,
										ControlleeDeviceCallbackEventHandler,
										&device_handle, &device_handle ) )
						!= UPNP_E_SUCCESS ) {
				APP_LOG("UPNP",LOG_ERR, "Error registering the rootdevice : %d\n", ret );
				UpnpFinish();
				return ret;
		}
		else
		{
				APP_LOG("UPNP",LOG_DEBUG, "RootDevice Registered\nInitializing State Table\n");
				ControlleeDeviceStateTableInit(desc_doc_url);
				APP_LOG("UPNP",LOG_DEBUG, "State Table Initialized\n");

				if( ( ret =
										UpnpSendAdvertisement( device_handle, default_advr_expire ) )
								!= UPNP_E_SUCCESS ) {
						APP_LOG("UPNP",LOG_ERR, "Error sending advertisements : %d\n", ret );
						UpnpFinish();
						return ret;
				}

				APP_LOG("UPNP",LOG_DEBUG, "Advertisements Sent\n");
		}

		if (getCurrentClientState())
		{
				initRule();
		}

		return UPNP_E_SUCCESS;
}

int ControlleeDeviceStateTableInit(char *DescDocURL)
{
		IXML_Document *DescDoc = NULL;
		int ret = UPNP_E_SUCCESS;
		char *servid = NULL;
		char *evnturl = NULL;
		char *ctrlurl = NULL;
		char *udn = NULL;

		/*Download description document */
		if (UpnpDownloadXmlDoc(DescDocURL, &DescDoc) != UPNP_E_SUCCESS)
		{
				APP_LOG("UPNP",LOG_DEBUG, "Controllee device table initialization -- Error Parsing %s\n",
								DescDocURL);
				ret = UPNP_E_INVALID_DESC;
				ixmlDocument_free(DescDoc);

				return ret;
		}
		else
		{
				APP_LOG("UPNP",LOG_DEBUG, "Down load %s success", DescDocURL);
		}

		udn = Util_GetFirstDocumentItem(DescDoc, "UDN");
		memset(g_szUDN, 0x00, sizeof(g_szUDN));

		if (udn) 
		{
				APP_LOG("UPNP",LOG_DEBUG, "UDN: %s\n", udn);
				strncpy(g_szUDN, udn, sizeof(g_szUDN)-1);
		} 
		else 
		{
				APP_LOG("UPNP",LOG_ERR, "UDN: reading failure");
		}

		//-Add setup service here
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE]);		   

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_SETUP_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_SETUP_SERVICE]);
		}

		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);
		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;


		//- Add Sync time service
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_TIME_SYNC_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_TIME_SYNC_SERVICE]);

		}

		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);
		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;

		//- Add basic event service
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_EVENT_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE]);
		}
		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);

		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;

		//- Add firmware update service
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_FIRMWARE_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_FIRMWARE_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_FIRMWARE_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_FIRMWARE_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_FIRMWARE_SERVICE]);
		}
		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);

		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;
		//- Add rule service
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_RULES_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_RULES_SERVICE]);
		}

		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);

		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;

		//-Add remote access service here
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_REMOTE_ACCESS_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_REMOTE_ACCESS_SERVICE]);
		}

		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);


		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;
		//- Add Meta service here
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_METAINFO_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_METAINFO_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_METAINFO_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_METAINFO_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_METAINFO_SERVICE]);
		}

		FreeXmlSource(servid);
	    FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);
		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;
		
#ifdef PRODUCT_WeMo_Insight
		//- Add Insight service here
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_INSIGHT_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_INSIGHT_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_INSIGHT_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_INSIGHT_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_INSIGHT_SERVICE]);
		}

		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);
		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;
#endif

		//- Add Device Information service here
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_DEVICEINFO_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_DEVICEINFO_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_DEVICEINFO_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_DEVICEINFO_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_DEVICEINFO_SERVICE]);
		}

		FreeXmlSource(servid);
		FreeXmlSource(evnturl);
		FreeXmlSource(ctrlurl);
		servid = NULL;
		evnturl = NULL;
		ctrlurl = NULL;

#ifdef WeMo_SMART_SETUP
		//- Add smart setup service here
		if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_SMART_SETUP_SERVICE], &servid, &evnturl, &ctrlurl)) 
		{
				APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_SMART_SETUP_SERVICE]);

				ret = UPNP_E_INVALID_DESC;
				goto FreeServiceResource;


				return ret;
		}
		else
		{
				CtrleeDeviceSetServiceTable(PLUGIN_E_SMART_SETUP_SERVICE, udn, servid,
								CtrleeDeviceServiceType[PLUGIN_E_SMART_SETUP_SERVICE],
								&SocketDevice.service_table[PLUGIN_E_SMART_SETUP_SERVICE]);
		}
#endif


//- Add Manufacture Information service here
  if (!Util_FindAndParseService(DescDoc, DescDocURL, CtrleeDeviceServiceType[PLUGIN_E_MANUFACTURE_SERVICE], &servid, &evnturl, &ctrlurl))
  {
    APP_LOG("UPNP",LOG_ERR, "%s -- Error: Could not find Service: %s\n", __FILE__, CtrleeDeviceServiceType[PLUGIN_E_MANUFACTURE_SERVICE]);

    ret = UPNP_E_INVALID_DESC;
    goto FreeServiceResource;
  }
  else
  {
    CtrleeDeviceSetServiceTable(PLUGIN_E_MANUFACTURE_SERVICE, udn, servid,
        CtrleeDeviceServiceType[PLUGIN_E_MANUFACTURE_SERVICE],
        &SocketDevice.service_table[PLUGIN_E_MANUFACTURE_SERVICE]);
  }


FreeServiceResource:
		{				
				ixmlDocument_free(DescDoc);
				FreeXmlSource(servid);
				FreeXmlSource(evnturl);
				FreeXmlSource(ctrlurl);
				FreeXmlSource(udn);

				servid = NULL;
				evnturl = NULL;
				ctrlurl = NULL;
		}


		return ret;
}

int CtrleeDeviceSetServiceTable(int serviceType,
				const char* UDN,
				const char* serviceId,
				const char* szServiceType,
				pPluginService pService)
{
		strncpy(pService->UDN, UDN, sizeof(pService->UDN)-1);
		strncpy(pService->ServiceId, serviceId, sizeof(pService->ServiceId)-1);
		strncpy(pService->ServiceType, szServiceType, sizeof(pService->ServiceType)-1);

		CtrleeDeviceSetActionTable(serviceType,

						&SocketDevice.service_table[serviceType]);

		return 0x00;
}

#ifdef PRODUCT_WeMo_CrockPot
/*
 * Set the Crockpot Status as requested the remote cloud server.
 */
int SetCrockpotStatus(int mode, int cooktime)
{
		int ret;
		/* Mode value varies from 0x0, 0x1 and 50 to 52 for the Cloud
		 * instead of 0x1 to 0x6 of local mode
		 */
		APP_LOG("REMOTE",LOG_DEBUG, "In SetCrockPotStatus Function: mode:%d, time:%d\n", mode, cooktime);      

		if(APP_MODE_ERROR != mode) {
				ret = HAL_SetMode(mode);
				if(ret >= 0) {        
						APP_LOG("REMOTE",LOG_DEBUG, "Crockpot Mode set to: %d\n", mode); 
				}
				else {
						APP_LOG("REMOTE",LOG_DEBUG, "Unable to Set Crockpot Mode to: %d\n", mode);
				}
		} /* if(APP_MODE_ERROR)... */

		if(APP_TIME_ERROR != cooktime) {
				ret = HAL_SetCookTime(cooktime);
				if(ret >= 0) {
						APP_LOG("REMOTE",LOG_DEBUG, "Crockpot time set to: %d\n", cooktime); 
				}
				else {
						APP_LOG("REMOTE",LOG_DEBUG, "Unable to Set Crockpot Cooktime to: %d\n", cooktime);
				}
		} /* if(APP_TIME_ERROR.).. */

		return 0;
}

/*
 * Get the current Crockpot Status as requested the remote cloud server.
 */
int GetCrockpotStatus(int *mode, int *cooktime)
{
		int tempMode;
		/* Mode value varies from 0, 1 and 0x32 to 0x34 for the Cloud
		 * instead of 0x0 to 0x4 of local mode
		 */

		*mode = HAL_GetMode();     
		tempMode = *mode;
		*mode = convertModeValue(tempMode,FROM_CROCKPOT);

		*cooktime = HAL_GetCookTime();

		return 0;
}
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/

int PluginDeviceHandleSubscriptionRequest( IN struct Upnp_Subscription_Request
				*sr_event )
{
		if (!sr_event)
		{
				APP_LOG("UPNPDevice", LOG_ERR,"Service subscription: parameter error, request stop");

				return 0x01;
		}
#ifdef PRODUCT_WeMo_CrockPot_DISABLE
		char* paramters[] = {"mode", "time"};
		char* values[SIZE_2B];
		int curState = 0x00;
		int mode=0, time=0, tmpMode=0;
		values[0x00] = (char*)malloc(SIZE_4B);
		values[0x01] = (char*)malloc(SIZE_4B);
#else
		char* paramters[] = {"BinaryState"};
		char* values[1];
		int curState = 0x00;
#ifdef PRODUCT_WeMo_Insight
		values[0x00] = (char*)malloc(SIZE_100B);
#else
		values[0x00] = (char*)malloc(SIZE_4B);
#endif /*#ifdef PRODUCT_WeMo_Insight */
#endif /*#ifdef PRODUCT_WeMo_CrockPot */

		if (DEVICE_SOCKET == g_eDeviceType)
		{
				//- Power state
				LockLED();
				curState = GetCurBinaryState();
				UnlockLED();
		}
#ifdef PRODUCT_WeMo_CrockPot_DISABLE
		else if(DEVICE_CROCKPOT == g_eDeviceType)
		{
				curState = GetCrockpotStatus(&mode,&time);
				tmpMode = convertModeValue(mode,FROM_CROCKPOT);

				snprintf(values[0], SIZE_4B, "%d", tmpMode);
				snprintf(values[1], SIZE_4B, "%d", time);
		}
#endif /* #ifdef PRODUCT_WeMo_CrockPot */
		else
		{
				//-Sensor state
				curState = GetSensorState();
		}

#ifdef PRODUCT_WeMo_Insight
		snprintf(values[0x00], SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%0.f",
						curState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_KWH14Days);
#else
		if (0x01 == curState)
		{
				strncpy(values[0x00],  "1", 3);
		}
		else
		{
				strncpy(values[0x00],  "0", 3);
		}
#endif
#ifdef PRODUCT_WeMo_CrockPot_DISABLE
		if (DEVICE_CROCKPOT == g_eDeviceType)
		{
				int retStatus = 1;

				retStatus = UpnpAcceptSubscription(device_handle,
								sr_event->UDN,
								sr_event->ServiceId,
								(const char **)paramters,
								(const char **)values,
								0x02,
								sr_event->Sid);
				if(UPNP_E_SUCCESS == retStatus){
						APP_LOG("UPNPDevice", LOG_DEBUG,"Service subscription: UDN- %s serID - %s: success", sr_event->UDN, sr_event->ServiceId);
				}
				else {
						APP_LOG("UPNPDevice", LOG_DEBUG,"Service subscription ERROR %d ####",retStatus);
				}		
		}
		else
#endif /* #ifdef PRODUCT_WeMo_CrockPot */
		{
				UpnpAcceptSubscription(device_handle,
								sr_event->UDN,
								sr_event->ServiceId,
								(const char **)paramters,
								(const char **)values,
								0x01,
								sr_event->Sid);
		}

#ifdef PRODUCT_WeMo_CrockPot_DISABLE
		free(values[0x00]);
		free(values[0x01]);
#else
		free(values[0x00]);
#endif

		APP_LOG("UPNPDevice", LOG_DEBUG,"Service subscription: %s: success", sr_event->ServiceId);

		if (strstr(sr_event->ServiceId, "basicevent"))
		{
				{
						UPnPTimeSyncStatusNotify();
				}
#ifdef PRODUCT_WeMo_Insight
				APP_LOG("UPNPDevice", LOG_DEBUG,"INSIGHT HOME SETTINGS NOTIFY...");
				//send notification on get request 
				SendHomeSettingChangeMsg();
#endif
		}
		if (strstr(sr_event->ServiceId, "remoteaccess"))
		{
				APP_LOG("UPNPDevice", LOG_DEBUG,"##################### REMOTEACCESS SUBSCRIBED...");
				{
						if ((0x00 != strlen(g_szHomeId) && 0x00 == atoi(g_szRestoreState)))	//notify only if self remote access is enabled
						{
								APP_LOG("UPNPDevice", LOG_DEBUG,"##################### sending signature notification...");
								UPnPSendSignatureNotify();
								pluginUsleep(1000000);
								APP_LOG("UPNPDevice", LOG_DEBUG,"##################### sending homeid and deviceid notification...");
								UPnPSetHomeDeviceIdNotify();

						}
						else
						{
								APP_LOG("UPNPDevice", LOG_ERR,"################## there is no homeid, private key, not sending homeid, deviceid and signature notifications");
								APP_LOG("UPNPDevice", LOG_CRIT,"There is no homeid, private key, not sending homeid, deviceid and signature notifications");
						}
				}

		}
		return (0x00);
}

int CtrleeDeviceSetActionTable(PLUGIN_SERVICE_TYPE serviceType, pPluginService pOut)
{
		switch (serviceType)
		{
				case PLUGIN_E_SETUP_SERVICE: 
						pOut->ActionTable  = g_Wifi_Setup_Actions;
						pOut->cntTableSize = sizeof(g_Wifi_Setup_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_SETUP_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;
				case PLUGIN_E_TIME_SYNC_SERVICE:
						pOut->ActionTable  = g_time_sync_Actions;
						pOut->cntTableSize = sizeof(g_time_sync_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_TIME_SYNC_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;
				case PLUGIN_E_EVENT_SERVICE:
						pOut->ActionTable  = g_basic_event_Actions;
						pOut->cntTableSize = sizeof(g_basic_event_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_EVENT_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;

				case PLUGIN_E_FIRMWARE_SERVICE:
						pOut->ActionTable  = g_firmware_event_Actions;
						pOut->cntTableSize = sizeof(g_firmware_event_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_FIRMWARE_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;


				case PLUGIN_E_RULES_SERVICE:
						pOut->ActionTable  = g_Rules_Actions;
						pOut->cntTableSize = sizeof(g_Rules_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_RULES_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);

						break;

				case PLUGIN_E_REMOTE_ACCESS_SERVICE:
						pOut->ActionTable  = g_remote_access_Actions;
						pOut->cntTableSize = sizeof(g_remote_access_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_REMOTE_ACCESS_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;


				case PLUGIN_E_METAINFO_SERVICE:
						pOut->ActionTable  = g_metaInfo_Actions;
						pOut->cntTableSize = sizeof(g_metaInfo_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_METAINFO_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;

				case PLUGIN_E_DEVICEINFO_SERVICE:
						pOut->ActionTable  = g_deviceInfo_Actions;
						pOut->cntTableSize = sizeof(g_deviceInfo_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_DEVICEINFO_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;

				 case PLUGIN_E_MANUFACTURE_SERVICE:
    					pOut->ActionTable 	= g_manufacture_Actions;
    					pOut->cntTableSize 	= sizeof(g_manufacture_Actions)/sizeof(PluginDeviceUpnpAction);
      					APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_MANUFACTURE_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
      					break;			

#ifdef PRODUCT_WeMo_Insight
				case PLUGIN_E_INSIGHT_SERVICE:
						pOut->ActionTable  = g_insight_Actions;
						pOut->cntTableSize = sizeof(g_insight_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_INSIGHT_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;
#endif

#ifdef WeMo_SMART_SETUP
				case PLUGIN_E_SMART_SETUP_SERVICE:
						pOut->ActionTable  = g_smart_setup_Actions;
						pOut->cntTableSize = sizeof(g_smart_setup_Actions)/sizeof(PluginDeviceUpnpAction);
						APP_LOG("UPNP", LOG_DEBUG, "PLUGIN_E_SMART_SETUP_SERVICE: service: %s Action Table set: %d", pOut->ServiceType, pOut->cntTableSize);
						break;
#endif
				default:
						APP_LOG("UPNP", LOG_ERR, "WRONG service ID");
						;
		}

		return UPNP_E_SUCCESS;
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * ************************************************************************/
#define 	AP_LIST_BUFF_SIZE	3*SIZE_1024B

#if 0
void* siteSurveyPeriodic(void *args)
{
		int count=0;

		if (pAvlAPList)
				free(pAvlAPList);

		/* buffer allocated is as per the WIFI driver. Refer: cmm_info.c*/
		/* Memory allocated for 64 entries */
		pAvlAPList = (PMY_SITE_SURVEY) malloc (sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);

		if(!pAvlAPList) 
		{
				APP_LOG("UPNPDevice",LOG_ERR,"Malloc Failed...");
				resetSystem();
		}

		while(1) {
				if(g_ra0DownFlag == 1)
						break;

				osUtilsGetLock(&gSiteSurveyStateLock);
				memset(pAvlAPList, 0x0, sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);
				getCurrentAPList (pAvlAPList, &count);
				g_cntApListNumber = count;
				osUtilsReleaseLock(&gSiteSurveyStateLock);

				pluginUsleep(10000000); //10 secs
		}

		if (pAvlAPList)
				free(pAvlAPList);
		pAvlAPList = 0x00;

		APP_LOG("UPNPDevice",LOG_DEBUG,"******* SiteSurvey Thread exiting ************");
		pthread_exit(0);
}
#endif

#ifdef WeMo_INSTACONNECT
extern pthread_t connInsta_thread;
int g_connectInstaStatus = APCLI_UNCONFIGURED;
int g_usualSetup = 0x00;
extern WIFI_PAIR_PARAMS gWiFiParams;
extern char gRouterSSID[SSID_LEN];

void GetBridgeTrailSerial(char* szSerialNo, char *szBuff)
{
		if ((0x00 == szSerialNo) || (0x00 == strlen(szSerialNo)))
		{
				szSerialNo = DEFAULT_SERIAL_NO;
		}

		//strncat(szBuff, szSerialNo + strlen(szSerialNo) - DEFAULT_SERIAL_TAILER_SIZE, MAX_APSSID_LEN);
		strncat(szBuff, szSerialNo + strlen(szSerialNo) - DEFAULT_SERIAL_TAILER_SIZE, DEFAULT_SERIAL_TAILER_SIZE);

		//printf("APSSID: %s\n", szBuff);
}

void NotifyApOpen()
{
		pMessage msg = createMessage(NETWORK_AP_OPEN_UPNP, 0x00, 0x00);
		SendMessage2App(msg);
}
void* connectToInstaApTask(void *args)
{
		int i = 0, count = 0, found = 0;
		PMY_SITE_SURVEY pAvlNetwork=NULL;
		int chan=0;
		int ret=0;
		int connectInstaStatus = 0x00;  //un configured

		pluginUsleep(5000000); //5 secs to defer it from site survey thread
		while(1)
		{
				/* 0x01 - means connecthomenetwork received from app on ra0... so do nothing meanwhile */
				if(g_connectInstaStatus == APCLI_CONFIGURING)
				{
						pluginUsleep(1000000);
						continue;
				}

				/* 0x02 - means connecthomenetwork succeeded... so exit*/
				if(g_connectInstaStatus == APCLI_CONFIGURED)
						break;

				/* connect Insta Status = 0x01 - configuring */
				if(connectInstaStatus == 0x01)
				{
						if(isAPConnected(INSTA_AP_SSID) < SUCCESS)
						{
								APP_LOG("UPNP", LOG_ERR,"Insta AP disconnected...undo settings");
								undoInstaApSettings();
								NotifyApOpen();
								connectInstaStatus = 0x00;
								found = 0;
								pluginUsleep(5000000);	// additional 5 secs to let upnp to switch over to local mode
						}
						pluginUsleep(5000000);  //5 secs to check for insta AP connection status
						continue;
				}

				osUtilsGetLock(&gSiteSurveyStateLock);
				count = g_cntApListNumber;
				pAvlNetwork = pAvlAPList;

				if(!pAvlNetwork)
				{
						APP_LOG("UPNP", LOG_ERR,"No AP List....");
						osUtilsReleaseLock(&gSiteSurveyStateLock);
						pluginUsleep(1000000);
						continue;
				}

				APP_LOG("UPNP", LOG_DEBUG,"Avl network list cnt: %d", count);
				for (i=0;i<count;i++) 
				{
						if (!strcmp (pAvlNetwork[i].ssid, INSTA_AP_SSID)) 
						{

								APP_LOG("UPNP", LOG_DEBUG,
												"pAvlNetwork[%d].channel: %s,pAvlNetwork[%d].ssid: %s", 
												i, pAvlNetwork[i].channel, i, pAvlNetwork[i].ssid);
								found = 1;
								APP_LOG("WiFiApp", LOG_DEBUG, "WEMO INSTA AP found from SITE SURVEY");
								chan = atoi((const char *)pAvlNetwork[i].channel);
								break;
						}
				}
				osUtilsReleaseLock(&gSiteSurveyStateLock);
				if(found)
				{
						ret = connectToInstaAP(chan);
						if (ret >= SUCCESS)
						{
								//APP_LOG("WiFiApp", LOG_DEBUG, "Connection to Insta AP successful, break from main loop");
								//break;
								APP_LOG("WiFiApp", LOG_DEBUG, "Connection to Insta AP successful, now check for configuring state");
								connectInstaStatus = 0x01;  //configuring
								continue;
						}
						else 
						{
								pluginUsleep(2000000);
								APP_LOG("UPNP", LOG_ERR,"connection to Insta Ap failed....retry");
								found = 0;
								continue;
						}
				}
				else
						pluginUsleep(10000000); //10 secs
		}

		APP_LOG("UPNPDevice",LOG_DEBUG,"******* connect to Insta AP Thread exiting ************");
		connInsta_thread = -1;
		pthread_exit(0);
}

void GetTrailSerial(char *szBuff)
{
		char* szSerialNo = GetSerialNumber();
		if ((0x00 == szSerialNo) || (0x00 == strlen(szSerialNo)))
		{
				szSerialNo = DEFAULT_SERIAL_NO;
		}

		//printf("serialNumber: %s\n", szSerialNo);
		//strncat(szBuff, szSerialNo + strlen(szSerialNo) - DEFAULT_SERIAL_TAILER_SIZE, MAX_APSSID_LEN);
		strncat(szBuff, szSerialNo + strlen(szSerialNo) - DEFAULT_SERIAL_TAILER_SIZE, DEFAULT_SERIAL_TAILER_SIZE);

		//printf("APSSID: %s\n", szBuff);
}
#endif

int GetApList(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szAplistBuffer[AP_LIST_BUFF_SIZE];
		char szApEntry[MAX_RESP_LEN];
		int i=0, count=0;
		int listCnt=0;
		PMY_SITE_SURVEY pAvlAPList = NULL;

		memset(szAplistBuffer, 0x0, AP_LIST_BUFF_SIZE);

		pAvlAPList = (PMY_SITE_SURVEY) malloc (sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);
		if(!pAvlAPList) 
		{
				APP_LOG("UPNPDevice",LOG_ERR,"Malloc Failed...");
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetApList", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"ApList", "FAILURE");

				return FAILURE;
		}

		APP_LOG("UPNPDevice",LOG_DEBUG,"Get List...");
		memset(pAvlAPList, 0x0, sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);
		getCurrentAPList (pAvlAPList, &count);

		listCnt = count;

		for (i=0; i < count; i++) 
		{
				if((strstr(pAvlAPList[i].ssid, ",") != NULL) || (strstr(pAvlAPList[i].ssid, "|") != NULL))
				{
						listCnt--;
						APP_LOG("UPNP: DEVICE",LOG_DEBUG, "Updated listcnt: %d for SSID: %s", listCnt, pAvlAPList[i].ssid);
				}
		}

		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "count: %d, listcnt: %d\n", count, listCnt);

		snprintf(szAplistBuffer, sizeof(szAplistBuffer), "Page:1/1/%d$\n", listCnt);

		for (i=0; i < count; i++) 
		{
				if((strstr(pAvlAPList[i].ssid, ",") == NULL) && (strstr(pAvlAPList[i].ssid, "|") == NULL))
				{
						memset(szApEntry, 0x00, sizeof(szApEntry));

						snprintf(szApEntry, sizeof(szApEntry), "%s|%d|%d|%s,\n",
										pAvlAPList[i].ssid,
										atoi((const char *)pAvlAPList[i].channel),
										atoi((const char *)pAvlAPList[i].signal),
										pAvlAPList[i].security
										);
						strncat(szAplistBuffer, szApEntry, sizeof(szAplistBuffer)-strlen(szAplistBuffer)-1);
				}
				else
						APP_LOG("UPNP: DEVICE",LOG_DEBUG, "Skipping entry %d for SSID: %s", i, pAvlAPList[i].ssid);
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetApList", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"ApList", szAplistBuffer);

		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "%s\n", szAplistBuffer);

		if(pAvlAPList)
		{
				free(pAvlAPList);
				pAvlAPList = NULL;
		}

		return UPNP_E_SUCCESS;
}

int GetNetworkList(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szAplistBuffer[AP_LIST_BUFF_SIZE];
		char szApEntry[MAX_RESP_LEN];
		int i=0, ssidLen = 0;
		int count=0;
		PMY_SITE_SURVEY pAvlAPList = NULL;

		memset(szAplistBuffer, 0x0, AP_LIST_BUFF_SIZE);

		pAvlAPList = (PMY_SITE_SURVEY) malloc (sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);
		if(!pAvlAPList) 
		{
				APP_LOG("UPNPDevice",LOG_ERR,"Malloc Failed...");
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetNetworkList", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"NetworkList", "FAILURE");

				return FAILURE;
		}
		APP_LOG("UPNPDevice",LOG_DEBUG,"Get Network List...");
		memset(pAvlAPList, 0x0, sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);
		getCurrentAPList (pAvlAPList, &count);

		snprintf(szAplistBuffer, sizeof(szAplistBuffer), "Page:1/1/%d$\n", count);

		for (i=0; i < count; i++) 
		{
				memset(szApEntry, 0x00, sizeof(szApEntry));

				ssidLen = strlen(pAvlAPList[i].ssid);
				snprintf(szApEntry, sizeof(szApEntry), "%d|%s|%d|%d|%s|\n",
								ssidLen,
								pAvlAPList[i].ssid,
								atoi((const char *)pAvlAPList[i].channel),
								atoi((const char *)pAvlAPList[i].signal),
								pAvlAPList[i].security
								);
				strncat(szAplistBuffer, szApEntry, sizeof(szAplistBuffer)-strlen(szAplistBuffer)-1);
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetNetworkList", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"NetworkList", szAplistBuffer);

		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "%s\n", szAplistBuffer);

		if(pAvlAPList)
		{
				free(pAvlAPList);
				pAvlAPList = NULL;
		}

		return UPNP_E_SUCCESS;
}



/*
 * Unsets the u-boot env variable 'boot_A_args'.
 *
 * Will work only on the OPENWRT boards.
 * Gemtek boards follow different procedure to handle the scenario.
 *
 * Done for the Story: 2187, To restore the state of the switch before power failure
 */
#ifdef _OPENWRT_
void correctUbootParams() {
		char sysString[SIZE_128B];
		int bootArgsLen = 0;

		memset(sysString, '\0', SIZE_128B);
		bootArgsLen = strlen(g_szBootArgs);

		if(bootArgsLen) {
				snprintf(sysString, sizeof(sysString), "fw_setenv boot_A_args %s", g_szBootArgs);
				system(sysString);
		}
}
#endif

void AsyncControlleeDeviceStop()
{
    pMessage msg = createMessage(META_CONTROLLEE_DEVICE_STOP, 0x00, 0x00);
    SendMessage2App(msg);
}

void* resetThread(void *arg)
{
		int resetType = *(int *)arg;
                tu_set_my_thread_name( __FUNCTION__ );

		free(arg);

		switch(resetType)
		{
				case META_SOFT_RESET:
						APP_LOG("ResetThread", LOG_ALERT, "Processing META_SOFT_RESET");
#ifdef PRODUCT_WeMo_Insight
						ClearUsageData();
#endif
						resetToDefaults();
						ClearRuleFromFlash();
						UpdateXML2Factory();
						AsyncSaveData();
						break;

				case META_FULL_RESET:
						APP_LOG("ResetThread", LOG_ALERT, "Processing META_FULL_RESET");
#ifdef _OPENWRT_
						correctUbootParams();
#endif
#ifdef __ORESETUP__
						/* Remove saved IP from flash */
						UnSetBelkinParameter ("wemo_ipaddr");
						/* Unsetting IconVersion */
						UnSetBelkinParameter (ICON_VERSION_KEY);
						gWebIconVersion=0;
						AsyncControlleeDeviceStop();
						setRemoteRestoreParam(0x1);
						remotePostDeRegister(NULL, META_FULL_RESET);
#else
						AsyncControlleeDeviceStop();
						setRemoteRestoreParam(0x1);
						remotePostDeRegister(NULL, META_FULL_RESET);
						/* Remove saved IP from flash */
						UnSetBelkinParameter ("wemo_ipaddr");
						/* Unsetting IconVersion */
						UnSetBelkinParameter (ICON_VERSION_KEY);
						gWebIconVersion=0;

#endif
						pluginUsleep(CONTROLLEE_DEVICE_STOP_WAIT_TIME);
						ResetToFactoryDefault(0);
						break;

				case META_REMOTE_RESET:
						APP_LOG("ResetThread", LOG_ALERT, "Processing META_REMOTE_RESET");
						UnSetBelkinParameter (DEFAULT_HOME_ID);	
						memset(g_szHomeId, 0x00, sizeof(g_szHomeId));
						UnSetBelkinParameter (DEFAULT_PLUGIN_PRIVATE_KEY);	
						memset(g_szPluginPrivatekey, 0x00, sizeof(g_szPluginPrivatekey));
						UnSetBelkinParameter (RESTORE_PARAM);
						memset(g_szRestoreState, 0x0, sizeof(g_szRestoreState));
						gpluginRemAccessEnable = 0;
						/* server environment settings cleanup and nat client destroy */

						APP_LOG("ResetThread",LOG_DEBUG, "Entry Re-Initialize nat client on ENV change");
						while(1) 
						{
								APP_LOG("ResetThread",LOG_DEBUG, "Inside while Re-Initialize nat client on ENV change");
								APP_LOG("ResetThread",LOG_HIDE, "gpluginRemAccessEnable = %d\ng_szHomeId = %s\n", gpluginRemAccessEnable, g_szHomeId);
								sleep(5);
								if (gpluginRemAccessEnable && strlen(g_szHomeId)) 
								{
										APP_LOG("ResetThread",LOG_DEBUG, "Re-Initialize nat client on ENV change");
										invokeNatReInit(NAT_REINIT);
										APP_LOG("ResetThread",LOG_DEBUG, "Done! Re-Initialize nat client on ENV change");
										break;
								}
						}
						APP_LOG("ResetThread",LOG_CRIT, "Exit Re-Initialize nat client on ENV change");

						SaveSetting();
						break;
#ifdef PRODUCT_WeMo_Insight
				case META_CLEAR_USAGE:
						APP_LOG("ResetThread", LOG_ALERT, "Processing META_CLEAR_USAGE");
						ClearUsageData();
						break;
#endif
				case META_WIFI_SETTING_RESET:
						APP_LOG("ResetThread", LOG_ALERT, "Processing META_WIFI_SETTING_RESET");
						ResetWiFiSetting();
						break;	

		}
		reset_thread = -1;
		pthread_exit(0);
		return 0;
}

void ResetWiFiSetting(void)
{
		APP_LOG("ResetWiFiSetting:", LOG_DEBUG, "Reset WiFi Settings");
		/* Remove saved IP from flash */
		UnSetBelkinParameter ("wemo_ipaddr");
		AsyncControlleeDeviceStop();
		setRemoteRestoreParam(0x1);
		SetBelkinParameter(WIFI_CLIENT_SSID,"");
		APP_LOG("ResetButtonTask:", LOG_DEBUG, "Going To Self Reboot...");
		pluginUsleep(CONTROLLEE_DEVICE_STOP_WAIT_TIME);
		system("reboot");
}

int ExecuteReset(int resetIndex)
{
		int *resetType = (int *)malloc(sizeof(int));
		int retVal;

		if(!resetType)
		{
				APP_LOG("UPnP: Device",LOG_ERR, "Memory could not be allocated for resetType");
				return SUCCESS;
		}

		if(0x01 == resetIndex)
				*resetType = META_SOFT_RESET;
		else if(0x02 == resetIndex)
				*resetType = META_FULL_RESET;
		else if(0x03 == resetIndex)
				*resetType = META_REMOTE_RESET;
#ifdef PRODUCT_WeMo_Insight
		else if(0x04 == resetIndex)
				*resetType = META_CLEAR_USAGE;
#endif
		else if(0x05 == resetIndex)
				*resetType = META_WIFI_SETTING_RESET;
		else 
				return FAILURE;

		/* first of all remove the reset thread, if running */
		if(reset_thread != -1)
		{
				if((retVal = pthread_kill(reset_thread, SIGRTMIN)) == 0)
				{
						reset_thread = -1;
						APP_LOG("UPnP: Device",LOG_DEBUG,"reset thread removed successfully....");
				}
				else
				{
						APP_LOG("UPnP: Device",LOG_ERR,"reset thread removal failed [%d] ....",retVal);
				}
		}
		else
				APP_LOG("UPnP: Device",LOG_DEBUG,"reset thread doesn't exist. Creating reset thread....");

		pthread_attr_init(&reset_attr);
		pthread_attr_setdetachstate(&reset_attr,PTHREAD_CREATE_DETACHED);
		retVal = pthread_create(&reset_thread,&reset_attr,
						(void*)&resetThread, (void *)resetType);

		if(retVal < SUCCESS) {
				APP_LOG("UPnP: Device",LOG_CRIT, "RESET Thread not created");
				return SUCCESS;
		}

		return SUCCESS;
}

#ifndef __ORESETUP__
pthread_mutex_t g_remoteDeReg_mutex;
pthread_cond_t g_remoteDeReg_cond;
#define WAIT_DREG_TIMEOUT	10
#endif

int ReSetup(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int retVal = UPNP_E_SUCCESS;

		APP_LOG("UPNPDevice: ReSetup", LOG_DEBUG, "%s", __FUNCTION__);

		if (0x00 == pActionRequest || 0x00 == request)
		{
				return 0x01;
		}
		char* paramValue = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Reset");

		if (paramValue)
				APP_LOG("UPNPDevice", LOG_DEBUG, "trying reset plugin to: %s", paramValue);

		int resettype = atoi(paramValue);

#ifdef __ORESETUP__
		/* Clear Rules info */
		if(resettype == 0x01)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "success");

				APP_LOG("UPNPDevice", LOG_DEBUG, "Reset Plugin-> (Mem Partitions) Rules:  done: %d", resettype);

				ExecuteReset(0x01);
		}
		/* Clear All info */
		else if(resettype == 0x02)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SUCCESS;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "success");
				ExecuteReset(0x02);
		}	
		/* Clear Remote info */
		else if(resettype == 0x03)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SUCCESS;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "success");
				ExecuteReset(0x03);
		}	
#ifdef PRODUCT_WeMo_Insight
		else if(resettype == 0x04)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SUCCESS;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "success");
				ExecuteReset(0x04);
		}	
#endif
		else if(resettype == 0x05)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SUCCESS;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "success");
				ExecuteReset(0x05);
		}
		else
		{
				APP_LOG("UPNPDevice", LOG_ERR, "Reset Plugin not done: %d", resettype);

				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "unsuccess");
		}
#else
		retVal = ExecuteReset(resettype);
		if (retVal < SUCCESS) {
				APP_LOG("UPNPDevice", LOG_ERR, "Reset Plugin not done: %d", resettype);
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "unsuccess");
		}else {
				//Wait on posix condition timed wait for 5 seconds max if resettype is 2 only
				struct timeval tmv;
				struct timespec tsv;
				if (resettype == 0x02) {
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "In reset_remote type %d\n", resettype);
						/* Convert from timeval to timespec */
						gettimeofday(&tmv, NULL);
						tsv.tv_sec  = tmv.tv_sec;
						tsv.tv_nsec = tmv.tv_usec * 1000;
						tsv.tv_sec += WAIT_DREG_TIMEOUT;
						/* wait here for response */
						pthread_mutex_lock(&g_remoteDeReg_mutex);
						retVal = pthread_cond_timedwait(&g_remoteDeReg_cond, &g_remoteDeReg_mutex, &tsv);
						pthread_mutex_unlock(&g_remoteDeReg_mutex);
						//check whether de-reg successfully happened
						retVal = remoteGetDeRegStatus(0);
						APP_LOG("REMOTEACCESS", LOG_DEBUG, "The status Code got is %d\n", retVal);
						if (retVal == 2) {
								pActionRequest->ActionResult = NULL;
								pActionRequest->ErrCode = SUCCESS;
								UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "reset_remote");
								goto retStatus;
						}
				}
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SUCCESS;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ReSetup",CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "Reset", "success");
		}
retStatus:
#endif
		return UPNP_E_SUCCESS;	
}

void StopPowerMonitorTimer()
{
		int ret = 0x01;
		if (-1 != ithPowerSensorMonitorTask)
		{
				ret = ithread_cancel(ithPowerSensorMonitorTask);

				if (0x00 != ret)
				{ 
						APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_cancel: Can not stop power monitor task thread ##############################");
				}

				ithPowerSensorMonitorTask = -1;

		}
}

void UpdatePowerMonitorTimer(int duration, int endAction)
{
		int ret = 0x01;
		if (0x00 == duration)
				return;

		StopPowerMonitorTimer();

		ret = ithread_create(&ithPowerSensorMonitorTask, 0x00, PowerSensorMonitorTask, 0x00);
		if (0x00 != ret)
		{ 
				APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_create: Can not create power monitor task thread ##############################");
		}

		ret = ithread_detach(ithPowerSensorMonitorTask);			
		if (0x00 != ret)
		{ 
				APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_detach: Can not detach power monitor task thread ##############################");
		}

}

void UPnPActionUpdate(int curState)
{
		pMessage msg = 0x00;

		if (0x00 == curState)
		{
				msg = createMessage(UPNP_ACTION_MESSAGE_OFF_IND, 0x00, 0x00);
		}
		else if (0x01 == curState)
		{
				msg = createMessage(UPNP_ACTION_MESSAGE_ON_IND, 0x00, 0x00);
		}

		SendMessage2App(msg);
}


void UPnPInternalToggleUpdate(int curState)
{
		pMessage msg = 0x00;
		if (0x00 == curState)
		{
				msg = createMessage(UPNP_MESSAGE_OFF_IND, 0x00, 0x00);
		}
		else if (0x01 == curState)
		{
				msg = createMessage(UPNP_MESSAGE_ON_IND, 0x00, 0x00);
		}
#ifdef PRODUCT_WeMo_Insight
		else if (POWER_SBY == curState)
		{
				msg = createMessage(UPNP_MESSAGE_SBY_IND, 0x00, 0x00);
		}
#endif

		//gautam:  Relay thread does nothing but sends it to main thread: unnecessary latency
		SendMessage2App(msg);
}

int SetBinaryState(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{ 
		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "SetBinaryState: command paramter invalid");
				return 0x01;
		}


		if (DEVICE_SENSOR == g_eDeviceType)
		{
				APP_LOG("UPNPDevice", LOG_ERR, "Sensor device, command not support");

				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetBinaryState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "BinaryState", "unsuccess"); 

				return 0x00;
		}

		char* paramValue = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "BinaryState");
		char* paramDuration = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Duration");
		char* paramEndAction = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "EndAction");

		if (0x00 == paramValue || 0x00 == strlen(paramValue))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetBinaryState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "BinaryState", "Error");  

				return 0x00;
		}

		if ((0x00 != paramDuration) && (0x00 != strlen(paramDuration)) && (0x00 != paramEndAction) && (0x00 != strlen(paramEndAction)))
		{
			/* Sensor triggered */
			APP_LOG("UPNPDevice", LOG_DEBUG, "Sensor trigger");
			setActuation(ACTUATION_SENSOR_RULE);
		}
		else
		{
			/* App triggered */
			APP_LOG("UPNPDevice", LOG_DEBUG, "App trigger");
			setActuation(ACTUATION_MANUAL_APP);
			setRemote("0");
		}

#ifdef PRODUCT_WeMo_Insight
		char szCurState[SIZE_100B];
#else
		char szCurState[SIZE_8B];
#endif
		memset(szCurState, 0x00, sizeof(szCurState));

		int toState  = atoi(paramValue);
		APP_LOG("UPNPDevice", LOG_DEBUG, "to request state change to %d", toState);  

		int ret = 0x01;

		ret = ChangeBinaryState(toState);
		char* szStateOuput = toState?"ON":"OFF";

#ifdef PRODUCT_WeMo_Insight
		if(toState == 1){
				//g_ONFor=0;
				//g_ONForChangeFlag = 1;
				toState = POWER_SBY;
				APP_LOG("UPNPDevice", LOG_DEBUG, "******Changed ON State To %d", toState);  
		}
#endif
		if (0x00 == ret)
		{
				UPnPInternalToggleUpdate(toState);
#ifdef PRODUCT_WeMo_Insight
				if (g_InitialMonthlyEstKWH){
						snprintf(szCurState, SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%d",
										toState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_InitialMonthlyEstKWH);
				}
				else{
						snprintf(szCurState, SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%0.f",
										toState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_KWH14Days);
				}
				APP_LOG("UPNP", LOG_DEBUG, "Local Binary State Insight Parameters: %s", szCurState);
#else
				snprintf(szCurState, sizeof(szCurState), "%d", toState);
#endif
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetBinaryState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "BinaryState", szCurState);  

				APP_LOG("UPNPDevice", LOG_ERR, "State changed, current state: %d", toState);  

		}
		else
		{
#ifdef PRODUCT_WeMo_Insight
				if (g_InitialMonthlyEstKWH){
						snprintf(szCurState, SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%d",
										toState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_InitialMonthlyEstKWH);
				}
				else{
						snprintf(szCurState, SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%0.f",
										toState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_KWH14Days);
				}
				APP_LOG("UPNP", LOG_DEBUG, "Local Binary State Insight Parameters on failure: %s", szCurState);
#else
				snprintf(szCurState, sizeof(szCurState), "%s", "Error");
#endif

				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetBinaryState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "BinaryState", szCurState);  

				APP_LOG("UPNPDevice", LOG_ERR, "State not changed, current state: %s", szStateOuput);  
		}

		if ((0x00 != paramDuration) && (0x00 != strlen(paramDuration)) && (0x00 != paramEndAction) && (0x00 != strlen(paramEndAction)))
		{
				//- JIRA: WEMO-4605: to check the ON logic from user and 8029
#ifdef PRODUCT_WeMo_Insight
				if ((IsLastUserActionOn()) &&((toState == POWER_ON) ||(toState == POWER_SBY)))
#else
						if ((IsLastUserActionOn()) &&(toState == POWER_ON))
#endif
						{
								APP_LOG("Rule", LOG_DEBUG, "Last user action to ON, no timer");
								StopPowerMonitorTimer();
								//- do nothing here
						}
						else
						{
								APP_LOG("UPNPDevice", LOG_DEBUG, "Sensor event, create management thread");

								sPowerDuration  = atoi(paramDuration);
								sPowerEndAction = atoi(paramEndAction);

								APP_LOG("UPnP: Sensor rule", LOG_DEBUG, "duration: %d, endAction: %d", sPowerDuration, sPowerEndAction);  

								UpdatePowerMonitorTimer(sPowerDuration, sPowerEndAction);
						}

		}
		else
		{
				//- Action from phone, so notify the sensor
				SetLastUserActionOnState(toState);
				if (0x00 == ret)
				{
						StopPowerMonitorTimer();

						OverrideRule(ACTION_PHONE_CONTROL);
						//- I am not sure I have a sensor rule or not
						//LocalUserActionNotify(toState + 2);
#ifdef PRODUCT_WeMo_Insight
						//UPnPActionUpdate(g_PowerStatus); //To-Do in Insight Rules Stories.
#else
						//UPnPActionUpdate(toState);
#endif
				}
		}

		FreeXmlSource(paramValue);
		FreeXmlSource(paramDuration);
		FreeXmlSource(paramEndAction);

		return UPNP_E_SUCCESS;
}

void nat_set_log_level(void);
#ifdef PRODUCT_WeMo_CrockPot
/* This function converts the mode values from actual crockpot value to app/cloud expected value
 * and vice versa
 * flag decides conversion to crockpot or conversion from crockpot
 */
int convertModeValue(int mode, int flag)
{
		int retMode;
		if(TO_CROCKPOT == flag) {
				/* Change the app value to Crockpot value */
				switch(mode) {
						case 0:
								retMode = MODE_OFF;
								break;
						case 1:
								retMode = MODE_IDLE;
								break;
						case 50:
								retMode = MODE_WARM;
								break;
						case 51:
								retMode = MODE_LOW;
								break;
						case 52:
								retMode = MODE_HIGH;
								break;
						default:
								retMode = APP_MODE_ERROR;
				}
		}
		else { /*FROM_CROCKPOT*/
				/* Change the Crcokpot value to App value */
				switch(mode) {
						case MODE_OFF:
								retMode = APP_MODE_OFF;
								break;
						case MODE_IDLE:
								retMode = APP_MODE_IDLE;
								break;
						case MODE_WARM:
								retMode = APP_MODE_WARM;
								break;
						case MODE_LOW:
								retMode = APP_MODE_LOW;
								break;
						case MODE_HIGH:
								retMode = APP_MODE_HIGH;
								break;
						default:
								retMode = APP_MODE_ERROR;
				}
		}
		return retMode;
}

/* SetCrockpotState function is to change the Crockpot settings 
 * This function make use of HAL_SetMode() & HAL_SetCooktime() functions from HAL layer
 * pActionRequest - Handle to request
 *
 */
int SetCrockpotState(pUPnPActionRequest pActionRequest, 
				IXML_Document *request, 
				IXML_Document **out, 
				const char **errorString)
{
		int crockpot_mode;
		int tempmode, curMode;	
		int crockpot_cooktime;
		int retstatus,allowSetTime=FALSE;
		char szCurState[SIZE_16B];

		memset(szCurState, 0x00, sizeof(szCurState));

		if (NULL == pActionRequest || NULL == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "SetCrockpotState: command paramter invalid");
				return SET_PARAMS_NULL;
		}
		/*else {
			APP_LOG("UPNPDevice", LOG_DEBUG, "SETCROCKPOTSTATE: Begining of set operation");
			}*/

		char* parammode = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "mode");
		if (0x00 == parammode || 0x00 == strlen(parammode))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SET_PARAM_ERROR;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Parameter Error");  

				return UPNP_E_SUCCESS;
		}
		char* paramtime = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "time");
		if (0x00 == paramtime || 0x00 == strlen(paramtime))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = SET_PARAM_ERROR;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Parameter Error");  
				FreeXmlSource(parammode);
				return UPNP_E_SUCCESS;
		}

		/*APP_LOG("UPNPDevice", LOG_DEBUG, "SETCROCKPOTSTATE: First Doc item completed");*/

		crockpot_mode = atoi(parammode);
		crockpot_cooktime = atoi(paramtime);

		snprintf(szCurState, sizeof(szCurState), "%d", crockpot_mode);

		/*APP_LOG("UPNPDevice", LOG_DEBUG, "SETCROCKPOTSTATE: Setting the Crockpot to %d mode with %d time", crockpot_mode,crockpot_cooktime);  */

		/* Change the mode to crockpot values from 0, 1, 50, 51, 52 to maintain 
		 * cosistency between local & remote mode 
		 */
		tempmode = convertModeValue(crockpot_mode,TO_CROCKPOT);

		if(MODE_IDLE != tempmode) {
				if(APP_MODE_ERROR != tempmode) {
						/* Call HAL API's to set mode and set time for a crockpot */
						retstatus = HAL_SetMode(tempmode);
						if (0x00 == retstatus) {
								pActionRequest->ActionResult = NULL;
								pActionRequest->ErrCode = 0;
								UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState", 
												CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", szCurState);  

								APP_LOG("UPNPDevice", LOG_ERR, "SETCROCKPOTSTATE: Mode changed to: %d a.k.a %d mode", crockpot_mode, tempmode);
						}
						else {
								pActionRequest->ActionResult = NULL;
								pActionRequest->ErrCode = SET_CPMODE_FAILURE;
								UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState", 
												CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error");  

								APP_LOG("UPNPDevice", LOG_ERR, "SETCROCKPOTSTATE: Mode not changed");  
						}
				} /* if(APP_MODE_ERROR.).. */
		} /* end of if(MODE_IDLE)... */
		else { /* Set to IDLE mode is not allowed */
				APP_LOG("Crockpot", LOG_ERR, "SETCROCKPOTSTATE: Invalid mode value");
		}

		snprintf(szCurState, sizeof(szCurState), "%d", crockpot_cooktime);

		/* Allow setting of time only in LOW or HIGH temperature mode */
		if((tempmode == MODE_LOW) || (tempmode == MODE_HIGH)) {
				allowSetTime = TRUE;
		}
		else {
				curMode = HAL_GetMode();
				if((curMode == MODE_LOW) || (curMode == MODE_HIGH)) {
						allowSetTime = TRUE;
				}
				else {
						allowSetTime = FALSE;
				}
		}

		if((APP_TIME_ERROR != crockpot_cooktime) && (TRUE == allowSetTime)) {
				retstatus = HAL_SetCookTime(crockpot_cooktime);
				if (0x00 == retstatus) {
						/* Is internal broadcast is required? */
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", szCurState);  

						APP_LOG("UPNPDevice", LOG_ERR, "SETCROCKPOTSTATE: Time changed to: %d minutes", crockpot_cooktime);  

				}
				else {
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = SET_CPTIME_FAILURE;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error");  

						APP_LOG("UPNPDevice", LOG_ERR, "SETCROCKPOTSTATE: Time not changed\n");  
				}
		} /* end of if(APP_TIME_ERROR.).. */
		else {
				APP_LOG("Crcokpot", LOG_ERR, "SETCROCKPOTSTATE: Time set not performed\n"); 
		}

		FreeXmlSource(parammode);
		FreeXmlSource(paramtime);

		/*APP_LOG("UPNPDevice", LOG_DEBUG, "SETCROCKPOTSTATE: End of set operation");*/
		return UPNP_E_SUCCESS;
}

/* GetCrockpotState function is to change the Crockpot settings 
 * This function make use of HAL_GetMode() & HAL_GetCooktime() functions from HAL layer
 * pActionRequest - Handle to request
 *
 */
int GetCrockpotState(pUPnPActionRequest pActionRequest, 
				IXML_Document *request, 
				IXML_Document **out, 
				const char **errorString)
{
		CROCKPOT_MODE crockpot_mode;
		int tempmode;
		int crockpot_cooktime;
		char getStatusRespMode[SIZE_16B];
		char getStatusRespTime[SIZE_16B];
		int ret;

		memset(getStatusRespMode, 0x00, sizeof(getStatusRespMode));
		memset(getStatusRespTime, 0x00, sizeof(getStatusRespTime));

		if (NULL == pActionRequest || NULL == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "GetCROCKPOTState: command paramter invalid");
				return GET_PARAMS_NULL;
		}

		/* Get the current mode setting */
		crockpot_mode = HAL_GetMode();
		if (0x00 > crockpot_mode)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = GET_CPMODE_FAILURE;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error");  

				APP_LOG("UPNPDevice", LOG_ERR, "GetCROCKPOTState: Unable to get the mode: ");  
		}

		/* Get the time information */
		crockpot_cooktime = HAL_GetCookTime();
		if (0x00 > crockpot_cooktime)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = GET_CPTIME_FAILURE;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error");  

				APP_LOG("UPNPDevice", LOG_ERR, "GetCROCKPOTState: Time not changed, current time: ");  
		}

		tempmode = convertModeValue(crockpot_mode,FROM_CROCKPOT);  	
		snprintf(getStatusRespMode, sizeof(getStatusRespMode), "%d",tempmode);

		ret = UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState", 
						CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", getStatusRespMode);

		APP_LOG("UPNP: Device", LOG_DEBUG, "GetCROCKPOTState:Mode %s", getStatusRespMode);

		snprintf(getStatusRespTime, sizeof(getStatusRespTime), "%d",crockpot_cooktime);

		ret = UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState", 
						CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", getStatusRespTime);

		APP_LOG("UPNP: Device", LOG_DEBUG, "GetCROCKPOTState:Time %s minutes", getStatusRespTime);

		return UPNP_E_SUCCESS;
}
#endif /* #ifdef PRODUCT_WeMo_CrockPot */


int SetLogLevelOption (pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int lvl;
		int opt;

		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_ERR, "SetLogLevelOption: command paramter invalid");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}
		char* szLevel = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Level");
		if(szLevel != NULL)
		{
				if (0x00 == strlen(szLevel))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetLogLevelOption", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"Level", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"Set Log Level parameter: failure"); 
						return PLUGIN_ERROR_E_BASIC_EVENT;
				}
		}
		char* szOption = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Option");
		if(szOption != NULL)
		{
				if (0x00 == strlen(szOption))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetLogLevelOption", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"Option", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"Set Log Option parameter: failure"); 
						return PLUGIN_ERROR_E_BASIC_EVENT;
				}
		}

		lvl = atoi(szLevel);
		opt = atoi(szOption);
		APP_LOG("UPNP: Device", LOG_DEBUG,"Setting Log level: %d and option: %d", lvl, opt); 

		loggerSetLogLevel (lvl, opt);
		nat_set_log_level();
		return UPNP_E_SUCCESS;
}

int GetBinaryState(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int curState = 0x00;

		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "GetBinaryState: paramters error");
				return 0x01;
		}

		if (DEVICE_SOCKET == g_eDeviceType)
		{
				LockLED();
				curState = GetCurBinaryState();
				if(curState)
				{
						APP_LOG("UPNPDevice", LOG_ERR,"Switch State: ON");
				}
				else	
				{
						APP_LOG("UPNPDevice", LOG_ERR,"Switch State: OFF");
				}
				UnlockLED();
		}
		else if (DEVICE_SENSOR == g_eDeviceType)
		{
				LockSensor();
				curState = GetSensorState();
				UnlockSensor();

				if(curState)
				{
						APP_LOG("UPNPDevice", LOG_ERR,"Motion Detected: TRUE");
				}
				else	
				{
						APP_LOG("UPNPDevice", LOG_ERR,"Motion Detected: FALSE");
				}
		}

		char szCurState[SIZE_4B];
		memset(szCurState, 0x00, sizeof(szCurState));

		snprintf(szCurState, sizeof(szCurState), "%d", curState);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetBinaryState", 
						CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "BinaryState", szCurState);

		APP_LOG("UPNPDevice", LOG_DEBUG, "GetBinaryState: %s", szCurState);


		//- Check the override status
		if (IsOverriddenStatus())
				PushStoredOverriddenStatus();


		return UPNP_E_SUCCESS;
}

int StopPair(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		if (0x00 == pActionRequest)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "Parameter error");
				return 0x00;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "StopPair", 
						CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"status", "success");

		system("ifconfig apcli0 down");
		pluginUsleep(500000);
		system("ifconfig apcli0 up");

		APP_LOG("UPNP", LOG_DEBUG, "WiFi pairing stopped");
		return 0;	
}

int ConnectHomeNetwork(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int channel;    int rect = 0x00;
		char* paramValue = 0x00;
		paramValue = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "channel");

		gAppCalledCloseAp=0;
		gBootReconnect=0; 
#ifdef WeMo_SMART_SETUP
		/* reset smart setup presence */
		gSmartSetup = 0;
#endif
#ifdef WeMo_INSTACONNECT
		g_connectInstaStatus = APCLI_CONFIGURING;	//connectToInstaApTask do nothing
		g_usualSetup = 0x01;
#endif

		APP_LOG("UPNPDevice", LOG_CRIT,"%s", __FUNCTION__); 
		if(isSetupRequested())
		{
				APP_LOG("UPNPDevice", LOG_ERR, "#### Setup request already executed ######");
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "ConnectHomeNetwork", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE], "status", "unsuccess");
				return 0x01;
		}

		setSetupRequested(1);

		UpdateUPnPNetworkMode(UPNP_LOCAL_MODE);

		channel = atoi(paramValue);

		char* ssid 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "ssid");
		char* auth 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "auth");
		char* encrypt	= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "encrypt");
		char* password;

		if(strcmp(auth,"OPEN"))
				password = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "password");
		else
				password = "NOTHING";

		/* Save the password in a global variable - to be used later by WifiConn thread */
		memset(gUserKey, 0, sizeof(gUserKey));
		memcpy(gUserKey, password, sizeof(gUserKey));

		APP_LOG("UPNPDevice",LOG_HIDE,"Attempting to connect home network: %s, channel: %d, auth:%s, encrypt:%s, password:%s", 
						ssid, channel, auth, encrypt, password);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "ConnectHomeNetwork", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"PairingStatus", "Connecting");

		if(CHAN_HIDDEN_NW == channel) 
		{
				PWIFI_PAIR_PARAMS pHiddenNwParams=NULL;

				APP_LOG("UPNPDevice",LOG_DEBUG,"Connect request for hidden network: %s......", ssid);
				pHiddenNwParams = malloc(sizeof(WIFI_PAIR_PARAMS));
				memset(pHiddenNwParams,0,sizeof(WIFI_PAIR_PARAMS));

				strncpy(pHiddenNwParams->SSID, ssid, sizeof(pHiddenNwParams->SSID)-1);
				strncpy(pHiddenNwParams->AuthMode, auth, sizeof(pHiddenNwParams->AuthMode)-1);
				strncpy(pHiddenNwParams->EncrypType, encrypt, sizeof(pHiddenNwParams->EncrypType)-1);
				strncpy(pHiddenNwParams->Key, password, sizeof(pHiddenNwParams->Key)-1);
				pHiddenNwParams->channel = CHAN_HIDDEN_NW;

				rect = threadConnectHiddenNetwork(pHiddenNwParams);
		}
		else
		{
				APP_LOG("UPNPDevice",LOG_DEBUG,"connect to selected network: %s", ssid);
				rect = threadConnectHomeNetwork(channel, ssid, auth, encrypt, password);
		}

		if(strcmp(auth,"OPEN"))
				FreeXmlSource(password);
		FreeXmlSource(paramValue);
		FreeXmlSource(ssid);
		FreeXmlSource(auth);
		FreeXmlSource(encrypt);

		return rect;

}

int GetNetworkStatus(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int WiFiClientCurrState = 0x00;
		char szStatus[SIZE_16B];
		memset(szStatus, 0x00, sizeof(szStatus));

		if (0x00 == pActionRequest) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "%s: request parameter error", __FUNCTION__); 

				return PLUGIN_ERROR_E_NETWORK_ERROR;
		}

		WiFiClientCurrState = getCurrentClientState();

		snprintf(szStatus, sizeof(szStatus), "%d", WiFiClientCurrState);

		APP_LOG("UPNPDevice", LOG_ERR,"NetworkStatus: %d:%s", WiFiClientCurrState, szStatus); 

#ifdef WeMo_SMART_SETUP_V2
		/* set app connnected flag to allow device to keep AP open for setup */
		if(!g_isAppConnected)
		{
		    g_isAppConnected = 1;
		    APP_LOG("UPNP", LOG_DEBUG, "is app connected flag set"); 
		}
#endif
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetNetworkStatus", 
						CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE],"NetworkStatus", szStatus);

		return UPNP_E_SUCCESS;
}


int GetDeviceTime(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		return UPNP_E_SUCCESS;
}

int IsApplyTimeSync(int utc, double timeZone, int dst)
{
		int rect = 0x00;

		/* 1. 24 Hours 
		 * 2. Time Zone difference.
		 */	

		APP_LOG("UPNPDevice", LOG_ERR,"TimeSync: %d:%f:%d:%f:%d", utc,timeZone,g_lastTimeSync,g_lastTimeZone,dst); 
		if(timeZone != (g_lastTimeZone+dst)) {
				rect = 0x01;
		}

		if(utc > (g_lastTimeSync+86400)) {
				rect = 0x01;
		}

		return rect;
}

int updateSysTime(void)
{
		int ret;
		struct timeval curtime;

		int iTimeZoneIndex = -1;
		if (gDstEnable)
		{
				{
						iTimeZoneIndex = GetTimeZoneIndex(g_lastTimeZone + 1);
						APP_LOG("Time",LOG_DEBUG, "Going to update systime: index: %d", iTimeZoneIndex);
				}
		}
		else
		{
				iTimeZoneIndex = GetTimeZoneIndex(g_lastTimeZone);
		}

		APP_LOG("Time",LOG_ALERT, "Going to update systime: g_lastTimeZone: %f, index: %d,  gDstEnable: %d", 
						g_lastTimeZone, iTimeZoneIndex, gDstEnable);
		gettimeofday(&curtime, NULL);

		APP_LOG("Time",LOG_ERR, "gettimeofday.sec: %u, utc_time: %u", 
						curtime.tv_sec,GetUTCTime());

		ret = SetTime(GetUTCTime(), iTimeZoneIndex, 0x00);

		if (0 == ret)
		{
				APP_LOG("Time", LOG_DEBUG, "set Time success"); 
		}
		else
		{
				APP_LOG("Time", LOG_ERR, "set Time failed!!"); 
		}

		sleep(2);
		ret = SetNTP(s_szNtpServer, iTimeZoneIndex, 0x00);


		return ret;
} 

static int calculateTimeZoneSpecificInfo(const int dstyr, const int nxtyr, int *sdate, int *smonth, int *shr, 
				int *edate, int *emonth, int *ehr, int *snxtdate, int *enxtdate)
{

		int isDSTTimeZone = 0;

		//US TIMEZONE
		{
				*sdate = 14-((int)(floor(1+(dstyr*5/4)))%7);
				*edate = 7-((int)(floor(1+(dstyr*5/4)))%7);

				*snxtdate = 14-((int)(floor(1+(nxtyr*5/4)))%7);

				*smonth = 2; //March
				*emonth = 10; //November
				*shr = 2;
				*ehr = 1; //2:00 AM
				isDSTTimeZone = 1;
		}

		int sval = 0, eval = 0, snxtval, enxtval;
		float delta, delta1, delta2;

		delta = (g_lastTimeZone - UK_TIMEZONE);
		if ((delta <= 0.001) && (delta >= -0.001))
		{
				/* Europe: UK(London) ... (GMT)
				 *
				 * Computation to calculate the last SUNDAY of March and
				 * last SUNDAY of October.
				 *
				 */
				sval = *sdate + 14;
				*sdate = ((sval+7) > 31) ? sval : (sval+7);

				eval = *edate - 7;
				*edate = (eval < 0) ? (31+eval) : 31;

				snxtval = *snxtdate + 14;
				*snxtdate = ((snxtval+7) > 31) ? snxtval : (snxtval+7);

				*smonth = 2; //March
				*emonth = 9; //October
				*shr = 1; //1:00 AM
				*ehr = 1; //2:00 AM
				isDSTTimeZone = 1;
				APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "sdate: %d, edate: %d, snxtdate: %d, smonth: %d, emonth: %d, shr: %d, ehr: %d, isDSTTimeZone: %d", *sdate, *edate, *snxtdate, *smonth, *emonth, *shr, *ehr, isDSTTimeZone);
				return isDSTTimeZone;
		}

		delta = (g_lastTimeZone - FRANCE_TIMEZONE);
		if ((delta <= 0.001) && (delta >= -0.001))
		{
				/* Europe: France(Paris), Spain(Madrid), Germany(Berlin) ... (GMT+1:00)
				 *
				 * Computation to calculate the last SUNDAY of March and
				 * last SUNDAY of October.
				 *
				 */
				sval = *sdate + 14;
				*sdate = ((sval+7) > 31) ? sval : (sval+7);

				eval = *edate - 7;
				*edate = (eval < 0) ? (31+eval) : 31;

				snxtval = *snxtdate + 14;
				*snxtdate = ((snxtval+7) > 31) ? snxtval : (snxtval+7);

				*smonth = 2; //March
				*emonth = 9; //October
				*shr = 2; //2:00 AM
				*ehr = 2; //3:00 AM
				isDSTTimeZone = 1;
				APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "sdate: %d, edate: %d, snxtdate: %d, smonth: %d, emonth: %d, shr: %d, ehr: %d, isDSTTimeZone: %d", *sdate, *edate, *snxtdate, *smonth, *emonth, *shr, *ehr, isDSTTimeZone);
				return isDSTTimeZone;
		}

		delta = (g_lastTimeZone - CHINA_TIMEZONE);

		if ((delta <= 0.001) && (delta >= -0.001))
		{
				//China ... NO DST
				isDSTTimeZone = 0;
				APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "isDSTTimeZone: %d", isDSTTimeZone);
				return isDSTTimeZone;
		}

		delta = (g_lastTimeZone - JAPAN_TIMEZONE);

		if ((delta <= 0.001) && (delta >= -0.001))
		{
				//Japan ... NO DST
				isDSTTimeZone = 0;
				APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "isDSTTimeZone: %d", isDSTTimeZone);
				return isDSTTimeZone;
		}

		/*************************************************************************************
		 * (Source: http://wwp.greenwichmeantime.com/time-zone/australia/time-zones/index.htm)
		 * Summer Time (Daylight Saving Time) runs in:
		 *************************************************************************************/

		delta = (g_lastTimeZone - AUS_TIMEZONE_1);
		delta1 = (g_lastTimeZone - AUS_TIMEZONE_2);
		delta2 = (g_lastTimeZone - AUS_TIMEZONE_3);
		if (((delta <= 0.001) && (delta >= -0.001)) ||
						((delta1 <= 0.001) && (delta1 >= -0.001)) ||
						((delta2 <= 0.001) && (delta2 >= -0.001)))
		{
				/*
				 * Computation to calculate the first SUNDAY of OCTOBER
				 * and first SUNDAY of April
				 *
				 */

				sval = ((31 + *edate)%7);
				if(sval == 0)
						sval = sval + 7;

				eval = (7 - ((31 - *sdate)%7));

				*sdate = sval; //Start date: first SUNDAY of October
				*edate = eval; //End date: first SUNDAY of April

				*enxtdate = 7-((int)(floor(1+(nxtyr*5/4)))%7);

				snxtval = ((31 + *enxtdate)%7);
				if(snxtval == 0)
						snxtval = snxtval + 7;

				enxtval = (7 - ((31 - *snxtdate)%7));

				*snxtdate = snxtval; //Next Year Start date: first SUNDAY of October
				*enxtdate = enxtval; //Next Year End date: first SUNDAY of April

				*smonth = 9; //October
				*emonth = 3; //April

				*shr = 2; //2:00 AM
				*ehr = 2; //3:00 AM

				isDSTTimeZone = 1;
				APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "sdate: %d, edate: %d, snxtdate: %d, enxtdate: %d, smonth: %d, emonth: %d, shr: %d, ehr: %d, isDSTTimeZone: %d", *sdate, *edate, *snxtdate, *enxtdate, *smonth, *emonth, *shr, *ehr, isDSTTimeZone);
				return isDSTTimeZone;
		}

		/*************************************************************************************
		 * (Source: http://wwp.greenwichmeantime.com/time-zone/pacific/new-zealand/
		 * Summer Time (Daylight Saving Time) runs in:
		 * 1)Wellington, Auckland
		 * 2)Waitangi, Chatham Island 
		 * from the last Sunday in September through to the first Sunday in April
		 *************************************************************************************/

		delta = (g_lastTimeZone - NZ_TIMEZONE_1);
		delta1 = (g_lastTimeZone - NZ_TIMEZONE_2);
		if (((delta <= 0.001) && (delta >= -0.001)) ||
						((delta1 <= 0.001) && (delta1 >= -0.001)))
		{
				/*
				 * Computation to calculate the last Sunday in September 
				 * and first SUNDAY of April
				 *
				 */

				sval = ((31 + *edate)%7);
				if(sval == 0)
						sval = sval + 7;

				eval = (7 - ((31 - *sdate)%7));

				/* for last sunday of sep */
				*sdate = (sval - 7) + 30; //Start date: last SUNDAY of September
				*edate = eval; //End date: first SUNDAY of April

				*enxtdate = 7-((int)(floor(1+(nxtyr*5/4)))%7);

				snxtval = ((31 + *enxtdate)%7);
				if(snxtval == 0)
						snxtval = snxtval + 7;

				enxtval = (7 - ((31 - *snxtdate)%7));

				/* for last sunday of sep */
				*snxtdate = (snxtval - 7) + 30; //Next Year Start date: last SUNDAY of September 
				*enxtdate = enxtval; //Next Year End date: first SUNDAY of April

				*smonth = 8; //September
				*emonth = 3; //April

				*shr = 2; //2:00 AM
				*ehr = 2; //3:00 AM

				isDSTTimeZone = 1;
				APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "sdate: %d, edate: %d, snxtdate: %d, enxtdate: %d, smonth: %d, emonth: %d, shr: %d, ehr: %d, isDSTTimeZone: %d", *sdate, *edate, *snxtdate, *enxtdate, *smonth, *emonth, *shr, *ehr, isDSTTimeZone);
				return isDSTTimeZone;
		}

		APP_LOG("calculateTimeZoneSpecificInfo",LOG_DEBUG, "sdate: %d, edate: %d, snxtdate: %d, smonth: %d, emonth: %d, shr: %d, ehr: %d, isDSTTimeZone: %d", *sdate, *edate, *snxtdate, *smonth, *emonth, *shr, *ehr, isDSTTimeZone);
		return isDSTTimeZone;
}

/*
 * dstComputationThread: 
 *  Compute DST time for different timezones 
 */
void* dstComputationThread(void *arg)
{
		int retVal = 0;
		int sdate = 0, edate = 0, dstime = 0, snxtdate=0, enxtdate=0;
		int dstyr = 0, nxtyr=0;
		struct tm *tm_struct;
		struct timeval dtime;
		struct tm mkTm = { 0 };
		unsigned int dsdate = 0, dedate = 0, dnsdate=0, dnedate=0;
		int dstToggleTime=0;
		int dstEnable=0;
		int updateDstime = *(int *)arg;
		float adjustment=0.0;
		int smonth = 0, emonth = 0;
		int shr = 0, ehr = 0;
		int isDSTTimeZone = 0; //0 = NO DST, 1 = DST
		char dstenable[SIZE_16B];
		float delta, delta1, delta2, delta3, delta4;
		char ltimezone[SIZE_16B];
		int index = 0;
                tu_set_my_thread_name( __FUNCTION__ );
		memset(dstenable, 0x0, sizeof(dstenable));

		free(arg);

restart_thread:

		/* sleep a while to let SyncTime finish its processing */
		APP_LOG("WiFiApp",LOG_DEBUG, "sleep a while to let SyncTime finish its processing, updateDstime: %d ..",updateDstime);
		sleep(30);

		tm_struct = (struct tm *)malloc(sizeof(struct tm));
		if (!tm_struct) {
				APP_LOG("WiFiApp",LOG_ERR, "Unable to allocate memory for tm struct ..");
				return NULL;
		}

		memset(tm_struct, 0, sizeof(struct tm));
		gettimeofday(&dtime, NULL);
		dstime = dtime.tv_sec; 
		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "Current time %u\n", dstime);

		dstime = GetUTCTime(); 
		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "UTC time: %u\n",dstime);
		gmtime_r((const time_t *) &dstime, tm_struct);

		dstyr = tm_struct->tm_year+1900;
		nxtyr = tm_struct->tm_year+1900+1;

		isDSTTimeZone = calculateTimeZoneSpecificInfo(dstyr, nxtyr, &sdate, &smonth, &shr, 
						&edate, &emonth, &ehr, &snxtdate, &enxtdate);

		if(!isDSTTimeZone) {
				free(tm_struct);
				tm_struct = NULL;
				return (void *)retVal;
		}


		//start date DST
		mkTm.tm_year = dstyr-1900;
		mkTm.tm_mday = sdate;
		mkTm.tm_mon = smonth; //2; //March
		mkTm.tm_hour = shr; //2;
		mkTm.tm_min = 0;
		mkTm.tm_sec = 0;
		dsdate = mktime(&mkTm);
		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "DST start day %d %s year %d having act time %u\n", sdate, convertMonth(smonth), dstyr, dsdate);

		//end date DST
		mkTm.tm_year = dstyr-1900;
		mkTm.tm_mday = edate;
		mkTm.tm_mon = emonth; //10; //November
		mkTm.tm_hour = ehr; //1;	//2:00 am
		mkTm.tm_min = 0;
		mkTm.tm_sec = 0;
		dedate = mktime(&mkTm);
		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "DST end day %d %s year %d having act time %u \n", edate, convertMonth(emonth), dstyr, dedate);

		//Next year's start date DST
		mkTm.tm_year = nxtyr-1900;
		mkTm.tm_mday = snxtdate;
		mkTm.tm_mon = smonth; //2; //March
		mkTm.tm_hour = shr; //2;
		mkTm.tm_min = 0;
		mkTm.tm_sec = 0;
		dnsdate = mktime(&mkTm);
		APP_LOG("UPNP: DEVICE",LOG_DEBUG, "DST start day %d %s year %d having act time %u \n", snxtdate, convertMonth(smonth), nxtyr, dnsdate);

		delta = (g_lastTimeZone - AUS_TIMEZONE_1);
		delta1 = (g_lastTimeZone - AUS_TIMEZONE_2);
		delta2 = (g_lastTimeZone - AUS_TIMEZONE_3);
		delta3 = (g_lastTimeZone - NZ_TIMEZONE_1);
		delta4 = (g_lastTimeZone - NZ_TIMEZONE_2);
		if (((delta <= 0.001) && (delta >= -0.001)) ||
						((delta1 <= 0.001) && (delta1 >= -0.001)) ||
						((delta2 <= 0.001) && (delta2 >= -0.001)) ||
						((delta3 <= 0.001) && (delta3 >= -0.001)) ||
						((delta4 <= 0.001) && (delta4 >= -0.001)))
		{
				mkTm.tm_year = nxtyr-1900;
				mkTm.tm_mday = enxtdate;
				mkTm.tm_mon = emonth;
				mkTm.tm_hour = ehr;
				mkTm.tm_min = 0;
				mkTm.tm_sec = 0;
				dnedate = mktime(&mkTm);
				APP_LOG("UPNP: DEVICE",LOG_DEBUG, "DST end day %d %s year %d having act time %u \n", enxtdate, convertMonth(emonth), nxtyr, dnedate);
		}

		free(tm_struct);
		tm_struct = NULL;

		/*
		 * dstime -> current time
		 * dsdate -> DST start time
		 * dedate -> DST end time
		 */	

		/* we have to re-compute DST toggle if it was set last by phone app*/
		if(updateDstime)
		{ 
				APP_LOG("WiFiApp",LOG_DEBUG, "old dstime: %d seconds", dstime);
				adjustment = (g_lastTimeZone*3600);
				dstime = GetUTCTime() + (int)adjustment;
				APP_LOG("WiFiApp",LOG_DEBUG, "updated dstime: %d seconds adjusted %d secs", dstime, (int)adjustment);
		}

		/* Countries which are in Southern Hemisphere follow AUS_TIMEZONE and NZ_TIMEZONE rules */
		delta = (g_lastTimeZone - AUS_TIMEZONE_1);
		delta1 = (g_lastTimeZone - AUS_TIMEZONE_2);
		delta2 = (g_lastTimeZone - AUS_TIMEZONE_3);
		delta3 = (g_lastTimeZone - NZ_TIMEZONE_1);
		delta4 = (g_lastTimeZone - NZ_TIMEZONE_2);
		if (((delta <= 0.001) && (delta >= -0.001)) ||
						((delta1 <= 0.001) && (delta1 >= -0.001)) ||
						((delta2 <= 0.001) && (delta2 >= -0.001)) ||
						((delta3 <= 0.001) && (delta3 >= -0.001)) ||
						((delta4 <= 0.001) && (delta4 >= -0.001)))
		{
				//manupulation is done for the Waitangi, Chatham isl., New Zealand for additional 45 mins i.e GMT+12.45, 2700 comes from 45*60) 
				delta = (g_lastTimeZone - NZ_TIMEZONE_2);
				if((delta <= 0.001) && (delta >= -0.001)) {
						dedate = dedate + 2700;
						dsdate = dsdate + 2700;
						dnedate = dnedate + 2700;
						dnsdate = dnsdate + 2700;
				}

				if(dstime <= dedate) /* dstime <= dedate */
				{
						dstToggleTime= (dedate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST end: %d seconds\n", dstToggleTime);
						dstEnable = 0; /* DST currenty enabled, will remain enabled another (dedate-dstime) seconds */
				}
				else if(dstime <= dsdate)
				{
						dstToggleTime = (dsdate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST start: %d seconds\n", dstToggleTime);
						dstEnable = 1; /* DST currenty disabled, will enable (dstEnable=1) in (dsdate-dstime) seconds */
				}
				else if(dstime <= dnedate) /* dsdate < dstime <= dnedate */
				{
						dstToggleTime= (dnedate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST end: %d seconds\n", dstToggleTime);
						dstEnable = 0; /* DST currenty enabled, will remain enabled another (dnedate-dstime) seconds */
				}
				else
				{
						dstToggleTime= (dnsdate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST start next year: %d seconds\n", dstToggleTime);
						dstEnable = 1; /* DST currenty disabled, will enable in another (dnsdate-dstime) seconds */
				}
		} else {
				if(dstime <= dsdate)
				{
						dstToggleTime = (dsdate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST start: %d seconds\n", dstToggleTime);
						dstEnable = 1; /* DST currenty disabled, will enable (dstEnable=1) in (dsdate-dstime) seconds */
				}
				else if(dstime <= dedate) /* dsdate < dstime <= dedate */
				{
						dstToggleTime= (dedate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST end: %d seconds\n", dstToggleTime);
						dstEnable = 0; /* DST currenty enabled, will remain enabled another (dedate-dstime) seconds */
				}
				else
				{
						dstToggleTime= (dnsdate - dstime);
						APP_LOG("WiFiApp",LOG_DEBUG, "Time before DST start next year: %d seconds\n", dstToggleTime);
						dstEnable = 1; /* DST currenty disabled, will enable in another (dnsdate-dstime) seconds */
				}
		}



		/* start a timer for toggleTime seconds */
		gDstEnable = dstEnable;
		snprintf(dstenable, sizeof(dstenable), "%d", gDstEnable);
		SetBelkinParameter (LASTDSTENABLE, dstenable);
		gTimeZoneUpdated = 1;
		APP_LOG("WiFiApp",LOG_DEBUG, "############ wait %d seconds to set dst=%d ###########", dstToggleTime, gDstEnable);
		AsyncSaveData();
		sleep(dstToggleTime);
		updateSysTime();
		sleep(5);
#if defined(PRODUCT_WeMo_Insight)
		g_isDSTApplied=1;
#endif
		RestartRule4DST();

		/* call is to propagate DST information to PJ lib */
		memset(ltimezone, 0x0, sizeof(ltimezone));
		snprintf(ltimezone, sizeof(ltimezone), "%f", g_lastTimeZone);
		index = getTimeZoneIndexFromFlash();
		APP_LOG("UPNP: Device", LOG_DEBUG,"set pj dst data index: %d, lTimeZone:%s gDstEnable: %d success", index, ltimezone, gDstEnable); 
		pj_dst_data_os(index, ltimezone, gDstEnable);

		//APP_LOG("WiFiApp",LOG_DEBUG, "dstComputation thread exiting...");
		/* toggle dst setting */

		gDstEnable = !gDstEnable;
		memset(dstenable, 0x0, sizeof(dstenable));
		snprintf(dstenable, sizeof(dstenable), "%d", gDstEnable);
		APP_LOG("UPNP: Device", LOG_DEBUG,"Updating gDstEnable to: %d", gDstEnable); 
		SetBelkinParameter (LASTDSTENABLE, dstenable);
		gTimeZoneUpdated = 1;
		AsyncSaveData();

		/* wait for next DST switch-over */
		goto restart_thread;

		return (void *)retVal;
}

/*
 * computeDstToggleTime: 
 *  Method to remove DST compute thread,if any and create a new DST compute thread 
 */
int computeDstToggleTime(int updateDstime) 
{
		int retVal = 0;
		int *pUpdateDstime;

		pUpdateDstime = (int *)malloc(sizeof(int));
		if(!pUpdateDstime)
		{
				APP_LOG("WiFiApp",LOG_ERR,"Unable to allocate memory for an int....");
				return -1;
		}

		*pUpdateDstime = updateDstime;

		/* first of all remove the dst thread, if running */
		if(dstmainthread != -1)
		{
				if((retVal = pthread_kill(dstmainthread, SIGRTMIN)) == 0)
				{
						dstmainthread=-1;
						APP_LOG("WiFiApp",LOG_DEBUG,"DST thread removed successfully....");
				}
				else
						APP_LOG("WiFiApp",LOG_ERR,"DST thread removal failed [%d] ....",retVal);
		}
		else
				APP_LOG("WiFiApp",LOG_DEBUG,"DST thread doesn't exist....");

		/* create a thread which sleeps for dstToggleTime and then updates system time */
		pthread_attr_init(&dst_main_attr);
		pthread_attr_setdetachstate(&dst_main_attr,PTHREAD_CREATE_DETACHED);
		retVal = pthread_create(&dstmainthread,&dst_main_attr,
						(void*)&dstComputationThread, (void *)pUpdateDstime);

		if(retVal < 0) {
				APP_LOG("WiFiApp",LOG_ERR,
								"DST main Thread not created");
		}
		else
		{
				APP_LOG("WiFiApp",LOG_DEBUG,
								"DST main  Thread created successfully");
		}

		return 0;
}
/***
 *  Function to get timezone_index value
 *	from flash
 *
 ******************************************/

int getTimeZoneIndexFromFlash(void)
{
		int Index = 0;
		char *timeZIdx = NULL;

		timeZIdx = GetBelkinParameter("timezone_index");
		if(timeZIdx && strlen(timeZIdx) != 0) 
		{
				Index = atoi(timeZIdx);
		}

		APP_LOG("UPNP", LOG_DEBUG,"Index: %d", Index);
#ifdef PRODUCT_WeMo_NetCam
		if (timeZIdx)
				free(timeZIdx);
#endif
		return Index;
}

#ifdef PRODUCT_WeMo_NetCam
extern int IsTimeZoneSetupByNetCam();
extern int UpdateNetCamTimeZoneInfo(const char* szRegion);
int GetTimezoneRegionIndex(float iTimeZoneValue)
{
		printf("Time zone: look for table of %f\n", iTimeZoneValue);
		int counter = sizeof(g_tTimeZoneList)/sizeof(tTimeZone);
		int loop = 0x00;
		int index = -1;

		for (; loop < counter; loop++)
		{
				float delta = iTimeZoneValue - g_tTimeZoneList[loop].iTimeZone;

				if ((delta <= 0.001) && (delta >= -0.001))
				{
						index = loop;
						break;
				}
		}

		if (loop == counter)
				printf("Can not find: %f\n", iTimeZoneValue);

		return index;
}
#endif

/***
 *
 *
 *
 *
 *
 *
 ******************************************/
int SyncTime(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
#ifdef PRODUCT_WeMo_NetCam
		if (IsTimeZoneSetupByNetCam())
		{
				APP_LOG("TimeZone", LOG_DEBUG, "TimeZone set already from NetCam"); 
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SyncTime", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "TimeZone updated already from NetCam");
				return 0x00;
		}
#endif

		int isLocalDstSupported = LOCAL_DST_SUPPORTED_ON;
		int index = 0;

		if (0x00 == pActionRequest || 0x00 == pActionRequest)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_TIME_SYNC;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SyncTime", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "Parameters Error!");
				return PLUGIN_ERROR_E_TIME_SYNC;
		}

		char* szUtc 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "UTC");
		char* szTimeZone	= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "TimeZone");
		char* szDst			= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "dst");
		//- The read the local phone, dst is supported now or not, 
		char* szIsLocalDst	= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DstSupported");
		char ltimezone[SIZE_16B];
		memset(ltimezone, 0x0, sizeof(ltimezone));

		APP_LOG("UPNP: Device",LOG_DEBUG,"set time: utc: %s, timezone: %s, dst: %s", szUtc, szTimeZone, szDst); 

		if (0x00 == szUtc || 0x00 == szTimeZone || 0x00 == szDst)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_TIME_SYNC;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "TimeSync", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "failure");
				APP_LOG("UPNP: Device", LOG_DEBUG, "paramters error"); 

				return 0x01;
		}

		float TimeZone = 0.0;	

		int utc 			= atoi(szUtc);

		//- atof not working well under this compiler, so calculated manually		
		if (szTimeZone[0x00] == '-')
		{		
				TimeZone 	= atoi(szTimeZone);
				if (0x00!= strstr(szTimeZone, ".5"))		
				{
						TimeZone -= 0.5;		
				}		
				else if (0x00!= strstr(szTimeZone, ".75"))
				{
						TimeZone += 0.25;
				}	
		}	
		else
		{
				TimeZone 	= atoi(szTimeZone);
				if (0x00!= strstr(szTimeZone, ".5"))
				{
						TimeZone += 0.5;
				}		
				else if (0x00!= strstr(szTimeZone, ".75"))
				{
						TimeZone += 0.75;
				}
		}


		int dst			= atoi(szDst);
#ifdef PRODUCT_WeMo_NetCam
		if (DST_ON == dst)
		{
				TimeZone = TimeZone - 1.0;
		}
#endif

		if (!IsApplyTimeSync(utc, TimeZone, dst))
		{
				//- UPnP response here with failure
				APP_LOG("UPNP: Device", LOG_DEBUG, ": *****NOT APPLYING TIME SYNC"); 
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "TimeSync", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "failure");
				return 0x01;
		}

		//- Get the local Dst supported flag
		if ( (0x00 != szIsLocalDst) && 
						0x00!= strlen(szIsLocalDst) )
		{
				isLocalDstSupported = atoi(szIsLocalDst);
				gDstSupported = isLocalDstSupported;
				SetBelkinParameter(SYNCTIME_DSTSUPPORT, szIsLocalDst);

				if(!gDstSupported)
				{
					UnSetBelkinParameter(LASTDSTENABLE);
				}
		}


#ifdef PRODUCT_WeMo_NetCam
		g_lastTimeZone = TimeZone;
		gNTPTimeSet = 1;
		g_lastTimeSync = utc;

		int iRegionIndex = GetTimezoneRegionIndex(g_lastTimeZone);

		if (-1 == iRegionIndex)
		{
				iRegionIndex = DEFAULT_REGION_INDEX;
		}
		printf("Time zone: region index is %d\n", iRegionIndex);

		UpdateNetCamTimeZoneInfo(g_tTimeZoneList[iRegionIndex].szLinuxRegion);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SyncTime", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "TimeZone updated already from NetCam");
		FreeXmlSource(szUtc);
		FreeXmlSource(szTimeZone);
		FreeXmlSource(szDst);

		if (isLocalDstSupported)
		{
			computeDstToggleTime(1);
		}

		/* call is to propagate DST information to PJ lib */
		char lzFlashTimezone[64] = {0x00};
		sprintf(lzFlashTimezone, "%f", g_lastTimeZone);
		SetBelkinParameter(SYNCTIME_LASTTIMEZONE, lzFlashTimezone);
		if (!isLocalDstSupported)
		{
			gTimeZoneUpdated = 1;
		}

		return 0x00;
#endif



		int iTimeZoneIndex = GetTimeZoneIndex(TimeZone);
		APP_LOG("UPNP: Device",LOG_DEBUG,"set time: utc: %d, timezone: %f, dst: %d", utc, TimeZone, dst); 

		int rect = SetNTP(s_szNtpServer, iTimeZoneIndex, 0x00);

		if (Gemtek_Success == rect)
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "set NTP server success: %s", s_szNtpServer); 
		}
		else
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "set NTP server unsuccess: %s", s_szNtpServer); 
		}

		pluginUsleep(500000);

		rect = SetTime(utc, iTimeZoneIndex, 0x00);
		if (Gemtek_Success == rect)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = UPNP_E_SUCCESS;

				gNTPTimeSet = 1;
				g_lastTimeSync = utc;

				if (DST_ON == dst)
				{
						// calculate the absolute one
						{
								g_lastTimeZone = TimeZone - 1;
						}
				}
				else
				{
						//- Keep unchange
						g_lastTimeZone = TimeZone;
				}

				/* call is to propagate DST information to PJ lib */
				snprintf(ltimezone, sizeof(ltimezone), "%f", g_lastTimeZone);
				index = getTimeZoneIndexFromFlash();
				APP_LOG("UPNP: Device", LOG_DEBUG,"set pj dst data index: %d, lTimeZone:%s gDstEnable: %d success", index, ltimezone, gDstEnable); 
				pj_dst_data_os(index, ltimezone, gDstEnable);

				SetBelkinParameter (SYNCTIME_LASTTIMEZONE, ltimezone);

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "TimeSync", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "success");
				APP_LOG("UPNP: Device", LOG_CRIT,"set time: utc: %s, timezone: %s, dst: %s g_lastTimeZone:%f success", szUtc, szTimeZone, szDst,g_lastTimeZone);

				//- only if isLocalDstSupported true, the calculate and toggle?
				if (isLocalDstSupported)
				{
						//- Arizona and Hawii will not?
						computeDstToggleTime(1); 
				}
				else
					gTimeZoneUpdated = 1;

				AsyncSaveData();
#ifdef PRODUCT_WeMo_Insight
				APP_LOG("UPNP: Device", LOG_ERR,"Restarting Data export scheduler on time sync"); 
				g_ReStartDataExportScheduler = 1;
				ReStartDataExportScheduler();
#endif
		}
		else
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_TIME_SYNC;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "TimeSync", CtrleeDeviceServiceType[PLUGIN_E_TIME_SYNC_SERVICE],"status", "failure");
				APP_LOG("UPNP: Device", LOG_ERR,"set time: utc: %s, timezone: %s, dst: %s failure", szUtc, szTimeZone, szDst); 
		}

		FreeXmlSource(szUtc);
		FreeXmlSource(szTimeZone);
		FreeXmlSource(szDst);

		return  pActionRequest->ErrCode;
}

int DeviceActionResponse(pUPnPActionRequest pActionRequest, 
				const char* responseName, 
				const char* serviceType, 
				const char* variabName, 
				const char *variableValue
				)
{
		UpnpAddToActionResponse(&pActionRequest->ActionResult, responseName, serviceType, variabName, variableValue);
		return 0;
}

int SyncTimeEx(int utc, int timezone, int isDstEnable)
{
		return SetTime(utc, timezone, isDstEnable);
}

void replace_str(char *buffer, char *str, char *orig, char *rep)
{
  char *p;
  if(!(p = (char *)strstr(str, orig))){ // Is 'orig' even in 'str'?
    strncpy(buffer, str, 256);
    return;
  }

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

}



/**
 * Get Friendly Name:
 * 	Callback to get device friendly name
 * 	
 * 
 * *****************************************************************************************************************/
int GetFriendlyName(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		//char buf[SIZE_256B];
		//memset(buf, 0, 256);

		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);
#ifdef PRODUCT_WeMo_NetCam    
		AsyncRequestNetCamFriendlyName();	  
#endif //PRODUCT_WeMo_NetCam
		char *szFriendlyName = GetDeviceConfig("FriendlyName");

		APP_LOG("UPNP: Device", LOG_DEBUG, "Read name from flash: %s", szFriendlyName);

		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetFriendlyName", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"FriendlyName", g_szFriendlyName);

		
        /*
        APP_LOG("UPNP: Device", LOG_DEBUG,"Get friendly name before modification:%s", g_szFriendlyName);
		//replace_str(buf, g_szFriendlyName,"&amp;", "&" );
		replace_str(buf, szFriendlyName, "&", "&amp;");

		memset(g_szFriendlyName, 0, 256);
		memcpy(g_szFriendlyName ,buf, 256);
		//strcpy(g_szFriendlyName, "india&");
		APP_LOG("UPNP: Device", LOG_DEBUG,"get friendly name after modification:%s", g_szFriendlyName); 
		
	     //end of gaurav
            */
		APP_LOG("UPNP: Device", LOG_DEBUG,"Get friendly name: %s", g_szFriendlyName); 
		return UPNP_E_SUCCESS;
}

int SetHomeId(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char* szHomeId = NULL;
		char homeId[SIZE_20B];
		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure");
				return UPNP_E_INVALID_PARAM;
		}

		szHomeId = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "HomeId");
		if( (szHomeId!=NULL) && (0x00 != strlen(szHomeId)) && !strcmp(g_szRestoreState, "0"))
		{
				memset(homeId, 0x0, sizeof(homeId));
				if(decryptWithMacKeydata(szHomeId, homeId) != SUCCESS)
				{
						APP_LOG("UPNP: Device", LOG_ERR, "set home id decryption failure");
						FreeXmlSource(szHomeId);
						return FAILURE;
				}
				APP_LOG("UPNP: Device", LOG_HIDE, "decrypted home id: %s", homeId);
				SetBelkinParameter("home_id", homeId);
				AsyncSaveData();
				APP_LOG("UPNP: Device", LOG_HIDE, "home id update successfully: %s", homeId);
				/* update the global variable to contain updated home_id */
				memset(g_szHomeId, 0, sizeof(g_szHomeId));
				strncpy(g_szHomeId, homeId, sizeof(g_szHomeId)-1);
				FreeXmlSource(szHomeId);
		}
		else
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure or restored device, g_szRestoreState: %s", g_szRestoreState);

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetHomeId", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "HomeId", "SUCCESS");
		APP_LOG("UPNP: Device", LOG_DEBUG, "set home id response sent");

		return UPNP_E_SUCCESS;
}

int SetDeviceId(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		return UPNP_E_SUCCESS;
}

int GetDeviceId(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		return UPNP_E_SUCCESS;
}

int SetSmartDevInfo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);

		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "UPNP paramter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}
		char* szSmartDevUrl = NULL;
		char buf[SIZE_256B] = {'\0'};
		int retVal = 0;

		szSmartDevUrl = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "SmartDevURL");
		if(szSmartDevUrl!=NULL){
				if (0x00 == strlen(szSmartDevUrl))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetSmartDevInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"SmartDevURL", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"SmartDevURL parameter failure");
						FreeXmlSource(szSmartDevUrl);
						return PLUGIN_ERROR_E_BASIC_EVENT;
				}
		}

		APP_LOG("UPNP: Device",LOG_DEBUG, "SmartDevURL is: %s", szSmartDevUrl);
		snprintf(buf, sizeof(buf), "wget %s -O %s", szSmartDevUrl, SMARTDEVXML);
		retVal = system(buf);
		if (retVal != 0) {
				APP_LOG("UPNP: Device", LOG_ERR,"Download smart device info xml %s failed\n", buf);
		}else {
				APP_LOG("UPNP: Device", LOG_NOTICE,"Downloaded smart device info xml from location %s successfully\n", buf);
		}
		FreeXmlSource(szSmartDevUrl);

		return UPNP_E_SUCCESS;
}

int GetSmartDevInfo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);

		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "UPNP paramter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		char szSmartDevInfoURL[MAX_FW_URL_LEN];
		memset(szSmartDevInfoURL, 0x00, sizeof(szSmartDevInfoURL));

		if (strlen(g_server_ip) > 0x00)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				snprintf(szSmartDevInfoURL, sizeof(szSmartDevInfoURL), "http://%s:%d/smartDev.txt", g_server_ip, g_server_port);
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetSmartDevInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "SmartDevInfo", szSmartDevInfoURL);
		}
		else
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetSmartDevInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "SmartDevInfo", "");
		}

		APP_LOG("UPNP: Device", LOG_DEBUG,"GetSmartDevInfoURL: %s", szSmartDevInfoURL);

		return UPNP_E_SUCCESS;
}

int GetShareHWInfo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);
		char* szMac = NULL;
		char* szSerial = NULL;
		char* szUdn = NULL;
		char* szRestoreState = NULL;
		char* szHomeId = NULL;
		char* szPvtKey = NULL;


		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}
		/* to restrict this request if there is already one in process */
		if(g_pxRemRegInf)
		{
				APP_LOG("UPNP: Device", LOG_ERR, "proxy registration already in process");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		/* allocate proxy registration request structure */
		g_pxRemRegInf = (ProxyRemoteAccessInfo *)calloc(1, sizeof(ProxyRemoteAccessInfo));
		if(g_pxRemRegInf == NULL)
		{
				APP_LOG("UPNP",LOG_ERR, "proxy RemRegInf mem allocation FAIL");
				return FAILURE;
		}

		szMac = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Mac");
		if(szMac!=NULL)
		{
				if (0x00 == strlen(szMac))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_REMOTE_ACCESS;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "ShareHWInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"Mac", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"Proxy Mac parameter failure");
						return PLUGIN_ERROR_E_REMOTE_ACCESS;
				}
		}

		szSerial = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Serial");
		if(szSerial!=NULL)
		{
				if (0x00 == strlen(szSerial))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_REMOTE_ACCESS;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "ShareHWInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"Serial", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"Proxy Serial number parameter failure");
						FreeXmlSource(szMac);
						return PLUGIN_ERROR_E_REMOTE_ACCESS;
				}
		}

		szUdn = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Udn");
		if(szUdn!=NULL)
		{
				if (0x00 == strlen(szUdn))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_REMOTE_ACCESS;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "ShareHWInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"Udn", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"Proxy Udn parameter failure");
						FreeXmlSource(szMac);
						FreeXmlSource(szSerial);
						return PLUGIN_ERROR_E_REMOTE_ACCESS;
				}
		}

		szRestoreState = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "RestoreState");
		if(szRestoreState!=NULL)
		{
				if (0x00 == strlen(szRestoreState))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = PLUGIN_ERROR_E_REMOTE_ACCESS;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "ShareHWInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"RestoreState", "Parameter Error");
						APP_LOG("UPNP: Device", LOG_ERR,"Proxy Restore State parameter failure");
						FreeXmlSource(szMac);
						FreeXmlSource(szSerial);
						FreeXmlSource(szUdn);
						return PLUGIN_ERROR_E_REMOTE_ACCESS;
				}
		}

		szHomeId = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "HomeId");
		if(szHomeId!=NULL)
		{
				if (0x00 == strlen(szHomeId))
				{
						APP_LOG("UPNP: Device", LOG_ERR,"Proxy Home Id parameter failure");
						FreeXmlSource(szHomeId);
						szHomeId = 0x00;
				}
		}

		szPvtKey = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "PluginKey");
		if(szPvtKey!=NULL)
		{
				if (0x00 == strlen(szPvtKey))
				{
						APP_LOG("UPNP: Device", LOG_ERR,"Proxy Pvt Key parameter failure");
						FreeXmlSource(szPvtKey);
						szPvtKey = 0x00;
				}
		}

		strncpy(g_pxRemRegInf->proxy_macAddress, szMac, sizeof(g_pxRemRegInf->proxy_macAddress)-1);
		strncpy(g_pxRemRegInf->proxy_serialNumber, szSerial, sizeof(g_pxRemRegInf->proxy_serialNumber)-1);
		strncpy(g_pxRemRegInf->proxy_pluginUniqueId, szUdn, sizeof(g_pxRemRegInf->proxy_pluginUniqueId)-1);
		strncpy(g_pxRemRegInf->proxy_restoreState, szRestoreState, sizeof(g_pxRemRegInf->proxy_restoreState)-1);
		if((szHomeId!= NULL) && (strlen(szHomeId)> 0x00))
				strncpy(g_pxRemRegInf->proxy_homeId, szHomeId, sizeof(g_pxRemRegInf->proxy_homeId)-1);
		if((szPvtKey!= NULL) && (strlen(szPvtKey)> 0x00))
				strncpy(g_pxRemRegInf->proxy_privateKey, szPvtKey, sizeof(g_pxRemRegInf->proxy_privateKey)-1);

		APP_LOG("UPNP: Device", LOG_DEBUG,"Proxy MAC: <%s> SERIAL: <%s> UDN: <%s>", g_pxRemRegInf->proxy_macAddress, g_pxRemRegInf->proxy_serialNumber, g_pxRemRegInf->proxy_pluginUniqueId);	

		FreeXmlSource(szMac);
		FreeXmlSource(szSerial);
		FreeXmlSource(szUdn);
		FreeXmlSource(szRestoreState);
		if(szHomeId!=NULL)
				FreeXmlSource(szHomeId);
		if(szPvtKey!=NULL)
				FreeXmlSource(szPvtKey);

		return UPNP_E_SUCCESS;
}

int GetMacAddr(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szPluginUDN[SIZE_128B];
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);

		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}
		char *szMacAddr = g_szWiFiMacAddress;
		if(szMacAddr != NULL)
		{
				if (0x00 == strlen(szMacAddr))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						APP_LOG("UPNP: Device", LOG_ERR,"Failure Get Mac Addr: %s", ""); 
						return PLUGIN_ERROR_E_BASIC_EVENT;
				}
		}
		char *szSerialNo = g_szSerialNo;
		if(szSerialNo != NULL)
		{ 
				if (0x00 == strlen(szSerialNo))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						APP_LOG("UPNP: Device", LOG_ERR,"Failure Get Serial: %s", ""); 
						return PLUGIN_ERROR_E_BASIC_EVENT;
				}
		}
		memset(szPluginUDN, 0x00, sizeof(szPluginUDN));
		strncpy(szPluginUDN, g_szUDN, sizeof(szPluginUDN)-1);
		if(szPluginUDN != NULL)
		{ 
				if (0x00 == strlen(szPluginUDN))
				{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						APP_LOG("UPNP: Device", LOG_ERR,"Failure Get UDN: %s", ""); 
						return PLUGIN_ERROR_E_BASIC_EVENT;
				}
		}

		APP_LOG("UPNP: Device", LOG_DEBUG, "Read  from flash Mac Addr: %s Serial No: %s UDN: %s\n", szMacAddr, szSerialNo, szPluginUDN);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetMacAddr", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"MacAddr", szMacAddr);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetSerialNo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"SerialNo", szSerialNo);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetPluginUDN", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"PluginUDN", szPluginUDN);

		return UPNP_E_SUCCESS;
}
int GetSerialNo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		return UPNP_E_SUCCESS;
}
int GetPluginUDN(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		return UPNP_E_SUCCESS;
}

int GetHomeId(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{

		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);

		APP_LOG("UPNP: Device", LOG_HIDE, "Read homeid from flash: %s\n", g_szHomeId);

		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		//- Zero name
		if (0x00 == strlen(g_szHomeId))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetHomeId", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"HomeId", "failure");
				APP_LOG("UPNP: Device", LOG_ERR,"Failure Get home id: %s", ""); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetHomeId", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"HomeId", g_szHomeId);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetDeviceId", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"DeviceId", g_szSmartDeviceId);


		APP_LOG("UPNP: Device", LOG_HIDE,"Get home id: %s", g_szHomeId); 

		return UPNP_E_SUCCESS;
}

int GetHomeInfo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szResponse[SIZE_512B];
		authSign *assign = NULL;
		char szSign[SIZE_256B];

		memset(szResponse, 0x00, sizeof(szResponse));
		memset(szSign, 0x00, sizeof(szSign));

		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		snprintf(szResponse, sizeof(szResponse), "%s|", g_szRestoreState); 

		if (0x00 == strlen(g_szHomeId))
				strncat(szResponse, "NO_HOME", sizeof(szResponse)-strlen(szResponse)-1);
		else
				strncat(szResponse, g_szHomeId, sizeof(szResponse)-strlen(szResponse)-1);

		/* Create & Append the encrypted Auth header as well */

		if(strlen(g_szPluginPrivatekey))
		{
				assign = createAuthSignature(g_szWiFiMacAddress, g_szSerialNo, g_szPluginPrivatekey);
				if (!assign) {
						APP_LOG("UPNP: DEVICE", LOG_ERR, "\n Signature Structure returned NULL\n");
						goto on_failure;
				}

				APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################Before encryption: Signature: <%s>", assign->signature);
				encryptWithMacKeydata(assign->signature, szSign);
				if (!strlen(szSign))
				{
				    APP_LOG("UPNP: DEVICE", LOG_ERR, "\n Signature Encrypt returned NULL\n");
				    goto on_failure;
				}
				APP_LOG("UPNP: DEVICE", LOG_HIDE, "###############################After encryption: Signature: <%s>", szSign);
		}
		else
		{
				strncpy(szSign, "NO_SIGNATURE", sizeof(szSign)-1);
				APP_LOG("UPNP: DEVICE", LOG_HIDE, "Signature: <%s>", szSign);
		}
		strncat(szResponse, "|", sizeof(szResponse)-strlen(szResponse)-1);
		strncat(szResponse, szSign, sizeof(szResponse)-strlen(szResponse)-1);
		strncat(szResponse, "|", sizeof(szResponse)-strlen(szResponse)-1);

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetHomeInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"HomeInfo", szResponse);
		APP_LOG("UPNP: Device", LOG_HIDE,"Get home info: %s", szResponse); 

		if (assign) {free(assign); assign = NULL;};
		return UPNP_E_SUCCESS;

on_failure:
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetHomeInfo", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"HomeInfo", "failure");
		APP_LOG("UPNP: Device", LOG_ERR,"Failure in Get home info"); 
		if (assign) {free(assign); assign = NULL;};
		return PLUGIN_ERROR_E_BASIC_EVENT;

}

void sendRemoteUpnpSuccessResponse(pUPnPActionRequest pActionRequest, pluginRegisterInfo *pPlgReg)
{
		APP_LOG("UPnP: Device", LOG_ERR, "Sending remote access success upnp response");
		char homeId[MAX_RVAL_LEN];

		memset(homeId, 0, sizeof(homeId));
		snprintf(homeId, sizeof(homeId), "%lu", pPlgReg->pluginDevInf.homeId);

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		if(0x00 != strlen(homeId))
		{
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"homeId",homeId);
		}
		if (0x00 != pPlgReg->pluginDevInf.privateKey && (0x01 < strlen(pPlgReg->pluginDevInf.privateKey)))
		{
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"pluginprivateKey", pPlgReg->pluginDevInf.privateKey);
		}
		if (0x00 != pPlgReg->smartDevInf.privateKey && (0x01 < strlen(pPlgReg->smartDevInf.privateKey)))
		{
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"smartprivateKey", pPlgReg->smartDevInf.privateKey);
		}
		else
		{
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"smartprivateKey", "NOKEY");
		}
		if(pPlgReg->resultCode!=NULL){
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"resultCode", pPlgReg->resultCode);
		}
		if(pPlgReg->description!=NULL){
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"description", pPlgReg->description);
		}
		if(pPlgReg->statusCode!=NULL){
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"statusCode", pPlgReg->statusCode);
		}
		if(pPlgReg->smartDevInf.smartUniqueId!=NULL){
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"smartUniqueId", pPlgReg->smartDevInf.smartUniqueId);
		}

		APP_LOG("UPnP: Device", LOG_ERR, "Remote access success upnp response sent");
}

void sendRemoteUpnpErrorResponse(pUPnPActionRequest pActionRequest, const char *pdesc, const char *pcode)
{
		APP_LOG("UPnP: Device", LOG_ERR, "Sending remote access error upnp response");
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = PLUGIN_ERROR_E_REMOTE_ACCESS;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"homeId","FAIL");
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"pluginprivateKey", "FAIL");
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"smartprivateKey", "FAIL");
		if (pdesc) {
				APP_LOG("UPnP: Device", LOG_ERR, "ERROR: Remote response message and re-register message %s\n", pdesc);
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"description", pdesc);
		}else{
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"description", "FAIL");
		}
		if (pcode) {
				APP_LOG("UPnP: Device", LOG_ERR, "ERROR: Remote re-register error code %s\n", pcode);
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"resultCode", pcode);
		}else{
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"resultCode", "FAIL");
		}
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "RemoteAccess", CtrleeDeviceServiceType[PLUGIN_E_REMOTE_ACCESS_SERVICE],"statusCode", "F");
		APP_LOG("UPnP: Device", LOG_ERR, "Remote access error upnp response sent");
}

void freeRemoteXMLRes(char* pDeviceId, char *pHomeId, char *pDevicePrivKey, char *pMacAddress, char *pDeviceName)
{
		FreeXmlSource(pHomeId);
		FreeXmlSource(pMacAddress);
		FreeXmlSource(pDeviceId);
		FreeXmlSource(pDeviceName);
		FreeXmlSource(pDevicePrivKey);
}

int createAutoRegThread(void)
{
		int retVal = SUCCESS;

		if ( (-1 != remoteRegthread) || (-1 != firmwareUpThread) )
		{
				APP_LOG("UPNP: Device", LOG_ERR, "############registration or firmware update thread already running################");
				APP_LOG("UPNP: Device", LOG_CRIT, "Registration or firmware update thread already running");
				retVal = FAILURE;
				return retVal;
		}

		pthread_attr_init(&remoteReg_attr);
		pthread_attr_setdetachstate(&remoteReg_attr,PTHREAD_CREATE_DETACHED);
		retVal = pthread_create(&remoteRegthread,&remoteReg_attr,(void*)&remoteRegThread, NULL);
		if(retVal < SUCCESS) {
				APP_LOG("UPnPApp",LOG_ERR, "************registration thread not Created");
				APP_LOG("UPnPApp",LOG_CRIT, "Registration thread not Created");
				retVal = FAILURE;
				return retVal;
		}

		APP_LOG("UPnPApp",LOG_DEBUG, "************registration thread Created");
		APP_LOG("UPnPApp",LOG_CRIT, "Registration thread Created");
		return retVal;
}

int createRemoteRegThread(RemoteAccessInfo remRegInf)
{
		int retVal = SUCCESS;
		RemoteAccessInfo *premRegInf = NULL;

		if ( (-1 != remoteRegthread) || (-1 != firmwareUpThread) )
		{
				APP_LOG("UPNP: Device", LOG_ERR, "############remote registration or firmware update thread already running################");
				APP_LOG("UPNP: Device", LOG_CRIT, "Remote registration or firmware update thread already running");
				retVal = FAILURE;
				return retVal;
		}

		premRegInf = (RemoteAccessInfo*)calloc(1, sizeof(RemoteAccessInfo));
		if(!premRegInf) 
		{
				APP_LOG("UPNPApp",LOG_DEBUG,"calloc failed...");
				retVal = FAILURE;
				return retVal;
		}

		//*premRegInf = remRegInf;
		memcpy(premRegInf, (RemoteAccessInfo *)&remRegInf, sizeof(RemoteAccessInfo));

		/* set remote access by mobile App flag */
		g_isRemoteAccessByApp = 0x1;

		pthread_attr_init(&remoteReg_attr);
		pthread_attr_setdetachstate(&remoteReg_attr,PTHREAD_CREATE_DETACHED);
		retVal = pthread_create(&remoteRegthread,&remoteReg_attr,(void*)&remoteRegThread, (void*)premRegInf);
		if(retVal < SUCCESS) {
				APP_LOG("UPnPApp",LOG_ERR, "************remote registration thread not Created");
				APP_LOG("UPnPApp",LOG_CRIT, "Remote registration thread not Created");
				retVal = FAILURE;
				return retVal;
		}

		APP_LOG("UPnPApp",LOG_DEBUG, "************registration thread Created");
		APP_LOG("UPnPApp",LOG_CRIT, "Registration thread Created");
		return retVal;
}

void initRemoteAccessLock(void)
{
		pthread_cond_init (&g_remoteAccess_cond, NULL);
		pthread_mutex_init(&g_remoteAccess_mutex, NULL);
#ifndef __ORESETUP__
		pthread_mutex_init(&g_remoteDeReg_mutex, NULL);
		pthread_cond_init (&g_remoteDeReg_cond, NULL);
#endif
}

/**
 * RemoteAccess:
 * 	Callback to Set Remote Access
 * 
 * 
 * *****************************************************************************************************************/
int RemoteAccess(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		pluginRegisterInfo *pPlgReg = NULL;
		RemoteAccessInfo remAcInf;
		char* szMacAddress = NULL;
		char* szDevicePrivKey = NULL;
		char* szDeviceId = NULL;
		char* szDeviceName = NULL;
		char* szHomeId = NULL;
		int ret = SUCCESS;

		APP_LOG("UPNPDevice", LOG_ERR,"Cloud Connection: NO REMOTE");

		/* sanitize remote access request parameters */
		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR,"INVALID PARAMETERS"); 
				sendRemoteUpnpErrorResponse(pActionRequest, "parameter failure",  "FAIL");
				goto on_return;
		}

#ifdef WeMo_SMART_SETUP
		/* Reset smart setup progress to allow responses to be sent back to App */
		gSmartSetup = 0;
#endif
		memset(&remAcInf, 0x00, sizeof(RemoteAccessInfo));

		szDeviceId = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DeviceId");
		if(0x0 != szDeviceId && 0x00 == strlen(szDeviceId))
		{
				APP_LOG("UPNP: Device", LOG_ERR,"smart device udid param is null"); 
				sendRemoteUpnpErrorResponse(pActionRequest, "deviceid parameter failure",  "FAIL");
				goto on_return;
		}

		APP_LOG("UPNPDevice", LOG_HIDE,"SMART DEVICE ID: %s received in upnp action request", szDeviceId);
		strncpy(remAcInf.smartDeviceId, szDeviceId, sizeof(remAcInf.smartDeviceId)-1);

		szDeviceName = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DeviceName");
		if(0x0 != szDeviceName && 0x00 != strlen(szDeviceName))
		{
				APP_LOG("UPNP: Device", LOG_ERR,"smart device name is: %s", szDeviceName); 
				strncpy(remAcInf.smartDeviceName, szDeviceName, sizeof(remAcInf.smartDeviceName)-1);
		}
		else
		{
				APP_LOG("UPNP: Device", LOG_ERR,"smart device name param is null"); 
		}


		szHomeId = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "HomeId");
		if (0x00 != szHomeId && (0x00 != strlen(szHomeId)))
				strncpy(remAcInf.homeId, szHomeId, sizeof(remAcInf.homeId)-1);

		/* proxy registration */
		if(!strcmp(szDeviceName, PLUGIN_REMOTE_REQ))
		{
				szMacAddress = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "MacAddr");
				if(0x0 != szMacAddress && 0x00 == strlen(szMacAddress))
				{
						APP_LOG("UPNP: Device", LOG_ERR,"mac parameter in proxy remote request is null"); 
						sendRemoteUpnpErrorResponse(pActionRequest, "mac parameter failure",  "FAIL");
						goto on_return;
				}
				/* check for hw info */
				if(!g_pxRemRegInf)
				{
						APP_LOG("UPNP: Device", LOG_DEBUG,"Failed in getting HW info of new plugin......");
						sendRemoteUpnpErrorResponse(pActionRequest, "hw info failure",  "FAIL");
						goto on_return;
				}
				else
				{
						/* check for mac, should be same in remote request and hw info received */
						if(strcmp(szMacAddress, g_pxRemRegInf->proxy_macAddress))
						{
								APP_LOG("UPNP: Device", LOG_DEBUG,"Share hardware info MacAddr and RemoteAccess MacAddr are different, Not sending remote req");
								sendRemoteUpnpErrorResponse(pActionRequest, "mac mismatch failure",  "FAIL");
								goto on_return;
						}
						else
						{
								APP_LOG("UPNP: Device", LOG_DEBUG,"Processing proxy registration request");
						}
				}
		}
		/* smart device key from App */
		else
		{
				szDevicePrivKey = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "smartprivateKey");
				if (0x00 != szDevicePrivKey && (0x00 != strlen(szDevicePrivKey)))
				{
						APP_LOG("UPNP: Device", LOG_HIDE,"smart device private key received in request: %s", szDevicePrivKey);
						strncpy(remAcInf.smartDevicePrivateKey, szDevicePrivKey, sizeof(remAcInf.smartDevicePrivateKey)-1);
				}
				else
				{
						APP_LOG("UPNP: Device", LOG_DEBUG,"smart device private key received in request is null");
				}
		}

		/* defer request for network reachability */
		pluginUsleep(2000000);

		/* create remote registration thread */	
		ret = createRemoteRegThread(remAcInf);
		if(ret != SUCCESS)
		{
				sendRemoteUpnpErrorResponse(pActionRequest, "Fw upd or reg under process, mem failure",  "FAIL");
				goto on_return;
		}

		/* wait here for response */
		pthread_mutex_lock(&g_remoteAccess_mutex);
		pthread_cond_wait(&g_remoteAccess_cond,&g_remoteAccess_mutex);
		pthread_mutex_unlock(&g_remoteAccess_mutex);

		if(pgPluginReRegInf != NULL)
		{
				/* success response */
				if(!strcmp(pgPluginReRegInf->statusCode, "S"))
				{
						APP_LOG("UPNPDevice", LOG_ERR,"Cloud Connection: AUTHENTICATED");
						pPlgReg = pgPluginReRegInf;
				}
				/* failure response */
				else
				{
						sendRemoteUpnpErrorResponse(pActionRequest, pgPluginReRegInf->resultCode,  pgPluginReRegInf->description);
						goto on_return;
				}
		}
		/* null response */
		else
		{
				sendRemoteUpnpErrorResponse(pActionRequest, NULL,  NULL);
				goto on_return;
		}

		/* send success response */
		sendRemoteUpnpSuccessResponse(pActionRequest, pPlgReg);


on_return:

		/* free registration response & proxy registration request & xml res */
		if (pgPluginReRegInf) {free(pgPluginReRegInf); pgPluginReRegInf = NULL;}
		if (g_pxRemRegInf) {free(g_pxRemRegInf); g_pxRemRegInf = NULL;}
		freeRemoteXMLRes(szHomeId, szMacAddress, szDeviceId, szDeviceName, szDevicePrivKey);
		/* reset remote request by App flag */
		g_isRemoteAccessByApp = 0;
		return UPNP_E_SUCCESS;
}




/**
 * SetIcon:
 * 	Callback to Get icon URL
 * 
 * 
 * *****************************************************************************************************************/
int SetFriendlyName(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
        //char *buf = (char*)malloc(sizeof(char)*256);
		//memset(buf, 0, sizeof(buf));
		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG,"Set friendly name: paramter failure"); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		char* szFriendlyName = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "FriendlyName");
        /*
		APP_LOG("UPNP: Device", LOG_DEBUG,"!!!!!!!!!!!!!!!! Set friendly name before modification:%s", szFriendlyName);
		replace_str(buf, szFriendlyName, "&", "&amp;");
        	szFriendlyName = realloc(szFriendlyName, 256);
		strncpy(szFriendlyName, buf, 256);
		APP_LOG("UPNP: Device", LOG_DEBUG,"######################### Set friendly name after modification:%s", szFriendlyName); 
              */
		if (0x00 == strlen(szFriendlyName))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetFriendlyName", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"status", "Parameters Error");
				APP_LOG("UPNP: Device", LOG_ERR,"Set friendly name: failure"); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetFriendlyName", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "FriendlyName", szFriendlyName);

		strncpy(g_szFriendlyName, szFriendlyName, sizeof(g_szFriendlyName)-1);
		APP_LOG("UPNP: Device", LOG_DEBUG,"Set friendly name: %s", szFriendlyName); 
		FreeXmlSource(szFriendlyName);

		SetBelkinParameter("FriendlyName", g_szFriendlyName);
		UpdateXML2Factory();
		/* Issue 2894 */
		AsyncSaveData();
		remoteAccessUpdateStatusTSParams (0xFF);

		return UPNP_E_SUCCESS;

}

/**
 * GetIcon:
 * 	Callback to Get icon URL
 * 
 * 
 * *****************************************************************************************************************/
int GetIcon(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szIconURL[MAX_FW_URL_LEN];
		memset(szIconURL, 0x00, sizeof(szIconURL));

		//-Return the icon path of the device
		if (strlen(g_server_ip) > 0x00)
		{		
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				snprintf(szIconURL, sizeof(szIconURL), "http://%s:%d/icon.jpg", g_server_ip, g_server_port);
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetIconURL", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "URL", szIconURL);
		}
		else
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;				
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetIconURL", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "URL", "");
		}

		APP_LOG("UPNP: Device", LOG_DEBUG,"GetIcon: %s", szIconURL); 

		return UPNP_E_SUCCESS;
}

/**
 * GetIconVersion:
 * 	Callback to Get Icon Version
 * 
 * 
 * *****************************************************************************************************************/
int GetIconVersion(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szIconVersion[SIZE_4B];

		memset(szIconVersion, 0, SIZE_4B);

		if (pActionRequest == 0x00 || pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG,"GetIconVersion : parameter failure"); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		snprintf(szIconVersion, sizeof(szIconVersion), "%d", gWebIconVersion);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		APP_LOG("UPNP: Device", LOG_DEBUG, "Icon version:%s", szIconVersion);

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetIconVersion", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "IconVersion", szIconVersion);  

		return UPNP_E_SUCCESS;
}

#ifdef WeMo_SMART_SETUP_V2
/**
 * GetInformation:
 * 	Callback to Get the device information in XML format 
 * 
 * 
 * *****************************************************************************************************************/
int GetInformation(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
    char szBuff[SIZE_1024B];
    int port = 0, state = 0, customizedState = 0;

    memset(szBuff, 0x0, sizeof(szBuff));

    port = UpnpGetServerPort();
    state = GetCurBinaryState();
#ifdef WeMo_SMART_SETUP
    if(g_customizedState)
	customizedState = DEVICE_CUSTOMIZED;
#endif

    snprintf(szBuff, sizeof(szBuff),"<Device><DeviceInformation><firmwareVersion>%s</firmwareVersion><iconVersion>%d</iconVersion><iconPort>%d</iconPort><macAddress>%s</macAddress><FriendlyName>%s</FriendlyName><binaryState>%d</binaryState><CustomizedState>%d</CustomizedState></DeviceInformation></Device>", g_szFirmwareVersion, gWebIconVersion, port, g_szWiFiMacAddress, g_szFriendlyName, state, customizedState);

    pActionRequest->ActionResult = NULL;
    pActionRequest->ErrCode = 0x00;

    APP_LOG("UPNP: Device", LOG_DEBUG, "Device information: %s", szBuff);

    UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetInformation", CtrleeDeviceServiceType[PLUGIN_E_DEVICEINFO_SERVICE], "Information", szBuff);

    return UPNP_E_SUCCESS;
}
#endif

/**
 * GetDeviceInformation:
 * 	Callback to Get the device information
 * 
 * 
 * *****************************************************************************************************************/
int GetDeviceInformation(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szBuff[MAX_BUF_LEN];
		int port = 0, state = 0;

		port = UpnpGetServerPort();

		state = GetCurBinaryState();

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		memset(szBuff, 0x0, MAX_BUF_LEN);
		snprintf(szBuff, MAX_BUF_LEN,"%s|%s|%d|%d|%d|%s", g_szWiFiMacAddress, g_szFirmwareVersion, gWebIconVersion, port, state, g_szFriendlyName);

		APP_LOG("UPNP: Device", LOG_HIDE, "DeviceInformation: %s", szBuff);

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetDeviceInformation", CtrleeDeviceServiceType[PLUGIN_E_DEVICEINFO_SERVICE], "DeviceInformation", szBuff);

		return UPNP_E_SUCCESS;
}

/**
 * GetLogFilePath:
 * 	Callback to Get log file URL
 * 
 * 
 * *****************************************************************************************************************/
int GetLogFilePath(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szLogFileURL[MAX_FW_URL_LEN];
		memset(szLogFileURL, 0x00, sizeof(szLogFileURL));

		//-Return the log file path of the device
		if (strlen(g_server_ip) > 0x00)
		{		
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				snprintf(szLogFileURL, sizeof(szLogFileURL), "http://%s:%d/PluginLogs.txt", g_server_ip, g_server_port);
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetLogFileURL", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "LOGURL", szLogFileURL);
		}
		else
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;				
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetLogFileURL", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "LOGURL", "");
		}

		APP_LOG("UPNP: Device", LOG_DEBUG,"Log File URL: %s", szLogFileURL); 

		return UPNP_E_SUCCESS;
}
ithread_t CloseApWaiting_thread = -1;

/**
 * GetWatchdogFile:
 * 	Callback to Get Watchdog Log file
 * 
 * 
 * *****************************************************************************************************************/
int GetWatchdogFile(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		if (pActionRequest == NULL) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;				
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetWatchdogFile", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "WDFile", "Sending");

		APP_LOG("UPNP: Device", LOG_DEBUG,"GetWatchdogFile: Sending the wdLogFile");

		APP_LOG("UPNP: Device",LOG_DEBUG, "***************LOG Thread created***************\n");
		pthread_attr_init(&wdLog_attr);
		pthread_attr_setdetachstate(&wdLog_attr, PTHREAD_CREATE_DETACHED);
		ithread_create(&logFile_thread, &wdLog_attr, uploadLogFileThread, NULL);


		return UPNP_E_SUCCESS;
}

/**
 * Get Signal Strength:
 * 	Callback to get device present signal Strength
 * 	
 * 
 * *****************************************************************************************************************/
int SignalStrengthGet(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szSignalSt[MAX_FW_URL_LEN];
		memset(szSignalSt, 0x00, sizeof(szSignalSt));

		APP_LOG("UPNP: Device", LOG_DEBUG, "%s, called", __FUNCTION__);

		if (pActionRequest == NULL) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "UPNP parameter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		/*Update signal strength*/
		if(!gSignalStrength)
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "Update signal strength!");
				chksignalstrength();
		}

		snprintf(szSignalSt, sizeof(szSignalSt), "%d", gSignalStrength);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetSignalStrength", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"SignalStrength", szSignalSt);


		APP_LOG("UPNP: Device", LOG_DEBUG,"Get Signal Strength: %s", szSignalSt); 
		return UPNP_E_SUCCESS;
}


/**
 * SetServerEnvironment:
 * 	Callback to Set Server Environment IP  
 * 
 * 
 * *****************************************************************************************************************/

int SetServerEnvironment(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char ServerEnvType[SIZE_8B] = {0};
		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG,"Set Server Environment: paramter failure"); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		char* szserverEnvIPaddr = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "ServerEnvironment");
		char* szturnserverEnvIPaddr = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "TurnServerEnvironment");
		char* szserverEnvType = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "ServerEnvironmentType");

		if (0x00 == strlen(szserverEnvIPaddr))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"ServerEnvironment", "Parameter Error");
				APP_LOG("UPNP: Device", LOG_ERR,"Set Server Environment: parameter error"); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		if (0x00 == strlen(szturnserverEnvIPaddr))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"TurnServerEnvironment", "Parameter Error");
				APP_LOG("UPNP: Device", LOG_ERR,"Set Turn Server Environment: parameter error"); 
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		if (0x00 == strlen(szserverEnvType))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = PLUGIN_ERROR_E_BASIC_EVENT;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"ServerEnvironmentType", "Parameter Error");
				APP_LOG("UPNP: Device", LOG_ERR,"Set Server Environment: parameter error");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "ServerEnvironment", "success");
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "TurnServerEnvironment", "success");
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "ServerEnvironmentType", "success");

		/* cleanup old environment & remote settings and destroy nat client sesstion*/
		APP_LOG("UPNP: Device actual", LOG_DEBUG,"server Environment not set \n");
		ExecuteReset(0x03);
		serverEnvIPaddrInit();

		strncpy(g_serverEnvIPaddr, szserverEnvIPaddr, sizeof(g_serverEnvIPaddr)-1);
		strncpy(g_turnServerEnvIPaddr, szturnserverEnvIPaddr, sizeof(g_turnServerEnvIPaddr)-1);
		g_ServerEnvType = (SERVERENV)atoi(szserverEnvType); 

		FreeXmlSource(szserverEnvIPaddr);
		FreeXmlSource(szserverEnvType);
		FreeXmlSource(szturnserverEnvIPaddr);

		snprintf(ServerEnvType, sizeof(ServerEnvType), "%d", g_ServerEnvType);
		SetBelkinParameter(CLOUD_SERVER_ENVIRONMENT, g_serverEnvIPaddr);
		SetBelkinParameter(CLOUD_SERVER_ENVIRONMENT_TYPE, ServerEnvType);
		SetBelkinParameter(CLOUD_TURNSERVER_ENVIRONMENT, g_turnServerEnvIPaddr);
		AsyncSaveData();
		APP_LOG("UPNP: Device set ENV", LOG_DEBUG,"Set server environment IP: %s \n Set turn server IP: %s \n Set server environment type: %d \n", g_serverEnvIPaddr, g_turnServerEnvIPaddr, g_ServerEnvType); 

		return UPNP_E_SUCCESS;

}

/**
 * GetServerEnvironment:
 * 	Callback to Get Server Environment IP
 * 	
 * 
 * *****************************************************************************************************************/
int GetServerEnvironment(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char ServerEnvType[SIZE_8B] = {0};
		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_DEBUG, "Get Server Environment: paramter failure");
				return PLUGIN_ERROR_E_BASIC_EVENT;
		}

		APP_LOG("UPNP: Device", LOG_HIDE, "Server Environment IP is: %s \n turn server IP is: %s \n server environment type: %d \n", g_serverEnvIPaddr, g_turnServerEnvIPaddr, g_ServerEnvType);

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"ServerEnvironment", g_serverEnvIPaddr);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"TurnServerEnvironment", g_turnServerEnvIPaddr);

		snprintf(ServerEnvType, sizeof(ServerEnvType), "%d", g_ServerEnvType);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetServerEnvironment", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"ServerEnvironmentType", ServerEnvType);


		return UPNP_E_SUCCESS;
}

/**
 * 
 * 
 * 
 * 
 * 
 ******************************************/
#define	MAX_AP_CLOSE_WAITING_TIME	10
void *CloseApWaitingThread(void *args)
{
		//- Close Setup here

		int k = 0x00;
		int isSetup = 0x00;
		char routerMac[MAX_MAC_LEN];
		char routerssid[MAX_ESSID_LEN];
                tu_set_my_thread_name( __FUNCTION__ );
#ifdef PRODUCT_WeMo_Insight
		//InitOnSetup();
		char SetUpCompleteTS[SIZE_32B];
		memset(SetUpCompleteTS, 0, sizeof(SetUpCompleteTS));
		if(!g_SetUpCompleteTS)
		{
				g_SetUpCompleteTS = GetUTCTime();
				sprintf(SetUpCompleteTS, "%lu", g_SetUpCompleteTS);
				SetBelkinParameter(SETUP_COMPLETE_TS, SetUpCompleteTS);
				AsyncSaveData();
		}
		APP_LOG("ITC: network", LOG_ERR,"UPnP  updated on setup complete g_SetUpCompleteTS---%lu, SetUpCompleteTS--------%s:", g_SetUpCompleteTS, SetUpCompleteTS);
#endif
		//- Stop WiFi pairing thread if necessary
		StopWiFiPairingTask();
		memset(routerMac, 0, sizeof(routerMac));
		memset(routerssid, 0, sizeof(routerssid));

		while (k++ < MAX_AP_CLOSE_WAITING_TIME)
		{
				ip_address = NULL;
				ip_address = wifiGetIP(INTERFACE_CLIENT);

				if ((0x01 == getCurrentClientState()) || (0x03 == getCurrentClientState()))
				{
						isSetup = 0x01;
						break;
				}
				else
				{
						APP_LOG("UPNP: setup", LOG_DEBUG, "###### Network not connected yet, how this could be ?########"); 
						APP_LOG("UPNP: setup", LOG_CRIT, "Network not connected yet, how this could be ?"); 
				}

				pluginUsleep(1000000);
		}
		if (isSetup)
		{
#ifdef WeMo_INSTACONNECT
				char password[PASSWORD_MAX_LEN];
				memset(password, 0, sizeof(password));
				if(strcmp(gWiFiParams.AuthMode,"OPEN"))
				{
						if(SUCCESS != decryptPassword(gWiFiParams.Key, password))
						{
								APP_LOG("WiFiApp", LOG_DEBUG,"decrypt Password failed...not setting up bridge control");
						}
						else
						{
								APP_LOG("WiFiApp", LOG_HIDE,"decrypt passwd: %s success...setting up bridge control", password);
								/* configure Bridge Ap */
								configureBridgeAp(gWiFiParams.SSID, gWiFiParams.AuthMode, gWiFiParams.EncrypType, password, gWiFiParams.channel); 
						}
				}
				else
				{
						strncpy(password,"NOTHING", sizeof(password)-1);
						APP_LOG("WiFiApp", LOG_HIDE,"passwd: %s...setting up bridge control", password);
						/* configure Bridge Ap */
						configureBridgeAp(gWiFiParams.SSID, gWiFiParams.AuthMode, gWiFiParams.EncrypType, password, gWiFiParams.channel); 
				}
				//g_usualSetup= 0x00;    //unset
#endif
				ControlleeDeviceStop();
				system("ifconfig ra0 down");
				g_ra0DownFlag = 1; //RA0 interface is Down
				ip_address = NULL;

				ip_address = wifiGetIP(INTERFACE_CLIENT);

				if (ip_address && (0x00 != strcmp(ip_address, DEFAULT_INVALID_IP_ADDRESS)))
				{
						//-Start new UPnP in client AP new address
						APP_LOG("UPNP: Device", LOG_HIDE,"start new UPnP session on %d: %s", g_eDeviceType, ip_address); 
						UpdateUPnPNetworkMode(UPNP_INTERNET_MODE);
						//gautam: update the Insight and LS Makefile to copy Insightsetup.xml and Lightsetup.xml in /sbin/web/ as setup.xml 
						ControlleeDeviceStart(ip_address, 0x00, "setup.xml", "/tmp/Belkin_settings");
						getRouterEssidMac (routerssid, routerMac, INTERFACE_CLIENT);
						if(strlen(gGatewayMAC) > 0x0)
						{
								memset(routerMac, 0, sizeof(routerMac));
								strncpy(routerMac, gGatewayMAC, sizeof(routerMac)-1);
						}

						/* Range Extender fix: Irrespective of device type, start the control point */
						StartPluginCtrlPoint(ip_address, 0x00);
						EnableContrlPointRediscover(TRUE);

						if (g_eDeviceType == DEVICE_SENSOR)
						{
								if(0x01 == atoi(g_szRestoreState))
								{
#ifdef WeMo_INSTACONNECT
										if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
												createAutoRegThread();
								}
								else if(0x00 == atoi(g_szRestoreState))
								{
										if((strlen(g_szHomeId) == 0x0) && (strlen(g_szPluginPrivatekey) == 0x0))
										{	
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is not Enabled... sensor");
												if(strlen(gGatewayMAC) > 0x0)
												{
														SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
														memset(g_routerMac, 0x0, sizeof(g_routerMac));
														strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
												}
												SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
												memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
												strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
										}
										else
										{
												if ( (strlen(g_routerMac) == 0x00) && (strlen(g_routerSsid) == 0x00) )
												{
														if(strlen(gGatewayMAC) > 0x0)
														{
																SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
																memset(g_routerMac, 0x0, sizeof(g_routerMac));
																strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
														}
														SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
														memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
														strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
												}
												else if( (strcmp(g_routerMac, routerMac) != 0) && (g_routerSsid!=NULL && strlen (g_routerSsid) > 0) )
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "router is not same.. sensor");
#ifdef WeMo_INSTACONNECT
														if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
																createAutoRegThread();
												}
												else
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is Enabled and router is same... sensor\n");
														if(strlen(gGatewayMAC) > 0x0)
														{
																SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
																memset(g_routerMac, 0x0, sizeof(g_routerMac));
																strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
														}
														SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
														memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
														strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
												}
										}	
								}
						}
						if (g_eDeviceType == DEVICE_SOCKET)
						{
								if(0x01 == atoi(g_szRestoreState))
								{
#ifdef WeMo_INSTACONNECT
										if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
												createAutoRegThread();
								}
								else if(0x00 == atoi(g_szRestoreState))
								{
										if((strlen(g_szHomeId) == 0x0) && (strlen(g_szPluginPrivatekey) == 0x0))
										{	
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is not Enabled.. opening socket control point");
												if(strlen(gGatewayMAC) > 0x0)
												{
														SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
														memset(g_routerMac, 0x0, sizeof(g_routerMac));
														strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
												}
												SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
												memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
												strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
										}
										else
										{
												if ( (strlen(g_routerMac) == 0x00) && (strlen(g_routerSsid) == 0x00) )
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is Enabled, having no router info.. not opening socket control point");
														if(strlen(gGatewayMAC) > 0x0)
														{
																SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
																memset(g_routerMac, 0x0, sizeof(g_routerMac));
																strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
														}
														SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
														memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
														strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
												}
												else if( (strcmp(g_routerMac, routerMac) != 0) && (g_routerSsid!=NULL && strlen (g_routerSsid) > 0) )
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is Enabled but router is not same, opening socket control point");
#ifdef WeMo_INSTACONNECT
														if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
																createAutoRegThread();
												}
												else
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is Enabled and router is same.. not opening socket control point");
														if(strlen(gGatewayMAC) > 0x0)
														{
																SetBelkinParameter (WIFI_ROUTER_MAC,routerMac);
																memset(g_routerMac, 0x0, sizeof(g_routerMac));
																strncpy(g_routerMac, routerMac, sizeof(g_routerMac)-1);
														}
														SetBelkinParameter (WIFI_ROUTER_SSID,routerssid);
														memset(g_routerSsid, 0x0, sizeof(g_routerSsid));
														strncpy(g_routerSsid, routerssid, sizeof(g_routerSsid)-1);
												}
										}
								}
						}
				}
				else
				{
						APP_LOG("UPNP: Device", LOG_ERR,"IP address is not correct"); 
						CloseApWaiting_thread = -1;
						return (void *)0x01;
				}
		}
		else
		{
				APP_LOG("UPNP: Device", LOG_ERR, "Network is not connected, setup not closed"); 	
		}

		AsyncSaveData();
		CloseApWaiting_thread = -1;
		return NULL;
}


/* CloseSetup:
 * 	Close the setup so AP will be dropp and UPnP FINISH and restart again
 * 
 * 
 * 
 * 
 * 
 * 
 *********************************************************************************************************************/
int CloseSetup(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{

		APP_LOG("UPNP: Device", LOG_DEBUG,"%s", __FUNCTION__); 
		gAppCalledCloseAp=1;

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "CloseSetup", CtrleeDeviceServiceType[PLUGIN_E_SETUP_SERVICE], "status", "success");

		ithread_create(&CloseApWaiting_thread, NULL, CloseApWaitingThread, NULL);
		APP_LOG("UPNP: Device", LOG_DEBUG, "AP closing in %d seconds .......", MAX_AP_CLOSE_WAITING_TIME); 

		return UPNP_E_SUCCESS;
}

void setCurrFWUpdateState(int state)
{
		osUtilsGetLock(&gFWUpdateStateLock);
		currFWUpdateState = state;
		osUtilsReleaseLock(&gFWUpdateStateLock);
		APP_LOG("UPNP",LOG_DEBUG, "currFWUpdateState updated: %d", currFWUpdateState);
}

int getCurrFWUpdateState(void)
{
		int state;
		osUtilsGetLock(&gFWUpdateStateLock);
		state = currFWUpdateState;
		osUtilsReleaseLock(&gFWUpdateStateLock);
		APP_LOG("UPNP",LOG_DEBUG, "currFWUpdateState updated: %d", state);
		return state;
}



/* UpdateFirmware:
 * 	update firmware notification from APP
 * 	"NewFirmwareVersion"[szVersion]
 *	       "ReleaseDate"[szReleaseDate]
 *                  "URL"[szURL]
 *	       "Signature"[szSignature]
 * 
 ********************************************************************************/
int UpdateFirmware(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int state = -1;
		FirmwareUpdateInfo fwUpdInf;

		APP_LOG("UPNP: Device", LOG_CRIT,"%s", __FUNCTION__); 

		//-Read out all paramters from APP
		char* szNewFirmwareVersion = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "NewFirmwareVersion");
		char* szReleaseDate = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "ReleaseDate");
		char* szURL = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "URL");
		char* szSignature = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Signature");
		char* szDownloadStartTime = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DownloadStartTime");
		char* szWithUnsignedImage = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "WithUnsignedImage");

		if((0x00 == szURL) || (0x00 == strlen(szURL))) 
		{
				APP_LOG("Firmware Update",LOG_ERR, "URL empty, command not executed");
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = UPNP_E_INVALID_PARAM;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "UpdateFirmware", 
								CtrleeDeviceServiceType[PLUGIN_E_FIRMWARE_SERVICE], "status", "failure");
				goto on_return;
		}

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = UPNP_E_SUCCESS;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "UpdateFirmware", 
						CtrleeDeviceServiceType[PLUGIN_E_FIRMWARE_SERVICE], "status", "success");

		state = getCurrFWUpdateState();
		if( (state == atoi(FM_STATUS_DOWNLOADING)) || (state == atoi(FM_STATUS_DOWNLOAD_SUCCESS)) || (state == atoi(FM_STATUS_UPDATE_STARTING)) )
		{
				APP_LOG("UPnPApp",LOG_ERR, "************Firmware Update Already in Progress...");
				goto on_return;
		}

		memset(&fwUpdInf, 0x00, sizeof(FirmwareUpdateInfo));

		if(szDownloadStartTime) 
				fwUpdInf.dwldStartTime = atoi(szDownloadStartTime)*60; //Seconds

		if((0x00 != szWithUnsignedImage) && (0x00 != strlen(szWithUnsignedImage))) 
				fwUpdInf.withUnsignedImage = atoi(szWithUnsignedImage); // 1 = using unsigned image

		strncpy(fwUpdInf.firmwareURL, szURL, sizeof(fwUpdInf.firmwareURL)-1);
		StartFirmwareUpdate(fwUpdInf);

on_return:
		FreeXmlSource(szNewFirmwareVersion);
		FreeXmlSource(szReleaseDate);
		FreeXmlSource(szURL);
		FreeXmlSource(szSignature);
		FreeXmlSource(szDownloadStartTime);

		return UPNP_E_SUCCESS;
}

int DownLoadFirmware(const char *FirmwareURL, int deferWdLogging, int withUnsigned)
{
		int state=0;
		char firmwareURL[MAX_FW_URL_LEN];
		int retVal = FAILURE;

		SetWiFiLED(0x00);
		FirmwareUpdateStatusNotify(FM_STATUS_DOWNLOADING);
		setCurrFWUpdateState(atoi(FM_STATUS_DOWNLOADING));
		RemoteFirmwareUpdateStatusNotify();
		state = getCurrFWUpdateState();
		APP_LOG("UPNP: Device", LOG_DEBUG,"******** current firmware update state is:%d type:%d", state, withUnsigned);

		memset(firmwareURL, 0x0, sizeof(firmwareURL));
		strncpy(firmwareURL, FirmwareURL, sizeof(firmwareURL)-1);
		strncat(firmwareURL, "?mac=", sizeof(firmwareURL)-strlen(firmwareURL)-1);
		strncat(firmwareURL, g_szWiFiMacAddress, sizeof(firmwareURL)-strlen(firmwareURL)-1);

		if(!deferWdLogging) {//Log the event in the WDLogFile only once
				APP_LOG("UPNP: Device", LOG_CRIT, "Starting firmware download ......[%s]", firmwareURL);
		} else {
				APP_LOG("UPNP: Device", LOG_DEBUG, "Starting firmware download ......[%s]", firmwareURL);
		}

		retVal = webAppFileDownload(firmwareURL, "/tmp/firmware.bin.gpg");
		if (retVal != SUCCESS) {
				APP_LOG("Firmware", LOG_ERR,"Download firmware image failed");
		}else {
				APP_LOG("Firmware", LOG_CRIT,"Downloaded firmware image from location %s successfully\n", firmwareURL);
		}

		return retVal;
}

/* GetFirmwareVersion:
 * 	Firmware version request from APP
 * 
 ********************************************************************************/
int GetFirmwareVersion(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szSkuNumber[SIZE_32B];
		char szResponse[SIZE_128B];
		memset(szSkuNumber, 0x00, sizeof(szSkuNumber));
		memset(szResponse, 0x00, sizeof(szResponse));

		char* szPreviousSkuNo   = GetBelkinParameter("SkuNo");

		if (0x00 == szPreviousSkuNo || 0x00 == strlen(szPreviousSkuNo))
		{
				snprintf(szSkuNumber, sizeof(szSkuNumber), "%s", DEFAULT_SKU_NO);
		}
		else
		{
				snprintf(szSkuNumber, sizeof(szSkuNumber), "%s", szPreviousSkuNo);
		}

		snprintf(szResponse, sizeof(szResponse), "FirmwareVersion:%s|SkuNo:%s", g_szBuiltFirmwareVersion, szSkuNumber);

		APP_LOG("UPNP: Device", LOG_DEBUG, "Firmware:%s", szResponse);

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

#ifdef PRODUCT_WeMo_NetCam
		if (szPreviousSkuNo)
				free(szPreviousSkuNo);
#endif

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetFirmwareVersion", CtrleeDeviceServiceType[PLUGIN_E_FIRMWARE_SERVICE], 
						"FirmwareVersion", szResponse);



		return UPNP_E_SUCCESS;
}
//-----------------TODO: tmp here ----------------------------------

void FirmwareUpdate_AsyncUpdateNotify()
{
		pMessage msg = createMessage(META_FIRMWARE_UPDATE, 0x00, 0x00);
		SendMessage2App(msg);
}

void *updateMonitorCheckTh (void *args)
{
		APP_LOG("UPnPApp",LOG_ERR, "************Firmware Update Check Monitor thread Created");
		pluginUsleep(180000000);	// 3 mins
		APP_LOG("UPnPApp",LOG_ALERT, "************Firmware Update Check Monitor thread rebooting system...");
		if ((gpluginRemAccessEnable) && (gpluginNatInitialized)) {
				invokeNatDestroy();
		}
		system("reboot");
		return NULL;
}

#ifndef _OPENWRT_
int dumpFlashVariables(void)
{
		char *val;
		int num_vars=0, i=0,ret=0;
		FILE* pfWriteStream = 0x00;
		char buff[NVRAM_SETTING_BUF_SIZE];
		char tmp[SIZE_128B];

		memset(buff, 0, NVRAM_SETTING_BUF_SIZE);
		snprintf(buff, sizeof(buff), "nvram reset\n");

		num_vars = sizeof(g_apu8NvramVars)/sizeof(char *);

		for(i=0;i<num_vars;i++)
		{
				val = GetBelkinParameter(g_apu8NvramVars[i]);
				if(val && strlen(val))
				{
						memset(tmp, 0, sizeof(tmp));
						if(!(strcmp(g_apu8NvramVars[i], "cwf_serial_number")))
								snprintf(tmp, sizeof(tmp), "nvram_set %s \"%s\"\n", "SerialNumber", val);	
						else if(!(strcmp(g_apu8NvramVars[i], "ntp_server1")))
								snprintf(tmp, sizeof(tmp), "nvram_set %s \"%s\"\n", "NTPServerIP", val);	
						else
								snprintf(tmp, sizeof(tmp), "nvram_set %s \"%s\"\n", g_apu8NvramVars[i], val);	

						strncat(buff, tmp, sizeof(buff)-strlen(buff)-1);	
				}
		}

		if(strlen(buff))
		{
				pfWriteStream = fopen(NVRAM_FILE_NAME, "w");

				if (NULL == pfWriteStream)
				{
						APP_LOG("UPnPApp",LOG_ERR, "Open file %s for writing failed...", NVRAM_FILE_NAME);
						return -1;
				}


				ret = fwrite(buff, 1, strlen(buff), pfWriteStream);
				APP_LOG("UPnPApp",LOG_DEBUG, "Written %d bytes of data from %d bytes input", ret, strlen(buff));

				fclose(pfWriteStream);
		}

		return 0;
}
#endif

void *firmwareUpdateTh(void *args)
{
		int status = FAILURE, retryFlag = 0, state = 0, rect = 0;
		char *fwUpStr = NULL;
		FirmwareUpdateInfo *pfwUpdInf = NULL;
		char FirmwareURL[MAX_FW_URL_LEN];
		int deferWdLogging = 0, withUnsignedImage = 0;
                tu_set_my_thread_name( __FUNCTION__ );
		memset(FirmwareURL, 0, sizeof(FirmwareURL));

		if(args)
		{
				pfwUpdInf = (FirmwareUpdateInfo *)args;
				withUnsignedImage = pfwUpdInf->withUnsignedImage;
				strncpy(FirmwareURL, pfwUpdInf->firmwareURL, sizeof(FirmwareURL)-1);
				free(args);
				args = NULL;
		}

		APP_LOG("UPnPApp",LOG_ERR, "**** Firmware Update thread Created with URL: %s ****", FirmwareURL);

		do {
				gStopDownloadFW = 0;	//reset stop FW download flag used by httpwrapper curl
				status  = DownLoadFirmware(FirmwareURL, deferWdLogging, withUnsignedImage);
				if (status == SUCCESS)
				{
						setCurrFWUpdateState(atoi(FM_STATUS_DOWNLOAD_SUCCESS));
						state = getCurrFWUpdateState();
						APP_LOG("UPNP: Device", LOG_DEBUG,"******** current firmware update state is:%d", state); 
						FirmwareUpdate_AsyncUpdateNotify();
						RemoteFirmwareUpdateStatusNotify();
						setCurrFWUpdateState(atoi(FM_STATUS_UPDATE_STARTING));

						APP_LOG("UPNP: Device", LOG_DEBUG,"Unsetting the firmwareUpdate flag"); 
						RemoteFirmwareUpdateStatusNotify();
						pluginUsleep(1000000);
						UnSetBelkinParameter("FirmwareUpURL");
						AsyncSaveData();
						/* wait for remote notification thread to send out the notifications for download success & upgrade start */
						pluginUsleep(6*1000000);
						APP_LOG("UPNP: Device", LOG_DEBUG,"firmwareUpdate continuing after sleep");

#if defined(PRODUCT_WeMo_Insight)		
						Update30MinDataOnFlash();
#endif

#ifndef _OPENWRT_
						dumpFlashVariables(); /* to use in contrast with gemtek and openwrt */
#endif

						state = getCurrFWUpdateState();
						APP_LOG("UPNP: Device", LOG_DEBUG,"******** current firmware update state is:%d", state); 

						if ((gpluginRemAccessEnable) && (gpluginNatInitialized)) {
								invokeNatDestroy();
						}
#if defined(PRODUCT_WeMo_Baby)		
						EnableWatchDog(0,WD_DEFAULT_TRIG_TIME);
						APP_LOG("UPNP: Device", LOG_DEBUG,"******** disableWatchDog:\n");
						system("killall -9 monitor_launcher");
						APP_LOG("UPNP: Device", LOG_DEBUG,"******** killall -9 monitor_launcher:\n");
						system("killall -9 monitor");
						APP_LOG("UPNP: Device", LOG_DEBUG,"******** killall -9 monitor:\n");
#endif

#ifdef __OLDFWAPI__
						rect = Firmware_Update("/tmp/firmware.bin.gpg");
#else
						APP_LOG("UPNP: Device", LOG_DEBUG,"******** current firmware update state is:%d-%d", state, withUnsignedImage); 
						if (withUnsignedImage) {
								rect = New_Firmware_Update("/tmp/firmware.bin.gpg", 0);
						}else {
								rect = New_Firmware_Update("/tmp/firmware.bin.gpg", 1);
						}
#endif

						if (0x00 != rect)
						{
								//- In case Gemtek API called failure, though it would not occur And - If failure since the UPnP already stop, should reboot
								//SetBelkinParameter("FirmwareUpURL", FirmwareURL);
								//AsyncSaveData();
								system("rm -f /tmp/firmware.bin.gpg");
								system("rm -f /tmp/firmware.img");
								APP_LOG("UPNP: Device", LOG_ALERT, "Gemtek API Firmware_Update called failure");

								if ((gpluginRemAccessEnable) && (gpluginNatInitialized)) {
										invokeNatDestroy();
								}
								pluginUsleep(120000000);	// 2 mins
								firmwareUpThread = -1;
								system("reboot");
						}
				}
				else
				{
						deferWdLogging++;
						setCurrFWUpdateState(atoi(FM_STATUS_DOWNLOAD_UNSUCCESS));
						state = getCurrFWUpdateState();
						APP_LOG("UPNP: Device", LOG_DEBUG,"Current firmware update state is:%d", state); 
						APP_LOG("UPNP: Device", LOG_ERR, "Firmware download failure");
#if defined(PRODUCT_WeMo_Baby) || defined(PRODUCT_WeMo_Streaming)
						SetWiFiGPIOLED(0x04);
#else
						SetWiFiLED(0x04);
#endif
						FirmwareUpdateStatusNotify(FM_STATUS_DOWNLOAD_UNSUCCESS);
						RemoteFirmwareUpdateStatusNotify();
				}

				fwUpStr = GetBelkinParameter("FirmwareUpURL");
				if(fwUpStr && strlen(fwUpStr) != 0)
						retryFlag = 1;
				else
						retryFlag = 0;

		} while(retryFlag);

		firmwareUpThread=-1;
		return NULL;
}

extern void SendEvent2ControlledDevices();


void NoMotionSensorInd()
{
		if (!IsUPnPNetworkMode())
				return;
		LocalBinaryStateNotify(SENSORING_OFF);
}



void MotionSensorInd()
{
		if (!IsUPnPNetworkMode())
				return;

		int state = 0x00;

		lockRule();
		state = g_isSensorRuleActivated;
		unlockRule();

		if (state)
		{
				SendEvent2ControlledDevices();
		}
		else
		{
				APP_LOG("UPNP: Rule", LOG_DEBUG, "Sensor rule OFF, no controll on switch");
		}

		LocalBinaryStateNotify(SENSORING_ON);


}


//---------- Button Status change notify -------
void LocalUserActionNotify(int curState)
{
		return;
		if (device_handle == -1)
		{
				return;
		}

		if (!IsUPnPNetworkMode())
		{
				//- Not report since not on router or internet
				return;
		}

		char* szCurState[1];
		szCurState[0x00] = (char*)malloc(SIZE_2B);
		memset(szCurState[0x00], 0x00, SIZE_2B);
		snprintf(szCurState[0x00], SIZE_2B, "%d", curState);

		char* paramters[] = {"UserAction"} ;

		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)paramters, (const char **)szCurState, 0x01);

		APP_LOG("UPNP: Device", LOG_DEBUG, "Notification: UserAction: state: %d", curState);

		free(szCurState[0x00]);

}


//---------- Button Status change notify -------
void LocalBinaryStateNotify(int curState)
{
#if _OPENWRT_
		char sysString[SIZE_128B];
		int bootArgsLen = 0;
#endif
		if (device_handle == -1)
		{
				return;
		}

		if (!IsUPnPNetworkMode())
		{
				//- Not report since not on router or internet
				APP_LOG("UPNP", LOG_DEBUG, "Notification:BinaryState: Not in home network, ignored");
				return;
		}

		char* szCurState[1];
#ifdef PRODUCT_WeMo_Insight
		if ((0x00 == curState) || (0x01 == curState) || (0x08 == curState))
		{
				StateChangeTimeStamp(curState);
		}
		szCurState[0x00] = (char*)malloc(SIZE_100B);
		memset(szCurState[0x00], 0x00, SIZE_100B);
		if (g_InitialMonthlyEstKWH){
				snprintf(szCurState[0x00], SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%d", 
								curState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_InitialMonthlyEstKWH);
		}
		else{
				snprintf(szCurState[0x00], SIZE_100B, "%d|%u|%u|%u|%u|%u|%u|%u|%u|%0.f",
								curState,g_StateChangeTS,g_ONFor,g_TodayONTimeTS ,g_TotalONTime14Days,g_HrsConnected,g_AvgPowerON,g_PowerNow,g_AccumulatedWattMinute,g_KWH14Days);
		}
		APP_LOG("UPNP", LOG_DEBUG, "Local Binary State Insight Parameters: %s", szCurState[0x00]);
#else
		szCurState[0x00] = (char*)malloc(SIZE_2B);
		memset(szCurState[0x00], 0x00, SIZE_2B);
		snprintf(szCurState[0x00], SIZE_2B, "%d", curState);
#endif
		char* paramters[] = {"BinaryState"} ;

		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)paramters, (const char **)szCurState, 0x01);


		APP_LOG("UPNP", LOG_DEBUG, "Notification:BinaryState:state: %d", curState);

		/*
		 * Sets the u-boot env variable 'boot_A_args'. This variable is parsed in the kernel
		 * to fetch the state of the variable and take action accordingly.
		 *
		 * Will work only on the OPENWRT boards.
		 * Gemtek boards follow different procedure to handle the scenario.
		 *
		 * Done for the Story: 2187, To restore the state of the switch before power failure
		 */
#if _OPENWRT_
		memset(sysString, '\0', SIZE_128B);
		bootArgsLen = strlen(g_szBootArgs);
		if(bootArgsLen) {
				if(curState) {
						snprintf(sysString, sizeof(sysString), "fw_setenv boot_A_args %s switchStatus=1", g_szBootArgs);
						system(sysString);
				}
				else {
						snprintf(sysString, sizeof(sysString), "fw_setenv boot_A_args %s switchStatus=0", g_szBootArgs);
						system(sysString);
				}
		}
#endif

		free(szCurState[0x00]);

		if ((0x00 == curState) || (0x01 == curState) || (0x08 == curState))
		{
				//- push to cloud only ON/OFF
				remoteAccessUpdateStatusTSParams (curState);
		}
}

#ifdef PRODUCT_WeMo_CrockPot

void crockPotStatusChange(int mode, int time)
{
		LocalCrockpotStateNotify(mode, time);
		return;
}

/*---------- Crockpot Status change notify -------*/
void LocalCrockpotStateNotify(int cpMode, int cpCookTime)
{
		int ret=-1;
		int tempMode=0;
		if (!IsUPnPNetworkMode())
		{
				//- Not report since not on router or internet
				APP_LOG("UPNP", LOG_DEBUG, "Notification:Crockpot State: Not in home network, ignored");
				return;
		}

		char* szCurState[SIZE_2B];
		szCurState[0] = (char*)malloc(sizeof(cpMode));
		szCurState[1] = (char*)malloc(sizeof(cpCookTime));

		memset(szCurState[0], 0x00, sizeof(cpMode));
		memset(szCurState[0], 0x00, sizeof(cpCookTime));

		tempMode = convertModeValue(cpMode,FROM_CROCKPOT); 

		snprintf(szCurState[0], sizeof(cpMode), "%d", tempMode);
		snprintf(szCurState[1], sizeof(cpCookTime), "%d", cpCookTime);

		char* paramters[] = {"mode","time"} ;

		ret = UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)paramters, (const char **)szCurState, 0x02);
		if(UPNP_E_SUCCESS != ret)
		{
				APP_LOG("UPNP", LOG_DEBUG, "########## UPnP Notification ERROR #########");
		}
		else {
				APP_LOG("UPNP", LOG_DEBUG, "Notification:- mode is:: %s, ##### time is:: %s", szCurState[0],szCurState[1]);
		}

		free(szCurState[0]);
		free(szCurState[1]);
		/* Push to the cloud */
		{
				remoteAccessUpdateStatusTSParams(1);
		}
		return;
}
#endif /* #ifdef PRODUCT_WeMo_CrockPot */

void RuleOverrideNotify(int action, const char* szUDN, int startTime, int startAction)
{

		if (0x00 == szUDN)
				return;

		APP_LOG("UPNP: Device", LOG_DEBUG,"Not sending Override notifications for action: %d", action);
		return;

}



//-------------------------------------------------------- Rule --------------------------------------------
//----------------------------------------- Rules related -------------------------------------------------
/*
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 **********************************************************/
int AddRule(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);

		char* szName 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Name");
		char* szType		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Type");
		char* szIsEnable 	= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "IsEnable");

		char* szMon 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Mon");
		char* szTues 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Tues");
		char* szWed 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Wed");
		char* szThurs 	= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Thurs");
		char* szFri 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Fri");
		char* szSat 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Sat");
		char* szSun 		= Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "Sun");

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "AddRule", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], "status", "success");  

		FreeXmlSource(szName);
		FreeXmlSource(szType);
		FreeXmlSource(szIsEnable);
		FreeXmlSource(szMon);
		FreeXmlSource(szTues);
		FreeXmlSource(szWed);
		FreeXmlSource(szThurs);
		FreeXmlSource(szFri);
		FreeXmlSource(szSat);
		FreeXmlSource(szSun);

		return UPNP_E_SUCCESS;
}

int EditRule(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "EditRule", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], "status", "success");  
		return UPNP_E_SUCCESS;
}

int RemoveRule(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__); 
		return UPNP_E_SUCCESS;
}

int EnableRule(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);  
		return UPNP_E_SUCCESS;
}
int DisableRule(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		return UPNP_E_SUCCESS;
}
int GetRules(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "EditRule", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], "GetRules", "success");    
		return UPNP_E_SUCCESS;
}

#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

int SetRuleID(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int retVal=UPNP_E_SUCCESS;
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);
		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "SetRuleID: command paramter invalid");
				return UPNP_E_INVALID_ARGUMENT;
		}
		char* RuleID = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "RuleID");
		char* RuleMsg = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "RuleMsg");
		char* RuleFreq = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "RuleFreq");
		if (0x00 == RuleID || 0x00 == strlen(RuleID))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNPDevice", LOG_DEBUG, "SetRuleID: No RuleID Value");
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleID", "Error");  

				retVal=UPNP_E_INVALID_ARGUMENT;
		}
		if (0x00 == RuleMsg || 0x00 == strlen(RuleMsg))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNPDevice", LOG_DEBUG, "SetRuleID: No RuleMsg Value");
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleMsg", "Error");  

				retVal=UPNP_E_INVALID_ARGUMENT;
		}
		if(RuleMsg[0] == '\''){
				APP_LOG("UPNPDevice", LOG_DEBUG, "RuleMSG value is %s", RuleMsg);
		}
		else {
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNPDevice", LOG_ERR, "Rule message in wrong format");
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleMsg", "Error");  
				retVal = UPNP_E_INVALID_ARGUMENT;	
		}
		if (0x00 == RuleFreq || 0x00 == strlen(RuleFreq))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNPDevice", LOG_DEBUG, "SetRuleID: No RuleFreq Value");
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleFreq", "Error");  

				retVal=UPNP_E_INVALID_ARGUMENT;
		}
		if(UPNP_E_SUCCESS == retVal)
		{
				lockRule();
				retVal = UpdateAPNSDB(RuleID,RuleMsg,RuleFreq,0);
				unlockRule();
				if(!retVal){
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"RuleID" , RuleID);  
				}
				else{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x01;
						APP_LOG("UPNPDevice", LOG_DEBUG, "SetRuleID: No RuleID Value");
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleID", "Error");  
				}
		}
		FreeXmlSource(RuleID);
		FreeXmlSource(RuleMsg);
		FreeXmlSource(RuleFreq);
		return retVal;
}
#ifdef PRODUCT_WeMo_Insight
void DeleteAlreadyExecutedRuleID(unsigned int APNSRuleId)
{
		int loopcntr=0;
		for(loopcntr=0;loopcntr < MAX_APNS_SUPPORTED; loopcntr++)
		{
				if(InsightActive[loopcntr].RuleId == APNSRuleId)
				{
						InsightActive[loopcntr].RuleId = 0;
						InsightActive[loopcntr].isActive = 0;
				}
		}
}
#endif
int DeleteRuleID(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int retVal=UPNP_E_SUCCESS;
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);
		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "DeleteRuleID: command paramter invalid");
				return UPNP_E_INVALID_ARGUMENT;
		}
		char* RuleID = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "RuleID");
		if (0x00 == RuleID || 0x00 == strlen(RuleID))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNPDevice", LOG_DEBUG, "DeleteRuleID: No RuleID Value");
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "DeleteRuleID", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleID", "Error");  

				retVal=UPNP_E_INVALID_ARGUMENT;
		}
		if(UPNP_E_SUCCESS == retVal)
		{
				lockRule();
				retVal = UpdateAPNSDB(RuleID,0,0,1);
				unlockRule();
				if(!retVal){
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"RuleID" , RuleID);  
				}
				else{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x01;
						APP_LOG("UPNPDevice", LOG_DEBUG, "DeleteRuleID: No RuleID Value");
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRuleID", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleID", "Error");  
				}
		}
#ifdef PRODUCT_WeMo_Insight
		APP_LOG("UPNPDevice", LOG_DEBUG, "DeleteRuleID: Deleteing RuleID from Execution List: %d",atoi(RuleID));
		DeleteAlreadyExecutedRuleID(atoi(RuleID));
#endif
		FreeXmlSource(RuleID);
		return retVal;
}
int UpdateAPNSDB(char* RuleID,char* RuleMSG, char* RuleFreq,int Action)
{
		int retVal = PLUGIN_SUCCESS;
		int s32NoOfRows=0,s32NoOfCols=0;
		char **s8ResultArray=NULL;
		char query[MAX_DBQUERY_LEN];
		char DBCondition[MAX_RULEMSG_LEN];
		APP_LOG("APNS", LOG_DEBUG, "UpdateAPNSDB Called for RuleID: %s RuleMSG: %s, RuleFreq: %s, Action: %d",RuleID,RuleMSG,RuleFreq,Action);
		if(0 == Action){
				APP_LOG("APNS", LOG_DEBUG, "SetRuleID Setting RuleID: %s",RuleID);
				memset(query, 0x00, sizeof(query));
				snprintf(query, sizeof(query),"select RULEID from RuleIDInfo where RULEID=%s;",RuleID);
				APP_LOG("APNS", LOG_DEBUG, "Executing Query: %s on DB Handle: %d" ,query,g_APNSRuleDB);
				retVal= WeMoDBGetTableData(&g_APNSRuleDB,query,&s8ResultArray,&s32NoOfRows,&s32NoOfCols);
				if(!retVal)
				{
						if(0 != s32NoOfCols)
						{
								ColDetails RuleColInfo[] = {{"RULEID",RuleID},{"RULEMSG",RuleMSG},{"FREQ",RuleFreq}};
								snprintf(DBCondition, sizeof(DBCondition), "RULEID = %s", RuleID);
								retVal= WeMoDBUpdateTable(&g_APNSRuleDB, "RuleIDInfo", RuleColInfo, 3, DBCondition);
								if (retVal)
								{
										APP_LOG("APNS", LOG_DEBUG, "SetRuleID: Cannot Update Into DB");
										retVal = PLUGIN_FAILURE;
								}
						}
						else
						{
								ColDetails RuleColInfo[] = {{"RULEID",RuleID},{"RULEMSG",RuleMSG},{"FREQ",RuleFreq}};
								retVal= WeMoDBInsertInTable(&g_APNSRuleDB,"RuleIDInfo", RuleColInfo, 3);
								if (retVal)
								{
										APP_LOG("APNS", LOG_DEBUG, "SetRuleID: Cannot Insert Into DB");
										retVal = PLUGIN_FAILURE;
								}
						}
				}
		}
		else if (1 == Action){ 
				APP_LOG("APNS", LOG_DEBUG, "Deleting RuleID: %s",RuleID);
				snprintf(DBCondition, sizeof(DBCondition), "RULEID = %s", RuleID);
				retVal = WeMoDBDeleteEntry(&g_APNSRuleDB,"RuleIDInfo",DBCondition);
				if(retVal)
				{
						APP_LOG("APNS", LOG_DEBUG, "DeleteRuleID: Cannot delete ruleID from DB");
						retVal = PLUGIN_FAILURE;

				}
		}
		return retVal;

}
#endif

#ifdef PRODUCT_WeMo_Light

int changeNightLightStatus(char *DimValue)
{
		int IntDimVal=0, retVal=SUCCESS;

		SetBelkinParameter(DIMVALUE, DimValue);
		AsyncSaveData();
		IntDimVal = atoi(DimValue);
		APP_LOG("UPNPDevice", LOG_DEBUG, "Changing Night Light With DimValue: %d",IntDimVal);
		retVal = ChangeNightLight(IntDimVal);

		if(!retVal && IntDimVal==1)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "DimNightLight: ...........Rebooting the System for Diming the Night Light");
				AsyncRebootSystem();
		}

		return retVal;
}

void AsyncRebootSystem(void)
{
		pMessage msg = createMessage(NIGHTLIGHT_DIMMING_MESSAGE_REBOOT, 0x00, 0x00);
		SendMessage2App(msg);
}

int SetNightLightStatus(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int retVal=UPNP_E_SUCCESS;
		APP_LOG("UPNP: DimNightLight", LOG_DEBUG, "%s called", __FUNCTION__);
		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "DimNightLight: command paramter invalid");
				return UPNP_E_INVALID_ARGUMENT;
		}
		char* DimValue = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DimValue");
		if (0x00 == DimValue || 0x00 == strlen(DimValue))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNPDevice", LOG_DEBUG, "DimNightLight: No DimValue");
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "DimNightLight", 
								CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "DimValue", "Error");  

				retVal=UPNP_E_INVALID_ARGUMENT;
		}
		if(UPNP_E_SUCCESS == retVal)
		{
				retVal = changeNightLightStatus(DimValue);
				if(!retVal){
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x00;
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "DimNightLight", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE],"DimValue" , "success");  
				}
				else{
						pActionRequest->ActionResult = NULL;
						pActionRequest->ErrCode = 0x01;
						APP_LOG("UPNPDevice", LOG_DEBUG, "DimNightLight: ChangeNightLight Failed");
						UpnpAddToActionResponse(&pActionRequest->ActionResult, "DimNightLight", 
										CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "DimValue", "Error");  
				}
		}
		FreeXmlSource(DimValue);
		return retVal;
}

int GetNightLightStatus(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int retVal=UPNP_E_SUCCESS;
		char szResponse[SIZE_256B];
		memset(szResponse, 0x00, sizeof(szResponse));
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s", __FUNCTION__);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		APP_LOG("UPNP: GetDimNightLight", LOG_DEBUG, "%s called", __FUNCTION__);
		char *dimVal = GetBelkinParameter (DIMVALUE);
		APP_LOG("UPNP: GetDimNightLight", LOG_DEBUG, "DimValue in Flash: %s ",dimVal);

		snprintf(szResponse, sizeof(szResponse), "%s", dimVal);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetDimNightLight", CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], 
						"DimValue", szResponse);  

		return UPNP_E_SUCCESS;
}
#endif

void PopulatePluginParams(int DeviceType)
{
		int hr=0, min=0, sec=0;
		memset(gPluginParms.UpTime, 0x0, SIZE_64B);
		memset(gPluginParms.DeviceInfo, 0x0, SIZE_128B);

		detectUptime(&hr, &min, &sec);
		snprintf(gPluginParms.UpTime, sizeof(gPluginParms.UpTime), "%d:%d:%d", hr, min, sec);
#ifdef PRODUCT_WeMo_Insight
		char *paramVersion = NULL,*paramPerUnitcost = NULL,*paramCurrency = NULL;
#endif
		gPluginParms.Internet = getCurrentClientState();
		if (gIceRunning)
				gPluginParms.CloudVia = gIceRunning;
		else 
				gPluginParms.CloudVia = 0;
		gPluginParms.CloudConnectivity = gpluginNatInitialized;
		gPluginParms.LastAuthVal=gLastAuthVal;//setting in API webAppErrorHandling
		gPluginParms.LastFWUpdateStatus=getCurrFWUpdateState();
		gPluginParms.NowTime=(int) GetUTCTime();
		gPluginParms.HomeID=g_szHomeId;
		gPluginParms.DeviceID=g_szWiFiMacAddress;
		gPluginParms.RemoteAccess=gpluginRemAccessEnable;
		switch(DeviceType)
		{
				case DEVICE_SOCKET:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Socket"); 
						break;
				case DEVICE_SENSOR:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Sensor"); 
						break;
				case DEVICE_BABYMON:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Baby"); 
						break;
				case DEVICE_STREAMING:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Streaming"); 
						break;
				case DEVICE_BRIDGE:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Bridge"); 
						break;
				case DEVICE_INSIGHT:
#ifdef PRODUCT_WeMo_Insight
						paramVersion = GetBelkinParameter(ENERGYPERUNITCOSTVERSION);
						paramPerUnitcost = GetBelkinParameter(ENERGYPERUNITCOST);
						paramCurrency   = GetBelkinParameter(CURRENCY);
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s|%s|%s|%s|%d|%d|%d","Insight",
										paramVersion,paramPerUnitcost,paramCurrency,g_PowerThreshold,g_EventEnable,g_s32DataExportType); 
#endif
						break;
				case DEVICE_CROCKPOT:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","crockpot"); 
						break;
				case DEVICE_LIGHTSWITCH:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Lightswitch"); 
						break;
				case DEVICE_NETCAM:
						snprintf(gPluginParms.DeviceInfo, sizeof(gPluginParms.DeviceInfo), "%s","Netcam"); 
						break;
				default:
						break;
		}

}
//------------------------------------------ Extended Meta info -----------------
int GetExtMetaInfo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szResponse[SIZE_256B];
		memset(szResponse, 0x00, sizeof(szResponse));
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s", __FUNCTION__);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;


		{
#ifdef PRODUCT_WeMo_CrockPot
				if (DEVICE_CROCKPOT == g_eDeviceType) {

						PopulatePluginParams(g_eDeviceType);
				}
				else
#endif
#ifdef PRODUCT_WeMo_Light
						if (DEVICE_LIGHTSWITCH == g_eDeviceTypeTemp) {

								PopulatePluginParams(g_eDeviceTypeTemp);
						}
						else
#endif
#ifdef PRODUCT_WeMo_Insight
								if(DEVICE_INSIGHT == g_eDeviceTypeTemp){

										PopulatePluginParams(g_eDeviceTypeTemp);

								}
								else
#endif
								{
										PopulatePluginParams(g_eDeviceType);
								}
		}


		snprintf(szResponse, sizeof(szResponse), "%d|%d|%d|%d|%s|%d|%d|%s|%d|%s",
						gPluginParms.Internet,gPluginParms.CloudVia,
						gPluginParms.CloudConnectivity,gPluginParms.LastAuthVal,gPluginParms.UpTime,
						gPluginParms.LastFWUpdateStatus,gPluginParms.NowTime,gPluginParms.HomeID,
						gPluginParms.RemoteAccess,gPluginParms.DeviceInfo);
		APP_LOG("UPNP: Device", LOG_DEBUG, "ExtMetaInfo:%s", szResponse);
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetExtMetaInfo", CtrleeDeviceServiceType[PLUGIN_E_METAINFO_SERVICE], 
						"ExtMetaInfo", szResponse);  

		return UPNP_E_SUCCESS;
}

//------------------------------------------ Meta info -----------------
int GetMetaInfo(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char szResponse[SIZE_256B];
		memset(szResponse, 0x00, sizeof(szResponse));
		APP_LOG("UPNP: Device", LOG_DEBUG, "%s", __FUNCTION__);
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;


		{
#ifdef PRODUCT_WeMo_CrockPot
				if (DEVICE_CROCKPOT == g_eDeviceType) {

						snprintf(szResponse, sizeof(szResponse), "%s|%s|%s|%s|%s|%s", 
										g_szWiFiMacAddress, g_szSerialNo, g_szSkuNo, g_szFirmwareVersion, 
										g_szApSSID, "crockpot");
				}
				else
#endif
#ifdef PRODUCT_WeMo_Light
						if (DEVICE_LIGHTSWITCH == g_eDeviceTypeTemp) {

								snprintf(szResponse, sizeof(szResponse), "%s|%s|%s|%s|%s|%s", 
												g_szWiFiMacAddress, g_szSerialNo, g_szSkuNo, g_szFirmwareVersion, 
												g_szApSSID, "Lightswitch");
						}
						else
#endif
#ifdef PRODUCT_WeMo_Insight
								if(DEVICE_INSIGHT == g_eDeviceTypeTemp){

										snprintf(szResponse, sizeof(szResponse), "%s|%s|%s|%s|%s|%s", 
														g_szWiFiMacAddress, g_szSerialNo, g_szSkuNo, g_szFirmwareVersion, 
														g_szApSSID, "Insight");

								}
								else
#endif
								{
										snprintf(szResponse, sizeof(szResponse), "%s|%s|%s|%s|%s|%s", 
														g_szWiFiMacAddress, g_szSerialNo, g_szSkuNo, g_szFirmwareVersion, 
														g_szApSSID, (g_eDeviceType == DEVICE_SENSOR)? "Sensor":"Socket");
								}
		}

		APP_LOG("UPNP: Device", LOG_DEBUG, "%s", szResponse);

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetMetaInfo", CtrleeDeviceServiceType[PLUGIN_E_METAINFO_SERVICE], 
						"MetaInfo", szResponse);  

		return UPNP_E_SUCCESS;
}

char* CreateManufactureData()
{

#ifdef PRODUCT_WeMo_LEDLight
  mxml_node_t *pRespXml = NULL;
  mxml_node_t *pNodeRoot = NULL;
  mxml_node_t *pNode = NULL;
  mxml_node_t *pPowerMeter = NULL;
  char *pszResp = NULL;
#endif


  char *pRetVal = NULL;
  char *pSerialNumber = NULL;
  FILE *fp = NULL;

#ifdef PRODUCT_WeMo_Insight
  DataValues Values = {0,0,0,0,0};  
  int Ret = 0;  
#endif

#ifdef PRODUCT_WeMo_LEDLight
  pRespXml = mxmlNewXML("1.0");
  pNodeRoot = mxmlNewElement(pRespXml, "ManufactureData");
#endif
  char *pCountryCode   = GetBelkinParameter ("country_code");
  //char *pFirmwareVer   = GetBelkinParameter ("WeMo_version");
  char *pFirmwareVer   = g_szFirmwareVersion;
  char *pAPMacAddress  = GetBelkinParameter ("wl0_macaddr");
  char *pTargetCountry = GetBelkinParameter ("target_country");
#if 0
  char *pSTAMacAddress = GetBelkinParameter ("apcli0_macaddr");
#else
  
  char ucaMacAddress[SIZE_20B] = {0};  
  struct ifreq  s;
  char *pSTAMacAddress = ucaMacAddress;
  int fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_IP);
  strcpy (s.ifr_name, "apcli0");
  if (0 == ioctl (fd, SIOCGIFHWADDR, &s))
  {
    const unsigned char* mac = (unsigned char *) s.ifr_hwaddr.sa_data;
    snprintf (ucaMacAddress, sizeof (ucaMacAddress),
              "%02X:%02X:%02X:%02X:%02X:%02X",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }
  else
  {
    strncpy (ucaMacAddress, "00:00:00:00:00:00", sizeof (ucaMacAddress));
  }
  APP_LOG ("UPNP: Device", LOG_DEBUG, "&&**&& apcli0 MacAddress = %s &&**&&", \
           pSTAMacAddress);
#endif
#ifdef PRODUCT_WeMo_Insight
  /*Reading Instantaneous Power from daemon*/ 
  if((Ret = HAL_GetCurrentReadings(&Values)) != 0) {
	APP_LOG("UPNP: Device", LOG_DEBUG, "\nMetering Daemon Not Responding!!!\n");
  }
  else
  {
	APP_LOG("UPNP: Device", LOG_DEBUG, "\nRead Instantaneous Power values from daemon %d && %f\n",Values.vRMS,(float)(Values.vRMS/1000));

  }
#endif   

#ifdef PRODUCT_WeMo_LEDLight
  pSerialNumber = GetBelkinParameter("SerialNumber");
#else
  pSerialNumber = GetBelkinParameter("cwf_serial_number");
  //TODO
  //Get PowerMeter and USB information
#endif

  char *pSSID = g_szApSSID;


#ifndef PRODUCT_WeMo_LEDLight
  if ((pRetVal = malloc(MAX_STATUS_BUF)) == NULL)
  return (NULL);
#ifndef PRODUCT_WeMo_Insight
  snprintf(pRetVal, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"utf-8\"?><ManufactureData><CountryCode>%s</CountryCode><FirmwareVersion>%s</FirmwareVersion><APMacAddress>%s</APMacAddress><STAMacAddress>%s</STAMacAddress><SSID>%s</SSID><TargetCountry>%s</TargetCountry><SerialNumber>%s</SerialNumber></ManufactureData>", \
	  pCountryCode, pFirmwareVer, pAPMacAddress, pSTAMacAddress, pSSID, pTargetCountry, pSerialNumber);
#else
  snprintf(pRetVal, MAX_STATUS_BUF, "<?xml version=\"1.0\" encoding=\"utf-8\"?><ManufactureData><CountryCode>%s</CountryCode><FirmwareVersion>%s</FirmwareVersion><APMacAddress>%s</APMacAddress><STAMacAddress>%s</STAMacAddress><SSID>%s</SSID><TargetCountry>%s</TargetCountry><SerialNumber>%s</SerialNumber><PowerMeter><vRMS>%d.%03d</vRMS><iRMS>%d.%03d</iRMS><Watts>%d.%03d</Watts><PF>%d.%03d</PF><Freq>%d.%03d</Freq><IntTemp>%d.%03d</IntTemp><ExtTemp>%d.%03d</ExtTemp></PowerMeter><USB></USB></ManufactureData>", \
	  pCountryCode, pFirmwareVer, pAPMacAddress, pSTAMacAddress, pSSID, pTargetCountry, pSerialNumber, (Values.vRMS/1000),(Values.vRMS%1000), (Values.iRMS/1000), (Values.iRMS%1000),(Values.Watts/1000),(Values.Watts%1000),(Values.PF/1000),(Values.PF%1000),(Values.Freq/100),(Values.Freq%100),(Values.InternalTemp/1000),(Values.InternalTemp%1000),(Values.ExternalTemp/1000),(Values.ExternalTemp%1000));
#endif

	
   APP_LOG("UPNP: Device", LOG_DEBUG, "Response: %s", pRetVal);

#else

  pNode = mxmlNewElement(pNodeRoot, "CountryCode");
  if( pCountryCode && pCountryCode[0] )
  	mxmlNewText(pNode, 0, pCountryCode);
  else
  	mxmlNewText(pNode, 0, "");

  pNode = mxmlNewElement(pNodeRoot, "FirmwareVesrion");
  if( pFirmwareVer && pFirmwareVer[0] )
  	mxmlNewText(pNode, 0, pFirmwareVer);
  else
  	mxmlNewText(pNode, 0, "");

  pNode = mxmlNewElement(pNodeRoot, "APMacAddress");
  if( pAPMacAddress && pAPMacAddress[0] )
  	mxmlNewText(pNode, 0, pAPMacAddress);
  else
  	mxmlNewText(pNode, 0, "");

  pNode = mxmlNewElement (pNodeRoot, "STAMacAddress");
  if (pSTAMacAddress && pSTAMacAddress[0])
     mxmlNewText (pNode, 0, pSTAMacAddress);
  else
  	mxmlNewText (pNode, 0, "");
	
  pNode = mxmlNewElement(pNodeRoot, "SSID");
  if( pSSID && pSSID[0] )
  	mxmlNewText(pNode, 0, pSSID);
  else
  	mxmlNewText(pNode, 0, "");

  pNode = mxmlNewElement (pNodeRoot, "TargetCountry");
  if (pTargetCountry && pTargetCountry[0])
  	mxmlNewText (pNode, 0, pTargetCountry);
  else
  	mxmlNewText (pNode, 0, "");
	
  pNode = mxmlNewElement (pNodeRoot, "SerialNumber");
  if (pSerialNumber && pSerialNumber[0])
  	mxmlNewText (pNode, 0, pSerialNumber);
  else
  	mxmlNewText (pNode, 0, "");
	
  pNode = mxmlNewElement(pNodeRoot, "PowerMeter");
  pNode = mxmlNewElement(pNodeRoot, "USB");
#endif

  /* write to the file */
  fp = fopen("/tmp/Belkin_settings/ManufactureData.xml", "w");
  
#ifdef PRODUCT_WeMo_LEDLight
  if( pRespXml )
  {
  	if(!fp)
  	{
		APP_LOG("UPNP: Device", LOG_ERR, "Could not open file for writing err: %s",strerror(errno));
 	}
	else
	{
      		mxmlSaveFile(pRespXml, fp, MXML_NO_CALLBACK);
      		fclose(fp);
	}
      pszResp = mxmlSaveAllocString(pRespXml, MXML_NO_CALLBACK);
  }

  if( pRespXml )
	  mxmlDelete(pRespXml);

  return pszResp;
#else

  if(!fp)
  {
	  APP_LOG("UPNP: Device", LOG_ERR, "Could not open file for writing err: %s",strerror(errno));
  }
  else
  {
	  fwrite(pRetVal, 1, strlen(pRetVal), fp);
      	  fclose(fp);
  }
  return pRetVal;
#endif
}


//------------------------------------------ Manufacture info -----------------
int GetManufactureData(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
  ( *out ) = NULL;
  ( *errorString ) = NULL;

  char *pszRespXML = NULL;
  int retVal = UPNP_E_SUCCESS;

  pszRespXML = CreateManufactureData();

  // create a response
  if( UpnpAddToActionResponse( out, "GetManufactureData",
            CtrleeDeviceServiceType[PLUGIN_E_MANUFACTURE_SERVICE],
            "ManufactureData", pszRespXML) != UPNP_E_SUCCESS )
  {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    APP_LOG("UPNPDevice", LOG_DEBUG, "GetManufactureData: Internal Error");
    retVal = UPNP_E_INTERNAL_ERROR;
  }

  if(pszRespXML)
  free (pszRespXML);

  return retVal;
}


#define MAX_RULE_ENTRY_SIZE 1024



//-- NEW RULE implementation ---------------------

int UpdateWeeklyCalendar(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char* 	szWeekList[WEEK_DAYS_NO] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		int 		loop = 0x00;
		int 		resp = 0x00;


		APP_LOG("Rule", LOG_CRIT,"%s", __FUNCTION__); 
		if (0x00 == pActionRequest)
		{
				return 0x01;
		}
#ifdef SIMULATED_OCCUPANCY
		unsetSimulatedData();
		//gSimManualTrigger = 0;
#endif
		//-Get week entry list

		for (loop = 0x00; loop < WEEK_DAYS_NO; loop++)
		{
				szWeekList[loop] = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, szWeekDayShortName[loop]); 
				if (szWeekList[loop] && strlen(szWeekList[loop]))
				{
						APP_LOG("UPNP: Rule", LOG_DEBUG, "%s", szWeekList[loop]);  
				}
		}

		resp = UpdateRule((const char **)szWeekList);

		if (0x00 == resp)
		{
				pActionRequest->ActionResult = 0x00;
				pActionRequest->ErrCode = 0x00;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "UpdateWeeklyCalendar", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"status", "sucess");  
		}
		else
		{
				APP_LOG("Rule", LOG_DEBUG, "##################### Rule update failure #############");

				pActionRequest->ActionResult = 0x00;
				pActionRequest->ErrCode = 0x01;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "UpdateWeeklyCalendar", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"status", "unsucess");	
		}


		for (loop = 0x00; loop < WEEK_DAYS_NO; loop++)
		{
				FreeXmlSource(szWeekList[loop]);
		}

		return UPNP_E_SUCCESS;

}

void	SaveDbVersion(char* szVersion)
{
		if (!szVersion)
				return;

		SaveDeviceConfig(RULE_DB_VERSION_KEY, szVersion);
}

/**
 *
 *
 *
 *
 *
 *
 *********************************************************************/
int SetRulesDBVersion(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{

		char* szVersion = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "RulesDBVersion");
		if (szVersion && strlen(szVersion))
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				APP_LOG("UPNP: Rule", LOG_DEBUG, "New database version %s", szVersion);  

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRulesDBVersion", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"RulesDBVersion", szVersion);

				SetBelkinParameter(RULE_DB_VERSION_KEY, szVersion);
		}
		else
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;
				APP_LOG("UPNP: Rule", LOG_ERR, "parameters error: database version empty");  
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetRulesDBVersion", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"status", "unsuccess");  
		}

		return UPNP_E_SUCCESS;
}

int GetRulesDBVersion(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char *szVersion = GetDeviceConfig(RULE_DB_VERSION_KEY);
		if (szVersion && strlen(szVersion))
		{  		
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				APP_LOG("UPNP: Rule", LOG_DEBUG, "Database version:%s", szVersion); 

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetRulesDBVersion", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"RulesDBVersion", szVersion);  
		}
		else
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;
				APP_LOG("UPNP: Rule", LOG_ERR, "database version not available"); 

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetRulesDBVersion", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"RulesDBVersion", "0");  

		}

		return UPNP_E_SUCCESS;

}

int StopWeeklyScheduler()
{
		g_isSensorRuleActivated = 0x00;
#ifdef PRODUCT_WeMo_Insight
		g_isInsightRuleActivated = 0x00;
#endif

		if (INVALID_THREAD_HANDLE != g_handlerSchedulerTask)
		{
				int ret = ithread_cancel(g_handlerSchedulerTask);

				if (0x00 != ret)
				{
						APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_cancel: Can not stop rule task thread ##############################");
				}
				else
				{
						APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_cancel: Successfully stop rule task thread ##############################");
				}

				ret = ithread_detach(g_handlerSchedulerTask);

				if (0x00 != ret)
				{
						APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_detach: Can not detatch rule task thread ##############################");
				}

				g_handlerSchedulerTask = INVALID_THREAD_HANDLE;


		}
		return 0;
}

int RunWeeklyScheduler()
{
#ifdef SIMULATED_OCCUPANCY
		checkLastManualToggleState();
#endif
		//kill the thread if it is there, and then start a new thread
		int ret = ithread_create(&g_handlerSchedulerTask, 0x00, RulesTask, 0x00);
		if (0x00 != ret)
		{
				APP_LOG("UPNP: Rule", LOG_DEBUG, "######################## ithread_create: Can not create rule task thread ##############################");
		}

		APP_LOG("UPNP: Rule", LOG_DEBUG, "scheduler thread created: %d", g_handlerSchedulerTask);

		return UPNP_E_SUCCESS;
}

int EditWeeklycalendar(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char* szAction = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "action");

		if ((0x00 == szAction) || (0x00 == strlen(szAction)))
		{

				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x01;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "EditWeeklycalendar", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"status", "unsuccess");  

				APP_LOG("UPNP: Rule", LOG_ERR, "%s: paramters error", __FUNCTION__);

				return 0x00;
		}

		int action = atoi(szAction);

		APP_LOG("UPNP: Rule", LOG_DEBUG, "Rule stop command request:%d", action);

		if (RULE_ACTION_REMOVE == action)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = 0x00;

				UpnpAddToActionResponse(&pActionRequest->ActionResult, "EditWeeklycalendar", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
								"status", "success");

				system("rm /tmp/Belkin_settings/rule.txt");		
				lockRule();
				curTimerEntry 			= 0x00;
				g_pCurTimerControlEntry = 0x00;
				StopWeeklyScheduler();
				CleanupOverrideTable();
				ResetOverrideStatus();
				unlockRule();
#ifdef SIMULATED_OCCUPANCY
				unsetSimulatedData();
				gSimManualTrigger = 0;
				system("rm /tmp/Belkin_settings/simulatedRule.txt");
				UnSetBelkinParameter(SIM_DEVICE_COUNT);
				UnSetBelkinParameter(SIM_MANUAL_TRIGGER);
				UnSetBelkinParameter(SIM_MANUAL_TRIGGER_DATE);
				AsyncSaveData();
				gSimulatedDeviceCount = 0;
#endif

				//- Reset to default sensor
				ResetSensor2Default();

				APP_LOG("UPNP: Rule", LOG_DEBUG, "Rule stop command executed");
		}
		else
		{
				APP_LOG("UPNP: Rule", LOG_DEBUG, "Rule stop command not executed: %d", action);		
		}


		return UPNP_E_SUCCESS;
}


int GetRulesDBPath(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		char szDBURL[MAX_FW_URL_LEN];
		memset(szDBURL, 0x00, sizeof(szDBURL));

		//-Return the icon path of the device
		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		snprintf(szDBURL, sizeof(szDBURL), "http://%s:%d/rules.db", g_server_ip, g_server_port);
		APP_LOG("UPNP: Rule", LOG_DEBUG, "DBRule:%s", szDBURL);

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetRulesDBPath", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], 
						"RulesDBPath", szDBURL);  

		return UPNP_E_SUCCESS;
}

#ifdef SIMULATED_OCCUPANCY
int NotifyManualToggle(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		APP_LOG("UPNPDevice", LOG_DEBUG, "Notify Manual Toggle");
		gSimManualTrigger = 0x01;
		saveManualTriggerData();

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "NotifyManualToggle", 
						CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "ManualToggle", "success");

		APP_LOG("UPNPDevice", LOG_DEBUG, "gSimManualTrigger: %d", gSimManualTrigger);
		return UPNP_E_SUCCESS;
}

int GetSimulatedRuleData(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		char RuleData[SIZE_256B];
		int selfindex = -1;
		int remtimetotoggle = -1;

		APP_LOG("UPNPDevice", LOG_DEBUG, "GetSimulatedRuleData");
		if (0x00 == pActionRequest || 0x00 == request)
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "GetSimulatedRuleData: paramters error");
				return 0x01;
		}

		memset(RuleData, 0x00, sizeof(RuleData));

		LockLED();
		int curState = GetCurBinaryState();
		UnlockLED();

		LockSimulatedOccupancy();
		selfindex = gSelfIndex;
		remtimetotoggle = gRemTimeToToggle; //(nextscheduletime - currenttime)
		UnlockSimulatedOccupancy();

		snprintf(RuleData, sizeof(RuleData), "%d|%d|%d|%s", selfindex, curState, remtimetotoggle, g_szUDN_1);

		pActionRequest->ActionResult = NULL;
		pActionRequest->ErrCode = 0x00;

		UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetSimulatedRuleData", 
						CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "RuleData", RuleData);

		APP_LOG("UPNPDevice", LOG_DEBUG, "GetSimulatedRuleData: %s", RuleData);
		return UPNP_E_SUCCESS;
}

int SimulatedRuleData(pUPnPActionRequest pActionRequest, IXML_Document *request, IXML_Document **out, const char **errorString)
{
		int retVal=SUCCESS;
		char* DeviceList = NULL; 
		char* DeviceCount = NULL;
		char devCount[SIZE_8B];

		APP_LOG("UPNP: Device", LOG_DEBUG, "%s called", __FUNCTION__);
		if (pActionRequest == 0x00) 
		{
				APP_LOG("UPNP: Device", LOG_ERR,"INVALID PARAMETERS"); 
				retVal = FAILURE;
				goto on_return;
		}

		DeviceList = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DeviceList");
		DeviceCount = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "DeviceCount");
		if (0x00 == DeviceList || 0x00 == strlen(DeviceList))
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "Simulated Rule Data: No DeviceList");
				retVal = FAILURE;
				goto on_return;
		}
		if (0x00 == DeviceCount|| 0x00 == strlen(DeviceCount))
		{
				APP_LOG("UPNPDevice", LOG_DEBUG, "Simulated Rule Data: No DeviceCount");
				retVal = FAILURE;
				goto on_return;
		}

		pActionRequest->ActionResult = 0x00;
		pActionRequest->ErrCode = 0x00;
		UpnpAddToActionResponse(&pActionRequest->ActionResult, "SimulatedRuleData", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], "status", "success");  
		setSimulatedRuleFile(DeviceList);
		LockSimulatedOccupancy();
		gSimulatedDeviceCount = atoi(DeviceCount);
		UnlockSimulatedOccupancy();
		memset(devCount, 0, sizeof(devCount));
		snprintf(devCount, sizeof(devCount), "%d", gSimulatedDeviceCount);
		SetBelkinParameter(SIM_DEVICE_COUNT, devCount);
		AsyncSaveData();
		APP_LOG("UPNPDevice", LOG_DEBUG, "Simulated Rule Data: gSimulatedDeviceCount: %d", gSimulatedDeviceCount);

on_return:
		FreeXmlSource(DeviceList);
		FreeXmlSource(DeviceCount);
		if(retVal != SUCCESS)
		{
				pActionRequest->ActionResult = NULL;
				pActionRequest->ErrCode = UPNP_E_INVALID_ARGUMENT;
				UpnpAddToActionResponse(&pActionRequest->ActionResult, "SimulatedRuleData", CtrleeDeviceServiceType[PLUGIN_E_RULES_SERVICE], "status", "failure");  
		}
		return retVal;
}
#endif
/**
 *  To get ssid prefix so that can form device AP ssid.
 *  Please note that, prefix table not pre-built since the list will not be long and "hard-coding"
 *  
 *  @ szKey 
 *  @ szBuffer char* INPUT, the buffer to save and return the results, please allocate 16 bytes for it
 */
char* GetSsidPrefix(char* szSerialNo, char* szBuffer)
{
		if ((0x00 == szBuffer) || 
						(0x00 == szSerialNo)
			 )
		{
				//- Error
				return 0x00;
		}

		if (MAX_SERIAL_LEN != strlen(szSerialNo))
		{
				strncpy(szBuffer, SSID_PREFIX_ERROR, MAX_APSSID_LEN-1);
				return 0x00;
		}

		APP_LOG("Init", LOG_DEBUG, "Product type(K/L/B/M/V): %C", szSerialNo[SERIAL_TYPE_INDEX]);

		if ('K' == szSerialNo[SERIAL_TYPE_INDEX])
		{
				/* Relay based products */
				if ('1' == szSerialNo[SERIAL_TYPE_INDEX+2])
				{
						strncpy(szBuffer, SSID_PREFIX_SWICTH, MAX_APSSID_LEN-1);
				}
				else if ('2' == szSerialNo[SERIAL_TYPE_INDEX+2])
				{
						strncpy(szBuffer, SSID_PREFIX_INSIGHT, MAX_APSSID_LEN-1);
				}
				else if ('3' == szSerialNo[SERIAL_TYPE_INDEX+2])
				{
						strncpy(szBuffer, SSID_PREFIX_LIGHT, MAX_APSSID_LEN-1);
				}
#ifdef PRODUCT_WeMo_CrockPot
				else if ('4' == szSerialNo[SERIAL_TYPE_INDEX+2])
				{
						strncpy(szBuffer, SSID_PREFIX_CROCK, MAX_APSSID_LEN-1);
				}
#endif /*#ifdef PRODUCT_WeMo_CrockPot*/
				else
				{
						strncpy(szBuffer, SSID_PREFIX_ERROR, MAX_APSSID_LEN-1);
				}
		}
		else if ('L' == szSerialNo[SERIAL_TYPE_INDEX])
		{
				//- Sensor
				strncpy(szBuffer, SSID_PREFIX_MOTION, MAX_APSSID_LEN-1);
		}
		else if ('M' == szSerialNo[SERIAL_TYPE_INDEX])
		{
				//- Baby, please note now just WeMo_Baby, not WeMo.Baby
				strncpy(szBuffer, SSID_PREFIX_BABY, MAX_APSSID_LEN-1);
		}
		else
		{
				//- Error SSID
				strncpy(szBuffer, SSID_PREFIX_ERROR, MAX_APSSID_LEN-1);
		}


		return szBuffer;

}

int getNvramParameter(char paramVal[])
{
		FILE *pSrFile = 0x00;
		const char *srFile = "/tmp/serFile";
		char srCmd[SIZE_128B];
		char szflag[SIZE_128B];
		char *var = NULL, *val = NULL;
		int len = 0;
		char buf[SIZE_64B];

		memset(srCmd, 0x0, sizeof(srCmd));

		snprintf(srCmd, SIZE_128B, "fw_printenv SerialNumber > /tmp/serFile");

		system(srCmd);

		pSrFile = fopen(srFile, "r");
		if (pSrFile == 0x00)
		{
				APP_LOG("UPNP: DEVICE",LOG_DEBUG, "File opening %s error", srFile);
				strcpy(paramVal, DEFAULT_SERIAL_NO);
				return 0;
		}   

		memset(szflag, 0x0, sizeof(szflag));
		if(fgets(szflag, sizeof(szflag), pSrFile))
		{
				var = strtok(szflag,"=");
				val = strtok(NULL,"=");
		}   
		fclose(pSrFile);

		memset(buf, 0x00, sizeof(buf));
		sprintf(buf, "rm %s", srFile);
		system(buf);

		if(val && (len = strlen(val)) > 2) {
				if(val[len - 1] == '\n') {
						val[len - 1] = '\0';
						len = len -1;
				}

				strcpy(paramVal, val);
		} else {
				strcpy(paramVal, DEFAULT_SERIAL_NO);
		}

		return len;
}

void SetAppSSID()
{
		char szBuff[MAX_APSSID_LEN];

		memset(szBuff, 0x00, sizeof(szBuff));

		char* szSerialNo = GetSerialNumber();
		if ((0x00 == szSerialNo) || (0x00 == strlen(szSerialNo)))
		{
#ifdef _OPENWRT_
				char serBuff[SIZE_128B];
				memset(serBuff, 0x0, SIZE_128B);
				getNvramParameter(serBuff);
				szSerialNo = serBuff;
				SaveDeviceConfig("SerialNumber", serBuff);

#else
				//-User default one
				szSerialNo = DEFAULT_SERIAL_NO;
#endif
		}

		//-Get prefix of the AP SSID
		GetSsidPrefix(szSerialNo, szBuff);

		strncpy(g_szSerialNo, szSerialNo, sizeof(g_szSerialNo)-1);

		strncat(szBuff, szSerialNo + strlen(szSerialNo) - DEFAULT_SERIAL_TAILER_SIZE, sizeof(szBuff)-strlen(szBuff)-1);

		APP_LOG("UPNP: Device", LOG_DEBUG, "APSSID: %s", szBuff);
		APP_LOG("STARTUP: Device", LOG_DEBUG, "serial: %s", g_szSerialNo);


		wifisetSSIDOfAP(szBuff);
		memset(g_szApSSID, 0x00, sizeof(g_szApSSID));
		strncpy(g_szApSSID, szBuff, sizeof(g_szApSSID)-1);
}

//---------------------------- POST FILE -------------------
int PostFile(UpnpWebFileHandle fileHnd, char *buf, int buflen)
{
		APP_LOG("UPNP: DEVICE", LOG_DEBUG, "to write data: %d bytes\n", buflen);
		return 0x00;
}

UpnpWebFileHandle OpenWebFile(char *filename, enum UpnpOpenFileMode Mode)
{
		APP_LOG("UPNP DEVICE", LOG_DEBUG, "File is to open\n");
		return 0x00;
}

int GetFileInfo(const char *filename,    struct File_Info *info)
{
		APP_LOG("UPNP DEVICE", LOG_DEBUG, "Get File inform called\n");
		return 0x00;
}


//------------------------------ Sensor control --------------------
void *PowerSensorMonitorTask(void *args)
{
                tu_set_my_thread_name( __FUNCTION__ );
		APP_LOG("UPNP: sensor rule", LOG_DEBUG, "Sensor event, monitoring thread running until %d seconds:", sPowerDuration);
		pluginUsleep(sPowerDuration*1000000);

		APP_LOG("UPNP: sensor rule", LOG_DEBUG, "Sensor event, monitoring thread stop: to change state tp: %d", sPowerEndAction);	

		setActuation(ACTUATION_SENSOR_RULE);
		ChangeBinaryState(sPowerEndAction);	
		RuleToggleLed(sPowerEndAction);
		LocalBinaryStateNotify(sPowerEndAction);

		int ret = 0x00;
		//- Move the reset before exit
		ithPowerSensorMonitorTask = -1;
		ithread_exit(&ret);
}


void FirmwareUpdateStatusNotify(const char* szStatus)
{
		char* szCurState[1];
		szCurState[0x00] = (char*)malloc(SIZE_2B);
		memset(szCurState[0x00], 0x00, SIZE_2B);
		snprintf(szCurState[0x00], SIZE_2B, "%s", szStatus);

		char* paramters[] = {"FirmwareUpdateStatus"} ;

		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_FIRMWARE_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_FIRMWARE_SERVICE].ServiceId, (const char **)paramters, (const char **)szCurState, 0x01);

		free(szCurState[0x00]);
		APP_LOG("UPNP: firmware update", LOG_DEBUG, "current status: %s", szStatus);
}


void RemoteFirmwareUpdateStatusNotify(void)
{
		APP_LOG("UPNP",LOG_DEBUG, "REMOTE Firmware update status NOTIFICATION");
		remoteAccessUpdateStatusTSParams (0xFF);
}

int GetTimeZoneIndex(float iTimeZoneValue)
{

		APP_LOG("UPNP: time sync", LOG_DEBUG, "to lookup time zone: %f", iTimeZoneValue);

		int counter = sizeof(g_tTimeZoneList)/sizeof(tTimeZone);
		int loop = 0x00;
		int index = 0x00;

		for (; loop < counter; loop++)
		{
				float delta = iTimeZoneValue - g_tTimeZoneList[loop].iTimeZone;

				if ((delta <= 0.001) && (delta >= -0.001))
				{
						APP_LOG("UPNP: time sync", LOG_DEBUG, "time zone index found: %f, %f, %s", delta, iTimeZoneValue, g_tTimeZoneList[loop].szDescription);
						index = g_tTimeZoneList[loop].index;
						break;
				}
		}

		if (0x00 == index)
				APP_LOG("UPNP: time sync", LOG_ERR, "time zone index not found: %f", iTimeZoneValue);

		//- Reset NTP server
		if (index <= TIME_ZONE_NORTH_AMERICA_INDEX)
				s_szNtpServer = TIME_ZONE_NORTH_AMERICA_NTP_SERVER;
		else if(index == TIME_ZONE_UK_INDEX || index == TIME_ZONE_FRANCE_INDEX)
				s_szNtpServer = TIME_ZONE_EUROPE_NTP_SERVER;
		else if ((index > TIME_ZONE_NORTH_AMERICA_INDEX) && (index <= TIME_ZONE_ASIA_INDEX))
				s_szNtpServer = TIME_ZONE_ASIA_NTP_SERVER;		//- Use asia for Europ
		else
				s_szNtpServer = TIME_ZONE_ASIA_NTP_SERVER;

		return index;

}


int IsUPnPNetworkMode()
{
		if ((0x00 == strcmp(g_server_ip, "0.0.0.0")) ||
						0x00 == strcmp(g_server_ip, AP_LOCAL_IP))
		{
				return 0x00;
		}

		return 0x01;
}

void UpdateUPnPNetworkMode(int networkMode)
{
		if (UPNP_LOCAL_MODE == networkMode)
		{
				APP_LOG("UPNP: network", LOG_DEBUG, "network mode changed to: UPNP_LOCAL_MODE");
				g_IsUPnPOnInternet	= FALSE;
				g_IsDeviceInSetupMode	= TRUE;
		}
		else if (UPNP_INTERNET_MODE == networkMode)
		{
				APP_LOG("UPNP: network", LOG_DEBUG, "network mode changed to: UPNP_INTERNET_MODE");
				g_IsUPnPOnInternet 	= TRUE;
				g_IsDeviceInSetupMode	= FALSE;
		}
		else
		{
				APP_LOG("UPNP: network", LOG_ERR, "wrong network mode");
		}
}


void		NotifyInternetConnected()
{
		pMessage msg = createMessage(NETWORK_INTERNET_CONNECTED, 0x00, 0x00);
		SendMessage2App(msg);
}

void UpdateUPnPNetwork(int status)
{
		//- If IP address change and etc
		char* ip_address = wifiGetIP(INTERFACE_CLIENT);
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
		if (status == NETWORK_INTERNET_CONNECTED)
		{
				APP_LOG("ITC: network", LOG_DEBUG, "router connection done, UPnP re-runs again");
				ControlleeDeviceStop();
				StopPluginCtrlPoint();
				pluginUsleep(2000000);

				if (ip_address && (0x00 != strcmp(ip_address, DEFAULT_INVALID_IP_ADDRESS)))
				{
						//-Start new UPnP in client AP new address
						APP_LOG("ITC: network", LOG_CRIT,"start new UPnP session: %s", ip_address); 
						//gautam: update the Insight and LS Makefile to copy Insightsetup.xml and Lightsetup.xml in /sbin/web/ as setup.xml 
						ControlleeDeviceStart(ip_address, 0x00, "setup.xml", DEFAULT_WEB_DIR);
						StartPluginCtrlPoint(ip_address, 0x00);	
						EnableContrlPointRediscover(TRUE);
						if (DEVICE_SENSOR == g_eDeviceType)
						{
								if(0x01 == atoi(g_szRestoreState))
								{
#ifdef WeMo_INSTACONNECT
										if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
												createAutoRegThread();
								}
								else if(0x00 == atoi(g_szRestoreState))
								{
										if((strlen(g_szHomeId) == 0x0) && (strlen(g_szPluginPrivatekey) == 0x0))
										{	
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "UpdateUPnPNetwork: Remote Access is not Enabled... sensor\n");
										}
										else
										{
												if( (strcmp(g_routerMac, routerMac) != 0) && (g_routerSsid!=NULL && strlen (g_routerSsid) > 0) )
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "UpdateUPnPNetwork: router is not same.. sensor\n");
#ifdef WeMo_INSTACONNECT
														if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
																createAutoRegThread();
												}
												else
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "UpdateUPnPNetwork: Remote Access is Enabled.. sensor\n");
												}
										}	
								}	
						}
						if (DEVICE_SOCKET == g_eDeviceType)
						{
								if(0x01 == atoi(g_szRestoreState))
								{
										APP_LOG("REMOTEACCESS", LOG_DEBUG, "\n Restore state set to 1... Remote Access is not Enabled.. opening socket control point\n");
#ifdef WeMo_INSTACONNECT
										if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
												createAutoRegThread();
								}
								else if(0x00 == atoi(g_szRestoreState))
								{
										if ((0x00 == strlen(g_szHomeId) ) && (0x00 == strlen(g_szPluginPrivatekey)))
										{
												APP_LOG("REMOTEACCESS", LOG_DEBUG, "\n Remote Access is not Enabled.. opening socket control point\n");
										}
										else
										{	
												if( (strcmp(g_routerMac, routerMac) != 0) && (g_routerSsid!=NULL && strlen (g_routerSsid) > 0) )
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "Remote Access is Enabled but router is different, openg socket control point");
#ifdef WeMo_INSTACONNECT
														if(g_connectInstaStatus == APCLI_CONFIGURED)
#endif
																createAutoRegThread();
												}
												else
												{
														APP_LOG("REMOTEACCESS", LOG_DEBUG, "\n Remote Access is Enabled.. not opening socket control point\n");
												}
										}
								}
						}
				}

				UpdateUPnPNetworkMode(UPNP_INTERNET_MODE);
#ifdef WeMo_INSTACONNECT
				system("killall -9 udhcpd");   //to make sure udhpcpd is killed 
#endif
		}
		else if (status == NETWORK_AP_OPEN_UPNP)
		{
				//- Get gateway address, check current running status
				char* szAPIp = GetLanIPAddress();
				if ((0x00 == szAPIp) || 0x00 == strlen(szAPIp))
				{
						APP_LOG("ITC: network", LOG_ERR,"AP IP address unknown");
						return;
				}

				if (0x00 == strcmp(szAPIp, g_server_ip))
				{
						APP_LOG("ITC: network", LOG_ERR,"UPnP already running on AP network");
						return;
				}
				else
				{
						APP_LOG("ITC: network", LOG_ERR,"UPnP is switching to AP network:%s", szAPIp);
						ControlleeDeviceStop();
						StopPluginCtrlPoint();
						pluginUsleep(2000000);
						//gautam: update the Insight and LS Makefile to copy Insightsetup.xml and Lightsetup.xml in /sbin/web/ as setup.xml 
						ControlleeDeviceStart(szAPIp, 0x00, "setup.xml", DEFAULT_WEB_DIR);
						UpdateUPnPNetworkMode(UPNP_LOCAL_MODE);
				}
		}
}


int ProcessItcEvent(pNode node)
{
		if (0x00 == node)
				return 0x01;

		if (0x00 == node->message)
				return 0x01;
		switch(node->message->ID)
		{
				case NETWORK_INTERNET_CONNECTED:
						APP_LOG("ITC: network", LOG_DEBUG, "NETWORK_INTERNET_CONNECTED");
						UpdateUPnPNetwork(NETWORK_INTERNET_CONNECTED);
						break;

				case META_SAVE_DATA:
						APP_LOG("ITC: meta", LOG_DEBUG, "META_SAVE_DATA");
						SaveSetting();
						break;

				case META_SOFT_RESET:
						APP_LOG("ITC: meta", LOG_CRIT, "META_SOFT_RESET");
						resetToDefaults();
						ClearRuleFromFlash();

						break;
				case META_FULL_RESET:
						APP_LOG("ITC: meta", LOG_CRIT, "META_FULL_RESET");
						pluginUsleep(1000000);
						setRemoteRestoreParam(0x1);
						remotePostDeRegister(NULL, META_FULL_RESET);
						ResetToFactoryDefault(0);
						break;
				case META_REMOTE_RESET:
						APP_LOG("ITC: meta", LOG_DEBUG, "META_REMOTE_RESET");
						UnSetBelkinParameter (DEFAULT_HOME_ID);	
						memset(g_szHomeId, 0x00, sizeof(g_szHomeId));
						UnSetBelkinParameter (DEFAULT_PLUGIN_PRIVATE_KEY);	
						memset(g_szPluginPrivatekey, 0x00, sizeof(g_szPluginPrivatekey));
						UnSetBelkinParameter (RESTORE_PARAM);
						memset(g_szRestoreState, 0x0, sizeof(g_szRestoreState));
						/* server environment settings cleanup and nat client destroy */
						serverEnvIPaddrInit();
						invokeNatDestroy();
						SaveSetting();
						break;
				case NETWORK_AP_OPEN_UPNP:
						APP_LOG("ITC: network", LOG_DEBUG, "NETWORK_AP_OPEN_UPNP");
						UpdateUPnPNetwork(NETWORK_AP_OPEN_UPNP);
						break;

				case BTN_MESSAGE_ON_IND:
						APP_LOG("ITC: button", LOG_DEBUG, "BTN_MESSAGE_ON_IND");

						ToggleUpdate(0x01);
						break;
				case BTN_MESSAGE_OFF_IND:
						//nat_trav_destroy(0);
						APP_LOG("ITC: button", LOG_DEBUG, "BTN_MESSAGE_OFF_IND");
						ToggleUpdate(0x00);
						break;

				case UPNP_MESSAGE_ON_IND:
						APP_LOG("ITC: button", LOG_DEBUG, "UPNP_MESSAGE_ON_IND");
						LocalBinaryStateNotify(0x01);
#if 0 
//#ifdef SIMULATED_OCCUPANCY
						if(gSimulatedRuleRunning && (DEVICE_INSIGHT != g_eDeviceTypeTemp))
						{
								gSimManualTrigger = 0x01;
								APP_LOG("ITC:UPNP", LOG_DEBUG, "simulated rule: manual toggle, UPNP_MESSAGE_ON_IND");
								notifyManualToggle();
						}
#endif  
						break;
				case UPNP_MESSAGE_OFF_IND:
						APP_LOG("ITC: button", LOG_DEBUG, "UPNP_MESSAGE_OFF_IND");
						LocalBinaryStateNotify(0x00);
#if 0 
//#ifdef SIMULATED_OCCUPANCY
						if(gSimulatedRuleRunning)
						{
								gSimManualTrigger = 0x01;
								APP_LOG("ITC:UPNP", LOG_DEBUG, "simulated rule: manual toggle, UPNP_MESSAGE_OFF_IND");
								notifyManualToggle();
						}
#endif  
						break;			
#ifdef PRODUCT_WeMo_Insight
				case UPNP_MESSAGE_SBY_IND:
						APP_LOG("ITC: button", LOG_DEBUG, "UPNP_MESSAGE_SBY_IND");
						pluginUsleep(500000);
						LocalBinaryStateNotify(POWER_SBY);
#if 0 
//#ifdef SIMULATED_OCCUPANCY
						if(gSimulatedRuleRunning)
						{
								gSimManualTrigger = 0x01;
								APP_LOG("ITC:UPNP", LOG_DEBUG, "simulated rule: manual toggle, UPNP_MESSAGE_SBY_IND");
								notifyManualToggle();
						}
#endif  
						break;
				case UPNP_MESSAGE_PWR_IND:
						APP_LOG("ITC: Power", LOG_DEBUG, "UPNP_MESSAGE_PWR_IND");
						pluginUsleep(500000);
						if(!g_NoNotificationFlag)
						{
								LocalInsightParamsNotify();
						}
						else
						{
								g_NoNotificationFlag =0;
						}
						break;
				case UPNP_MESSAGE_PWRTHRES_IND:
						APP_LOG("ITC: Power", LOG_DEBUG, "UPNP_MESSAGE_PWRTHRES_IND");
						pluginUsleep(500000);
						LocalPowerThresholdNotify(g_PowerThreshold);
						//Send this response towards cloud synchronously using same data socket
						break;
				case UPNP_MESSAGE_ENERGY_COST_CHANGE_IND:
						APP_LOG("ITC: Power", LOG_DEBUG, "UPNP_MESSAGE_ENERGY_COST_CHANGE_IND");
						pluginUsleep(500000);
						LocalInsightHomeSettingNotify();
						break;
				case UPNP_MESSAGE_DATA_EXPORT:
						APP_LOG("ITC: Power", LOG_DEBUG, "UPNP_MESSAGE_DATA_EXPORT");
						pluginUsleep(500000);
						StartDataExportSendThread((void*)(node->message->message));
						break;
#endif


				case UPNP_ACTION_MESSAGE_ON_IND:
						APP_LOG("ITC: button", LOG_DEBUG, "UPNP_ACTION_MESSAGE_ON_IND");
						pluginUsleep(500000);
						LocalUserActionNotify(0x03);
						break;
				case UPNP_ACTION_MESSAGE_OFF_IND:
						APP_LOG("ITC: button", LOG_DEBUG, "UPNP_ACTION_MESSAGE_OFF_IND");
						pluginUsleep(500000);
						LocalUserActionNotify(0x02);
						break;			

				case META_FIRMWARE_UPDATE:
						APP_LOG("ITC: FIRMWARE_UPDATE", LOG_DEBUG, "META_FIRMWARE_UPDATE");
						APP_LOG("UPNP: Device", LOG_DEBUG, "Firmware download done, UPnP stop");
						FirmwareUpdateStatusNotify(FM_STATUS_DOWNLOAD_SUCCESS);
						FirmwareUpdateStatusNotify(FM_STATUS_UPDATE_STARTING);
						pluginUsleep(3000000);
						StopPluginCtrlPoint();
						ControlleeDeviceStop();
						break;

				case RULE_MESSAGE_RESTART_REQ:
						APP_LOG("ITC: RULE_MESSAGE_RESTART_REQ", LOG_DEBUG, "RULE_MESSAGE_RESTART_REQ");			
						//- DST change to adv again, I am now living in new century
						Advertisement4TimeUpdate();
						//- DST time change, need to reload again
						ReloadRuleFromFlash();
						break;
#ifdef PRODUCT_WeMo_Light
				case NIGHTLIGHT_DIMMING_MESSAGE_REBOOT:
						APP_LOG("ITC: NIGHTLIGHT_DIMMING_MESSAGE_REBOOT", LOG_DEBUG, "NIGHTLIGHT_DIMMING_MESSAGE_REBOOT");			
						pluginUsleep(2000000);
						system("reboot"); 
#endif		
				case META_CONTROLLEE_DEVICE_STOP:
						APP_LOG("ITC: CONTROLLEE_DEVICE_STOP", LOG_DEBUG, "META_CONTROLLEE_DEVICE_STOP");
						APP_LOG("UPNP: Device", LOG_DEBUG, "reset plugin, stop controllee device");
						ControlleeDeviceStop();
						break;

				default:
						break;
		}

		if (0x00 != node->message->message)
				free(node->message->message);
		free(node->message);
		free(node);
		return 0;
}


void	AsyncSaveData()
{
		pMessage msg = createMessage(META_SAVE_DATA, 0x00, 0x00);
		SendMessage2App(msg);
}


void RestoreIcon()
{
		APP_LOG("UPNP", LOG_DEBUG, "To use default icon");

		if (DEVICE_SOCKET == g_eDeviceType)
		{
				system("cp -f /etc/icon.jpg /tmp/Belkin_settings");
		}
		else
		{
				system("cp -f /etc/sensor.jpg /tmp/Belkin_settings/icon.jpg");
		}
}



int GetDeviceStateIF()
{
		int curState = 0x00;

		if (DEVICE_SOCKET == g_eDeviceType)
		{
				LockLED();
				curState = GetCurBinaryState();
				UnlockLED();
		}
		else if (DEVICE_SENSOR == g_eDeviceType)
		{
				LockSensor();
				curState = GetSensorState();
				UnlockSensor();
		}

		return curState;
}

void* FirmwareUpdateStart(void *args)
{
		int retVal = SUCCESS;
		int count=0, state=0, dwldStTime = 0, withUnsignedImage = 0;
		FirmwareUpdateInfo *pfwUpdInf = NULL;
		char FirmwareURL[MAX_FW_URL_LEN];
                tu_set_my_thread_name( __FUNCTION__ );
		memset(FirmwareURL, 0, sizeof(FirmwareURL));

		if(args)
		{
				pfwUpdInf = (FirmwareUpdateInfo *)args;
				dwldStTime = pfwUpdInf->dwldStartTime;
				withUnsignedImage = pfwUpdInf->withUnsignedImage;
				strncpy(FirmwareURL, pfwUpdInf->firmwareURL, sizeof(FirmwareURL)-1);
		}

		APP_LOG("UPnPApp",LOG_DEBUG, "**** Saving firmware update URL: %s ****", FirmwareURL);
		SetBelkinParameter("FirmwareUpURL", FirmwareURL);
		AsyncSaveData();

		if(dwldStTime)
		{
				APP_LOG("UPnPApp", LOG_DEBUG,"******** Sleeping for %d mins zzzzzz", dwldStTime/60);
				pluginUsleep(dwldStTime * 1000000); //Staggering the downloading of firmware
		}

		if (getCurrentClientState() != STATE_CONNECTED) {
				APP_LOG("UPnPApp",LOG_ERR, "**** Internet connection not yet up but Firmware Update thread to proceed: ****");
				FirmwareUpdateStatusNotify(FM_STATUS_DOWNLOAD_UNSUCCESS);
		}

		pthread_attr_init(&firmwareUp_attr);
		pthread_attr_setdetachstate(&firmwareUp_attr, PTHREAD_CREATE_DETACHED);
		retVal = pthread_create(&firmwareUpThread, &firmwareUp_attr, (void*)&firmwareUpdateTh, (void *)args);
		if(retVal < SUCCESS) {
				APP_LOG("UPnPApp",LOG_ERR, "****Firmware Update thread not Created****");
				goto on_return;
		}

		pluginUsleep (500000); //500 ms
		APP_LOG("UPnPApp",LOG_ERR, "************Firmware Update Monitor thread Created");
		while(1)
		{
				count = 0;
				while (count < MAX_FW_DL_TIME_OUT) 
				{
						pluginUsleep (10000000);
						if ( (getCurrFWUpdateState() == atoi(FM_STATUS_DOWNLOAD_SUCCESS)) || (getCurrFWUpdateState() == atoi(FM_STATUS_UPDATE_STARTING)) ) 
						{
								APP_LOG("UPNP: Device", LOG_DEBUG,"********Download is Complete"); 
								pluginUsleep(180000000);	// 3 mins
								APP_LOG("UPnPApp",LOG_ALERT, "************Firmware Update Monitor thread rebooting system...");
								if ((gpluginRemAccessEnable) && (gpluginNatInitialized))
										invokeNatDestroy();
								firmwareUpThread = -1;
								system("reboot");
						}
						count +=10; 
				}

				state = getCurrFWUpdateState();
				APP_LOG("UPNP: Device", LOG_DEBUG,"Going to stop downloading...:%d", state); 

				//- time out here, thread itself quit and the thread ID set to 0x00 for further probable re-visit
				//firmwareUpThread = -1;
				//- Remove the file already created to save more space
				system("rm /tmp/firmware.bin.gpg");
				//- Reset the LED state back so that not confuse the user
				SetWiFiLED(0x04);
				//Stop download firmware request
				StopDownloadRequest();
		}
on_return:
		fwUpMonitorthread = -1;
		return NULL;
}

int StartFirmwareUpdate(FirmwareUpdateInfo fwUpdInf)
{
		int retVal = SUCCESS;
		FirmwareUpdateInfo *pfwUpdInf = NULL;

		if (-1 != fwUpMonitorthread)
		{
				APP_LOG("UPNP: Device", LOG_ERR, "############Firmware Update thread already created################");
				retVal = FAILURE;
				return retVal;
		}

		pfwUpdInf = (FirmwareUpdateInfo *)calloc(1, sizeof(FirmwareUpdateInfo));
		if(!pfwUpdInf) 
		{
				APP_LOG("UPNPApp",LOG_ERR,"pfwUpdInf calloc failed...");
				return FAILURE;
		}

		pfwUpdInf->dwldStartTime = fwUpdInf.dwldStartTime;
		pfwUpdInf->withUnsignedImage = fwUpdInf.withUnsignedImage;
		strncpy(pfwUpdInf->firmwareURL, fwUpdInf.firmwareURL, sizeof(pfwUpdInf->firmwareURL)-1);

		pthread_attr_init(&updateFw_attr);
		pthread_attr_setdetachstate(&updateFw_attr,PTHREAD_CREATE_DETACHED);
		retVal = pthread_create(&fwUpMonitorthread,&updateFw_attr,(void*)&FirmwareUpdateStart, (void*)pfwUpdInf);
		if(retVal < SUCCESS) {
				APP_LOG("UPnPApp",LOG_ERR, "************Firmware Update Start thread not Created");
				retVal = FAILURE;
		}

		return retVal;
}

static int IsRuleDbCreated()
{
		int ret = 0x00;
		FILE* pfDb = fopen(RULE_DB_PATH , "r");
		if (pfDb)
		{
				ret = 0x01;
		}

		fclose(pfDb);

		return ret;
}

char*	GetRuleDBVersionIF(char* buf)
{
		char *szVersion = GetDeviceConfig(RULE_DB_VERSION_KEY);
		if (szVersion && strlen(szVersion))
		{  		
				strncpy(buf, szVersion, SIZE_16B-1);	
		}
		else
		{
				strncpy(buf, "0", SIZE_16B-1);	
		}

		return buf;
}
void	SetRuleDBVersionIF(char* buf)
{
		if (0x00 == buf)
				return;

		SaveDbVersion(buf);
}


char*	GetRuleDBPathIF(char* buf)
{

		if (0x00 == buf)
				return 0x00;

		if (IsRuleDbCreated())
		{
				strncpy(buf, RULE_DB_PATH, strlen(RULE_DB_PATH));
		}


		return buf;
}

/*
 *
 * Make sure device is not in debounce test
 */

static ithread_t sensorDebounceHandle = -1;
int IsSensorDebounceTime()
{
		int ret = 0x00;

		LockLED();

		if (-1 != sensorDebounceHandle)
		{
				ret = 0x01;
		}

		UnlockLED();

		return ret;
}


void *RemoveSensorDebounce(void *args)
{
                tu_set_my_thread_name( __FUNCTION__ );
		pluginUsleep(20000000);
		APP_LOG("timer: network", LOG_DEBUG, "sensor debounce monitor expires");
		LockLED();
		sensorDebounceHandle = -1;
		UnlockLED();
		int status;
		ithread_exit(&status);
}


int CreateSensorDebounceMonitor()
{
		LockLED();
		if (-1 != sensorDebounceHandle)
		{
				ithread_cancel(sensorDebounceHandle);
				sensorDebounceHandle = -1;
				APP_LOG("timer: network", LOG_DEBUG, "sensor debounce monitor removed");
		}

		ithread_create(&sensorDebounceHandle, NULL, RemoveSensorDebounce, NULL);
		ithread_detach(sensorDebounceHandle);

		UnlockLED();

		APP_LOG("timer: network", LOG_DEBUG, "new sensor debounce monitor created");
		return 0;
}


void RestartRule4DST()
{

		//- Calculate now time to deal with edge case of 1:00 AM (DST OFF) and 3:00 AM
		time_t rawTime;
		struct tm * timeInfo;
		time(&rawTime);
		timeInfo = localtime (&rawTime);
		APP_LOG("timer: DST", LOG_DEBUG, "Local time hour:%d", timeInfo->tm_hour);


		if ((DST_TIME_NOW_OFF == timeInfo->tm_hour) || 
						(DST_TIME_NOW_ON == timeInfo->tm_hour) ||
						(DST_TIME_NOW_ON_2 == timeInfo->tm_hour)    //TODO: need to handle similar case of chatham isl. NZ, if gemtek will add support for it
			 )
		{
				g_iDstNowTimeStatus = timeInfo->tm_hour;
				APP_LOG("timer: DST", LOG_DEBUG, "g_iDstNowTimeStatus:%d", g_iDstNowTimeStatus);
		}
		else
		{
				g_iDstNowTimeStatus = 0x00;
		}


		APP_LOG("Rule", LOG_DEBUG, "DST changed, rule to restart to get executed again");

		pMessage msg = createMessage(RULE_MESSAGE_RESTART_REQ, 0x00, 0x00);

		if (0x00 != msg)
		{
				SendMessage2App(msg);
		}	
}


char* GetOverridenStatusIF(char* szOutBuff)
{
		if (0x00 == szOutBuff)
				return 0x00;

		lockRule();
		strncpy(szOutBuff, szOverridenStatus, (SIZE_256B*2)-1);
		unlockRule();

		return szOutBuff;
}

/*	@ Function:
 *		ResetOverrideStatus

 *	@ Description:
 *		clean up the storage of override rule, this will be called upon rule get executed and notified
 since it means the executing rule not overridden
 *	@ Parameters:
 *
 *	
 *
 *
 */
void ResetOverrideStatus()
{
		memset(szOverridenStatus, 0x00, MAX_OVERRIDEN_STATUS_LEN);
}

/*	IsOverriddenStatus
 *
 *		Check device overridden status in case pushing to device for update
 *
 *
 *
 *
 *
 *
 ********************************************************************************************/
int  IsOverriddenStatus()
{
		int isOverridden = 0x00;
		lockRule();
		if (strlen(szOverridenStatus) > 0x00)

				isOverridden = 0x01;
		unlockRule();

		if (isOverridden)
				PushStoredOverriddenStatus();
		return 0;
}


void PushStoredOverriddenStatus()
{
		char* szCurState[1];

		szCurState[0x00] = (char*)malloc(MAX_RESP_LEN);
		memset(szCurState[0x00], 0x00, MAX_RESP_LEN);

		lockRule();
		strncpy(szCurState[0x00], szOverridenStatus, MAX_RESP_LEN-1);
		unlockRule();

		int size = strlen((const char *)szCurState);
		if (size > 0x00)
		{
				//- Reset the last one
				if ('#' == (int)szCurState[size - 1])
						szCurState[0x00][size - 1] = 0x00;
		}

		char* paramters[] = {"RuleOverrideStatus"} ;

		UpnpNotify(device_handle, SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].UDN, 
						SocketDevice.service_table[PLUGIN_E_EVENT_SERVICE].ServiceId, (const char **)paramters, (const char **)szCurState, 0x01);

		APP_LOG("Rule", LOG_DEBUG, "historical overridden status pushed:%s", szCurState[0x00]);

		free(szCurState[0x00]);
}


void Advertisement4TimeUpdate()
{
		if (!IsUPnPNetworkMode())
		{
				APP_LOG("UPnP", LOG_DEBUG, "Not home network mode, %s not executed", __FUNCTION__);
				return;
		}

		//- Send per service, 7 services, so no repeating required here
		UpnpSendAdvertisement(device_handle, default_advr_expire);

		APP_LOG("UPnP", LOG_DEBUG, "Not home network mode, %s executed", __FUNCTION__);
}


/*	Function:
 *		isTimeEventInOVerrideTable
 *	Description:
 *		Check the timer event in historical table or not
 *
 **/
int isTimeEventInOVerrideTable(int time, int action)
{
		int isInTable = 0x00;
		char szMatch[SIZE_64B];
		memset(szMatch, 0x00, sizeof(szMatch));
		snprintf(szMatch, sizeof(szMatch), "%d|%d", time, action);

		APP_LOG("Rule", LOG_DEBUG, "rule string to match: %s", szMatch);

		if ( (0x00 != strlen(szOverridenStatus)) && (0x00 != strstr(szOverridenStatus, szMatch)))
		{
				APP_LOG("Rule", LOG_DEBUG, "string to match found in historical overridden information: %s", szOverridenStatus);
				isInTable = 0x01;
		}
		else
		{
				if (0x00 != strlen(szOverridenStatus))
				{
						APP_LOG("Rule", LOG_DEBUG, "string to match NOT found in historical overridden information: %s", szOverridenStatus);	
				}
				else
				{
						APP_LOG("Rule", LOG_DEBUG, "No historical overridden information found");	
				}
		}

		return isInTable;
}

int setStatusTS (int status) {
		int timeStamp = (int) GetUTCTime();
		char pTS[SIZE_64B]={'\0'};

		APP_LOG("Rule", LOG_DEBUG, "CurrTS: %d",timeStamp);	

		memset (pTS, 0, sizeof(pTS));
		snprintf (pTS, sizeof(pTS), "%d", timeStamp);
		SetBelkinParameter(STATUS_TS, pTS);
		APP_LOG("UPnP", LOG_DEBUG, "pTS: %s",pTS);	

		setPluginStatusTS(timeStamp);
		return timeStamp;
}

int getStatusTSFlash () {
		char *pTS = GetBelkinParameter (STATUS_TS);
		int tsFlash = 0;

		if(pTS != NULL && (strlen(pTS) > 0)) 
		{
				APP_LOG("Rule", LOG_DEBUG, "CurrTS: %s",pTS);	
				tsFlash = atoi(pTS);
				setPluginStatusTS(tsFlash);
		}

#ifdef PRODUCT_WeMo_NetCam
		if (pTS)
				free(pTS);
#endif
		return tsFlash;
}

/**
 *	@brief SetRouterInfo: to set router information if there
 *		   is no setup, to make sure this is inline with from setup
 *
 *	@param szMac the mac address of router
 *	@param szSSID the ssid of the router
 *  
 *  @return void
 */
void SetRouterInfo(const char* szMac, const char* szSSID)
{    
		if (0x00 == szMac || 0x00 == szSSID)        
				return;    

		strncpy(g_routerMac, szMac, sizeof(g_routerMac)-1);    
		strncpy(g_routerSsid, szSSID, sizeof(g_routerSsid)-1);
}

/**
 * @brief initSerialRequest: to initial serial number request
 *        Note: in SNS, this request was included in SetApSSID, 
 *              it was NOT generic and NOT good for integration
 *
 * @return void
 */
void initSerialRequest()
{
		char szBuff[SIZE_64B];
		memset(szBuff, 0x00, sizeof(szBuff));

		char* szSerialNo = GetSerialNumber();
		if ((0x00 == szSerialNo) || (0x00 == strlen(szSerialNo)))
		{
				//-User default one
				szSerialNo = DEFAULT_SERIAL_NO;
		}

		strncpy(g_szSerialNo, szSerialNo, sizeof(g_szSerialNo)-1);

		APP_LOG("STARTUP: Device", LOG_DEBUG, "serial: %s", g_szSerialNo);
}
