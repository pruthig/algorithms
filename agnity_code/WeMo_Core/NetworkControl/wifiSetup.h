/***************************************************************************
*
*
* wifiSetup.h
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
#ifndef _WIFI_SETUP_H_
#define _WIFI_SETUP_H_
#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#include "defines.h"

//Constants
#define WIFI_MAXSTR_LEN	    SIZE_64B

#define CHANNEL_LEN	    4
#define SSID_LEN	    WIFI_MAXSTR_LEN + 1
#define BSSID_LEN	    20
#define SECURITY_LEN	    23
#define SIGNAL_LEN	    9
#define WMODE_LEN	    8
#define EXTCH_LEN	    7
#define NT_LEN		    3
#define HIDDEN_LEN	    3
#define WPS_LEN		    4
#define DPID_LEN	    5

#define SSID_LOC	    CHANNEL_LEN
#define BSSID_LOC	    SSID_LOC + SSID_LEN
#define SECURITY_LOC	    BSSID_LOC + BSSID_LEN
#define SIGNAL_LOC	    SECURITY_LOC + SECURITY_LEN
#define WMODE_LOC	    SIGNAL_LOC + SIGNAL_LEN
#define EXTCH_LOC	    WMODE_LOC + WMODE_LEN
#define NT_LOC		    EXTCH_LOC + EXTCH_LEN
#define HIDDEN_LOC	    NT_LOC + NT_LEN
#define WPS_LOC		    HIDDEN_LOC + HIDDEN_LEN
#define DPID_LOC	    WPS_LOC + WPS_LEN
#define SITE_SURVEY_HDR_LEN	160


#define RTPRIV_IOCTL_SET          (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_STATISTICS   (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_GSITESURVEY  (SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_GET_MAC_TABLE (SIOCIWFIRSTPRIV + 0x0F)
#define RT_OID_APCLI_RSSI         0x0680

#define MIN_RSSI			    -50
#define NORMAL_RSSI			    -39
#define SCAN_MAX_DATA			    SIZE_8192B
#define MAX_LEN_OF_BSS_TABLE		    SIZE_64B
#define CONNECTION_IN_PROGRESS		    -2
#define NETWORK_NOT_FOUND		    -1
#define STATE_DISCONNECTED		    0
#define STATE_CONNECTED			    1
#define STATE_PAIRING_FAILURE_IND	    2
#define AP_MIXED_MODE_STR		    "11b/g/n"

#define STATE_INTERNET_NOT_CONNECTED	    3
#define STATE_IPADDR_NEGOTIATION_FAILED	    4
// DEFAULT GW DS
#define RBUFSIZE			    SIZE_8192B

#ifndef IF_NAMESIZE
#  ifdef IFNAMSIZ
#    define IF_NAMESIZE IFNAMSIZ
#  else
#    define IF_NAMESIZE 16
#  endif
#endif

#ifdef _OPENWRT_
#define UDHCPC_CMD  "udhcpc -i apcli0 &"
#else
#define UDHCPC_CMD  "udhcpc -i apcli0 -s /bin/udhcpc.sh &"
#endif
#define KILO    1e3
#define MEGA    1e6
#define GIGA    1e9

#define WAN_NETMASK "wan_netmask"
#define WAN_IPADDR "wan_ipaddr"

#define PRODUCT_VARIANT_NAME "WeMo_Variant"
typedef struct wireless_config
{
  int           has_nwid;
  int           has_freq;
  double        freq;                   /* Frequency/channel */
  int           freq_flags;
  int           has_key;
  int           key_size;               /* Number of bytes */
  int           key_flags;              /* Various flags */
  int           has_essid;
  int           essid_on;
  int           has_mode;
  int           mode;                   /* Operation mode */
} wireless_config;

typedef struct wireless_info
{
  struct wireless_config        b;      /* Basic information */

  int           has_sens;
  int           has_nickname;
  int           has_ap_addr;
  int           has_bitrate;
  int           has_rts;
  int           has_frag;
  int           has_power;
  int           has_txpower;
  int           has_retry;

  /* Stats */
  int           has_stats;
  int           has_range;

  /* Auth params for WPA/802.1x/802.11i */
  int           auth_key_mgmt;
  int           has_auth_key_mgmt;
  int           auth_cipher_pairwise;
  int           has_auth_cipher_pairwise;
  int           auth_cipher_group;
  int           has_auth_cipher_group;
} wireless_info;

typedef struct _WIFI_COUNTERS {
    unsigned long TxSuccessTotal;;
    unsigned long TxSuccessWithRetry;
    unsigned long TxFailWithRetry;
    unsigned long RtsSuccess;
    unsigned long RtsFail;
    unsigned long RxSuccess;
    unsigned long RxWithCRC;
    unsigned long RxDropNoBuffer;
    unsigned long RxDuplicateFrame;
    unsigned long FalseCCA;
    unsigned long RssiA;
    unsigned long RssiB;
} WIFI_STAT_COUNTERS,*PWIFI_STAT_COUNTERS;

typedef struct _MY_SITE_SURVEY
{
	unsigned char channel[CHANNEL_LEN];
	char ssid[SSID_LEN];
	unsigned char bssid[BSSID_LEN];
	char security[SECURITY_LEN];
	char signal[SIGNAL_LEN];
	char WMode[WMODE_LEN];
	unsigned char ExtCH[EXTCH_LEN];
	char NT[NT_LEN];
	char Hidden[HIDDEN_LEN];
	unsigned char WPS[WPS_LEN];
	unsigned char DPID[DPID_LEN];
} MY_SITE_SURVEY, *PMY_SITE_SURVEY;

extern char *pCommandList[];
//Data Structures

typedef struct WIFI_PAIR_PARAMS_T {
        int channel;
	char SSID[WIFI_MAXSTR_LEN];
	char AuthMode[WIFI_MAXSTR_LEN];
	char EncrypType[WIFI_MAXSTR_LEN];
	char signal [WIFI_MAXSTR_LEN];
	char Key[PASSWORD_MAX_LEN];
}WIFI_PAIR_PARAMS, *PWIFI_PAIR_PARAMS;

char gateway[SIZE_256B];
typedef struct ROUTE_INFO{
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
}ROUTE_INFO,*PROUTE_INFO;

/** Typedef enum for server environment */
typedef enum {
        E_SERVERENV_PROD = 0x0,
        E_SERVERENV_CI,
        E_SERVERENV_QA,
        E_SERVERENV_DEV,
        E_SERVERENV_STAG,
        E_SERVERENV_ALPHA,
        E_SERVERENV_CUSTOM,
        E_SERVERENV_OTHER
} SERVERENV;

extern int g_channelAp;
extern pthread_mutex_t   s_client_state_mutex;
extern int gWiFiClientCurrState;
extern SERVERENV g_ServerEnvType;
extern int gSignalStrength;

//Prototypes
int WifiInit();
int wifiCheckConfigAvl();
int wifiCheckConfigAvl();
int wifiSetCommand (char *pCommand, char *pInterface);
int wifiGetStatus (char *pEssid, char *pApmac, char *pInterface);
int wifiPair (PWIFI_PAIR_PARAMS pWifiParams, char *pInterface);
int wifiGetNetworkList(PMY_SITE_SURVEY pAvlAPList, char *pInterface, int *pListCount);
int wifiConnectControlPt (PWIFI_PAIR_PARAMS pWifiParams,char *pInterface);
int wifiTestConnection (char *pInterface,int count,int dhcp) ;
int wifiGetStats (char *pInterface, PWIFI_STAT_COUNTERS pWiFiStats);
int wifiChangeClientIfState (int state) ;
int wifiChangeAPIfState (int state) ;
int wifisetSSIDOfAP (char *pSSID) ;
int wifisetChannelOfAP (char *nChannel);
int wifiGetAPIfState () ;
int IsNetworkMixedMode(char* szApSSID);
int wifiGetChannel(char *pInterface, int *pChannel);
int isAPConnected(char *pSSID); 
int isValidIp(void);
void StopDhcpRequest(void);


int getCompleteAPList (PMY_SITE_SURVEY,int *); 
int getCurrentClientState(void);
int wifiGetRSSI(char *pInterface);

#ifdef PRODUCT_WeMo_Baby
#define SetAppSSID BMSetAppSSID
#endif

#if defined(PRODUCT_WeMo_Baby) || defined(PRODUCT_WeMo_Streaming)
#define SetWiFiLED SetWiFiGPIOLED
void BMSetAppSSID();
int SetCurrentClientState(int curState);
void initClientStatus();
int getDevNetworkState (void);
int SyncAppWatchDogState(void);
int DisableAppWatchDogState(void);
int GetAppWatchDogState(void);
int ResetAppWatchDogState(void);
int getCurrentServerEnv (void) ;
int getCurrentSignalStrength (void) ;
double GetDeviceTimeZone();
#endif /*PRODUCT_WeMo_Baby*/

#endif //_WIFI_SETUP_H_
