/***************************************************************************
*
*
* jardendevice.c
*
* Created by Belkin International, Software Engineering on Feb 6, 2013
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
#ifdef PRODUCT_WeMo_Jarden
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
#include "jarden.h"

#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#include "itc.h"
#include "fwDl.h"
#include "remoteAccess.h"
#include "pluginRemoteRegister.h"
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


extern void LocalBinaryStateNotify(int curState);
extern UpnpDevice_Handle device_handle;

void LocalJardenStateNotify(int jMode, int jDelayTime);

/*
 * Set the Jarden Device Status as requested by the remote cloud server.
 */
int SetJardenStatusRemote(int mode, int delaytime)
{
    int ret,newMode;
    int allowSetTime=FALSE;
    /* Mode value varies between 0x0 & 0x1
     */
    APP_LOG("REMOTE",LOG_DEBUG, "In SetJardenStatusRemote Function: mode:%d, time:%d\n", mode, delaytime);

    if (DEVICE_CROCKPOT == g_eDeviceType)
    {
        if( (MODE_IDLE != mode) ||
            (MODE_UNKNOWN != mode) )
            {
                if(APP_MODE_ERROR != mode) {
                    ret = HAL_SetMode(mode);
                if(ret >= 0) {
                    APP_LOG("REMOTE",LOG_DEBUG, "Crockpot Mode set to: %d\n", mode);
                }
                else {
                    APP_LOG("REMOTE",LOG_DEBUG, "Unable to Set Crockpot Mode to: %d, ERROR:%d\n", mode,ret);
                }
              } /* if(APP_MODE_ERROR... */
            } /* end of if(tmpMode== ... */
    } /* end of if(tmpMode== ... */
    else
    {
    if(MODE_UNKNOWN != mode) {
        ret = HAL_SetMode(mode);
        if(ret >= 0) {
            APP_LOG("REMOTE",LOG_DEBUG, "Jarden Device Mode set to: %d\n", mode);
        }
        else {
            APP_LOG("REMOTE",LOG_DEBUG, "Unable to Set Jarden Device Mode to: %d, ERROR:%d\n", mode,ret);
        }
    } /* end of if( mode== ... */
    }

    if (DEVICE_CROCKPOT == g_eDeviceType)
    {
        if((mode == MODE_LOW) || (mode == MODE_HIGH)) {
            allowSetTime = TRUE;
        }
        else {
            newMode = HAL_GetMode();
            if((newMode == MODE_LOW) || (newMode == MODE_HIGH)) {
                allowSetTime = TRUE;
            }
            else {
                allowSetTime = FALSE;
            }
        }
        if((APP_TIME_ERROR != delaytime) && (TRUE==allowSetTime)) {
            ret = HAL_SetCookTime(delaytime);
            if(ret >= 0) {
                APP_LOG("REMOTE",LOG_DEBUG, "Crockpot time set to: %d\n", delaytime);
            }
            else {
                APP_LOG("REMOTE",LOG_DEBUG, "Unable to Set Crockpot Cooktime to: %d, ERROR: %d\n", delaytime,ret);
            }
        } /* end of if(APP_TIME_ERROR... */
    }
    else
    {
    /* Delay setting is allowed only if device is powered on? */
    if((mode == MODE_ON) || (mode == MODE_OFF)) {
        allowSetTime = TRUE;
    }
    else {
        newMode = HAL_GetMode();
        if((newMode == MODE_ON) || (newMode == MODE_OFF)) {
            allowSetTime = TRUE;
        }
        else {
            allowSetTime = FALSE;
        }
    }

    if((0 < delaytime) && (MAX_DELAY_TIME >= delaytime) && (TRUE==allowSetTime)) {
        /* Set the time only if desired, otherwise don't do set */
        ret = HAL_SetDelayTime(delaytime);
        if(ret >= 0) {
            APP_LOG("REMOTE",LOG_DEBUG, "Jarden device delay set to: %d\n", delaytime);
        }
        else {
            APP_LOG("REMOTE",LOG_DEBUG, "Unable to Set Jarden Device delaytime to: %d, ERROR: %d\n", delaytime,ret);
        }
    } /* end of if(APP_TIME_ERROR... */
    }
    return 0;
}

/*
 * Get the current Jarden device Status as requested the remote cloud server.
 */
int GetJardenStatusRemote(int *mode, int *delaytime)
{
    int tempMode;
    if(DEVICE_CROCKPOT == g_eDeviceType)
    {
        *mode = HAL_GetMode();
        APP_LOG("JardenStatus",LOG_DEBUG, "GetJardenStatusRemote mode being returned is %d",*mode);

        tempMode = *mode;
        APP_LOG("JardenStatus",LOG_DEBUG, "GetJardenStatusRemote tempMode  is %d",tempMode);

        *mode = convertModeValue(tempMode,FROM_CROCKPOT);
        APP_LOG("JardenStatus",LOG_DEBUG, "convertModeValue mode being returned is %d",*mode);

        *delaytime = HAL_GetCookTime();
        APP_LOG("JardenStatus",LOG_DEBUG, "cooktime being returned is %d",*delaytime);
    }
    else
    {
    /* Mode value varies between 0 &  1
     * Max Delay can be 24hrs
     */

     *mode = HAL_GetMode();
     *delaytime = HAL_GetDelayTime();
    }

    return 0;
}

int convertModeValue(int mode, int flag)
{
    int retMode;
    if(TO_CROCKPOT == flag) {
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
/* SetJardenStatus function is to change the Jarden Device settings
 * This function make use of HAL_SetMode() & HAL_SetDelaytime() functions from HAL layer
 * pActionRequest - Handle to request
 *
 */
int SetJardenStatus(pUPnPActionRequest pActionRequest,
                     IXML_Document *request,
                     IXML_Document **out,
                     const char **errorString)
{
    int jardenMode;
    int tempmode, curMode;
    int delayTime;
    int retstatus,allowSetTime=FALSE;
    char szCurState[16];

    memset(szCurState, 0x00, sizeof(szCurState));

    if (NULL == pActionRequest || NULL == request)
    {
        APP_LOG("UPNPDevice", LOG_DEBUG, "SetJardenStatus: command paramter invalid");
        return SET_PARAMS_NULL;
    }
    /*else {
        APP_LOG("UPNPDevice", LOG_DEBUG, "SetJardenStatus: Begining of set operation");
    }*/

    char* parammode = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "mode");
    if (0x00 == parammode || 0x00 == strlen(parammode))
    {
        pActionRequest->ActionResult = NULL;
        pActionRequest->ErrCode = SET_PARAM_ERROR;
        /*UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Parameter Error");*/

        UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Parameter Error");

        APP_LOG("UPNPDevice", LOG_DEBUG, "SetCrockpotState: mode Parameter Error");

        return UPNP_E_SUCCESS;
    }
    char* paramtime = Util_GetFirstDocumentItem(pActionRequest->ActionRequest, "time");
    if (0x00 == paramtime || 0x00 == strlen(paramtime))
    {
        pActionRequest->ActionResult = NULL;
        pActionRequest->ErrCode = SET_PARAM_ERROR;
        /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Parameter Error");*/
        UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Parameter Error");

        APP_LOG("UPNPDevice", LOG_DEBUG, "SetCrockpotState: time Parameter Error");
        FreeXmlSource(parammode);
        return UPNP_E_SUCCESS;
    }

    /*APP_LOG("UPNPDevice", LOG_DEBUG, "SetJardenStatus: First Doc item completed");*/

    jardenMode = atoi(parammode);
    delayTime = atoi(paramtime);
    APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: jarden Mode received is: %d ", jardenMode);


    sprintf(szCurState, "%d", jardenMode);

    /*APP_LOG("UPNPDevice", LOG_DEBUG, "SetJardenStatus: Setting the Crockpot to %d mode with %d time", jardenMode,delayTime);  */

    if (DEVICE_CROCKPOT == g_eDeviceType)
    {
        /* Change the mode to crockpot values from 0, 1, 50, 51, 52 to maintain
        * cosistency between local & remote mode
        */
        tempmode = convertModeValue(jardenMode,TO_CROCKPOT);

	APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: temp Mode received is: %d ", tempmode);

        if(MODE_IDLE != tempmode) {
            APP_LOG("UPNPDevice", LOG_ERR, "Inside if(MODE_IDLE != tempmode)");

            if(APP_MODE_ERROR != tempmode) {

                APP_LOG("UPNPDevice", LOG_ERR, "Inside if(APP_MODE_ERROR != tempmode)");
                /* Call HAL API's to set mode and set time for a crockpot */
                retstatus = HAL_SetMode(tempmode);
                APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: retstatus = HAL_SetMode(tempmode) is : %d ", retstatus);
                if (0x00 == retstatus) {
                    /* Is this internal broadcast is required? */
                    pActionRequest->ActionResult = NULL;
                    pActionRequest->ErrCode = 0;

                    UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                            CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", szCurState);

                    APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: Mode changed to: %d a.k.a %d mode", jardenMode, tempmode);
                }
                else {
                    pActionRequest->ActionResult = NULL;
                    pActionRequest->ErrCode = SET_CPMODE_FAILURE;

                    UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                            CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error");

                    APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: Mode not changed");
                }
            } /* if(APP_MODE_ERROR... */
        } /* end of if(MODE_IDLE... */
        else { /* Set to IDLE mode is not allowed */
            APP_LOG("Jarden", LOG_ERR, "SETJARDENSTATUS: Invalid mode value");
        }

        sprintf(szCurState, "%d", delayTime);

        /* Allow setting of time only in LOW or HIGH temperature mode */
        if((tempmode == MODE_LOW) || (tempmode == MODE_HIGH)) {

	    APP_LOG("UPNPDevice", LOG_ERR, "Inside if((tempmode == MODE_LOW) || (tempmode == MODE_HIGH))");
	    allowSetTime = TRUE;
        }
        else {
            APP_LOG("UPNPDevice", LOG_ERR, "Inside else of if((tempmode == MODE_LOW) || (tempmode == MODE_HIGH))");
            curMode = HAL_GetMode();
            APP_LOG("UPNPDevice", LOG_ERR, "current mode returned is : %d ", curMode);
            if((curMode == MODE_LOW) || (curMode == MODE_HIGH)) {
                allowSetTime = TRUE;
            }
            else {
                allowSetTime = FALSE;
            }
        }

        if((APP_TIME_ERROR != delayTime) && (TRUE == allowSetTime)) {
            retstatus = HAL_SetCookTime(delayTime);
            if (0x00 == retstatus) {
                /* Is internal broadcast is required? */
                pActionRequest->ActionResult = NULL;
                pActionRequest->ErrCode = 0x00;
                /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", szCurState); */

                UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", szCurState);

                APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: Time changed to: %d minutes", delayTime);

            }
            else {
                pActionRequest->ActionResult = NULL;
                pActionRequest->ErrCode = SET_CPTIME_FAILURE;
                /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error");*/

                UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error");

                APP_LOG("UPNPDevice", LOG_ERR, "SETJARDENSTATUS: Time not changed\n");
            }
        } /* end of if(APP_TIME_ERROR... */
        else {
            APP_LOG("Jarden", LOG_ERR, "SETJARDENSTATUS: Time set not performed\n");
        }
    }
    else
    {
    if(MODE_UNKNOWN != jardenMode) {

        /* Call HAL API's to set mode and set time for a crockpot */
        retstatus = HAL_SetMode(jardenMode);
        if (0x00 == retstatus) {
            pActionRequest->ActionResult = NULL;
            pActionRequest->ErrCode = 0;
                /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", szCurState);*/

                UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", szCurState);

                APP_LOG("UPNPDevice", LOG_ERR, "SetJardenStatus: Mode changed to: %d mode", jardenMode);
        }
        else {
                pActionRequest->ActionResult = NULL;
                pActionRequest->ErrCode = 0;
                /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error"); */

                UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error");

                APP_LOG("UPNPDevice", LOG_ERR, "SetJardenStatus: Mode not changed");
        }
    } /* end of if(MODE_IDLE... */
    else { /* Set to IDLE mode is not allowed */
        APP_LOG("Jarden", LOG_ERR, "SetJardenStatus: Invalid mode value");
    }

    sprintf(szCurState, "%d", delayTime);

    /* Allow setting of time only in power on mode */
    if((jardenMode == MODE_ON) || (jardenMode == MODE_OFF)) {
        allowSetTime = TRUE;
    }
    else {
        curMode = HAL_GetMode();
        if((curMode == MODE_ON) || (curMode == MODE_OFF)) {
            allowSetTime = TRUE;
        }
        else {
            allowSetTime = FALSE;
        }
    }

    if((0 < delayTime) && (MAX_DELAY_TIME >= delayTime) && (TRUE == allowSetTime)) {
        retstatus = HAL_SetDelayTime(delayTime);
        if (0x00 == retstatus) {
            pActionRequest->ActionResult = NULL;
            pActionRequest->ErrCode = 0x00;
                /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", szCurState); */

                UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", szCurState);

            APP_LOG("UPNPDevice", LOG_ERR, "SetJardenStatus: Time changed to: %d minutes", delayTime);

        }
        else {
            pActionRequest->ActionResult = NULL;
            pActionRequest->ErrCode = SET_CPTIME_FAILURE;
                /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetJardenStatus",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error"); */

                UpnpAddToActionResponse(&pActionRequest->ActionResult, "SetCrockpotState",
                                        CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error");

            APP_LOG("UPNPDevice", LOG_ERR, "SetJardenStatus: Time not changed\n");
        }
    } /* end of if(0 < delayTime... */
    else {
        APP_LOG("Jarden", LOG_ERR, "SetJardenStatus: Time set not performed\n");
        }
    }

    FreeXmlSource(parammode);
    FreeXmlSource(paramtime);

    /*APP_LOG("UPNPDevice", LOG_DEBUG, "SetJardenStatus: End of set operation");*/
    return UPNP_E_SUCCESS;
}

/* GetJardenStatus function is to change the Jarden Device settings
 * This function make use of HAL_GetMode() & HAL_GetCooktime() functions from HAL layer
 * pActionRequest - Handle to request
 *
 */
int GetJardenStatus(pUPnPActionRequest pActionRequest,
                     IXML_Document *request,
                     IXML_Document **out,
                     const char **errorString)
{
    JARDEN_MODE jardenMode;
    int tempmode, delayTime;
    char getStatusRespMode[16];
    char getStatusRespTime[16];
    int ret;

    memset(getStatusRespMode, 0x00, sizeof(getStatusRespMode));
    memset(getStatusRespTime, 0x00, sizeof(getStatusRespTime));

    if (NULL == pActionRequest || NULL == request)
    {
        APP_LOG("UPNPDevice", LOG_DEBUG, "GetJardenStatus: command paramter invalid");
        return GET_PARAMS_NULL;
    }

    /* Get the current mode setting */
    jardenMode = HAL_GetMode();

    APP_LOG("UPNP: Device", LOG_DEBUG, "GetJardenStatus:jardenMode %d", jardenMode);
    if (0x00 > jardenMode)
    {
        pActionRequest->ActionResult = NULL;
        pActionRequest->ErrCode = GET_CPMODE_FAILURE;
        /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetJardenStatus",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error"); */

        UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", "Error");

        APP_LOG("UPNPDevice", LOG_ERR, "GetJardenStatus: Unable to get the mode: ");
    }

    if (DEVICE_CROCKPOT == g_eDeviceType)
    {
        /* Get the time information */
        delayTime = HAL_GetCookTime();

        tempmode = convertModeValue(jardenMode,FROM_CROCKPOT);

        APP_LOG("UPNP: Device", LOG_DEBUG, "GetJardenStatus:delaytime is  %d and tempmode is %d", delayTime,tempmode);
        sprintf(getStatusRespMode, "%d",tempmode);
    }
    else
    {
        /* Get the time information */
        delayTime = HAL_GetDelayTime();
        sprintf(getStatusRespMode, "%d",jardenMode);
    }

    if (0x00 > delayTime)
    {
        pActionRequest->ActionResult = NULL;
        pActionRequest->ErrCode = GET_CPTIME_FAILURE;
        /* UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetJardenStatus",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error"); */

        UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState",
                                CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", "Error");

        APP_LOG("UPNPDevice", LOG_ERR, "GetJardenStatus: Time not changed, current time: ");
    }

    /* ret = UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetJardenStatus",
                                  CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", getStatusRespMode); */


    ret = UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState",
                                  CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "mode", getStatusRespMode);

    APP_LOG("UPNP: Device", LOG_DEBUG, "GetJardenStatus:Mode %s", getStatusRespMode);

    sprintf(getStatusRespTime, "%d",delayTime);

    /* ret = UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetJardenStatus",
                                  CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", getStatusRespTime); */

    ret = UpnpAddToActionResponse(&pActionRequest->ActionResult, "GetCrockpotState",
                                  CtrleeDeviceServiceType[PLUGIN_E_EVENT_SERVICE], "time", getStatusRespTime);

    APP_LOG("UPNP: Device", LOG_DEBUG, "GetJardenStatus:Time %s minutes", getStatusRespTime);

    return UPNP_E_SUCCESS;
}

void jardenStatusChange(int mode, int time)
{
    if((DEVICE_CROCKPOT == g_eDeviceType)){
    LocalJardenStateNotify(mode, time);
    }
    else{
        APP_LOG("UPNP", LOG_DEBUG, "Inside else((DEVICE_CROCKPOT == g_eDeviceType of jardenstatuschange");
	LocalBinaryStateNotify(mode);
    }
    LocalBinaryStateNotify(mode);
    return;
}

/*---------- JArden Device Status change notify -------*/
void LocalJardenStateNotify(int jMode, int jDelayTime)
{
    APP_LOG("UPNP", LOG_DEBUG, "Inside LocalJardenStateNotify mode received is %d and delay/cooktime is %d",jMode,jDelayTime );
    int ret=-1;
    int tempMode=0;
    char* szCurState[2];
    char* paramters[2];

    if (!IsUPnPNetworkMode())
    {
        //- Not report since not on router or internet
        APP_LOG("UPNP", LOG_DEBUG, "Notification:Jarden Device Status: Not in home network, ignored");
        return;
    }

    szCurState[0] = (char*)malloc(sizeof(jMode));
    szCurState[1] = (char*)malloc(sizeof(jDelayTime));

    paramters[0] = (char*)malloc(20);
    paramters[1] = (char*)malloc(20);

    memset(szCurState[0], 0x00, sizeof(jMode));
    memset(szCurState[1], 0x00, sizeof(jDelayTime));

    memset(paramters[0], 0x00, 20);
    memset(paramters[1], 0x00, 20);

    if(DEVICE_CROCKPOT == g_eDeviceType)
    {
        APP_LOG("UPNP", LOG_DEBUG, "Inside LocalJardenStateNotify if(DEVICE_CROCKPOT == g_eDeviceType)");
        tempMode = convertModeValue(jMode,FROM_CROCKPOT);
        sprintf(szCurState[0], "%d", tempMode);
        APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify tempMode being set is %s",szCurState[0]);
        sprintf(szCurState[1], "%d", jDelayTime);

        sprintf(paramters[0], "%s", "mode");
        APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify mode being set is %s",paramters[0]);
        sprintf(paramters[1], "%s", "time");
        APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify time being set is %s",paramters[1]);
    }
    else
    {
        sprintf(szCurState[0], "%d", jMode);
        sprintf(szCurState[1], "%d", jDelayTime);
        APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify jDelayTime being set is %s",szCurState[1]);

        sprintf(paramters[0], "%s", "BinaryState");
        APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify BinaryState being set is %s",paramters[0]);
        sprintf(paramters[1], "%s", "time");
        APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify time being set is %s",paramters[1]);
    }

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

    free(paramters[0]);
    free(paramters[1]);
    /* Push to the cloud */
    {
        remoteAccessUpdateStatusTSParams(1);
	APP_LOG("UPNP", LOG_DEBUG, "LocalJardenStateNotify after remoteAccessUpdateStatusTSParams");
    }
    return;
}
#endif /* #ifdef PRODUCT_WeMo_Jarden */
