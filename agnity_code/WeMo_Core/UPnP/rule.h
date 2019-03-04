/***************************************************************************
*
*
* rule.h
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
#ifndef		__RULE__H__
#define		__RULE__H__

#include <ithread.h>
#include "defines.h"

#define INVALID_THREAD_HANDLE -1
#define RULEDBURL                       "/tmp/Belkin_settings/APNSRule.db"

extern ithread_t g_handlerSchedulerTask;


    #define	MAX_RULES_SIZE 60

typedef enum {
	WEEK_MON_E = 0x00, 
	WEEK_THUE_E, 
	WEEK_WEN_E, 
	WEEK_THURS_E, 
	WEEK_FRI_E, 
	WEEK_SAT_E, 
	WEEK_SUN_E,
	WEEK_DAYS_NO
	} DAY_OF_WEEK_INDEX;
	
typedef enum {
	NORMAL_RULE,
	SUN_RISE_RULE,
	SUN_SET_RULE,
	SIMULATED_OCCUPANCY_RULE,
	SIMULATED_SUNRISE_RULE,
	SIMULATED_SUNSET_RULE,
	MAX_RULES
	} SUNRISE_SET_RULES;
	
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

#define MAX_RULEMSG_SIZE    155
#define SET_RULE_FLAG	    1
#define RESET_RULE_FLAG	    0

#endif

#ifdef PRODUCT_WeMo_Insight
#define MAX_APNS_SUPPORTED    210 //(7*MAX_TIMER60)/2
typedef enum {
	E_EQUAL = 0x00,
	E_LARGER,
	E_LESS,
	E_EQUAL_OR_LARGER,
	E_EQUAL_OR_LESS,
	E_WRONG_OPCODE
	} CONDITION_OPCODE;

typedef enum {
	E_COST = 0x00,
	E_ON_DURATION,
	E_OFF_DURATION,
	E_SBY_DURATION,
	E_STATE,
	E_POWER,
	E_INVALID
	} INSIGHT_RULE_PARAMS;

struct __InsightCondition
{
    int        ParamCode;      //Prameter Code
    int        OPCode;           // Operation Code
    int        OPVal;            //Operation Value
    int        isTriggered;      //isTrigger Flag when condition is true
    int        SendApnsFlag;      //Send APNS Flag for that rule
    int        isActive;      //Check if rule is active
};

typedef struct __InsightCondition InsightCondition;
unsigned int g_SendRuleID;
struct __InsightActiveState
{
    unsigned int         RuleId;      //Rule Id
    char		 isActive;       // is rule active flag
};
typedef struct __InsightActiveState InsightActiveState;
InsightActiveState	InsightActive[MAX_APNS_SUPPORTED];
//unsigned int g_SendAPNS;
//unsigned int g_SendONForAPNS;
#endif
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

unsigned int g_RuleID;
unsigned char g_RuleMSG[MAX_RULEMSG_SIZE];
unsigned int g_RuleFreq;
int g_RuleExecutionTS;
#endif

struct __SensorControlledDevice
{
    char        UND[SIZE_64B];      //- UDN of device controlled
    int         triggerType;           // trigger device like socket to ON/OFF
    unsigned int         cntDuration;            //seconds of action response
    int			endAction;
#ifdef PRODUCT_WeMo_Insight
    InsightCondition		InsightList;
#endif
};


typedef struct __SensorControlledDevice SensorControlledDevice,
    *pSensorControlledDevice;

struct __TimerEntry
{
    unsigned long               time;           //seconds from 00:00 am
    int                         action;         //ON/OFF, detection/stop detection
    SensorControlledDevice*     pSocketList;         //Device List, NULL if it is simpler time device
    int                         count;          //Timer entry No.
    int							Delay;
	int							Sensitivity;
	int 						isOverridable;	//- Coube be override or not
	int							isExtraRule;	//- Breaking rules flag
	SUNRISE_SET_RULES			sunrise_set;
	int							sunrise_set_offset;
};

typedef struct __TimerEntry TimerEntry, *pTimerEntry;

#define MAX_TIMER_NO 10

struct __DailyTimerList
{
    TimerEntry*         pTimerList; //- pointer to timer entry array first element
    int                 count;
	unsigned char	  	day_sunrise_set;
	double				longitude;
	double				latitude;
};

typedef struct __DailyTimerList DailyTimerList, *pDailyTimerList;


extern DailyTimerList TimerCalender[WEEK_DAYS_NO];



/**
 *	Save all rule to local disk
 *
 *
 *
 *
 *************************************************/
void SaveRule2Flash();

/**
 *	Load all rule to local disk
 *
 *
 *
 *
 *************************************************/
void LoadRuleFromFlash();

/**
 *	PrintRules
 *
 *
 *
 *
 *************************************************/
void PrintRules();

/**
 *      ClearRulesFromFlash
 *
 *
 *
 *
 *************************************************/
void ClearRuleFromFlash();

/**
 *	Docode rule from UPnP message
 *
 *  Mon[NumberOfTimers|time,action,[deviceUDN, trigger Type, duration;deviceUDN;...]|time,action,[deviceUDN;deviceUDN...]
 *
 *
 *************************************************/
int DecodeUpNpRule(DAY_OF_WEEK_INDEX index, const char* szRule);


extern char* szWeekDayShortName[WEEK_DAYS_NO];

void CleanCalendar();


void *RulesTask(void *args);

void GetCalendarDayInfo(int* dayIndex, int* monthIndex, int* year, int* seconds);

DailyTimerList* GetFirstSchedulerNode(int DayOfWeekIndex);



//----------------- Rule storage and load---------------------------------------
/* Open the rule file for further storage
 *
 *
 *
 *
 *
 *
 ********************************************/
void OpenRuleStorage();

/* Close the rule file when done
 *
 *
 *
 *
 *
 *
 ********************************************/
void CloseRuleStorage();

/* Save rule entry
 *
 *
 *
 *
 *
 *
 ********************************************/
void SaveRuleEntry(const char* entry);

/* Load rule entry from flash storage
 *
 *
 *
 *
 *
 *
 ********************************************/
void LoadStoredRules();


/*****
	Check NTP latest update time

 */
int IsNtpUpdate();

void *RulesNtpTimeCheckTask(void *args);


SensorControlledDevice* DecodeControlledDeviceList(const char* source, int* deviceNumber, int* delay, int* sensitivity);
/***
 *
 *
 *
 *
 *
 *
 *
 ***********************************************************/
void  SocketTimerNotify(pTimerEntry pNowEntry);


/***
 *
 *
 *
 *
 *
 *
 *
 ***********************************************************/

void  SensorTimerNotify(pTimerEntry pNowEntry);

#ifdef PRODUCT_WeMo_Insight
void  InsightTimerNotify(pTimerEntry pNowEntry);
int InsightCompareParam(int Parameter,InsightCondition IList);
int CheckInsightTrigger(pTimerEntry pInsightNowEntry,int loop);
int ExecuteInsightRule(pTimerEntry pInsightNowEntry,int loop);
int FindRuleIndex(unsigned int APNSRuleId);
#endif
int GetRuleIDFlag();
void SetRuleIDFlag(int FlagState);
void LockRuleID();
void UnlockRuleID();
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

void InitAPNS(void);
void SendRuleNotification(unsigned int RuleID);
#endif

/** To mark current timer entry */
extern pTimerEntry curTimerEntry;

extern pTimerEntry g_pCurTimerControlEntry;

void initRule();

void lockRule();
void unlockRule();

int UpdateRule(const char** RuleList);

/**
 * Just update rule list for case loading from flash
 *
 *
 *
 *
 *
 *
 *****************************************************/
int UpdateRuleList(const char** RuleList);

int ActivateRuleTask();

#define 	ACTION_SENSOR_RULE 		0x00
#define 	ACTION_BUTTON_TOGGLE 	0x01
#define 	ACTION_PHONE_CONTROL 	0x02
#define		ACTION_REMOTE_CONTROL	0x03

int IsExecutedRuleNow(int DayOfWeekIndex, int nowSeconds, int* outCommand);


void RuleToggleLed(int curState);
int OverrideRule(int userActionIndex);


#define	RULE_DO_NOTHING 0x02

void ReloadRuleFromFlash();


#endif
