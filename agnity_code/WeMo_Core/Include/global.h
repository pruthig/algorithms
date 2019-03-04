/***************************************************************************
*
*
* global.h
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
#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <bits/sockaddr.h>
#include <linux/types.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/ether.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#define SUCCESS 0


#define FAILURE -1
#define INVALID_PARAMS    (FAILURE-1)

#define TURN_ON 1
#define TURN_OFF 0

#define INTERFACE_AP		"ra0"
#define INTERFACE_CLIENT	"apcli0"
#define INTERFACE_INSTA		"ra1"
#define INTERFACE_BRIDGE	"ra2"
#define INTERFACE_BR		"br0"

#define WAN_IP_ADR    "10.22.22.1"

#define WIFI_BRIDGE_LIST	"BridgeList"
#define WIFI_CLIENT_SSID	"ClientSSID"
#define WIFI_CLIENT_PASS	"ClientPass"
#define WIFI_CLIENT_AUTH	"ClientAuth"
#define WIFI_CLIENT_ENCRYP	"ClientEncryp"
#define WIFI_AP_CHAN		"APChannel"
#define WIFI_ROUTER_MAC		"RouterMac"
#define WIFI_ROUTER_SSID	"RouterSsid"
#define SYNCTIME_LASTTIMEZONE	"LastTimeZone"
#define SYNCTIME_DSTSUPPORT     "DstSupportFlag"
#define LASTDSTENABLE		"LastDstEnable"
#define NOTIFICATION_VALUE      "NotificationFlag"
#define LAST_PUSH_TIME		"LastPushTime"
#define NAT_TYPE        "nat_type"
#define WD_DEFAULT_PUSH_TIME	21600 //6hours

#define CHAN_HIDDEN_NW 255
#define SETTIME_SEC "settime_sec"
#define DEVICE_NETWORK_STATE "dev_network_state"
#define APP_WD_STATUS "app_wd_status"
#define MAX_APSSID_LEN		30
#endif //_GLOBAL_H
