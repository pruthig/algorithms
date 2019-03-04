/***************************************************************************
*
*
* coreInit.c 
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
#include "global.h"
#include <stdlib.h>
#include "fcntl.h"
#include "defines.h"
#include "fw_rev.h"
#include "ithread.h" 
#include "types.h"
#include "upnp.h"
#include "controlledevice.h"
#include "plugin_ctrlpoint.h"
#include "wifiHndlr.h"
#include "fwDl.h"
#include "utils.h"
#include "osUtils.h"
#include "httpsWrapper.h"
#include "logger.h"
#include "itc.h"
#include "gpio.h"
#include "remoteAccess.h"
#include "xgethostbyname.h"
#include "getRemoteMac.h"
#include "WemoDB.h"
#include "ipcUDS.h"
#include "pmortem.h"

#include 	"rule.h"
#include "watchDog.h"
#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include "thready_utils.h"
#include <sys/syscall.h>

#define WEMO_VERSION_GEMTEK_PROD   "WeMo_version"
#define BELKIN_DAEMON_SUCCESS "Belkin_daemon_success"
#define LAN_IPADDR "lan_ipaddr"
#define MY_FW_VERSION "my_fw_version"

#define SIGNED_PUBKEY_FILE_NAME	"WeMoPubKey.asc"
#define IMPORT_KEY_PARAM_NAME "import_pkey_name"

void disconnectFromRouter();


#ifndef _OPENWRT_
extern pthread_mutex_t lookupLock;
#else
extern char g_szBootArgs[SIZE_128B];
extern int g_PowerStatus;
extern void libNvramInit();
pthread_attr_t sysRestore_attr;

#define RALINK_GENERAL_READ             0x50

#endif

extern struct Command *front, *rear;
extern unsigned int g_queue_counter;
extern pthread_mutex_t g_queue_mutex;
extern pthread_mutex_t* arrPthreadMutex[];
extern pthread_attr_t wdLog_attr;
extern int pj_dst_data_os(int, char *, int);
ithread_t sendremoteupdstatus_thread = -1;

#ifdef __WIRED_ETH__
int g_bWiredEthernet;
extern void StartInetMonitorThread();
#endif


void releaseLocks()
{
    int i=0;
    for(i=0;(i<100 && arrPthreadMutex[i]);i++)
    {
	if((!osUtilsReleaseLock(arrPthreadMutex[i])))
	{
	    //APP_LOG("WiFiApp",LOG_DEBUG,"Successfully unlocked lock%d: %p...", i, arrPthreadMutex[i]);
	}
    }

    for(i=0;i< MAX_THREAD_NUMBER; i++)
    {
	if((!pthread_mutex_unlock(&ipcIFArray[i].mutex)))
	{
	    //APP_LOG("WiFiApp",LOG_DEBUG,"Successfully unlocked lock%d: %p...", i, &ipcIFArray[i].mutex);
	}
    }
}

static void handleSigSegv(int signum)
{
    uint32_t *up = &signum;
    unsigned int sp = 0;

#if defined (__mips__)
    asm volatile ("move %0, $sp" : "=r" (sp));

    APP_LOG("WiFiApp",LOG_ALERT,"[%d:%s] SIGNAL SIGSEGV [%d] RECEIVED, Best guess fault address: %08x, ra: %08x, sp: %08x", 
	(int)syscall(SYS_gettid), tu_get_my_thread_name(), signum, up[8], up[72], sp);

    pmortem_connect_and_send((uint32_t *) &signum, 4 * 1024);
#else
    APP_LOG("WiFiApp",LOG_ALERT,"[%d:%s] SIGNAL SIGSEGV [%d] RECEIVED, aborting...", 
	(int)syscall(SYS_gettid), tu_get_my_thread_name(), signum);
#endif
    pthread_exit(NULL);

    //abort();
}
static void handleSigAbrt(int signum)
{
    APP_LOG("WiFiApp",LOG_ERR,"[%d:%s] SIGNAL SIGABRT [%d] RECEIVED, Aborting.", (int)syscall(SYS_gettid), tu_get_my_thread_name(), signum);
    exit(0);
}
/* signal handler for wemoApp */
static void handleRtMinSignal(int signum)
{
	if(signum == SIGRTMIN)
	{
		APP_LOG("WiFiApp",LOG_ALERT,"SIGNAL [%d] RECEIVED ..",signum);	
#ifndef ROBUST_MUTEX
		/* calling release lock for various locks in the system so that if this dying thread has any of these locked,
		    it will be released. Observed that in case the dying thread holds a lock which is acquired later by any other
		    thread, it gets blocked on rt_sigsuspend. The correct way to handle this would have been to use 
		    pthread_mutexattr_setrobust() on encountering EOWNERDEAD and then call pthread_mutex_consistent followed by 
		    pthread_mutex_unlock in a lock operation. Since this is not available in current version of pthread library in
		    the gemtek SDK, using this approach to unlock any held locks before exiting.
		*/
		releaseLocks();
#endif
		pthread_exit(NULL);
	}
	else
	{
		APP_LOG("WiFiApp",LOG_ALERT,"UNEXPECTED SIGNAL RECEIVED [%d] ..",signum);	
	}
}
static void handleSigPipe(int signum)
{
    APP_LOG("WiFiApp",LOG_ALERT,"SIGNAL SIGPIPE [%d] RECEIVED ..",signum);
    return;
}
void setSignalHandlers(void)
{
   struct sigaction act, oldact;
   act.sa_flags = (SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART);
   act.sa_handler = handleRtMinSignal;

   if(sigaction(SIGRTMIN, &act, &oldact))
   {
	APP_LOG("WiFiApp",LOG_ERR,
		"sigaction failed... errno: %d", errno);
   }
   else
   {
	if(oldact.sa_handler == SIG_IGN)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGIGN");

        if(oldact.sa_handler == SIG_DFL)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGDFL");
   }

   act.sa_flags = (SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART);
   act.sa_handler = handleSigSegv;
   if(sigaction(SIGSEGV, &act, &oldact))
   {
	APP_LOG("WiFiApp",LOG_ERR,
		"sigaction failed... errno: %d", errno);
   }
   else
   {
	if(oldact.sa_handler == SIG_IGN)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGIGN");

        if(oldact.sa_handler == SIG_DFL)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGDFL");
   }
   act.sa_flags = (SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART);
   act.sa_handler = handleSigAbrt;
   if(sigaction(SIGABRT, &act, &oldact))
   {
	APP_LOG("WiFiApp",LOG_ERR,
		"sigaction failed... errno: %d", errno);
   }
   else
   {
	if(oldact.sa_handler == SIG_IGN)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGIGN");

        if(oldact.sa_handler == SIG_DFL)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGDFL");
   }
		act.sa_flags = (SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART);
   act.sa_handler = handleSigPipe;
   if(sigaction(SIGPIPE, &act, &oldact))
   {
	APP_LOG("WiFiApp",LOG_ERR,
		"sigaction failed... errno: %d", errno);
   }
   else
   {
	if(oldact.sa_handler == SIG_IGN)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGIGN");

        if(oldact.sa_handler == SIG_DFL)
		APP_LOG("WiFiApp",LOG_DEBUG,"oldact RTMIN: SIGDFL");
   }
}

#define MAX_CHANNELS 11
int selectQuietestApChannel()
{
	PMY_SITE_SURVEY pAvlAPList;
	int count=0,i,min,chan;
	int ApCnt[MAX_CHANNELS+1];


	memset(ApCnt, 0, sizeof(ApCnt));

	pAvlAPList = (PMY_SITE_SURVEY) malloc (sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE);
	if(!pAvlAPList) {
		APP_LOG("UPNP", LOG_DEBUG,"Malloc Failed...");
		return -1;
	}
	memset(pAvlAPList, 0, (sizeof(MY_SITE_SURVEY)*MAX_LEN_OF_BSS_TABLE));

	APP_LOG("UPNP", LOG_DEBUG,"Get List...");
	getCurrentAPList(pAvlAPList, &count);
	APP_LOG("UPNP", LOG_DEBUG,"List Size <%d>...",count);

	for (i=0;i<count;i++) {
		ApCnt[atoi((char *)pAvlAPList[i].channel)]++; 
	}

	min = ApCnt[1];
	chan = 1;

	/* find out the quietest channel */
	for(i=1;i<=MAX_CHANNELS;i++)
	{
		if(min > ApCnt[i])
		{
			min = ApCnt[i];
			chan = i;
		}
	}

	APP_LOG("UPNP", LOG_DEBUG,"Selected channel: <%d>, ApCnt: %d g_channelAp: <%d>...",chan, ApCnt[chan], g_channelAp);

	if(g_channelAp != chan) {
		char pCommand[SIZE_64B],chBuf[SIZE_16B];
		int ret;

		memset(pCommand, 0, SIZE_64B);
		memset(chBuf, 0, SIZE_16B);
		strncpy(pCommand, "Channel=",sizeof(pCommand)-1);
		snprintf(chBuf,sizeof(chBuf),"%d",chan);
		strncat(pCommand,chBuf,sizeof(pCommand) - strlen(pCommand) - 1);
		ret = wifiSetCommand (pCommand,INTERFACE_AP);
		if(ret < 0)
		{
			APP_LOG("NetworkControl", LOG_ERR, "%s - failed", pCommand);
			free (pAvlAPList);
			return FAILURE;
		}
	} 

	g_channelAp = chan;

	free (pAvlAPList);
	return 0;
}


#ifdef _OPENWRT_
#define FILE_BOOT_ARGS "/tmp/bootArgs"
/*
 * Function: parseBootArgs
 *  This parses the bootArgs which are written to the file by wemo.init
 *  Fetches the current state of the switch
 *
 * Done for the Story: 2187, To restore the state of the switch before power failure
 *
 */
static int parseBootArgs()
{
    FILE *fp;
    char line[SIZE_128B] = {'\0'};
    char *p = NULL, *name, *value;
    int i=0;
    char ch;

    memset(g_szBootArgs, '\0', SIZE_128B);

    fp = fopen(FILE_BOOT_ARGS, "rb");
    if (!fp)
    {
	perror("File opening error");
	return 0;
    }

    while (fgets(line, SIZE_128B, fp))
	if ((p = strstr(line, "boot_A_args")))
	    break;

    fclose(fp);

    name = strtok(line,"=");
    if(name)
    {
	value = line + strlen(name) + 1;

	if(value[strlen(value)-1] == '\n')
	    value[strlen(value)-1] = '\0';

	if((p = strstr(value, "switchStatus")))
	{
	    p = p + strlen("switchStatus") + 1;
	    ch = p[0];
	    g_PowerStatus = ch - '0';

	    for(i=strlen(value)-1; i>0; i--)
	    {
		if(value[i] == ' ')
		{
		    value[i] = '\0';
		    break;
		}
	    }
	}

	if(strstr(value, "root"))
	    strncpy(g_szBootArgs, value, sizeof(g_szBootArgs)-1);
    }
    return 0;
}
#endif

extern void initdevConfiglock();

void restoreRelayState()
{
	FILE * pRelayFile = 0x00;
#if defined(PRODUCT_WeMo_Insight) && (defined(BOARD_EVT) || defined(BOARD_PVT))
//#if defined(PRODUCT_WeMo_Insight)
        char* szRelayPath = "/proc/RELAY_LED";
#else
	char* szRelayPath = "/proc/GPIO9";
#endif
	char szflag[4];
	int command = 0;
	char* pResult = NULL;

	pRelayFile = fopen(szRelayPath, "r");
	if (pRelayFile == 0x00)
	{
		APP_LOG("InitializePowerLEDState:", LOG_DEBUG, "Error on Open file for read: %s ", szRelayPath);
		return;
	}

	memset(szflag, 0x00, sizeof(szflag));
	pResult = fgets(szflag, sizeof(szflag), pRelayFile);
	if (pResult)
	{
		fclose(pRelayFile);
		command = atoi(szflag);
		LockLED();
		setPower(!command);
		SetCurBinaryState(!command);
                UnlockLED();
		APP_LOG("InitializePowerLEDState", LOG_DEBUG, "BOOT TIME BINARY STATE: %d , power set : %d", command, !command);
	}    
	else
	{   
		APP_LOG("InitializePowerLEDState", LOG_DEBUG, "READ GPIO RETURNED NULL");
		fclose(pRelayFile);
	}
}

int startCtrlPoint()
{
	if(DEVICE_SENSOR == g_eDeviceType)
	{
		APP_LOG("CtrlPt", LOG_DEBUG, "Sensor Start Plugin Control Point");
		return TRUE;
	}
	else if (DEVICE_SOCKET == g_eDeviceType)
	{
		if ((0x00 == atoi(g_szRestoreState)) && (0x00 == strlen(g_szHomeId) ) && (0x00 == strlen(g_szPluginPrivatekey)))
		{
			APP_LOG("CtrlPt", LOG_DEBUG, "Socket Start Plugin Control Point");
			return TRUE;
		}
	}

	return FAILURE;
}

int startRemoteAccessReRegister()
{	
	char routerMac[MAX_MAC_LEN];
	char routerssid[SIZE_32B];

	memset(routerMac, 0, MAX_MAC_LEN);
	memset(routerssid, 0, SIZE_32B);

	getRouterEssidMac (routerssid, routerMac, INTERFACE_CLIENT);
	if(strlen(gGatewayMAC) > 0x0)
	{
		memset(routerMac, 0, sizeof(routerMac));
		strncpy(routerMac, gGatewayMAC, sizeof(routerMac)-1);
	}


	if(0x01 == atoi(g_szRestoreState))
	{
		return TRUE;
	}

	else if(0x00 == atoi(g_szRestoreState))
	{
		if((strlen(g_szHomeId) == 0x0) && (strlen(g_szPluginPrivatekey) == 0x0))
		{
			APP_LOG("ReReg", LOG_DEBUG, "main: Remote Access is not Enabled...");
			return FALSE;
		}
		else
		{
			if( (strcmp(g_routerMac, routerMac) != 0) && (g_routerSsid!=NULL && strlen (g_routerSsid) > 0) )
			{
				APP_LOG("ReReg", LOG_DEBUG, "Remote Access is not Enabled...");
				return TRUE;
			}
		}
	}

	return FALSE;
}

void HandleHardRestoreAndCtrlPoint(int forceStart, char* ip_address)
{
	if(forceStart || startCtrlPoint())
	{
		APP_LOG("CtrlPt", LOG_DEBUG, "Starting Plugin Ctrl point, forceStart: %d", forceStart);
		StartPluginCtrlPoint(ip_address, 0x00);	
		EnableContrlPointRediscover(TRUE);
	}

	if(startRemoteAccessReRegister())
	{
		APP_LOG("ReReg", LOG_DEBUG, "Starting Remote Re-register");
		createAutoRegThread();
	}
}


void startUPnP(int forceEnableCtrlPoint)
{
	char* ip_address = NULL;
	unsigned int port = 0;
	//-How to make sure it connects to home network?
	if(getCurrentClientState())
	{	
		ip_address = wifiGetIP(INTERFACE_CLIENT);
		APP_LOG("UPNP", LOG_DEBUG, "################### UPNP on router: %s ###################", ip_address);
		ControlleeDeviceStart(ip_address, port, desc_doc_name, web_dir_path);

		UpdateUPnPNetworkMode(UPNP_INTERNET_MODE);
		HandleHardRestoreAndCtrlPoint(forceEnableCtrlPoint, ip_address);
	}
	else
	{
		ip_address = wifiGetIP(INTERFACE_AP);
		APP_LOG("UPNP", LOG_DEBUG, "################### UPNP on local:%s ###################", ip_address);
		ControlleeDeviceStart(ip_address, port, desc_doc_name, web_dir_path);
		UpdateUPnPNetworkMode(UPNP_LOCAL_MODE);
		EnableSiteSurvey();
	}
}

#ifdef _OPENWRT_
void *systemRestore(void *args)
{
    int restoreValue;
    int err, fd;

    tu_set_my_thread_name( __FUNCTION__ );

    if( (fd = open("/dev/gpio", O_RDWR)) < 0 )
    {
	APP_LOG("WiFiApp",LOG_DEBUG, "Open /dev/gpio failed");
	return (void *)-1;
    }

    while(1){
	err = ioctl(fd, RALINK_GENERAL_READ, (void *)&restoreValue);
	if( err < 0 ){
	    APP_LOG("WiFiApp",LOG_DEBUG, "Ralink Read byte failed ");
	    close(fd);
	    return (void *)err;
	}

	usleep(500000);
	if(restoreValue == 1) {
	    correctUbootParams();
	    resetNetworkParams();
	    clearMemPartitions();
	    ClearRuleFromFlash();
	    resetToDefaults();
	    usleep(500000);
	    system("nvram restore");
	    APP_LOG("UPNP", LOG_DEBUG,"System rebooting........");
	    pluginUsleep(2000000);
	    system("reboot");
	    break;
	}
    }
}
#endif

void initCoreThreads()
{
	//Check remote access and initialize NAT
	ithread_t remoteinit_thread;
	char *fwUpURLStr = NULL;
	FirmwareUpdateInfo fwUpdInf;
	int tsFlash = 0;

	fwUpURLStr = GetBelkinParameter("FirmwareUpURL");
	if(fwUpURLStr && strlen(fwUpURLStr)!=0) {
		memset(&fwUpdInf, 0x00, sizeof(fwUpdInf));
		strncpy(fwUpdInf.firmwareURL, fwUpURLStr, sizeof(fwUpdInf.firmwareURL)-1);
		StartFirmwareUpdate(fwUpdInf);
	}

	if ((0x00 != strlen(g_szHomeId) ) && (0x00 != strlen(g_szPluginPrivatekey)) && (atoi(g_szRestoreState) == 0x0)) {
		gpluginRemAccessEnable = 1;
#if !defined(PRODUCT_WeMo_Baby)
		tsFlash = getStatusTSFlash ();
		if(tsFlash == 0) 
			APP_LOG("UPNP",LOG_DEBUG, "$$$$$$$$$NO PREVIOUS NOTIFICATION$$$$$$$$$\n");

		remoteAccessUpdateStatusTSParams (0xFF);

		ithread_create(&remoteinit_thread, NULL, remoteAccessInitThd, NULL);
		ithread_detach (remoteinit_thread);
#endif

	}
#if !defined(PRODUCT_WeMo_Baby)
	ithread_t remotereinit_thread;
	ithread_create(&remotereinit_thread, NULL, remoteInitNatClient24, NULL);
	ithread_detach (remotereinit_thread);

	ithread_create(&sendremoteupdstatus_thread, NULL, sendRemoteAccessUpdateStatusThread, NULL);
	ithread_detach (sendremoteupdstatus_thread);

	/* create send notification monitor thread */
	ithread_t mon_sendremoteupdstatus_thread;
	ithread_create(&mon_sendremoteupdstatus_thread, NULL, sendRemAccessUpdStatusThdMonitor, NULL);
	ithread_detach (mon_sendremoteupdstatus_thread);

	ithread_t sendConfigChange_thread;
	ithread_create(&sendConfigChange_thread, NULL, sendConfigChangeStatusThread, NULL);
	ithread_detach (sendConfigChange_thread);

	ithread_t rcvSendstatusRsp_thread;
	ithread_create(&rcvSendstatusRsp_thread, NULL, rcvSendstatusRspThread, NULL);
	ithread_detach (rcvSendstatusRsp_thread);
#endif

#ifdef PRODUCT_WeMo_Baby
	remoteAccessUpdateStatusTSParams (0xFF);
	
	ithread_create(&sendremoteupdstatus_thread, NULL, sendRemoteAccessUpdateStatusThread, NULL);
	ithread_detach (sendremoteupdstatus_thread);

	/* create send notification monitor thread */
	ithread_t mon_sendremoteupdstatus_thread;
	ithread_create(&mon_sendremoteupdstatus_thread, NULL, sendRemAccessUpdStatusThdMonitor, NULL);
	ithread_detach (mon_sendremoteupdstatus_thread);

#endif
	ithread_t logFile_thread;
	APP_LOG("UPNP",LOG_DEBUG, "***************LOG Thread created***************\n");
	pthread_attr_init(&wdLog_attr);
	pthread_attr_setdetachstate(&wdLog_attr, PTHREAD_CREATE_DETACHED);
	ithread_create(&logFile_thread, &wdLog_attr, uploadLogFileThread_6Hours, NULL);

#ifdef _OPENWRT_
	ithread_t systemRestore_thread;
	pthread_attr_init(&sysRestore_attr);
	pthread_attr_setdetachstate(&sysRestore_attr, PTHREAD_CREATE_DETACHED);
	ithread_create(&systemRestore_thread, &sysRestore_attr, systemRestore, NULL);
	

	parseBootArgs();
#endif

}


void core_init_late(int forceEnableCtrlPoint)
{
	setSignalHandlers();

#ifndef _OPENWRT_ 
	char lanIp[SIZE_64B];
	char *lanIp1 = GetBelkinParameter("lan_ipaddr");  
	int indx = strlen(WAN_IP_ADR);

	memset (lanIp,0,SIZE_64B);

	memcpy (lanIp, lanIp1,indx);    
	lanIp[indx] = '\0';
	APP_LOG("UPNP",LOG_DEBUG, "ip: %s\n",lanIp);
	if(strcmp(WAN_IP_ADR,lanIp) != 0) {
		APP_LOG("UPNP",LOG_DEBUG, "#########CHANGING THE WEMO WAN IP...SYSTEM WILL REBOOT##############\n" );
		SetBelkinParameter(LAN_IPADDR, WAN_IP_ADR);
		sleep(40);
		APP_LOG("UPNP", LOG_ALERT, "CHANGING THE WEMO WAN IP");
		system ("reboot");
		//NOT REACHED
		return;
	}
#endif

	initIPC();
	g_szBuiltFirmwareVersion = FW_REV;
	g_szBuiltTime			 = BUILD_TIME;

	{
		char buf[SIZE_256B];
		char *ver=NULL;

		memset (buf,0,SIZE_256B);

		APP_LOG("UPNP",LOG_DEBUG, "Update NVRAM with FW Details..." );
		SetBelkinParameter(WEMO_VERSION_GEMTEK_PROD, FW_REV1);
		strncpy (buf,"\"",sizeof(buf)-1);

#ifndef _OPENWRT_
		if((ver = GetBelkinParameter("my_fw_version")))
			strncat (buf,ver, sizeof(buf));
		else
			strncat (buf, "MY_FW_VER_NULL", sizeof(buf));
		strncat (buf," ", sizeof(buf) - strlen(buf)-1);
		strncat (buf, FW_REV1, sizeof(buf)- strlen(buf)-1);
		strncat (buf,"\"", sizeof(buf)- strlen(buf)-1);
		SetBelkinParameter(MY_FW_VERSION, buf);
#endif
	}
#if defined(PRODUCT_WeMo_Baby)
        BMSetAppSSID();
#else
	SetAppSSID();
#endif
	SetAppSSIDCommand();

	initDeviceUPnP();

	createPasswordKeyData();

	if (DEVICE_SOCKET == g_eDeviceType)
	{
		initLED();
#if defined(LONG_PRESS_SUPPORTED)
		initLongPressLock();
#endif
		ithread_create(&power_thread, NULL, PowerButtonTask, NULL);
		ithread_create(&relay_thread, NULL, RelayControlTask, NULL);
		ithread_detach (power_thread);
		ithread_detach (relay_thread);
		ithread_create(&ButtonTaskMonitor_thread, NULL, ButtonTaskMonitorThread, NULL);
		ithread_detach (ButtonTaskMonitor_thread);
	}

	wifiGetChannel(INTERFACE_AP, &g_channelAp);
	selectQuietestApChannel();
	APP_LOG("UPNP", LOG_DEBUG, "################### ra0 is on Channel: %d ###################", g_channelAp);

#ifdef __WIRED_ETH__
	if(g_bWiredEthernet) {
		StartInetMonitorThread();
	}
	else
#endif
	{
	initWiFiHandler ();
	startUPnP(forceEnableCtrlPoint);
	}

	if (DEVICE_SENSOR== g_eDeviceType)
	{
		StartSensorTask();		
		ithread_create(&SensorTaskMonitor_thread, NULL, SensorTaskMonitorThread, NULL);
		ithread_detach (SensorTaskMonitor_thread);
	}

  initNATCore(0);
	initCoreThreads();
	char *pszRespXML = CreateManufactureData();
  	if(pszRespXML)
  		free (pszRespXML);
}


void pjUpdateDstTz()
{
	char ltimezone[MAX_RVAL_LEN];
	int index = 0;

	memset(ltimezone, 0x0, sizeof(ltimezone));
	snprintf(ltimezone, sizeof(ltimezone),"%f", g_lastTimeZone);
	index = getTimeZoneIndexFromFlash();
	APP_LOG("UPNP", LOG_DEBUG,"set pj dst data index: %d, lTimeZone:%s gDstEnable: %d success", index, ltimezone, gDstEnable); 
	pj_dst_data_os(index, ltimezone, gDstEnable);
}

void core_init_early()
{

#ifdef _OPENWRT_
	char szBuff[MAX_DBVAL_LEN];
	char serBuff[MAX_DBVAL_LEN];
	char* DFT_SERIAL_NO = "0123456789";

	/* 
	 * libNvramInit initialize the semaphore in OpenWRT NVRAM.
	 * This should be called before any APP_LOG call, otherwise OpenWRT PVT build will break
	 */
	libNvramInit();
#endif

#ifdef __WIRED_ETH__
// sh: read wan_ether_name from nvram.  If it's "vlan1" then we're
// running in wired Ethernet mode, otherwise assume normal WiFi mode
	 {
		 char *WanEtherName = GetBelkinParameter("wan_ether_name");
		 g_bWiredEthernet = 0;

		 if(WanEtherName != NULL) {
			 APP_LOG("REMOTEACCESS",LOG_DEBUG,"wan_ether_name: %s",WanEtherName);
			 if(strcmp(WanEtherName,"vlan1") == 0) {
				 g_bWiredEthernet = 1;
				 APP_LOG("REMOTEACCESS",LOG_DEBUG,"Wired Ethernet mode");
			 }
		 }
	 }
#endif

	initFWUpdateStateLock();    
	initCommandQueueLock();
	initdevConfiglock();
	initRemoteAccessLock();
	initHomeIdListLock();
	setCurrFWUpdateState(atoi(FM_STATUS_DEFAULT));    //setting to default
	webAppInit(0); //call it when no other thread exists in wemoApp

	//pluginOpenLog (PLUGIN_LOGS_FILE, CONSOLE_LOGS_SIZE); 
#ifdef _OPENWRT_
	APP_LOG("WiFiApp",LOG_DEBUG, "\n #########Removing /tmp/*.starting files\n");
	system("rm -f /tmp/*.starting");

	memset(serBuff, 0x0, MAX_DBVAL_LEN);
	getNvramParameter(serBuff);

	if((0x00 != strlen(serBuff)) && (0x00 == strcmp(serBuff, DFT_SERIAL_NO))) {

		char* szSerialNo = GetSerialNumber();
		if ((0x00 != szSerialNo) && (0x00 != strlen(szSerialNo))) {
			memset(szBuff, 0x0, MAX_DBVAL_LEN);

			snprintf(szBuff, MAX_DBVAL_LEN, "fw_setenv SerialNumber %s", szSerialNo);
			system(szBuff);
		}
	}
#else
	SetBelkinParameter(BELKIN_DAEMON_SUCCESS, "1");
#endif

#if defined(PRODUCT_WeMo_Baby)
        StartIpcUdsServer();
#endif

	SetBelkinParameter(IMPORT_KEY_PARAM_NAME, SIGNED_PUBKEY_FILE_NAME);

	initWatchDog();
	//disconnect from router to start afresh in case of both reboot and restart of wemoApp
	disconnectFromRouter();

	//- reset possible 
	SetBelkinParameter(SETTIME_SEC, "");
	remoteCheckAttributes();
#ifdef SIMULATED_OCCUPANCY
	simulatedOccupancyInit();
#endif
#if !defined(PRODUCT_WeMo_Baby)
	/* pj_dst_data_os is to propagate DST information to PJ lib, but this call to pj_dst_data_os may not require here later , will review and update */
	pjUpdateDstTz();
	initRemoteUpdSync();
	initRemoteStatusTS();
	setNotificationStatus (1);
#endif

#ifdef PRODUCT_WeMo_Baby
	initRemoteStatusTS();
	setNotificationStatus (1);
#endif

#ifndef _OPENWRT_
	osUtilsCreateLock(&lookupLock);	
#endif
}


