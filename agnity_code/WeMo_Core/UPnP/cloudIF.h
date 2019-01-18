/***************************************************************************
*
*
* cloudIF.h
*
* Created by Belkin International, Software Engineering on Aug 8, 2011
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
#ifndef __CLOUDIF_H__
#define __CLOUDIF_H__
/**
 * - CloudUpdateRule
 * - char* WeeklyRuleList[7]
 *      Through Monday (index = 0x00) to Sunday (index = 0x06)
 *      Embedded Interface will take care the release of WeeklyRuleList, if any day
 *      is empty, please keep WeeklyRuleList[index] = 0x00;
 *
 * - Update rule to embedded device, interface itself will take care of data sync and
 *   protection
 *
 * - return:
 *      0x00: success
 *      otherwise: unsuccess
 ***********************************************/
int     CloudUpdateRule(char** WeeklyRuleList);

/**
 * - CloudGetRule
 * - char* WeeklyRuleList[7];
 *      Through Monday (index = 0x00) to Sunday (index = 0x06)
 *
 * - Get local rule from embedded, interface will take care of the memory allocated
 *   and caller (cloud client) should take care of memory release
 *
 * - return:
 *      0x00: success
 *      otherwise: unsuccess
 ***********************************************/
int     CloudGetRule(char** WeeklyRuleList);

/**
 * - CloudSetBinaryState
 *
 * - Set binary state
 *
 * - newState: ON(0x01), OFF(0x00)
 *
 * - return:
 *      0x00: success
 *      0x01: unsuccess
 *************************************************/
int     CloudSetBinaryState(int newState);

/**
 * - CloudGetBinaryState
 *
 * - Get binary state
 *
 * - newState: ON(0x01, motion detected), OFF(0x00, motion not detected)
 *
 * - return:
 *      0x00: success
 *      0x01: unsuccess
 *************************************************/
int     CloudGetBinaryState();


/**
 * - CloudGetRuleDBVersion
 *
 * - Get Rule DB version
 *
 *************************************************/
char* CloudGetRuleDBVersion(char* buf);

/**
 * - CloudSetRuleDBVersion
 *
 * - Set Rule DB version
 *
 *************************************************/
void CloudSetRuleDBVersion(char* buf);


/**
 * - CloudGetRuleDBPath
 *
 * - Get Rule DB version
 *
 *************************************************/
char* CloudGetRuleDBPath(char* buf);


/**
 * - BinaryStatusNotify
 *
 * - Cloud client needs to implement the sync and data protection
 *
 * - Called by local embedded when any status change
 *
 *
 */
int     DeviceBinaryStatusNotify(int newState);

int     DeviceRuleNotify(char** WeeklyRuleList);


char* GetOverriddenStatus(char* szOutBuff);

/**
 * - CloudUpgradeFwVersion
 *
 * - To Upgrade Firmware Version
 *
 * - dwldStartTime:
 *
 * - return:
 *      0x00: success
 *      0x01: unsuccess
 *************************************************/
int     CloudUpgradeFwVersion(int dwldStartTime, char *FirmwareURL, int fwUnSignMode);

#ifdef PRODUCT_WeMo_CrockPot
/**
 * - CloudSetCrockPotStatus
 *
 * - Set crockpot status
 *
 * - mode: OFF(0), WARM(0x2), LOW(0x3), HIGH(0x4)
 * - cooktime: in Minutes (0 to 1200)
 *
 * - return:
 *      0x00: success
 *      0x01: unsuccess
 *************************************************/
int CloudSetCrockPotStatus(int mode, int cooktime);

/**
 * - CloudGetCrockPotStatus
 *
 * - Get crockpot status
 *
 * - mode: OFF(0x0/0x30), IDLE(0x1/0x31), WARM(0x2/0x32), LOW(0x3/0x33), HIGH(0x4/0x34)
 * - cooktime: in minutes (0 to 1200)
 *
 * - return:
 *      0x00: success
 *      0x01: unsuccess
 *************************************************/
int CloudGetCrockPotStatus(int *mode, int *cooktime);
#endif  /*#ifdef PRODUCT_WeMo_CrockPot*/

#ifdef PRODUCT_WeMo_Light
/**
 * - CloudSetNightLightStatus
 *
 * - Set LS night light status
 *
 * - newState: HIGH(2), LOW(1), OFF(0)
 *
 * - return:
 *      0x00: success
 *      0x01: unsuccess
 *************************************************/
int CloudSetNightLightStatus(char *dimValue);
#endif

#endif /* CLOUDIF_H_ */
