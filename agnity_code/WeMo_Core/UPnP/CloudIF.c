/***************************************************************************
*
*
* cloudIF.c
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
#include <stdlib.h>
#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#include "cloudIF.h"
#include "gpio.h"
#include "controlledevice.h"
#include "rule.h"
#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif

#ifdef SIMULATED_OCCUPANCY
extern int gSimulatedRuleRunning;
#endif
char* GetOverridenStatusIF(char* szOutBuff);
extern char g_NotificationStatus[MAX_RES_LEN];

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
int     CloudUpdateRule(char** WeeklyRuleList)
{
    int ret = 0x00;

#ifdef SIMULATED_OCCUPANCY
	unsetSimulatedData();
#endif
    if (0x00 != WeeklyRuleList)
    {
	ret = UpdateRule((const char**)WeeklyRuleList);
    }

    return ret;
}

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
int     CloudGetRule(char** WeeklyRuleList)
{
    //- Not implemented since rule Get by database
    return 0x00;
}

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
int CloudSetNightLightStatus(char *dimValue)
{
    return changeNightLightStatus(dimValue);
}
#endif


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
int     CloudSetBinaryState(int newState)
{

    setRemote("1");
    setActuation(ACTUATION_MANUAL_APP);
    int ret = ChangeBinaryState(newState);
    SetLastUserActionOnState(newState);
    if (0x00 == ret)
    {
	//- Notify phone
	/* To avoid duplicate local notification, being send from InternalLocalUpdate function */

	OverrideRule(ACTION_REMOTE_CONTROL);

#ifdef PRODUCT_WeMo_Insight
	if (0x01 == newState)
	    newState=0x08;
#endif
	//- Notify sensor
	InternalToggleUpdate(newState);
    }

    return ret;
}

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
int     CloudUpgradeFwVersion(int dwldStartTime, char *FirmwareURL, int fwUnSignMode)
{
    int retVal = 0x00, state = -1;
    FirmwareUpdateInfo fwUpdInf;

    state = getCurrFWUpdateState();
    if( (state == atoi(FM_STATUS_DOWNLOADING)) || (state == atoi(FM_STATUS_DOWNLOAD_SUCCESS)) || (state == atoi(FM_STATUS_UPDATE_STARTING)))
    {
	APP_LOG("REMOTE",LOG_ERR, "************Firmware Update Already in Progress...");
	return 0x00;
    }

    fwUpdInf.dwldStartTime = dwldStartTime;
		fwUpdInf.withUnsignedImage = fwUnSignMode;
    memset(fwUpdInf.firmwareURL, 0x00, sizeof(fwUpdInf.firmwareURL));
    strncpy(fwUpdInf.firmwareURL, FirmwareURL, sizeof(fwUpdInf.firmwareURL)-1);

    retVal = StartFirmwareUpdate(fwUpdInf);
    return retVal;
}

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
int     CloudGetBinaryState()
{

    int ret = GetDeviceStateIF();

    return ret;
}

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
#ifdef PRODUCT_WeMo_CrockPot
int CloudSetCrockPotStatus(int mode, int cooktime)

{
    int tempMode;

    tempMode = convertModeValue(mode,TO_CROCKPOT);

    /* At this location using the mutex is relevant? Since this is not in a thread (I am not sure)*/
    LockCrockpot();
    g_modeStatus = tempMode; /*Remove the upper nibble value */
    g_timeCookTime = cooktime;        
    APP_LOG("REMOTE",LOG_DEBUG, "Set Crockpot status done: %d mode, %d time\n",g_modeStatus,g_timeCookTime);	
    UnlockCrockpot();
    g_remoteSetStatus = 1;

    /* Cloud set crockpot status function is invoked from a remote access function
     * remote access function is a handler for the rx/tx data handler for the n/w operations
     * I am not sure rx/tx data hanlder will run as thread or not.
     * If it is not a thread invoking set functions here may create problems like only few instructions 
     * may be executed and remaing may never get executed - like only set mode & set time will never
     * Best way is to set the global parameters (use lock) and do the actual set operation in a thread
     */
    //int ret = SetCrockpotStatus(mode, cooktime);

    {
	//- Notify
	//InternalToggleUpdate(newState);
    }

    return 0;
}

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
int CloudGetCrockPotStatus(int *mode, int *cooktime)
{
    int ret = GetCrockpotStatus(mode, cooktime);

    return ret;
}
#endif
char* CloudGetRuleDBVersion(char* buf)
{
    return GetRuleDBVersionIF(buf);
}

/**
 * - CloudSetRuleDBVersion
 *
 * - Set Rule DB version
 *
 *************************************************/
void CloudSetRuleDBVersion(char* buf)
{
    SetRuleDBVersionIF(buf);
}


/**
 * - CloudGetRuleDBPath
 *
 * - Get Rule DB path
 *
 *************************************************/
char* CloudGetRuleDBPath(char* buf)
{
    return GetRuleDBPathIF(buf);
}


/**
 * - BinaryStatusNotify
 *
 * - Cloud client needs to implement the sync and data protection
 *
 * - Called by local embedded when any status change
 *
 *
 */
int     DeviceBinaryStatusNotify(int newState)
{
    return 0x00;
}

int     DeviceRuleNotify(char** WeeklyRuleList)
{
    return 0x00;
}


/**
 * @Function:
 GetOverriddenStatus
 * @Description:
 Called to get override status remotely
 * 
 * @Parameters:
 *  szOutBuff
 *	buffer from caller, allocated and dellocated by caller, the size should be 512 bytes
 *
 */
char* GetOverriddenStatus(char* szOutBuff)
{

    char* ret = 0x00;

    if (0x00 == szOutBuff)
    {
	return ret;
    }

    ret = GetOverridenStatusIF(szOutBuff);


    return ret;
}
