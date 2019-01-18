/***************************************************************************
 *
 *
 * rule.c
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
#include <sys/time.h>
#include "utils.h"
#include "rule.h"
#include "global.h"
#include "logger.h"
#include <time.h>
#include "upnpCommon.h"
#include "gpio.h"
#include "controlledevice.h"
#include "plugin_ctrlpoint.h"
#include "sunriset.h"
#ifdef _OPENWRT_
#include "belkin_api.h"
#else
#include "gemtek_api.h"
#endif
#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#endif
extern unsigned long int GetNTPUpdatedTime(void);
extern void remoteAccessUpdateStatusTSParams(int status);

static FILE* s_fpRuleHandle = 0x00;

#define		RULE_FILE_PATH		"/tmp/Belkin_settings/rule.txt"
#define     UNITS_DIGIT_DET     10

#ifdef SIMULATED_OCCUPANCY
extern int gSimulatedRuleRunning;
extern int gSimulatedRuleScheduled;
extern int gSimRuleFirstOccurenceOfDay;
extern int gSimRuleLastOccurenceOfDay;
extern SimulatedDevInfo *pgSimdevList;
extern int gSimManualTrigger;
extern char gSimManualTriggerDate[SIZE_16B];
extern unsigned long gSimFirstTimer;
extern int gSimRuleFirstOccurenceOfNextDay;
extern int gProcessData;
extern int gSimRandomEndTime;
extern int gDefaultSimFirstOnTime;
extern int gSimulatedDeviceCount;
extern int gSimulatedRuleEndTime;
#endif
static int cntDuration2NextEvent = 0x00;
static int isOverriddenState = 0x0;	//- state of rule, whether overriden or not
pTimerEntry curTimerEntry = 0x00;
pTimerEntry nextTimerEntry = 0x00;
pTimerEntry g_pCurTimerControlEntry = 0x00;
extern int gDstSupported;

#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

unsigned int g_RuleID = 0x00;
unsigned char g_RuleMSG[MAX_RULEMSG_SIZE];
unsigned int g_RuleFreq=0x00;
unsigned int g_SendRuleID=0x00;
unsigned int g_DeferRuleID = 0x00;
int g_RuleExecutionTS=0x00;
#define MAX_APNS_INFO_COLUMN    3
#endif


#define ONE_DAY_SECONDS 86400 //-24 * 60 * 60
#define RULES_BASIC_TRIGGER_TIME		300  //00:05am in seconds	 					 

#define MAX_DEFER_RULE_ENTRIES		5

#define RULE_SUCCESS 				0x00
#define RULE_FAILURE 				0x01
#define TIME_EVENT_DO_NOTHING		0x02
#define	TIME_EVENT_BREAK_DURATION	0x0a	//- 10 seocnds
#define	OVER_NIGHT_RULE_NUMBER		0x02


static pthread_mutex_t   s_rule_mutex;
static pthread_mutex_t   s_ruleID_mutex;

#define MAX_RULE_ENTRY_SIZE SIZE_4096B

#define MAX_DEVICE_SIZE 6
#define MAX_ENTRY_ELEMENT_SIZE SIZE_4B
#define MAX_RULE_BUFFER_SIZE    SIZE_512B
#define MAX_RULE_SIZE   SIZE_512B

void *RulesNtpTimeCheckTask(void *args);
//------------------------------- -----------------------
ithread_t g_handlerSchedulerTask = INVALID_THREAD_HANDLE;

ithread_t g_handlerRuleNtpTimeCheckTask = INVALID_THREAD_HANDLE;

static int s_hasHistoricalRule = 0x00;

void setActuation(char *str)
{
	if(!str)
		return;

	memset(g_szActuation, 0, sizeof(g_szActuation));
	strncpy(g_szActuation, str, sizeof(g_szActuation)-1);
}

void setRemote(char *str)
{
	if(!str)
		return;

	memset(g_szRemote, 0, sizeof(g_szRemote));
	strncpy(g_szRemote, str, sizeof(g_szRemote)-1);
}

int ActivateRuleTask()
{
		lockRule();

		StopWeeklyScheduler();
		unlockRule();

		RunWeeklyScheduler();

		APP_LOG("Rule", LOG_DEBUG, "rule task activated and executed");
		return 0;
}

int UpdateRuleList(const char** RuleList)
{
		int 		loop = 0x00;
		int			cntValidDays = 0x00;	//To count how many days caculated
		char		szEntryBuffer[MAX_RULE_ENTRY_SIZE];

		lockRule();
		CleanCalendar();

		for (; loop < WEEK_DAYS_NO; loop++)
		{
				if ((0x00 != RuleList[loop]) && (strlen(RuleList[loop])))
				{
						memset(szEntryBuffer, 0x00, sizeof(szEntryBuffer));
						sprintf(szEntryBuffer, "%d,%s\n", loop, RuleList[loop]);

						int rect = DecodeUpNpRule(loop, RuleList[loop]);
						if (0x00 == rect)
						{
								cntValidDays++;
						}
						else
						{
								APP_LOG("UPNP: Device", LOG_DEBUG, "%s: rule decoding failed", szWeekDayShortName[loop]);
#ifdef SIMULATED_OCCUPANCY
								if (0x02 == rect)
								{
										APP_LOG("UPNP: Device", LOG_DEBUG, "simulated file error, return");
										break;
								}
#endif			
						}
				}
				else
				{
						APP_LOG("UPNP: Device", LOG_DEBUG, "%s: no rule data", szWeekDayShortName[loop]);
				}
		}
		unlockRule();

		return cntValidDays;
}

int UpdateRule(const char** RuleList)
{
		int 		loop = 0x00;
		int 		cntValidDays = 0x00;	//To count how many days caculated
		char 		szEntryBuffer[MAX_RULE_ENTRY_SIZE];

		int rect = 0x00;

		StopWeeklyScheduler();
		lockRule();
		g_isSensorRuleActivated = 0x00;
		g_isInsightRuleActivated = 0x00;
		curTimerEntry 		  = 0x00;
		g_pCurTimerControlEntry = 0x00;
		unlockRule();

		lockRule();
		OpenRuleStorage();

		CleanCalendar();

		APP_LOG("UPNP: Rule", LOG_DEBUG, "Clean up done and start new rule execution");

		for (; loop < WEEK_DAYS_NO; loop++)
		{
				if ((0x00 != RuleList[loop]) && (strlen(RuleList[loop])))
				{
						if (strlen(RuleList[loop]) > MAX_RULE_ENTRY_SIZE)
						{
								APP_LOG("UPNP: Device", LOG_DEBUG, "##################### rule payload too long #########################");
								return 0x01;
						}

						memset(szEntryBuffer, 0x00, sizeof(szEntryBuffer));
						sprintf(szEntryBuffer, "%d,%s\n", loop, RuleList[loop]);

						int rect = DecodeUpNpRule(loop, RuleList[loop]);
						if (0x00 == rect)
						{
								cntValidDays++;
								SaveRuleEntry(szEntryBuffer);
						}
						else
						{
								APP_LOG("UPNP: Device", LOG_DEBUG, "%s: rule decoding failed", szWeekDayShortName[loop]);			
#ifdef SIMULATED_OCCUPANCY
								if (0x02 == rect)
								{
										APP_LOG("UPNP: Device", LOG_DEBUG, "simulated file error, return");
										break;
								}
#endif			
						}
				}
				else
				{
						APP_LOG("UPNP: Device", LOG_DEBUG, "%s: no rule data", szWeekDayShortName[loop]);
				}

		}

		CloseRuleStorage();

		unlockRule();

		if (cntValidDays)
		{
				RunWeeklyScheduler();	
				rect = 0x00;
				APP_LOG("UPNP: Rule", LOG_DEBUG, "Rule update success: %d days update", cntValidDays);
		}
		else
		{
				rect = 0x01;
				APP_LOG("UPNP: Rule", LOG_DEBUG, "Rule update unsuccess");
		}

		return rect;
}

static int s_IsInitialized = 0x00;

void initRule()
{
		if (s_IsInitialized)
				return;

		//- Make sure reload will reset all params
		ResetOverrideStatus();

		ithread_mutexattr_t attr;
		ithread_mutexattr_init(&attr);
		ithread_mutexattr_setkind_np( &attr, ITHREAD_MUTEX_RECURSIVE_NP);
		ithread_mutex_init(&s_rule_mutex, &attr);
		ithread_mutexattr_destroy(&attr);

		memset(TimerCalender, 0x00, sizeof(TimerCalender));


		//- NTP check should from here since need to cover the service subscription issue because of time sync
		APP_LOG("UPNP: Rule", LOG_DEBUG, "Ntp time check thread is created");
		ithread_create(&g_handlerRuleNtpTimeCheckTask, 0x00, RulesNtpTimeCheckTask, 0x00);

		LoadRuleFromFlash();

		s_IsInitialized = 0x01;

}

void lockRule()
{
		ithread_mutex_lock(&s_rule_mutex);

}

void unlockRule()
{
		ithread_mutex_unlock(&s_rule_mutex);
}

void LockRuleID()
{
		ithread_mutex_lock(&s_ruleID_mutex);

}

#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)

int GetRuleIDFlag()
{
		return  g_SendRuleID;
}

void SetRuleIDFlag(int FlagState)
{
		g_SendRuleID = FlagState;
}

void UnlockRuleID()
{
		ithread_mutex_unlock(&s_ruleID_mutex);
}
#endif

extern int g_eDeviceType;

char* szWeekDayShortName[WEEK_DAYS_NO] = {"Mon", "Tues", "Wed", "Thurs", "Fri", "Sat", "Sun"};

DailyTimerList TimerCalender[WEEK_DAYS_NO] = {{0x00, 0x00}, 
		{0x00, 0x00}, 
		{0x00, 0x00}, 
		{0x00, 0x00},
		{0x00, 0x00}, 
		{0x00, 0x00},
		{0x00, 0x00}
};

char* szWeekDayName[] = 
{
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
		"Sunday"
};

int sunrise_set_calculated[WEEK_DAYS_NO] = {FALSE, 
		FALSE, 
		FALSE, 
		FALSE, 
		FALSE, 
		FALSE, 
		FALSE
}; // to set a flag if sunrise/set already calculated

int current_date;

/**
 *	Save all rule to local disk
 *
 *************************************************/
void SaveRule2Flash()
{

}

void *RulesNtpTimeCheckTask(void *args)
{
		tu_set_my_thread_name( __FUNCTION__ );

		sleep(DELAY_3SEC);
		while(1)
		{
				sleep(DELAY_3SEC);
				if (IsNtpUpdate())
				{
						APP_LOG("Rule", LOG_DEBUG, "NTP updated, rule NTP time check task stop");
						sleep(DELAY_5SEC);
						Advertisement4TimeUpdate();
						if (s_hasHistoricalRule)
						{
								sleep(DELAY_1SEC);
								ActivateRuleTask();
								s_hasHistoricalRule = 0x00;
						}
						break;
				}
		}
		return NULL;
}



/**
 *	Load all rule to local disk
 *
 *
 *
 *
 *************************************************/
#define MAX_WEEK_DAYS 7

void LoadRuleFromFlash()
{
		char 	*szWeeklyRule[WEEK_DAYS_NO] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		int 	counter = 0x00;

		FILE* pfRule = fopen(RULE_FILE_PATH, "r");
		if (pfRule)
		{
				//Load from file
				APP_LOG("Rule", LOG_DEBUG, "Rule file found, loading");
				char szEntryBuffer[MAX_RULE_ENTRY_SIZE];
				int iValidDays = 0x00;
				int iWeekIndex = 0x00;

#ifdef SIMULATED_OCCUPANCY
				APP_LOG("Rule", LOG_DEBUG, "unset simulated rule data...");
				unsetSimulatedData();
#endif

				while ((!feof(pfRule)) && (counter < WEEK_DAYS_NO))
				{
						memset(szEntryBuffer, 0x00, MAX_RULE_ENTRY_SIZE);
						fgets(szEntryBuffer, MAX_RULE_ENTRY_SIZE, pfRule);

						int len = strlen(szEntryBuffer);
						if ( len > 0x02)
						{
								szEntryBuffer[len - 1 ] = 0x00;
						}
						else
						{
								continue;
						}

						APP_LOG("Rule", LOG_DEBUG, "stored rule entry: %s", szEntryBuffer);
						char szStoredIndex[2];
						int  cntStoredIndex = 0x00;
						memset(szStoredIndex, 0x00, sizeof(szStoredIndex));

						strncpy(szStoredIndex, szEntryBuffer, 0x01);
						cntStoredIndex = atoi(szStoredIndex);

						iWeekIndex = cntStoredIndex;

						if (iWeekIndex >= WEEK_DAYS_NO)
						{
								APP_LOG("Rule", LOG_DEBUG, "Wrong rule entry, stop loading rule");		
								goto Cleanup;
						}

						if (0x00 == szWeeklyRule[iWeekIndex])
						{
								szWeeklyRule[iWeekIndex] = (char*)malloc(MAX_RULE_ENTRY_SIZE);
								memset(szWeeklyRule[iWeekIndex], 0x00, MAX_RULE_ENTRY_SIZE);
								strcpy(szWeeklyRule[iWeekIndex], szEntryBuffer + 2);
								APP_LOG("Rule", LOG_DEBUG, "%s: rule: %s", szWeekDayShortName[iWeekIndex], szEntryBuffer);
								iValidDays++;
						}
						else
						{
								APP_LOG("Rule", LOG_DEBUG, "%s: repeating rule entry, should not happen",  szWeekDayShortName[iWeekIndex]);
								//goto Cleanup;
						}

						counter++;
				}

				fclose(pfRule);
				pfRule = 0x00;

				if (iValidDays > 0x00)
				{
						//- TODO: Double check the Time Sync by NTP
						UpdateRuleList((const char**)szWeeklyRule);
						if (IsNtpUpdate())
						{
								APP_LOG("UPNP: Rule", LOG_DEBUG, "Ntp time update done, start rule task");
								ActivateRuleTask();
						}
						else
						{
								//- Time not update yet, so mark it and then wait the time sync occurs
								s_hasHistoricalRule = 0x01;
						}
				}
				else
				{
						APP_LOG("Rule", LOG_ERR, "####### No record in rule file ####################");
				}
		}
		else
		{
				APP_LOG("Rule", LOG_ERR, "No rule file found, no rule history setup");
		}

		goto Cleanup;

Cleanup:
		{
				APP_LOG("Rule", LOG_DEBUG, "clean up all data memory");

				int loop = 0x00;
				for (; loop < WEEK_DAYS_NO; loop++)
				{
						if (szWeeklyRule[loop])
						{
								free(szWeeklyRule[loop]);
								szWeeklyRule[loop] = 0x00;
						}
				}
		}
}

#define		NTP_UPDATE_TIMEOUT     86400	//24 hours
int IsNtpUpdate()
{
		int year = 0x00, monthIndex = 0x00, seconds = 0x00, dayIndex = 0x00;

		GetCalendarDayInfo(&dayIndex, &monthIndex, &year, &seconds);
		unsigned long int lNtpTime = GetNTPUpdatedTime();
		int delta = 0x00;

		struct timeval tvNow;
		gettimeofday(&tvNow, NULL);

		unsigned long int lNowTime = tvNow.tv_sec;

		APP_LOG("Rule", LOG_DEBUG, "NTP last update time: %d, Now time: %d", lNtpTime, lNowTime);

		delta = lNowTime - lNtpTime;
		if(year != 2000)
		{
		    if((delta > NTP_UPDATE_TIMEOUT) && (lNtpTime!=0))//Internet is connected after 24 hour
		    {
			APP_LOG("DEVICE:rule", LOG_DEBUG, "Time diff is more than 24hr");
			return 0x00;
		    }
		    if((delta < 0) && (lNtpTime!=0))//Time less than LastNTP time is set
		    {
			APP_LOG("DEVICE:rule", LOG_DEBUG, "Now Time is not correct");
			return 0x00;
		    }
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "************* ISNTPUPDATE NOW YEAR IS NOT 2000");
		    return 0x01;
		}

		if ((delta > NTP_UPDATE_TIMEOUT) || (delta < 0) || (0x00 == lNtpTime))
		{
		    return 0x00;
		}

		return 0x01;
}


/**
 *	PrintRules
 *
 *
 *
 *
 *************************************************/
void PrintRules()
{

}
/**
 *	Clear All rules related data from Flash
 *
 *
 *
 *
 *************************************************/
void ClearRuleFromFlash()
{
		char buf[SIZE_64B];
		system("rm /tmp/Belkin_settings/rule.txt");
		APP_LOG("Rule", LOG_DEBUG, "Remove rule txt file");
		usleep(500000);
		memset (buf,0,SIZE_64B);
		sprintf(buf,"rm %s",RULE_DB_PATH);
		system(buf);
		APP_LOG("Rule", LOG_DEBUG, "Remove rule db");
#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)
		usleep(500000);
		memset (buf,0,SIZE_64B);
		sprintf(buf,"rm %s",RULEDBURL);
		system(buf);
		APP_LOG("Rule", LOG_DEBUG, "Remove rule APNS db");
		InitAPNS();
#endif

		// -Stop scheduler
		StopWeeklyScheduler();
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
		SaveDeviceConfig(RULE_DB_VERSION_KEY, "");
}

#define MAX_CONTROLLED_DEVICE_SIZE 5

/**
 *	 Sample code: "[3;Belkin_socket_001,1,1800,0;Belkin_socket_002,1,1800,0;Belkin_socket_003,1,1800,0]";
 *
 *	 
 *
 *
 *
 *******************************************************************************/
#define SENSOR_RULE_PREFIX_LEN 3
SensorControlledDevice* DecodeControlledDeviceList(const char* source, int* deviceNumber, int* delay, int* sensitivity)
{
		int     varListLoop     = 0x00;
		char    delims[]        = ";";
#ifdef PRODUCT_WeMo_Insight
		char    szInsightRules[2][SIZE_512B];
#endif    
		char    szValues[SIZE_10B][SIZE_512B];
		char    *result         = 0x00;
		char    szLocal[MAX_RULE_BUFFER_SIZE * 2];
		SensorControlledDevice* pList = 0x00;

		if (0x00 == source)
				return pList;

		memset(szLocal, 0x00, sizeof(szLocal));

		strcpy(szLocal, source + 1);
		szLocal[strlen(szLocal) - 1 ] = '\0';

		if (0x00 == strstr(source, "[") || 0x00 == strstr(source, "]"))
		{
				APP_LOG("Rule", LOG_DEBUG, "Incorrect protocol data: %s", source);
				return pList;
		}
		else
		{
				//APP_LOG("Rule", LOG_DEBUG, "protocol: %s", szLocal);
		}

		//- 3;Belkin_socket_001,1,1800,0;Belkin_socket_002,1,1800,0;Belkin_socket_003,1,1800,0
		result = strtok(szLocal, delims);
		while((result != NULL) && (varListLoop < (MAX_DEVICE_SIZE + SENSOR_RULE_PREFIX_LEN)))
		{
				memset(szValues[varListLoop], 0x00, MAX_RULE_BUFFER_SIZE);
				strcpy(szValues[varListLoop], result);
				varListLoop++;
				result = strtok(NULL, delims);
		}


		//APP_LOG("Rule", LOG_DEBUG,"Device No: %d, var list: %d", atoi(szValues[0x00]), varListLoop);

		if (varListLoop > SENSOR_RULE_PREFIX_LEN)
		{
				*deviceNumber 	= atoi(szValues[0x00]);
				*delay		  	= atoi(szValues[0x01]);
				*sensitivity	= atoi(szValues[0x02]);

				if (varListLoop != *deviceNumber + SENSOR_RULE_PREFIX_LEN)
				{
						APP_LOG("Rule", LOG_DEBUG, "####### incorrect protocol, device number mismatch: %d:%d", *deviceNumber, 
										varListLoop - 3);
				}

				pList = (SensorControlledDevice*)malloc(sizeof(SensorControlledDevice)* (*deviceNumber));

				if (pList)
				{
				}
				else
				{
						APP_LOG("Rule", LOG_DEBUG, "Allocate pSensorList unsuccess");
						return 0x00;
				}

				memset(pList, 0x00, sizeof(SensorControlledDevice)*(*deviceNumber));

				int deviceLoopIndex = 0x00;
				for (deviceLoopIndex = 0x00; deviceLoopIndex < *deviceNumber; deviceLoopIndex++)
				{
						int curIndex = deviceLoopIndex+SENSOR_RULE_PREFIX_LEN;
						char szLocalValues[MAX_DEVICE_SIZE][MAX_RULE_BUFFER_SIZE];

						//APP_LOG("Rule", LOG_DEBUG,"Device entry: %s decoded", szValues[curIndex]);

						result = strtok(szValues[curIndex], ",");

						int entryLoop = 0x00;

						while (result != NULL && entryLoop < 10)
						{
								memset(szLocalValues[entryLoop], 0x00, SIZE_64B);
								strcpy(szLocalValues[entryLoop], result);
								entryLoop++;
								result = strtok(NULL, ",");
						}

						strcpy(pList[deviceLoopIndex].UND, szLocalValues[0x00]);
						pList[deviceLoopIndex].triggerType 		= atoi(szLocalValues[0x01]);
						pList[deviceLoopIndex].cntDuration 		= atoi(szLocalValues[0x02]);
#ifdef PRODUCT_WeMo_Insight
						//APP_LOG("Rule", LOG_DEBUG,"Insight Rule: %s decoded RuleID=%d", szLocalValues[0x03], pList[deviceLoopIndex].cntDuration);
						entryLoop = 0x00;
						//start seperating the @ symbol for Insight Condition
						result = strtok(szLocalValues[0x03], "@"); //Insight rule condition is @paramCode:OPCode:OPValue@
						while (result != NULL && entryLoop < 3)
						{
								memset(szInsightRules[entryLoop], 0x00, MAX_RULE_BUFFER_SIZE);
								strncpy(szInsightRules[entryLoop], result, sizeof(szInsightRules[entryLoop])-1);
								//APP_LOG("Rule", LOG_DEBUG,"Insight %d@: %s ",entryLoop,szInsightRules[entryLoop]);
								entryLoop++;
								result = strtok(NULL, "@");
						}
						pList[deviceLoopIndex].endAction	  	= atoi(szInsightRules[0x00]);

						entryLoop = 0x00;
						// Start seperating the : from insight rule condition
						result = strtok(szInsightRules[0x01], ":");
						while (result != NULL && entryLoop < 3)
						{
								memset(szLocalValues[entryLoop], 0x00, SIZE_64B);
								strncpy(szLocalValues[entryLoop], result, sizeof(szLocalValues[entryLoop])-1);
								//APP_LOG("Rule", LOG_DEBUG,"Insight %d,: %s ",entryLoop,szLocalValues[entryLoop]);
								entryLoop++;
								result = strtok(NULL, ":");
						}
						pList[deviceLoopIndex].InsightList.ParamCode	  	= atoi(szLocalValues[0x00]);
						pList[deviceLoopIndex].InsightList.OPCode	  	= atoi(szLocalValues[0x01]);
						pList[deviceLoopIndex].InsightList.OPVal	  	= atoi(szLocalValues[0x02]);
						pList[deviceLoopIndex].InsightList.isTriggered          = 0;
						pList[deviceLoopIndex].InsightList.SendApnsFlag          = 0;
						pList[deviceLoopIndex].InsightList.isActive		= 0;


#else
						pList[deviceLoopIndex].endAction	  	= atoi(szLocalValues[0x03]);

#endif

				}

				//int loop = 0x00;
				//for (; loop < *deviceNumber; loop++)
				{
						//APP_LOG("Rule", LOG_DEBUG, "Device: %s", pList[loop].UND);
				}

		}
		else
		{
				APP_LOG("Rule", LOG_DEBUG, "No protocol data decoded\n");
		}

		return pList;

}


/**
 *	Docode rule from UPnP message
 *
 *  Mon[NumberOfTimers|time,action,[deviceUDN, trigger Type, duration;deviceUDN;...]
 *  |time,action,[deviceUDN;deviceUDN...]
 *
 *
 *************************************************/
#define	DEFAULT_RULE_DELIM	"|"
#define	MAX_RULE_TIMERS		10

int isRuleValid(int deviceType, const char* entry)
{
		int ret = 0x01;

		if (0x00 == entry)
		{
				ret = 0x00;
				return ret;
		}

		if (0x00 == strstr(entry, "|") || 
						0x00 == strstr(entry, ","))
		{
				ret = 0x00;
		}

		if(DEVICE_INSIGHT == g_eDeviceTypeTemp)
		{
				while((entry = strstr(entry, "]")) != NULL)// Check till all the ] are handled
				{
						//APP_LOG("Rule", LOG_DEBUG, "] found at index: %d findindex-1: %c",entry,*(entry-1));
						if ( (*(entry-1) != '@'))//If ] is found just before this @ must be there otherwise it is rule for Motion Sensor
						{
								//- Insight device should not process sensor rule
								APP_LOG("Rule", LOG_DEBUG, "entry dropped because of wrong device type: %s", entry);
								ret = 0x00;
						}
						entry++;
				} 
		}
		else if (DEVICE_SOCKET == g_eDeviceType)
		{
				if ( (0x00 != strstr(entry, "[")) | 
								(0x00 != strstr(entry, "]")) )
				{
						//- Socket device should not process sensor rule
						APP_LOG("Rule", LOG_DEBUG, "entry dropped because of wrong device type: %s", entry);
						ret = 0x00;
				}

		}

		return ret;
}

/**
 *	@ Function: 
 *		DeferSecondDayEntry
 *
 *	@ Description:
 *  	This function is to remove all the time events that greater than 24 hours to the second day.
 *		PLEASE note that, this function assumes that, the paramters feed to it already in sorted order
 *	@ Paramters:
 * 		- entry: pDailyTimerList
 *	@ return:
 *		void
 *	
 *
 ********************************************************************************************************/
int DeferSecondDayEntry(pDailyTimerList entry)
{	
		int ret = RULE_FAILURE;

		if ((0x00 == entry) || (0x00 == entry->count) || (0x00 == entry->pTimerList))
		{	
				APP_LOG("Rule", LOG_ERR, "Entry to be deferred invalid and stop");
				return ret;
		}

		int eventNumber 		= entry->count;
		int loop				= 0x00;
		//- Overriding time for the breaking rules
		int overriddingTime		= 0x00;


		int allocate_size 					= 0x00;

		for (loop = 0x00; loop < eventNumber; loop++)
		{
				if (entry->pTimerList[loop].time > ONE_DAY_SECONDS)
				{
						APP_LOG("Rule", LOG_ERR, "Over-night rule found from the list, they will be moved to second day list");
						break;
				}
		}

		if (loop != eventNumber)
		{
				APP_LOG("Rule", LOG_ERR, "%d: rules deferred to today's mornings", eventNumber - loop - 1);

				for (; loop < eventNumber; loop++)
				{
						/** reset the correct absolute time seconds value to list so that it is just deferred to second day*/
						entry->pTimerList[loop].time -= ONE_DAY_SECONDS;
						overriddingTime = entry->pTimerList[loop].time;
				}

				//- two more do nothing rules should be created and stack to the header and push to the end respectively 
				//- Follow the last one of over-night rule
				int nextDayIndex = eventNumber + 1;
				int lastDayIndex = eventNumber - 2;
				memcpy(&entry->pTimerList[eventNumber],     &entry->pTimerList[lastDayIndex], sizeof(TimerEntry));
				memcpy(&entry->pTimerList[nextDayIndex], &entry->pTimerList[lastDayIndex], sizeof(TimerEntry));

				//- If sensor rule, allocate the managed socket list too, so that can re-allocate later
				if (0x00 != entry->pTimerList[lastDayIndex].count)
				{
						if (0x00 != entry->pTimerList[lastDayIndex].pSocketList)
						{
								allocate_size = sizeof(SensorControlledDevice) * entry->pTimerList[lastDayIndex].count;
								pSensorControlledDevice pDeferList1 = (pSensorControlledDevice)malloc(allocate_size);
								pSensorControlledDevice pDeferList2 = (pSensorControlledDevice)malloc(allocate_size);

								if ((0x00 != pDeferList1) && (0x00 != pDeferList2))
								{
										memcpy(pDeferList1, entry->pTimerList[lastDayIndex].pSocketList, allocate_size);
										memcpy(pDeferList2, entry->pTimerList[lastDayIndex].pSocketList, allocate_size);
										entry->pTimerList[eventNumber].pSocketList 		= pDeferList1;
										entry->pTimerList[nextDayIndex].pSocketList 	= pDeferList2;
								}
								else
								{
										FreeResource(pDeferList1);
										FreeResource(pDeferList2);
										entry->pTimerList[eventNumber].pSocketList 		= 0x00;
										entry->pTimerList[nextDayIndex].pSocketList 	= 0x00;
								}
						}
						else
						{

						}
				}
				else
				{
						entry->pTimerList[eventNumber].pSocketList  	= 0x00;
						entry->pTimerList[nextDayIndex].pSocketList	= 0x00;
				}


				entry->count += OVER_NIGHT_RULE_NUMBER;
				entry->pTimerList[eventNumber].time 		 		= TIME_EVENT_BREAK_DURATION;
				entry->pTimerList[eventNumber].isOverridable 		= 0x01;
				entry->pTimerList[eventNumber].isExtraRule			= overriddingTime;


				entry->pTimerList[nextDayIndex].time 			    = ONE_DAY_SECONDS - TIME_EVENT_BREAK_DURATION;
				entry->pTimerList[nextDayIndex].isOverridable	    = 0x01;
				entry->pTimerList[nextDayIndex].isExtraRule		    = overriddingTime;

				ret = RULE_SUCCESS;
		}
		else
		{
				APP_LOG("Rule", LOG_ERR, "no over night rule found");
		}

		return ret;
}


/**
 *	@ Function: 
 *		ResortDailyEntry
 *
 *	@ Description:
 *		This fucntion is to re-sort the time events entries so that all are in increasingly order
 *		and, to remove the entry that is greater than 24 hours out of the entry and defer next day.
 *		Considering the number of time events for each day is small, now the "bubble Sort" method will 
 *		be applied
 *	@ Paramters:
 * 		- entry: pDailyTimerList
 *	@ return:
 *		void
 *	
 *
 ********************************************************************************************************/
static void ResortDailyEntry(pDailyTimerList entry)
{
		if ((0x00 == entry) || (0x00 == entry->count) || (0x00 == entry->pTimerList))
		{	
				APP_LOG("Rule", LOG_ERR, "Entry to be resorted invalid and stop");
				return;
		}

		APP_LOG("Rule", LOG_DEBUG, "#### Entry to be resorted: %d", entry->count);

		int outerLoop			= 0x00;
		int innerLoop 			= 0x00;
		int eventNumber 		= entry->count;
		int isSorted			= 0x00;
		TimerEntry 				tmpEntry;
		tmpEntry.time 			= 0x00;
		tmpEntry.action 		= 0x00;
		tmpEntry.count     	    = 0x00;
		tmpEntry.pSocketList	= 0x00;
		tmpEntry.isExtraRule    = 0x00;
		tmpEntry.isOverridable  = 0x00;

		int tmpIndex = 0x00;
		/*
			 for (outerLoop = 0x00; outerLoop < eventNumber; outerLoop++)
			 {
			 APP_LOG("Rule", LOG_ERR, "%d: %d->%d", outerLoop, 
			 entry->pTimerList[outerLoop].time, entry->pTimerList[outerLoop].action);
			 }
		 */  
		for (outerLoop = 0x00; outerLoop < eventNumber - 1; outerLoop++)
		{
				for (innerLoop = 0x00; innerLoop < eventNumber - 1 - outerLoop; innerLoop++)
				{
						tmpIndex = innerLoop+1;
						if (entry->pTimerList[innerLoop].time > entry->pTimerList[tmpIndex].time)
						{
								APP_LOG("Rule", LOG_DEBUG, "Entry resorted: %d:%d: with %d", innerLoop, 
												entry->pTimerList[innerLoop].time, entry->pTimerList[tmpIndex].time);
								memcpy(&tmpEntry, &(entry->pTimerList[tmpIndex]), sizeof(tmpEntry));
								entry->pTimerList[tmpIndex] = entry->pTimerList[innerLoop];
								entry->pTimerList[innerLoop]   = tmpEntry;
								isSorted = 0x01;
						}
				}
		}

		if (isSorted)
		{		
				APP_LOG("Rule", LOG_DEBUG, "Entry re-sorted, now the order is: ");
		}
		else
		{
				APP_LOG("Rule", LOG_DEBUG, "Entry not re-sorted");
		}
}


int DecodeUpNpRule(DAY_OF_WEEK_INDEX index, const char* szRule)
{

		int 	varListLoop 	= 0x00;
		char 	delims[] 	= "|";
		char 	delims_longlat[] 	= "#";
		char 	delims_offset[] 	= "$";
		char 	comma_delims[] 	= ",";
		char* szPayload 	= (char *)szRule;
		char* result 	= 0x00;
		char* coord_result 	= 0x00;
		char* payload_result 	= 0x00;
		char* offset_result 	= 0x00;
		char* offset_values 	= 0x00;
		char* lat 	= 0x00;
		char* lng 	= 0x00;
		char 	szValues[MAX_RULES_SIZE][SIZE_512B];
		char 	szUDNRules[SIZE_512B];	//-change later?
		char 	*OffsetValues[MAX_RULES_SIZE];
		int 	OffsetResult[MAX_RULES_SIZE] = {0};
		int 	offset_field=0x00;
		int 	timerLoop 	= 0x00;
		//int loop = 0x00;
		int 	timeSeconds 	= 0x00;
		int 	timerAction 	= 0x00;
		int 	isDeferred 		= 0x00;
		int   totalTimerCount   = 0x00;
		int   flag_lastbit = 0x00;
		double   latitude = 0x00;
		double   longitude = 0x00; 


		int iLoop = 0x00;
		int retVal = 0x00;
		int timerCount = 0x00;
		TimerEntry* pTimerList = 0x00;

		if ((0x00 == szRule) || 0x00 == strlen(szRule))
		{
				APP_LOG("Rule", LOG_DEBUG, "No rule to decode", szRule);
				return 0x01;
		}

		if (!isRuleValid(g_eDeviceType, szRule))
		{
				APP_LOG("Rule", LOG_DEBUG, "Rule entry is invalid: %s", szRule);
				return 0x01;	
		}

		if (index > WEEK_DAYS_NO)
		{
				//- Rollback to Mon
				index = WEEK_MON_E;
		}

		/* If string is Sunrise/Sunset, it contains # as delimiter before longitude/latitude parameters
			 if it contains #, delimit the string and give to szPayload, else it is normal rule
		 */
		if(strstr(szPayload, delims_longlat)!= NULL)
		{
				offset_result = strtok(szPayload, delims_offset);
				APP_LOG("Rule", LOG_DEBUG, "%s: After Offset Removed String is", offset_result);

				offset_values = strtok(NULL, delims_offset);
				APP_LOG("Rule", LOG_DEBUG, "%s: Offset Fields String is", offset_values); 

				payload_result = strtok(offset_result, delims_longlat);
				APP_LOG("Rule", LOG_DEBUG, "%s: Delimited String is", payload_result);

				coord_result = strtok(NULL, delims_longlat);
				APP_LOG("Rule", LOG_DEBUG, "%s: Latitude, Longitude String is", coord_result);      

				lat = strtok(coord_result, comma_delims);
				APP_LOG("Rule", LOG_DEBUG, "%s: Latitude is", lat);

				lng = strtok(NULL, comma_delims);
				APP_LOG("Rule", LOG_DEBUG, "%s: Longitude is", lng);

				latitude = atof(lat);
				longitude = atof(lng);

				/* To store Offset Values to array */
				OffsetValues[offset_field] = strtok(offset_values,comma_delims);
				APP_LOG("Rule", LOG_DEBUG, " OffsetValues string is %s\n",OffsetValues[offset_field]);

				while(OffsetValues[offset_field] != NULL)
				{		
						APP_LOG("Rule", LOG_DEBUG, " OffsetValues string is %s\n",OffsetValues[offset_field]);
						OffsetResult[offset_field] = atoi(OffsetValues[offset_field]);
						APP_LOG("Rule", LOG_DEBUG, " OffsetResults values is %d\n", OffsetResult[offset_field]);
						OffsetValues[++offset_field] = strtok(NULL, comma_delims);
				}

				result = strtok(payload_result, delims); 
		}
		else
		{
				result = strtok(szPayload, delims); 
				APP_LOG("Rule DecodeUpNpRule", LOG_DEBUG, "##################### Normal Rule #############");
		}

		//- Get all raw code in the protocol data
		while((result != NULL) && (varListLoop < MAX_RULES_SIZE))
		{
		    strcpy(szValues[varListLoop], result);
		    varListLoop++;
		    result = strtok(NULL, delims);
		}

		if (varListLoop > 0x00)
		{
				//- Get timer count in the rule date unit
				timerCount = atoi(szValues[0x00]);
				if (timerCount == 0)
				{
						APP_LOG("Rule", LOG_DEBUG, "%s: no timer data found", szWeekDayShortName[index]);
						retVal =0x01;
						goto onreturn;
						//return 0x01;
				}

				if (timerCount != varListLoop - 1)
				{
						APP_LOG("Rule", LOG_DEBUG, "number of timers mismatch: %d:%d", timerCount, varListLoop);		
						retVal =0x01;
						goto onreturn;
						//return 0x01;
				}

				//- Construct entry list of today
				if (0x00 == timerCount)
				{
						APP_LOG("Rule", LOG_DEBUG, "number of timers is 0x00");
						retVal =0x01;
						goto onreturn;
						//return 0x01;
				}

				TimerCalender[index].count = timerCount;

				/*
				 * Merge the defered time event entries if applicable
				 * if there is not over-night rules entry, these two entries will be empty
				 *
				 */
				totalTimerCount = timerCount + OVER_NIGHT_RULE_NUMBER;

				pTimerList = (TimerEntry*)malloc(sizeof(TimerEntry) * totalTimerCount);

				if (!pTimerList)
				{
						APP_LOG("Rule", LOG_DEBUG, "Allocate pTimerList unsuccess");
						retVal =0x01;
						goto onreturn;
						//return (int)pTimerList;
				}

				for (; iLoop < totalTimerCount; iLoop++)
				{
						pTimerList[iLoop].time 					= 0x00;
						pTimerList[iLoop].action 				= 0x00;
						pTimerList[iLoop].count     			= 0x00;
						pTimerList[iLoop].pSocketList			= 0x00;
						pTimerList[iLoop].isExtraRule   		= 0x00;
						pTimerList[iLoop].isOverridable 		= 0x00;
						pTimerList[iLoop].sunrise_set   		= 0x00;
						pTimerList[iLoop].sunrise_set_offset    = 0x00;
				}

				TimerCalender[index].count      = 0x00;
				TimerCalender[index].pTimerList = 0x00;
				TimerCalender[index].day_sunrise_set = 0x00;
				TimerCalender[index].longitude = 0x00;
				TimerCalender[index].latitude = 0x00;

				int nowIndex = 0x00;
				for (timerLoop = 0x01; timerLoop < varListLoop; timerLoop++)
				{
						//-Cleanup and format output data
						memset(szUDNRules, 0x00, sizeof(szUDNRules));
						if ( (szValues[timerLoop]) && (0x00 != strlen(szValues[timerLoop])) )
						{
								//APP_LOG("UPNP: Rule", LOG_DEBUG, "%s", szValues[timerLoop]);  
								sscanf(szValues[timerLoop], "%d,%d,%s", &timeSeconds, &timerAction, szUDNRules);

								/*Determining if the rule is a normal rule or sunrise sunset rule*/
								flag_lastbit = timeSeconds % UNITS_DIGIT_DET;

								if(flag_lastbit == 1){
										pTimerList[nowIndex].sunrise_set         = SUN_RISE_RULE;
										TimerCalender[index].day_sunrise_set     = (TimerCalender[index].day_sunrise_set | TRUE);
										pTimerList[nowIndex].time 		    	 = timeSeconds - 1;
								}
								else if(flag_lastbit == 2){
										pTimerList[nowIndex].sunrise_set         = SUN_SET_RULE;
										TimerCalender[index].day_sunrise_set     = (TimerCalender[index].day_sunrise_set | TRUE);
										pTimerList[nowIndex].time 		    	 = timeSeconds - 2;
								}
#ifdef SIMULATED_OCCUPANCY
								else if(flag_lastbit == 3)
								{
										APP_LOG("Rule", LOG_DEBUG, "simulated rule: %d", flag_lastbit);
										pTimerList[nowIndex].sunrise_set         = SIMULATED_OCCUPANCY_RULE;
										if(timeSeconds == (ONE_DAY_SECONDS+3))
										    pTimerList[nowIndex].time 		    	 = ONE_DAY_SECONDS;
										else
										    pTimerList[nowIndex].time 		    	 = timeSeconds ;
								}
								else if((flag_lastbit == 4) && ((!strlen(szUDNRules))||(!strstr(szUDNRules, "["))) )
								{
										APP_LOG("Rule", LOG_DEBUG, "simulated rule: %d", flag_lastbit);
										pTimerList[nowIndex].sunrise_set         = SIMULATED_SUNRISE_RULE;
										TimerCalender[index].day_sunrise_set     = (TimerCalender[index].day_sunrise_set | TRUE);
										if(timeSeconds == (ONE_DAY_SECONDS+4))
										    pTimerList[nowIndex].time 		    	 = ONE_DAY_SECONDS;
										else
										    pTimerList[nowIndex].time 		    	 = timeSeconds ;
								}
								else if(flag_lastbit == 5)
								{
										APP_LOG("Rule", LOG_DEBUG, "simulated rule: %d", flag_lastbit);
										pTimerList[nowIndex].sunrise_set         = SIMULATED_SUNSET_RULE;
										TimerCalender[index].day_sunrise_set     = (TimerCalender[index].day_sunrise_set | TRUE);
                                                                               if(timeSeconds == 15 || timeSeconds == (ONE_DAY_SECONDS+5))
                                                                               {
                                                                                   TimerCalender[index].day_sunrise_set = FALSE;
                                                                                   APP_LOG("Rule", LOG_DEBUG, "15 or 86405 timer set sunrise set \
                                                                                           flag to: %d", TimerCalender[index].day_sunrise_set);
                                                                               }

										if(timeSeconds == (ONE_DAY_SECONDS+5))
										    pTimerList[nowIndex].time 		    	 = ONE_DAY_SECONDS;
										else
										    pTimerList[nowIndex].time 		    	 = timeSeconds ;
								}

#endif
								else{
										/*Assuming the rule to be a normal rule if the last digit is neither 1 nor 2*/
										pTimerList[nowIndex].sunrise_set         = NORMAL_RULE;
										TimerCalender[index].day_sunrise_set     = (TimerCalender[index].day_sunrise_set | FALSE);
										pTimerList[nowIndex].time 			     = timeSeconds;
								}

#ifdef SIMULATED_OCCUPANCY
								if(flag_lastbit == 3 || ((flag_lastbit == 4) && ((!strlen(szUDNRules))||(!strstr(szUDNRules, "["))) ) || flag_lastbit == 5)
								{
										if(!pgSimdevList)
										{
												if(parseSimulatedRule() != SUCCESS)
												{
														APP_LOG("Rule", LOG_DEBUG, "Simulated rule file parse failed...");
														retVal =0x02;
														goto onreturn;
														//return 0x02;
												}
												APP_LOG("Rule", LOG_DEBUG, "simulated rule parse done");
										}
								}
								else
								{
										/* unset, if weekly calendar has no simulated rule and but last manual toggle state set */
										if(gSimManualTrigger)
										{
												gSimManualTrigger = 0;
												memset(gSimManualTriggerDate, 0, sizeof(gSimManualTriggerDate));
												UnSetBelkinParameter(SIM_MANUAL_TRIGGER);
												UnSetBelkinParameter(SIM_MANUAL_TRIGGER_DATE);
												AsyncSaveData();
												APP_LOG("Rule", LOG_DEBUG, "not a simulated rule, unset gSimManualTrigger: %d and gSimManualTriggerDate: %s", gSimManualTrigger, gSimManualTriggerDate);
										}
								}
#endif
								pTimerList[nowIndex].sunrise_set_offset      = OffsetResult[nowIndex];

								pTimerList[nowIndex].action 		= timerAction;

								//- Add some algorithm here to improve the overriddable logic
								//- First one should not be overiddable, and if 
								if (0x00 == nowIndex)
								{
										//- First one always not overriddable
										pTimerList[nowIndex].isOverridable = 0x00;
								}
								else
								{
										// not first one, then compare with the last second one nowIndex - 1 
										int lastIndex = nowIndex - 1;
										if ((pTimerList[nowIndex].action != pTimerList[lastIndex].action) && 
														(0x00 == pTimerList[lastIndex].isOverridable)
											 )
										{
												//- The previous one is not not overidable and the action different from previous one two
												pTimerList[nowIndex].isOverridable = 0x01;
												//APP_LOG("Rule", LOG_ERR, ":%d->%d overridable", pTimerList[nowIndex].time, pTimerList[nowIndex].action);
										}
										else
										{
												if(pTimerList[nowIndex].time == 86400)// WEMO-17892
														pTimerList[nowIndex].isOverridable = 0x01;	
												else
														pTimerList[nowIndex].isOverridable = 0x00;	
												//APP_LOG("Rule", LOG_ERR, ":%d->%d NOT overridable", pTimerList[nowIndex].time, pTimerList[nowIndex].action);
										}
								}

								if (strlen(szUDNRules))
								{
										//APP_LOG("Rule", LOG_ERR, "New Rule: SENSOR RULE");
										//APP_LOG("Rule", LOG_DEBUG, "sensor rule: %s, to create device management", szUDNRules);

										pTimerList[nowIndex].pSocketList 
												= DecodeControlledDeviceList(szUDNRules, 
																&pTimerList[nowIndex].count, 
																&pTimerList[nowIndex].Delay,  
																&pTimerList[nowIndex].Sensitivity
																);

								}
								else
								{
										//APP_LOG("Rule", LOG_ERR, "New Rule: SOCKET RULE");
								}

								//APP_LOG("Rule", LOG_ERR, "Day: %s Time: %d, Action:%d", 
								//szWeekDayShortName[index], pTimerList[nowIndex].time,  pTimerList[nowIndex].action);
								nowIndex++;
						}
						else
						{
								APP_LOG("Rule", LOG_DEBUG, "Invalid timer instance");
								free(pTimerList);
								retVal =0x01;
								goto onreturn;
								//return 0x01;
						}
				}

				TimerCalender[index].pTimerList = pTimerList;
				TimerCalender[index].count      = timerCount;
				TimerCalender[index].latitude = latitude;
				TimerCalender[index].longitude = longitude;

				/* Praveen */
				//APP_LOG("Rule DecodeUpNpRule", LOG_DEBUG, "Day Sunrise_Set: %d, Longitude: %lf: Latitude: %lf", 
				//	TimerCalender[index].day_sunrise_set, TimerCalender[index].longitude, TimerCalender[index].latitude);
#if 0
				for (loop = 0x00; loop < TimerCalender[index].count; loop++)
				{
						APP_LOG("Rule DecodeUpNpRule", LOG_DEBUG, "Time: %d, action: %d: overridable: %d,  Sunrise Set: %d, Sunrise Set Offset is: %d ", 
										TimerCalender[index].pTimerList[loop].time, TimerCalender[index].pTimerList[loop].action, 
										TimerCalender[index].pTimerList[loop].isOverridable, TimerCalender[index].pTimerList[loop].sunrise_set, 
										TimerCalender[index].pTimerList[loop].sunrise_set_offset);
				}
#endif
				ResortDailyEntry(&TimerCalender[index]);

				isDeferred = DeferSecondDayEntry(&TimerCalender[index]);
				if (!isDeferred)
				{
						//- Resort again
						ResortDailyEntry(&TimerCalender[index]);
				}
				else
				{
						//APP_LOG("Rule", LOG_DEBUG, "No defer executed");
				}
		}
		else
		{		
				APP_LOG("DEVICE:rule", LOG_DEBUG, "No rule content for today: %s", szWeekDayName[index]);
				retVal =0x01;
				goto onreturn;
				//return 0x01;
		}
onreturn:
		return retVal;

}

/**
 * TODO: cleanup all calendar memory
 * 
 * 
 ********************************************************/ 
void CleanCalendar()
{
		int loop = 0x00;

		for (loop = 0x00; loop < WEEK_DAYS_NO; loop++)
		{
				sunrise_set_calculated[loop] = FALSE;
				if (0x00 != TimerCalender[loop].count)
				{
						if (0x00 != TimerCalender[loop].pTimerList) 
						{
								//- Free sensor rule
								//APP_LOG("DEVICE:rule", LOG_DEBUG, "#### %dth time event list data removed", loop);

								int i = 0x00;
								for (; i < TimerCalender[loop].count; i++)
								{
										if (0x00 != TimerCalender[loop].pTimerList[i].count)
										{
												if (0x00 != TimerCalender[loop].pTimerList[i].pSocketList)
												{
														FreeResource(TimerCalender[loop].pTimerList[i].pSocketList);
														TimerCalender[loop].pTimerList[i].pSocketList = 0x00;
														//APP_LOG("DEVICE:rule", LOG_DEBUG, "#### %dth socket list data removed", loop);
												}
												TimerCalender[loop].pTimerList[i].count = 0x00;
										}
								}
								//- The two extra empty ones automatically free from this
								FreeResource(TimerCalender[loop].pTimerList);
								TimerCalender[loop].pTimerList = 0x00;
						}
				}

				//APP_LOG("DEVICE:rule", LOG_DEBUG, "#### %dth data removed", loop);

				TimerCalender[loop].count = 0x00;
		}

		memset(TimerCalender, 0x00, sizeof(TimerCalender));
}

void GetCalendarNextDayInfo(int loop, int DayIdx, int* dayIndex, int* monthIndex, int* year, int* seconds)
{
    time_t rawTime;
    long nextDayRawTime = 0;
    struct tm * timeInfo;
    time(&rawTime);

    if(!loop) // - for next week
	nextDayRawTime = (long)(((ONE_DAY_SECONDS * DayIdx) + rawTime) + 1);
    else
	nextDayRawTime = (long)(((ONE_DAY_SECONDS * (loop - DayIdx)) + rawTime) + 1);	// - TODO: rohit, need to check for 0 and 86400 rawtime case

    timeInfo = localtime (&nextDayRawTime);

    *year 		= timeInfo->tm_year + 1900;
    *monthIndex 		= timeInfo->tm_mon;
    int dayOfWeek 	= timeInfo->tm_wday;
    current_date = timeInfo->tm_mday;

    //-map the 
    if (0x00 == dayOfWeek)
	dayOfWeek = 0x06;
    else
	dayOfWeek -=1;

    *dayIndex = dayOfWeek;

    *seconds = timeInfo->tm_hour * 60 * 60 + timeInfo->tm_min * 60 + timeInfo->tm_sec;

}



/***
 * Internal use function
 * 
 * 
 *******************************************************************************/
void GetCalendarDayInfo(int* dayIndex, int* monthIndex, int* year, int* seconds)
{
		time_t rawTime;
		struct tm * timeInfo;
		time(&rawTime);
		timeInfo = localtime (&rawTime);

		*year 		= timeInfo->tm_year + 1900;
		*monthIndex 		= timeInfo->tm_mon;
		int dayOfWeek 	= timeInfo->tm_wday;
		current_date = timeInfo->tm_mday;

		//-map the 
		if (0x00 == dayOfWeek)
				dayOfWeek = 0x06;
		else
				dayOfWeek -=1;

		*dayIndex = dayOfWeek;

		*seconds = timeInfo->tm_hour * 60 * 60 + timeInfo->tm_min * 60 + timeInfo->tm_sec;

}

#define INSIGHT_RULE_DIFF 20
int IsExecutedRuleNow(int DayOfWeekIndex, int nowSeconds, int* outCommand)
{
		APP_LOG("DEVICE:rule", LOG_DEBUG, "%s called", __FUNCTION__);
		int  isExcuted = 0x00;
		DailyTimerList* pNode = GetFirstSchedulerNode(DayOfWeekIndex);
		//Get today

		if (0x00 == pNode)
		{
				APP_LOG("DEVICE:rule", LOG_DEBUG, "No node found today");
				return 0x00;
		}

		if (pNode)
		{
				int loop = 0x00;
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Today's timer list:");
				int count = pNode->count;
				for (loop = 0x00; loop < count; loop++)
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Time: %d, action: %d: overridable: %d", pNode->pTimerList[loop].time, 
										pNode->pTimerList[loop].action, pNode->pTimerList[loop].isOverridable);
				}

				for (loop = 0x00; loop < pNode->count; loop++)
				{
						if (0x00 != g_iDstNowTimeStatus)
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "DST switched just right now, check edge case 1:00 am or 3:00 am");	

								if (DST_TIME_NOW_ON == g_iDstNowTimeStatus)
								{
										//- 3:00 am
										if (DST_TIME_ON_SECONDS == pNode->pTimerList[loop].time)
										{
												*outCommand = pNode->pTimerList[loop].action;

												if (g_eDeviceType == DEVICE_SENSOR)					
												{
														lockRule();
														g_pCurTimerControlEntry	 =  pNode->pTimerList + loop;
														unlockRule();
												}

												g_iDstNowTimeStatus = 0x00;
												return 0x01;
										}
								}
								else if (DST_TIME_NOW_OFF == g_iDstNowTimeStatus)
								{
										if (DST_TIME_OFF_SECONDS == pNode->pTimerList[loop].time)
										{
												*outCommand = pNode->pTimerList[loop].action;
												if (g_eDeviceType == DEVICE_SENSOR)					
												{
														lockRule();
														g_pCurTimerControlEntry	 =  pNode->pTimerList + loop;
														unlockRule();
												}

												g_iDstNowTimeStatus = 0x00;
												return 0x01;
										}						
								}
								else if (DST_TIME_NOW_ON_2 == g_iDstNowTimeStatus) //TODO: need to handle similar case for chatham isl. NZ, if gemtek will add support for it 
								{
										//- 2:00 am
										if (DST_TIME_ON_SECONDS_2 == pNode->pTimerList[loop].time)
										{
												*outCommand = pNode->pTimerList[loop].action;

												if (g_eDeviceType == DEVICE_SENSOR)					
												{
														lockRule();
														g_pCurTimerControlEntry	 =  pNode->pTimerList + loop;
														unlockRule();
												}

												g_iDstNowTimeStatus = 0x00;
												return 0x01;
										}
								}
						} //- end of 0x00 != g_iDstNowTimeStatus

						if (pNode->pTimerList[loop].time > nowSeconds)
						{
								if (0x00 != loop)
								{
										if (g_eDeviceType == DEVICE_SENSOR)
										{
												//- Executed sensor immediately
												isExcuted = 0x01;
												*outCommand = pNode->pTimerList[loop-1].action;
												SetSensorConfig(pNode->pTimerList[loop-1].Delay, pNode->pTimerList[loop-1].Sensitivity);
												lockRule();
												g_pCurTimerControlEntry			 =  pNode->pTimerList + loop - 1;
												APP_LOG("DEVICE:rule", LOG_DEBUG, "Sensor last event to execute now: %d", pNode->pTimerList[loop-1].action);

												//- Decide to clean up the historical overriding table or not, remember, not return here
												int isInTable = 0x00;
												isInTable = isTimeEventInOVerrideTable(g_pCurTimerControlEntry->time, 
																g_pCurTimerControlEntry->action);

												if (isInTable)
												{
														//- The end time not change, new rule(?) or 
														int deviceCount = g_pCurTimerControlEntry->count;
														int loop = 0x00;
														APP_LOG("Rule", LOG_DEBUG, "Sensor rule historical information found");

														for (; loop < deviceCount; loop++)
														{
																char* udn = g_pCurTimerControlEntry->pSocketList[loop].UND;
																if (IsDeviceInOverrideTable(udn))
																{
																		//- Do nothing since the device still in the overriden table
																		APP_LOG("Rule", LOG_DEBUG, "Device found in historical overriden table:%s", udn);
																		break;
																}
														}

														if (loop == deviceCount)
														{
																//- No UDN in the overridding table, it means brand new
																APP_LOG("Rule", LOG_DEBUG, "No managed device in the overridden table, clean up");
																ResetOverrideStatus();
																CleanupOverrideTable();
														}
												}
												else
												{
														//- Reset? end time change, ensure reset the historical information
														APP_LOG("Rule", LOG_DEBUG, "End time NOT in the overridden table, clean up");
														ResetOverrideStatus();
														CleanupOverrideTable();
												}

												unlockRule();

										}
#ifdef PRODUCT_WeMo_Insight
										else if (DEVICE_INSIGHT == g_eDeviceTypeTemp)
										{
												//- Executed Insight Rule immediately
												g_pCurTimerControlEntry			 =  pNode->pTimerList + loop - 1;
												APP_LOG("DEVICE:rule", LOG_DEBUG, "Insight last event to execute now: %d for Timer Count : %d", pNode->pTimerList[loop-1].action,(loop-1));
												int deviceCount = g_pCurTimerControlEntry->count;
												int isInTable = 0x00;
												if(!deviceCount)
												{
														APP_LOG("DEVICE:rule", LOG_DEBUG, "Insight Timer Rule As Device Count is : %d",deviceCount);
														isInTable = isTimeEventInOVerrideTable(pNode->pTimerList[loop].time, pNode->pTimerList[loop].action);										
														if (isInTable)
														{

																//- Coverage
																isExcuted = 0x03;                       
																unlockRule();                      
																APP_LOG("Rule", LOG_DEBUG, "Insight Timer Next time event in the overridding table, not be executed");
																return isExcuted;
														}
														else
														{
																//- Reset?                          
																ResetOverrideStatus();                  
														}

														unlockRule(); 


														if ((pNode->pTimerList[loop].isOverridable) || 
																		(0x00 != pNode->pTimerList[loop].isExtraRule))
														{
																//- Executed sensor immediately
																isExcuted = 0x01;
																*outCommand = pNode->pTimerList[loop-1].action + 2;
																lockRule();
																g_pCurTimerControlEntry =  pNode->pTimerList + loop - 1;
																g_pCurTimerControlEntry->isOverridable = 0x01;	//-Timer interval, could be override
																unlockRule();
																APP_LOG("DEVICE:rule", LOG_DEBUG, "Insight Timer Rule last event to execute now: %d", pNode->pTimerList[loop-1].action);
														}
												}
												else
												{
														APP_LOG("DEVICE:rule", LOG_DEBUG, "----Insight Rule As Device Count is : %d",deviceCount);
														isExcuted = 0x01;
														// If prev to this rule is timer rule execute it now
														APP_LOG("DEVICE:rule", LOG_DEBUG, "->>Rule Tree Last Time : %d Last to Last Time : %d",pNode->pTimerList[loop-1].time, pNode->pTimerList[loop-2].time);
														if((pNode->pTimerList[loop-1].time -  pNode->pTimerList[loop-2].time) == INSIGHT_RULE_DIFF)
														{
																*outCommand = pNode->pTimerList[loop-2].action + 2;
																g_isExecuteTimerToo = 1;
																g_overlappedInsightAction = pNode->pTimerList[loop-1].action;
														}
														else{
																*outCommand = pNode->pTimerList[loop-1].action;
														}
														lockRule();

														//- Decide to clean up the historical overriding table or not, remember, not return here
														isInTable = isTimeEventInOVerrideTable(g_pCurTimerControlEntry->time, 
																		g_pCurTimerControlEntry->action);

														if (isInTable)
														{
																//- The end time not change, new rule(?) or 
																int loop = 0x00;
																APP_LOG("Rule", LOG_DEBUG, "Insight rule historical information found");

																for (; loop < deviceCount; loop++)
																{
																		char* udn = g_pCurTimerControlEntry->pSocketList[loop].UND;
																		if (IsDeviceInOverrideTable(udn))
																		{
																				//- Do nothing since the device still in the overriden table
																				APP_LOG("Rule", LOG_DEBUG, "Device found in historical overriden table:%s", udn);
																				break;
																		}
																}

																if (loop == deviceCount)
																{
																		//- No UDN in the overridding table, it means brand new
																		APP_LOG("Rule", LOG_DEBUG, "No managed device in the overridden table, clean up");
																		ResetOverrideStatus();
																		CleanupOverrideTable();
																}
														}
														else
														{
																//- Reset? end time change, ensure reset the historical information
																APP_LOG("Rule", LOG_DEBUG, "End time NOT in the overridden table, clean up");
																ResetOverrideStatus();
																CleanupOverrideTable();
														}

														unlockRule();
												}
										}
#endif
										else if (g_eDeviceType == DEVICE_SOCKET)
										{
												//- Check the hostorical overridding table here					
												int isInTable = 0x00;					
												lockRule();					
												isInTable = isTimeEventInOVerrideTable(pNode->pTimerList[loop].time, pNode->pTimerList[loop].action);										
												if (isInTable)
												{
														//- Coverage
														isExcuted = 0x03;                       
														unlockRule();                      
														APP_LOG("Rule", LOG_DEBUG, "Next time event in the overridding table, not be executed");
														return isExcuted;
												}
												else
												{
														//- Reset?                          
														ResetOverrideStatus();                  
												}

												unlockRule(); 


												if ((pNode->pTimerList[loop].isOverridable) || 
																(0x00 != pNode->pTimerList[loop].isExtraRule))
												{
														//- Executed sensor immediately
														isExcuted = 0x01;
														*outCommand = pNode->pTimerList[loop-1].action;
														lockRule();
														g_pCurTimerControlEntry =  pNode->pTimerList + loop - 1;
														g_pCurTimerControlEntry->isOverridable = 0x01;	//-Timer interval, could be override
														unlockRule();
														APP_LOG("DEVICE:rule", LOG_DEBUG, "Socket last event to execute now: %d", pNode->pTimerList[loop-1].action);
												}
										}
								}
								else
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "No previous event found today: now time: %d", nowSeconds);					
								}

								break;
						}
				}
		}

		return isExcuted;

}

#ifdef SIMULATED_OCCUPANCY
int isNowTimerSimulated(int DayOfWeekIndex, int nowSeconds)
{
		int return_value= FALSE;
		DailyTimerList* pNode = GetFirstSchedulerNode(DayOfWeekIndex);

		APP_LOG("Rule", LOG_DEBUG, "is now timer Simulated ...");
		if (pNode)
		{
				int loop = 0x00;
				APP_LOG("Rule", LOG_DEBUG, "is now timer Simulated type");

				for (loop = 0x00; loop < pNode->count; loop++)
				{
						if (pNode->pTimerList[loop].time > nowSeconds)
						{
								/* Check whether simulated rule type timer */
								if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set) )
								{
										return_value = TRUE;
								}
								APP_LOG("Rule", LOG_DEBUG, "is now timer Simulated type: %s", (return_value==TRUE)?"yes":"no");
								break;
						}
				}
		}

		return return_value;
}

int checkForSimulatedTypeTimer(int *duration, int *startTimer, int *endTimer, int *simulated_duration, int DayOfWeekIndex, int nowSeconds)
{
		int return_value= FALSE;
		int nextEvent = 0;
		int i = 0;
		int dstIndex = -1;
		int nextDayOfWeekIndex = -1;
		int simulatedRuleTimerCount = 0;
		DailyTimerList* pNode = GetFirstSchedulerNode(DayOfWeekIndex);
		DailyTimerList* pTodayTmpNode = 0x00;
		DailyTimerList* pNextDayNode = 0x00;
		*startTimer = 0, *endTimer = 0;

		APP_LOG("Rule", LOG_DEBUG, "check For Simulated Type Timer...");
		if (pNode)
		{
				int loop = 0x00;
				int isSimulatedRule = 0;
				APP_LOG("Rule", LOG_DEBUG, "check For Simulated Type Timer");

				for (loop = 0x00; loop < pNode->count; loop++)
				{
				    isSimulatedRule = 0;
				    /* to check and return, if next timer is not simulated timer */
				    if (pNode->pTimerList[loop].time > nowSeconds)
				    {
					if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set) )
					    isSimulatedRule = 1;

					if (!gSimulatedRuleRunning && !isSimulatedRule)
					{
					    APP_LOG("Rule", LOG_DEBUG, "Next timer not a Simulated Type Timer");
					    gSimulatedRuleScheduled = 0x00;
					    return return_value; 
					}
				    }
						/* Check whether simulated rule type timer */
						if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set) )
						{
								int curTime = nowSimSeconds();
								simulatedRuleTimerCount++;
								if(!(*startTimer) || (simulatedRuleTimerCount > SIMULATED_MAX_TIMER_COUNT))  //update simulated start and end timers
								{
										*startTimer = 0, *endTimer = 0;
										if(simulatedRuleTimerCount > SIMULATED_MAX_TIMER_COUNT)
												simulatedRuleTimerCount = 0;
										*startTimer = pNode->pTimerList[loop].time;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Start Timer: %d", *startTimer);

										for(i=loop+1;i<pNode->count;i++)
										{
												if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[i].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[i].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[i].sunrise_set) )
												{
														gSimulatedRuleEndTime = *endTimer = pNode->pTimerList[i].time;
														break;
												}
										}

										if(*endTimer == 0)
										{
												APP_LOG("DEVICE:rule", LOG_ERR, "No end timer !!");
										}

										/* set simulated rule running if simulated start timer is past and end timer is not */
										if((curTime >= *startTimer) && (curTime <= *endTimer) && !gSimManualTrigger && !gSimRuleLastOccurenceOfDay & !gSimulatedRuleRunning)
										{
										    gSimulatedRuleRunning = 0x01;
										    *startTimer = curTime;
										    APP_LOG("DEVICE:rule", LOG_DEBUG, "Update Start Timer: %d", *startTimer);
										    simulatedStartControlPoint();
										    *simulated_duration = updateSimulatedDuration(*startTimer, *endTimer);
										    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule running: %d, duration: %d", gSimulatedRuleRunning, *simulated_duration);
										}

										APP_LOG("DEVICE:rule", LOG_DEBUG, "End Timer: %d", *endTimer);
										if((*endTimer - *startTimer) > SIMULATED_RULE_DURATION)
										{ 
												gSimRandomEndTime = SIMULATED_TIME_NEAR_LARGERULE_END_TIME;
												APP_LOG("DEVICE:rule", LOG_DEBUG, "Large rule end time, gSimRandomEndTime: %d", gSimRandomEndTime);
										}
										else
										{
												gSimRandomEndTime = SIMULATED_TIME_NEAR_SHORTRULE_END_TIME;
												APP_LOG("DEVICE:rule", LOG_DEBUG, "Short rule end time, gSimRandomEndTime: %d", gSimRandomEndTime);
										}
										/* to test with very short values */
										if(gDefaultSimFirstOnTime != SIMULATED_FIRST_ON_TIME)
										{
												gSimRandomEndTime = SIMULATED_TIME_NEAR_TESTRULE_END_TIME;	//1 min
												APP_LOG("DEVICE:rule", LOG_DEBUG, "test purpose, Custom very short rule end time, gSimRandomEndTime: %d", gSimRandomEndTime);
										}
								}

								if (pNode->pTimerList[loop].time > nowSeconds)
								{
										{
												if(gSimManualTrigger || gSimRuleLastOccurenceOfDay) //manual trigger or last occurence of day
												{
														APP_LOG("DEVICE:rule", LOG_DEBUG, "manual trigger: %d or last occurence of day: %d", gSimManualTrigger, gSimRuleLastOccurenceOfDay);
														gSimRuleLastOccurenceOfDay = 0x00;
														resetManualToggle();
														gSimulatedRuleRunning = 0x00;
														APP_LOG("DEVICE:rule", LOG_DEBUG, "unsetting gSimRuleLastOccurenceOfDay= %d or gSimManualTrigger= %d", gSimRuleLastOccurenceOfDay, gSimManualTrigger);
														continue;
												}
												APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule type timer: %d", pNode->pTimerList[loop].sunrise_set);
												if(!gSimulatedRuleRunning)
												{
														int seconds = nowSimSeconds();
														if(seconds < pNode->pTimerList[loop].time)
														{
																gSimRuleFirstOccurenceOfDay = 0x01;
																APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated Rule First Occurence Of Day: %d", gSimRuleFirstOccurenceOfDay);
														}
														APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimulatedRuleRunning: %d and gSimRuleFirstOccurenceOfDay: %d", gSimulatedRuleRunning, gSimRuleFirstOccurenceOfDay);
												}
#if 0
												if (!gSimFirstTimer)
												{
														gSimFirstTimer = pNode->pTimerList[loop].time;
														APP_LOG("DEVICE:rule", LOG_DEBUG, "First Timer: gSimFirstTimer: %ld", gSimFirstTimer);
												}
#endif
#if 0
												if(pNode->pTimerList[loop].time == ONE_DAY_SECONDS)	/* to handle across day simulated rule */
												{
														nextDayOfWeekIndex = DayOfWeekIndex + 1;
														APP_LOG("DEVICE:rule", LOG_DEBUG, "next day of week index is: %d", nextDayOfWeekIndex);
														if(nextDayOfWeekIndex == WEEK_DAYS_NO)
														{
																nextDayOfWeekIndex = 0;
																APP_LOG("DEVICE:rule", LOG_DEBUG, "Last day of week, reset nextDayOfWeekIndex to index: %d", nextDayOfWeekIndex);
														}
														pNextDayNode = GetFirstSchedulerNode(nextDayOfWeekIndex);
														if ( pNextDayNode && ((SIMULATED_OCCUPANCY_RULE == pNextDayNode->pTimerList[0x00].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNextDayNode->pTimerList[0x00].sunrise_set) || (SIMULATED_SUNSET_RULE == pNextDayNode->pTimerList[0x00].sunrise_set)) && (pNextDayNode->pTimerList[0x00].time == TIME_EVENT_BREAK_DURATION) )
														{
																*duration += (*simulated_duration+1);
																APP_LOG("DEVICE:rule", LOG_DEBUG, "Across day Simulated Rule, UPDATE duration to handle it: %d", *duration);
														}
														else
														{
																APP_LOG("DEVICE:rule", LOG_DEBUG, "keep original duration: %d", *duration);
														}
												}
#endif
												gSimulatedRuleScheduled = 0x01;
												APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimulatedRuleScheduled is: %d", gSimulatedRuleScheduled);
										}
										nextEvent = 0x01;
										break;
								}
						}
				}

				if (loop == pNode->count)
				{
						APP_LOG("Rule", LOG_DEBUG,"Today: No timer later than now:%d", nowSeconds);
						pTodayTmpNode = pNode;
				}

				return_value= TRUE;
		}


		/* condition to check if first timer of next day is of simulated type */
		if (0x00 == nextEvent)
		{
				int loop = 0x01;
				int curDay = 0x01;
				gSimulatedRuleScheduled = 0x00;
				APP_LOG("Rule", LOG_DEBUG, "No data node found today, now seeking the weekly day data");

				for (; loop < WEEK_DAYS_NO; loop++)
				{
						curDay = DayOfWeekIndex + loop;
						pNode  = GetFirstSchedulerNode(curDay % WEEK_DAYS_NO);
						dstIndex = curDay % WEEK_DAYS_NO;
						if (pNode)
						{
								APP_LOG("Rule", LOG_DEBUG, "weekly date data found [index:dayOfweek]:%d:%d", curDay, dstIndex);
								break;
						}
				}

				if (loop == WEEK_DAYS_NO)
				{
						APP_LOG("Rule", LOG_DEBUG, "Exception: no any weekly date data found, should not occur");
						//- Need to jump back to today again since there is the only one rule of the entire week
						if (0x00 != pTodayTmpNode)
						{
								if ( (SIMULATED_OCCUPANCY_RULE == pTodayTmpNode->pTimerList[0x00].sunrise_set) || (SIMULATED_SUNRISE_RULE == pTodayTmpNode->pTimerList[0x00].sunrise_set) || (SIMULATED_SUNSET_RULE == pTodayTmpNode->pTimerList[0x00].sunrise_set) )
								{
										APP_LOG("Rule", LOG_DEBUG, "next week first timer is of simulated type");
										gSimRuleFirstOccurenceOfNextDay = 0x01;
										APP_LOG("Rule", LOG_DEBUG, "set gSimRuleFirstOccurenceOfNextDay flag: %d", gSimRuleFirstOccurenceOfNextDay);
#if 0
										gSimFirstTimer = ONE_DAY_SECONDS - nowSeconds + (WEEK_DAYS_NO - 1) * ONE_DAY_SECONDS + pTodayTmpNode->pTimerList[0x00].time;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Next week day First Timer: gSimFirstTimer: %ld", gSimFirstTimer);
#endif
										*startTimer = ONE_DAY_SECONDS - nowSeconds + (WEEK_DAYS_NO - 1) * ONE_DAY_SECONDS + pTodayTmpNode->pTimerList[0x00].time;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Next week day First Timer: %d", *startTimer);
										for(i=1;i<pTodayTmpNode->count;i++)
										{
												if ( (SIMULATED_OCCUPANCY_RULE == pTodayTmpNode->pTimerList[i].sunrise_set) || (SIMULATED_SUNRISE_RULE == pTodayTmpNode->pTimerList[i].sunrise_set) || (SIMULATED_SUNSET_RULE == pTodayTmpNode->pTimerList[i].sunrise_set) )
												{
														gSimulatedRuleEndTime = *endTimer = ONE_DAY_SECONDS - nowSeconds + (WEEK_DAYS_NO - 1) * ONE_DAY_SECONDS + pTodayTmpNode->pTimerList[i].time;
														break;
												}
										}

										if(*endTimer == 0)
										{
												APP_LOG("DEVICE:rule", LOG_ERR, "No end timer !!");
										}
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Next week day end Timer: %d", *endTimer);
										gSimulatedRuleScheduled = 0x01;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimulatedRuleScheduled is: %d", gSimulatedRuleScheduled);
								}
						}
						else
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Nothing find in current calendar, even there is no any day on calendar");
						}
				}
				else
				{
						if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[0x00].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[0x00].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[0x00].sunrise_set) )
						{
								APP_LOG("Rule", LOG_DEBUG, "next day first timer is of simulated type");
								gSimRuleFirstOccurenceOfNextDay = 0x01;
								APP_LOG("Rule", LOG_DEBUG, "set gSimRuleFirstOccurenceOfNextDay flag: %d", gSimRuleFirstOccurenceOfNextDay);
#if 0
								gSimFirstTimer = ONE_DAY_SECONDS - nowSeconds + (loop - 1) * ONE_DAY_SECONDS + pNode->pTimerList[0x00].time;
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Next day First Timer: gSimFirstTimer: %ld", gSimFirstTimer);
#endif
								*startTimer = ONE_DAY_SECONDS - nowSeconds + (loop - 1) * ONE_DAY_SECONDS + pNode->pTimerList[0x00].time;
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Next day First Timer: %d", *startTimer);
								for(i=1;i<pNode->count;i++)
								{
										if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[i].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[i].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[i].sunrise_set) )
										{
												gSimulatedRuleEndTime = *endTimer = ONE_DAY_SECONDS - nowSeconds + (loop - 1) * ONE_DAY_SECONDS + pNode->pTimerList[i].time;
												break;
										}
								}

								if(*endTimer == 0)
								{
										APP_LOG("DEVICE:rule", LOG_ERR, "No end timer !!");
								}
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Next day end Timer: %d", *endTimer);
								gSimulatedRuleScheduled = 0x01;
								APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimulatedRuleScheduled is: %d", gSimulatedRuleScheduled);
						}
				}
		}

		if(gSimRuleLastOccurenceOfDay)
		{
		    gSimRuleLastOccurenceOfDay = 0x00;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "unsetting gSimRuleLastOccurenceOfDay= %d", gSimRuleLastOccurenceOfDay);
		}

		APP_LOG("Rule", LOG_DEBUG, "checkForFirstSimulatedTypeTimerdone...");
		return return_value;
}
#endif

int UpdateSunriseSunset (int DayOfWeekIndex, int month_index, int year, int nowSeconds)
{
		int month;
		int return_value= FALSE;
		DailyTimerList* pNode = GetFirstSchedulerNode(DayOfWeekIndex);

		if (pNode)
		{
				int loop = 0x00;
				APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "Today's Sunrise/Sunset timer list:");

				if(pNode->day_sunrise_set == TRUE)
				{
						double lon=0x00, lat=0x00;
						double trise=0x00, tset=0x00;
						int rise_hours = 0x00;
						int rise_mins = 0x00;
						int set_hours = 0x00;
						int set_mins = 0x00;
						int timezone_offset = 0x00;
						int time_hours = 0x00;
						float time_minutes = 0.0;
						float adjusted_time_zone = 0.0;

						lon = pNode->longitude;
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "Longitude is %lf", lon);
						lat = pNode->latitude;
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "Latitude is %lf", lat);

						if(month_index == 11)
						{
								month = 0;
						}
						else
						{
								month= month_index+1;
						}
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "year: %d, month: %d, date: %d, day index: %d", year, month, current_date, DayOfWeekIndex);
						/* calculate sunrise/set -- call the macro from sunriset.h */
						sun_rise_set(year, month, current_date, lon, lat, &trise, &tset);

						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "After sun_rise_set(year, month, current_date, lon, lat, &trise, &tset)");
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "trise:%lf, tset:%lf", trise, tset);

						rise_hours = HOURS(trise);
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "rise_hours is %d", rise_hours);
						rise_mins = MINUTES(trise);
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "rise_mins is %d", rise_mins);
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "Sunrise timings for the day are: time: %d, minutes is %d, date is %d", rise_hours, 
										rise_mins, current_date );

						set_hours = HOURS(tset);
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "set_hours is %d", set_hours);
						set_mins = MINUTES(tset);
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "set_mins is %d", set_mins);
						APP_LOG("UpdateSunriseSunset", LOG_DEBUG, "Sunset timings for the day are: time: %d, minutes is %d, date is %d", set_hours, 
										set_mins, current_date );

						/* Time zone adjustment based on stored value */
						/* As we are storing value in float ex. 5.5 for 5:30, we need to convert .5 into minutes,
						 * So need to multiply with 60 before converting into Sec. 
						 */
						APP_LOG("DEVICE:rule", LOG_DEBUG, "g_lastTimeZone: %f, gDstSupported: %d, gDstEnable: %f", g_lastTimeZone, gDstSupported, gDstEnable);

						if(gDstSupported && !gDstEnable)
						{
							adjusted_time_zone =  g_lastTimeZone + 1;

							APP_LOG("DEVICE:rule", LOG_DEBUG, "g_lastTimeZone: %f, adjusted_time_zone: %f", g_lastTimeZone, adjusted_time_zone);
						}
						else
							adjusted_time_zone =  g_lastTimeZone;
							
						time_hours =  (int)adjusted_time_zone;
						time_minutes = adjusted_time_zone - time_hours;
						timezone_offset = ((time_hours * 60) +(time_minutes*60)) * 60;

						for (loop = 0x00; loop < pNode->count; loop++)
						{
								/* Check whether Sunrise timer and update new sunrise time*/
								if ( SUN_RISE_RULE == pNode->pTimerList[loop].sunrise_set)
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Sunrise Timer find: time: %d, rule is %d and offset is %d", pNode->pTimerList[loop].time, 
														pNode->pTimerList[loop].sunrise_set, pNode->pTimerList[loop].sunrise_set_offset);

										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = timezone_offset +(((rise_hours * 60) +rise_mins) * 60);
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Updated Sunrise time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);

										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = TimerCalender[DayOfWeekIndex].pTimerList[loop].time + 
												pNode->pTimerList[loop].sunrise_set_offset;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "After Updating Offset time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);
								}
								else if ( SUN_SET_RULE == pNode->pTimerList[loop].sunrise_set)
								{
										/* Check whether Sunset timer and update new sunset time*/
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Sunset Timer find: time: %d, rule is %d and offset is %d", pNode->pTimerList[loop].time, 
														pNode->pTimerList[loop].sunrise_set, pNode->pTimerList[loop].sunrise_set_offset);

										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = timezone_offset +(((set_hours * 60) +set_mins) * 60);
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Updated Sunset time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);

										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = TimerCalender[DayOfWeekIndex].pTimerList[loop].time + 
												pNode->pTimerList[loop].sunrise_set_offset;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "After Updating Offset time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);
								}
#ifdef SIMULATED_OCCUPANCY
								else if ( SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set)
								{
										/* Normal Simulated Rule: Proceed to next timer */
										APP_LOG("DEVICE:rule",LOG_DEBUG,"Normal Simulated Timer");
								}
								else if ( SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set)
								{
										APP_LOG("DEVICE:rule",LOG_DEBUG,"Simulated Sunrise Timer find: time: %d, rule is %d", pNode->pTimerList[loop].time, 
														pNode->pTimerList[loop].sunrise_set);

										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = timezone_offset +(((rise_hours * 60) +rise_mins) * 60);
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Updated Simulated Sunrise time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);
										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = TimerCalender[DayOfWeekIndex].pTimerList[loop].time +
										    pNode->pTimerList[loop].sunrise_set_offset + SIMULATED_SUNRISE_RULE;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "After Updating Offset time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);
								}
								else if ( SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set)
								{
										/* Check whether Sunset timer and update new sunset time*/
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Sunset Timer find: time: %d, rule is %d", pNode->pTimerList[loop].time, 
														pNode->pTimerList[loop].sunrise_set);

										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = timezone_offset +(((set_hours * 60) +set_mins) * 60);
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Updated Sunset time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);
										TimerCalender[DayOfWeekIndex].pTimerList[loop].time = TimerCalender[DayOfWeekIndex].pTimerList[loop].time +
										    pNode->pTimerList[loop].sunrise_set_offset + SIMULATED_SUNSET_RULE;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "After Updating Offset time: %d", TimerCalender[DayOfWeekIndex].pTimerList[loop].time);
								}
#endif
								else
								{
										/* Normal Rule: Proceed to next timer */
								}
						}
						return_value= TRUE;
				}
		}

		return return_value;
}

static int GetNextTimerDuration(int DayOfWeekIndex, int nowSeconds)
{
		cntDuration2NextEvent = 0x00;
		DailyTimerList* pNode = GetFirstSchedulerNode(DayOfWeekIndex);
		DailyTimerList* pTodayTmpNode = 0x00;

		int dstIndex = -1;
#ifdef SIMULATED_OCCUPANCY
		int checked = 0;
#endif
		//Get today
		if (pNode)
		{
				int loop = 0x00;
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Today's timer list:");
				//int count = pNode->count;
				//for (loop = 0x00; loop < count; loop++)
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Time: %d, action: %d, overriddable: %d", pNode->pTimerList[loop].time, 
										pNode->pTimerList[loop].action, pNode->pTimerList[loop].isOverridable);
				}

				for (loop = 0x00; loop < pNode->count; loop++)
				{
						if (pNode->pTimerList[loop].time > nowSeconds)
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "First Timer find: time: %d", pNode->pTimerList[loop].time);
#ifdef SIMULATED_OCCUPANCY
								if ( ((SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set)) && (gSimManualTrigger||gSimRuleLastOccurenceOfDay) && !checked )
								{
										checked = 1;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule type and manual trigger: %d or last occurence: %d and checked: %d", gSimManualTrigger, gSimRuleLastOccurenceOfDay, checked);
										gSimulatedRuleRunning = 0x00;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "unset gSimulatedRuleRunning: %d", gSimulatedRuleRunning);
										continue;
								}
								else if(((SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set)) && gSimulatedRuleRunning)
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule timer: %d  and simulated rule running: %d", pNode->pTimerList[loop].sunrise_set, gSimulatedRuleRunning);
								}
								else if(gSimulatedRuleRunning && !gSimRuleLastOccurenceOfDay)
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Non Simulated rule timer: %d  and simulated rule running: %d", pNode->pTimerList[loop].sunrise_set, gSimulatedRuleRunning);
										continue;
								}
								else
								{
										gSimulatedRuleRunning = 0x00;
										APP_LOG("DEVICE:rule", LOG_DEBUG, "Non Simulated rule timer: %d  and simulated rule running: %d", pNode->pTimerList[loop].sunrise_set, gSimulatedRuleRunning);
								}
#endif
								//- Uniform the interface here
								if (g_eDeviceType == DEVICE_SOCKET)
								{         
										//- check socket device if next one overridden or not
										int isIntable = isTimeEventInOVerrideTable(pNode->pTimerList[loop].time, 
														pNode->pTimerList[loop].action);

										if (isIntable)
										{
												// Cloest next one is overridden last time, so loop to next close one
												APP_LOG("Rule", LOG_DEBUG, "first time:%d:%d in overridden table, skip", pNode->pTimerList[loop].time, 
																pNode->pTimerList[loop].action);      
												continue;
										}         
								}
								cntDuration2NextEvent = pNode->pTimerList[loop].time - nowSeconds;
								curTimerEntry = pNode->pTimerList + loop;

								/*
									 Check it could be override or not, please note, it will only override today's since timer interval will only be 
									 applied within same day
								 */

								if (0x00 != loop)
								{
										//- not the first one, so have to check it is timer interval or not
										if(loop < (pNode->count - 1)){
												nextTimerEntry = pNode->pTimerList + (loop + 1);
										}
										else
										{
												nextTimerEntry = 0x00;
										}

								}
								else
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "###### Today's first event, no override check");
										curTimerEntry->isOverridable = 0x00;
								}

								dstIndex = DayOfWeekIndex;
								APP_LOG("DEVICE:rule", LOG_DEBUG, "%s: wait %d seconds to trigger event to: %d, overridable: %d", szWeekDayName[DayOfWeekIndex], 
												cntDuration2NextEvent, curTimerEntry->action, curTimerEntry->isOverridable);
								break;
						}
				}

				if (loop == pNode->count)
				{
						APP_LOG("Rule", LOG_DEBUG,"Today: No timer later than now:%d", nowSeconds);
						pTodayTmpNode = pNode;
						cntDuration2NextEvent = 0x00;
				}
		}
		else
		{
				APP_LOG("Rule", LOG_DEBUG, "No data node found today");
				cntDuration2NextEvent = 0x00;
		}

#ifdef SIMULATED_OCCUPANCY
		//else
		if(gSimManualTrigger)
		{
				APP_LOG("Rule", LOG_DEBUG, "Simulated rule: Get Next Timer Duration");
				gSimRuleFirstOccurenceOfDay = 0x00;
				gSimRuleLastOccurenceOfDay = 0x00;
				//gSimulatedRuleRunning = 0x00;
				//gSimFirstTimer = 0x00;
				APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimRuleFirstOccurenceOfDay: %d gSimRuleLastOccurenceOfDay: %d and gSimulatedRuleRunning: %d gSimFirstTimer: %d", gSimRuleFirstOccurenceOfDay, gSimRuleLastOccurenceOfDay, gSimulatedRuleRunning, gSimFirstTimer);
				//TODO: check condition to stop control point
				if(!gProcessData)
				{
						StopPluginCtrlPoint();
						APP_LOG("Rule", LOG_DEBUG, "Control point stopped...");
				}
		}
#endif

		if (0x00 == cntDuration2NextEvent)
		{
				//-nothing today
				int loop = 0x01;
				int curDay = 0x01;
				APP_LOG("Rule", LOG_DEBUG, "No data node found today, now seeking the weekly day data");
				for (; loop < WEEK_DAYS_NO; loop++)
				{
						curDay = DayOfWeekIndex + loop;
						pNode  = GetFirstSchedulerNode(curDay % WEEK_DAYS_NO);
						dstIndex = curDay % WEEK_DAYS_NO;
						if (pNode)
						{
								APP_LOG("Rule", LOG_DEBUG, "weekly date data found [index:dayOfweek]:%d:%d", curDay, dstIndex);
								break;
						}
				}

				if (loop == WEEK_DAYS_NO)
				{
					APP_LOG("Rule", LOG_DEBUG, "Exception: no any weekly date data found, should not occur");

					// - next week calculations
					int iLoop = 0, dayIdx = 0, curDay = 0, nextWeekDayIndex = 0, dayIndex = 0, monthIndex = 0, year = 0, seconds = 0;
					for (; dayIdx < WEEK_DAYS_NO; dayIdx++)
					{
						lockRule();
						sunrise_set_calculated[dayIdx] = FALSE;
						unlockRule();
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Unset all sunrise set calc flag= %d:%d", dayIdx, sunrise_set_calculated[dayIdx]);
						APP_LOG("DEVICE:rule", LOG_DEBUG, "TimerCaldr[%d].day_sunrise_set = %d", dayIdx, TimerCalender[dayIdx].day_sunrise_set);
					}
					nextWeekDayIndex = WEEK_DAYS_NO - DayOfWeekIndex; 
					APP_LOG("DEVICE:rule", LOG_DEBUG, "nextWeekDayIndex: %d", nextWeekDayIndex);

					for (dayIdx= 0; dayIdx< WEEK_DAYS_NO; dayIdx++)
					{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if( seconds <= RULES_BASIC_TRIGGER_TIME ): %d", dayIdx);
						curDay = DayOfWeekIndex + dayIdx;
						iLoop = curDay % WEEK_DAYS_NO;
						APP_LOG("DEVICE:rule", LOG_DEBUG, "*******curDay: %d and iLoop: %d*******", curDay, iLoop);

						if((sunrise_set_calculated[iLoop]==FALSE)&&(TimerCalender[iLoop].day_sunrise_set==TRUE))
						{
							APP_LOG("DEVICE:rule", LOG_DEBUG, "Value of sunrise_set_calculated[iLoop] = %d", sunrise_set_calculated[iLoop]);
							APP_LOG("DEVICE:rule", LOG_DEBUG, "Value of TimerCalender[iLoop].day_sunrise_set = %d", TimerCalender[iLoop].day_sunrise_set);

							//if(dayIdx > DayOfWeekIndex)
							{
								GetCalendarNextDayInfo(0, (nextWeekDayIndex+curDay), &dayIndex, &monthIndex, &year, &seconds);
								APP_LOG("DEVICE:rule", LOG_DEBUG, "year: %d, month: %d, date : %d, day: %s, now time in seconds: %d", year, monthIndex, 
										current_date, szWeekDayName[dayIndex], seconds);
							}

							int return_value = UpdateSunriseSunset(dayIndex, monthIndex, year, seconds);
							APP_LOG("DEVICE:rule", LOG_DEBUG, "After int return_value = UpdateSunriseSunset(iLoop, monthIndex, year, seconds) = %d",return_value);
							if (return_value == TRUE)
							{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if (return_value == TRUE)");
								lockRule();
								sunrise_set_calculated[iLoop] = TRUE;
								unlockRule();
								ResortDailyEntry(&TimerCalender[iLoop]);
								APP_LOG("DEVICE:rule", LOG_DEBUG, "AfterResortDailyEntry(&TimerCalender[iLoop])");
							}
						}
					}


					//- Need to jump back to today again since there is the only one rule of the entire week
					if (0x00 != pTodayTmpNode)
					{
						APP_LOG("DEVICE:rule", LOG_DEBUG,"Only one day entry found in the entire week, it is today");
						cntDuration2NextEvent = ONE_DAY_SECONDS - nowSeconds + (WEEK_DAYS_NO - 1) * ONE_DAY_SECONDS + pTodayTmpNode->pTimerList[0x00].time;
						curTimerEntry = pTodayTmpNode->pTimerList;
					}
					else
					{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Nothing find in current calendar, even there is no any day on calendar");
					}

				}
				else
				{
					cntDuration2NextEvent = ONE_DAY_SECONDS - nowSeconds + (loop - 1) * ONE_DAY_SECONDS + pNode->pTimerList[0x00].time;
					curTimerEntry = pNode->pTimerList;
					// - unset current day sunrise set calc flag
					lockRule();
					sunrise_set_calculated[DayOfWeekIndex] = FALSE;
					unlockRule();
					APP_LOG("DEVICE:rule", LOG_ERR, "Unset sunrise set calculated flag for day index: %d to: %d", DayOfWeekIndex,
							sunrise_set_calculated[DayOfWeekIndex]);
				}
		}

		if (-1 != dstIndex)
		{
				APP_LOG("DEVICE:rule", LOG_DEBUG, "%s: cntDuration2NextEvent: %d", szWeekDayName[dstIndex], cntDuration2NextEvent);
		}
		else
		{
				APP_LOG("DEVICE:rule", LOG_ERR, "Exception: can not find any date index in the table");
		}

		return cntDuration2NextEvent;

}

void RuleToggleLed(int curState)
{
		pMessage msg = 0x00;

		if (0x00 == curState)
		{
				msg = createMessage(RULE_MESSAGE_OFF_IND, 0x00, 0x00);
		}
		else if (0x01 == curState)
		{
				msg = createMessage(RULE_MESSAGE_ON_IND, 0x00, 0x00);
		}

		SendMessage(PLUGIN_E_RELAY_THREAD, msg);
		SetLastUserActionOnState(curState);

		APP_LOG("Button", LOG_DEBUG, "rule ON/OFF message sent out");
}

void *RulesTask(void *args)
{
		tu_set_my_thread_name( __FUNCTION__ );
		APP_LOG("DEVICE:rule", LOG_DEBUG, "to start rules task");
		//TODO: Add protection here
		sleep(2);

#ifdef SIMULATED_OCCUPANCY
		int simulatedduration = 0x00;
		int duration_delta = 0x00;
		int original_duration = 0x0;
		int startTimer = 0x00;
		int endTimer = 0x00;
#endif

		int year = 0x00, monthIndex = 0x00, seconds = 0x00, dayIndex = 0x00;
		int duration = 0x00;

		int FirstCommand = 0x00;
		int isExecuteLasteEvent = 0x00;

		while(1)
		{
				GetCalendarDayInfo(&dayIndex, &monthIndex, &year, &seconds);
				if(year != 2000)
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "*************start rules TASK NOW YEAR IS NOT 2000");
						break;
				}
				pluginUsleep(10*1000000);  //wait for 10 sec and check again

		}

		/* update once sunrise set timers, if any */
		int loop = 0, iLoop = 0, curDay = 0; 
		int curDayIndex = dayIndex;		
		APP_LOG("DEVICE:rule", LOG_DEBUG, "Current day index = %d", curDayIndex);

		for (iLoop=0; iLoop < WEEK_DAYS_NO; iLoop++)
		{
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if( seconds <= RULES_BASIC_TRIGGER_TIME ): %d", iLoop);
		    curDay = curDayIndex + iLoop;
		    loop = curDay % WEEK_DAYS_NO;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "*******curDay: %d and loop: %d*******", curDay, loop);

		    if((sunrise_set_calculated[loop]==FALSE)&&(TimerCalender[loop].day_sunrise_set==TRUE))
		    {
			/* Reset the yesterday sunrise/set calculation to allow calculation for next week*/
			if (0x00 == loop){
			    APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if (0x00 == loop)");
			    lockRule();
			    sunrise_set_calculated[0x06] = FALSE;
			    unlockRule();
			}
			else{
			    APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside else of if (0x00 == loop)");
			    lockRule();
			    sunrise_set_calculated[loop-1] = FALSE; 
			    unlockRule();
			}

			APP_LOG("DEVICE:rule", LOG_DEBUG, "Value of sunrise_set_calculated[loop] = %d", sunrise_set_calculated[loop]);
			APP_LOG("DEVICE:rule", LOG_DEBUG, "Value of TimerCalender[loop].day_sunrise_set = %d", TimerCalender[loop].day_sunrise_set);

			//if ((sunrise_set_calculated[loop] == FALSE) && (TimerCalender[loop].day_sunrise_set == TRUE))
			{

			    if(curDay > curDayIndex)
			    {
				GetCalendarNextDayInfo(curDay, curDayIndex, &dayIndex, &monthIndex, &year, &seconds);
				APP_LOG("DEVICE:rule", LOG_DEBUG, "year: %d, month: %d, date : %d, day: %s, now time in seconds: %d", year, monthIndex, 
					current_date, szWeekDayName[dayIndex], seconds);
			    }

			    int return_value = UpdateSunriseSunset(dayIndex, monthIndex, year, seconds);
			    APP_LOG("DEVICE:rule", LOG_DEBUG, "After int return_value = UpdateSunriseSunset(loop, monthIndex, year, seconds) = %d",return_value);
			    if (return_value == TRUE)
			    {
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if (return_value == TRUE)");
				lockRule();
				sunrise_set_calculated[loop] = TRUE;
				unlockRule();
				APP_LOG("DEVICE:rule", LOG_DEBUG, "set sunrise set calc flag- %d:%d", loop, sunrise_set_calculated[loop]);
				ResortDailyEntry(&TimerCalender[loop]);
				APP_LOG("DEVICE:rule", LOG_DEBUG, "AfterResortDailyEntry(&TimerCalender[loop])");
			    }
			}
		    }
		}
		/* endof sunrise set timers update */

		GetCalendarDayInfo(&dayIndex, &monthIndex, &year, &seconds);
		APP_LOG("DEVICE:rule", LOG_DEBUG, "year: %d, month: %d, date : %d, day: %s, now time in seconds: %d", year, monthIndex, current_date,
			szWeekDayName[dayIndex], seconds);

		isExecuteLasteEvent = IsExecutedRuleNow(dayIndex, seconds, &FirstCommand);
		if (0x01 == isExecuteLasteEvent)
		{
				if (DEVICE_INSIGHT == g_eDeviceTypeTemp)
				{
#ifdef PRODUCT_WeMo_Insight
						// check if simple timer rule is to be executed or Insight rule to be execute
						APP_LOG("DEVICE:rule", LOG_DEBUG, "~~~~~~~~~~~~~~~~FirstCommand: %d",FirstCommand);
						lockRule();
						if(FirstCommand > 1)
						{
								int InsightRuleAction = !(3 - FirstCommand);//On/Off action is derived from value of First command 
								APP_LOG("DEVICE:rule", LOG_DEBUG, "~~~~~~~~~~~~~~~~InsightRuleAction: %d",InsightRuleAction);
								if(g_StateLog)
										APP_LOG("Rule", LOG_ALERT, "######### RELAY ON/OFF VALUE: %d FROM RULE EXECUTE NOW",InsightRuleAction);
#ifdef SIMULATED_OCCUPANCY
								int ret;
								if(isNowTimerSimulated(dayIndex, seconds))
								{
										APP_LOG("Rule", LOG_DEBUG, "#### Is execute Now: simulated rule...");
								}
								else
								{
										setActuation(ACTUATION_AWAY_RULE);
										ret = ChangeBinaryState(InsightRuleAction);
								}
#else
								setActuation(ACTUATION_TIME_RULE);
								int ret = ChangeBinaryState(InsightRuleAction);
#endif
								if (ret == 0x00)
								{
										RuleToggleLed(InsightRuleAction);
								}
								if(g_isExecuteTimerToo == 1) 
								{
										g_isExecuteTimerToo = 0;//Execute the Timer as well as Insight Rule.
										g_isInsightRuleActivated = g_overlappedInsightAction;
										APP_LOG("Rule", LOG_DEBUG, "#### Is execute Now ACTIVATING INSIGHT RULE ALONG WITH TIMER RULE: %d",g_isInsightRuleActivated);	
								}
						}
						else
						{
								g_isInsightRuleActivated = FirstCommand;
								APP_LOG("Rule", LOG_DEBUG, "#### Is execute Now ACTIVATING INSIGHT RULE");	
						}

						unlockRule();
#endif
				}
				else if (DEVICE_SOCKET == g_eDeviceType)
				{
#ifdef SIMULATED_OCCUPANCY
						int ret = -1;
						if(isNowTimerSimulated(dayIndex, seconds))
						{
								APP_LOG("Rule", LOG_DEBUG, "#### Is execute Now: simulated rule...");
						}
						else
						{
								setActuation(ACTUATION_AWAY_RULE);
								ret = ChangeBinaryState(FirstCommand);
						}
#else
						//TODO: check if change binary state requires here, gSimulatedRuleRunning is 1 
						setActuation(ACTUATION_TIME_RULE);
						int ret = ChangeBinaryState(FirstCommand);
#endif
						if (ret == 0x00)
						{
								RuleToggleLed(FirstCommand);
						}
				}
				else if (DEVICE_SENSOR == g_eDeviceType)
				{
						lockRule();
						g_isSensorRuleActivated = FirstCommand;
						unlockRule();
				}
		}
		else if (0x00 == isExecuteLasteEvent)
		{
				//- Not at all
				lockRule();
				CleanupOverrideTable();
				ResetOverrideStatus();
				unlockRule();
		}
		else
		{
				//- Do nothing and keep all historical info
		}

		while(1)
		{
				year = 0;
				monthIndex = 0;
				seconds = 0;
				dayIndex = 0;
				duration = 0;

				/* Sunrise/set calculation */
				GetCalendarDayInfo(&dayIndex, &monthIndex, &year, &seconds);
#ifdef SIMULATED_OCCUPANCY
				APP_LOG("DEVICE:rule", LOG_DEBUG, "now time in seconds: %d duration delta: %d", seconds, duration_delta);
				seconds = (seconds + duration_delta);
				if(seconds > ONE_DAY_SECONDS)
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "now time in seconds > one day seconds, adjust seconds");
						seconds = ONE_DAY_SECONDS;
				}
				APP_LOG("DEVICE:rule", LOG_DEBUG, "now time in seconds after duration delta adjust: %d", seconds);
#endif

				APP_LOG("DEVICE:rule", LOG_DEBUG, "After GetCalendarDayInfo");

				APP_LOG("DEVICE:rule", LOG_DEBUG, "year: %d, month: %d, date : %d, day: %s, now time in seconds: %d", year, monthIndex, current_date,
						szWeekDayName[dayIndex], seconds);

				loop = 0; iLoop = 0; curDay = 0; 
				curDayIndex = dayIndex;		
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Current day index = %d", curDayIndex);

				//if( seconds <= RULES_BASIC_TRIGGER_TIME ) 		
				{    
					for (iLoop=0; iLoop < WEEK_DAYS_NO; iLoop++)
					{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if( seconds <= RULES_BASIC_TRIGGER_TIME ): %d", iLoop);
						curDay = curDayIndex + iLoop;
						loop = curDay % WEEK_DAYS_NO;
						APP_LOG("DEVICE:rule", LOG_DEBUG, "*******curDay: %d and loop: %d*******", curDay, loop);

						if((sunrise_set_calculated[loop]==FALSE)&&(TimerCalender[loop].day_sunrise_set==TRUE))
						{
							/* Reset the yesterday sunrise/set calculation to allow calculation for next week*/
							if (0x00 == loop){
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if (0x00 == loop)");
								lockRule();
								sunrise_set_calculated[0x06] = FALSE;
								unlockRule();
							}
							else{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside else of if (0x00 == loop)");
								lockRule();
								sunrise_set_calculated[loop-1] = FALSE; 
								unlockRule();
							}

							APP_LOG("DEVICE:rule", LOG_DEBUG, "Value of sunrise_set_calculated[loop] = %d", sunrise_set_calculated[loop]);
							APP_LOG("DEVICE:rule", LOG_DEBUG, "Value of TimerCalender[loop].day_sunrise_set = %d", TimerCalender[loop].day_sunrise_set);

							//if ((sunrise_set_calculated[loop] == FALSE) && (TimerCalender[loop].day_sunrise_set == TRUE))
							{

								if(curDay > curDayIndex)
								{
									GetCalendarNextDayInfo(curDay, curDayIndex, &dayIndex, &monthIndex, &year, &seconds);
									APP_LOG("DEVICE:rule", LOG_DEBUG, "year: %d, month: %d, date : %d, day: %s, now time in seconds: %d", year, monthIndex, 
											current_date, szWeekDayName[dayIndex], seconds);
								}

								int return_value = UpdateSunriseSunset(dayIndex, monthIndex, year, seconds);
								APP_LOG("DEVICE:rule", LOG_DEBUG, "After int return_value = UpdateSunriseSunset(loop, monthIndex, year, seconds) = %d",return_value);
								if (return_value == TRUE)
								{
									APP_LOG("DEVICE:rule", LOG_DEBUG, "Inside if (return_value == TRUE)");
									lockRule();
									sunrise_set_calculated[loop] = TRUE;
									unlockRule();
									APP_LOG("DEVICE:rule", LOG_DEBUG, "set sunrise set calc flag- %d:%d", loop, sunrise_set_calculated[loop]);
									ResortDailyEntry(&TimerCalender[loop]);
									APP_LOG("DEVICE:rule", LOG_DEBUG, "AfterResortDailyEntry(&TimerCalender[loop])");
								}
							}
						}
					}
				}

				GetCalendarDayInfo(&dayIndex, &monthIndex, &year, &seconds);
#ifdef SIMULATED_OCCUPANCY
				APP_LOG("DEVICE:rule", LOG_DEBUG, "now time in seconds: %d duration delta: %d", seconds, duration_delta);
				seconds = (seconds + duration_delta);
				if(seconds > ONE_DAY_SECONDS)
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "now time in seconds > one day seconds, adjust seconds");
						seconds = ONE_DAY_SECONDS;
				}
				APP_LOG("DEVICE:rule", LOG_DEBUG, "now time in seconds after duration delta adjust: %d", seconds);
#endif

				lockRule();
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Before duration = GetNextTimerDuration(dayIndex, seconds)");
				duration = GetNextTimerDuration(dayIndex, seconds);
				APP_LOG("DEVICE:rule", LOG_DEBUG, "duration = %d", duration);
				unlockRule();

				if (duration == 0)
				{
						duration = 1000;
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Exception: duration is 0, should not come here");

						return NULL;
				}

#ifdef SIMULATED_OCCUPANCY
				original_duration = duration;
				checkForSimulatedTypeTimer(&duration, &startTimer, &endTimer, &simulatedduration, dayIndex, seconds);

				if(gSimulatedRuleScheduled)
				{
						APP_LOG("Rule", LOG_DEBUG, "simulated duration: %d, duration: %d and duration delta: %d", simulatedduration, duration, duration_delta);
						duration = updateDuration(simulatedduration, duration, startTimer, endTimer, &duration_delta);
						APP_LOG("Rule", LOG_DEBUG, "Updated duration: %d and updated duration delta: %d", duration, duration_delta);
				}

				/* unset manual trigger flag, if set */
				resetManualToggle();

				if(!gSimulatedRuleRunning)
				{
						if(original_duration>duration_delta)
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "start waiting now :%d", (original_duration-duration_delta));
								sleep((original_duration-duration_delta));
						}

						if(gSimulatedRuleScheduled)
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "simulated rule type: %d", gSimulatedRuleScheduled);
								gSimulatedRuleRunning= 0x01;
								gSimulatedRuleScheduled = 0x00;
								APP_LOG("DEVICE:rule", LOG_DEBUG, "simulated rule type: %d and running: %d", gSimulatedRuleScheduled, gSimulatedRuleRunning);
								simulatedStartControlPoint();

								if(duration)  //non-zero simulated duration
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "more time waiting now :%d", (duration-(original_duration-duration_delta)));
										if(duration > (original_duration-duration_delta))
												sleep(duration-(original_duration-duration_delta));
								}
								else
								{
										APP_LOG("DEVICE:rule", LOG_DEBUG, "simulated duration calculated 0,sleep now for: %d seconds only", SIMULATED_DURATION_ADDLTIME); 
										sleep(SIMULATED_DURATION_ADDLTIME);
								}
						}
				}
				else
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, "start waiting now :%d", duration);
						sleep(duration);
				}
#endif
#ifndef SIMULATED_OCCUPANCY
				APP_LOG("DEVICE:rule", LOG_DEBUG, "start waiting now :%d", duration);

				sleep(duration);
#endif

				lockRule();
				if (0x00 == curTimerEntry)
				{
						APP_LOG("DEVICE:rule", LOG_DEBUG, " Exception or rule overrided");
				}

				g_pCurTimerControlEntry = curTimerEntry;
				unlockRule();

				if(isOverriddenState == 0x1)	//in case this flag not get reset some how
				{
						isOverriddenState = 0x0;
				}

				if (0x00 != g_pCurTimerControlEntry)
				{
						if(nextTimerEntry != 0x00)
						{
								if (g_pCurTimerControlEntry->time == nextTimerEntry->time)
								{
										if (g_pCurTimerControlEntry->action != nextTimerEntry->action)
										{
												APP_LOG("DEVICE:rule", LOG_DEBUG, " overlapping rule.... not changing state...");
												continue;
										}
								}
						}
						APP_LOG("Sensor", LOG_DEBUG, "Timer expires, to trigger event: %d", g_pCurTimerControlEntry->action);
				}

				if (DEVICE_INSIGHT == g_eDeviceTypeTemp)
				{
#ifdef PRODUCT_WeMo_Insight
#ifdef SIMULATED_OCCUPANCY
						if(gSimulatedRuleRunning || gSimRuleFirstOccurenceOfNextDay)	//TODO: check for gSimRuleFirstOccurenceOfNextDay
						{
								APP_LOG("Rule", LOG_DEBUG, "Simulated rule running: %d, next day first occur: %d duration: %d", gSimulatedRuleRunning, gSimRuleFirstOccurenceOfNextDay, duration);
								simulatedduration = updateSimulatedDuration(startTimer, endTimer);
								APP_LOG("Rule", LOG_DEBUG, "Updated simulated duration: %d", simulatedduration);
						}
						else
#endif
						{
								InsightTimerNotify(g_pCurTimerControlEntry);
								//- reset overriden status since it gets applied
								ResetOverrideStatus();
								CleanupOverrideTable();
						}
#endif
				}
				else if (DEVICE_SOCKET == g_eDeviceType)
				{
						/* currently g_eDeviceType is set to DEVICE_SOCKET for Switch, LS, ABM and Insight */
#ifdef SIMULATED_OCCUPANCY
						if(gSimulatedRuleRunning || gSimRuleFirstOccurenceOfNextDay)	//TODO: check for gSimRuleFirstOccurenceOfNextDay
						{
								APP_LOG("Rule", LOG_DEBUG, "Simulated rule running: %d, next day first occur: %d duration: %d", gSimulatedRuleRunning, gSimRuleFirstOccurenceOfNextDay, duration);
								simulatedduration = updateSimulatedDuration(startTimer, endTimer);
								APP_LOG("Rule", LOG_DEBUG, "Updated simulated duration: %d", simulatedduration);
						}
						else
#endif
						{
								SocketTimerNotify(g_pCurTimerControlEntry);
								//- reset overriden status since it gets applied
								ResetOverrideStatus();
						}
				}
				else if (DEVICE_SENSOR == g_eDeviceType)
				{
						SensorTimerNotify(g_pCurTimerControlEntry);
						//- reset overriden status since it gets applied
						ResetOverrideStatus();
						CleanupOverrideTable();
				}
				else
				{
						APP_LOG("Sensor", LOG_DEBUG, "Device type unknown");
				}

				cntDuration2NextEvent = 0x00;
		}
		//-Get day index of today
}

void  SensorTimerNotify(pTimerEntry pNowEntry)
{
		//-Sensor event triggered
		APP_LOG("Rule", LOG_DEBUG, "SensorTimerNotify called");
		if (0x00 == pNowEntry)
		{
				APP_LOG("Rule", LOG_DEBUG, "Current timer entry is not found");
		}
		else
		{
				if (0x00 != pNowEntry->pSocketList)
				{
						//Create the thread to manage the sensor
						if (0x01 == pNowEntry->action)
						{
								g_cntSensorDelay 	= pNowEntry->Delay;
								g_cntSensitivity	= pNowEntry->Sensitivity;
								APP_LOG("Rule", LOG_DEBUG, "Delay: %d, sensitivity:%d", g_cntSensorDelay, g_cntSensitivity);	
								SetSensorConfig(g_cntSensorDelay, g_cntSensitivity);
								lockRule();
								g_isSensorRuleActivated = 0x01;
								unlockRule();
								APP_LOG("Rule", LOG_DEBUG, "Sensor rule is activated");	
						}
						else
						{
								APP_LOG("Rule", LOG_DEBUG, "Sensor rule is deactivated");
								lockRule();
								g_isSensorRuleActivated = 0x00;
								g_pCurTimerControlEntry = 0x00;
								unlockRule();

								//- rule OFF noe
								ResetSensor2Default();
						}
				}
				else
				{
						APP_LOG("Rule", LOG_DEBUG, "No sensor controlled device found in list, the end of sensor time event");
						lockRule();
						g_isSensorRuleActivated = 0x00;
						g_pCurTimerControlEntry = 0x00;
						unlockRule();
				}
		}
}


void  SocketTimerNotify(pTimerEntry pNowEntry)
{
		if (0x00 != pNowEntry)
		{
#ifdef PRODUCT_WeMo_Insight
				if(g_StateLog)
						APP_LOG("Rule", LOG_ALERT, "######### RELAY ON/OFF VALUE : %d FROM RULE",pNowEntry->action);
#endif
				setActuation(ACTUATION_TIME_RULE);
				int ret = ChangeBinaryState(pNowEntry->action);
				if (0x00 == ret)
				{
						RuleToggleLed(pNowEntry->action);
						//LocalBinaryStateNotify(pNowEntry->action);
				}
		}
		else
		{
				APP_LOG("Rule", LOG_DEBUG, "Timer entry invalid, overrided or wrong");
		}

}
#ifdef PRODUCT_WeMo_Insight
void  InsightTimerNotify(pTimerEntry pNowEntry)
{
		//-Insight event triggered
		APP_LOG("Rule", LOG_DEBUG, "InsightTimerNotify called");
		if (0x00 == pNowEntry)
		{
				APP_LOG("Rule", LOG_DEBUG, "Current timer entry is not found");
		}
		else
		{
				if (0x00 != pNowEntry->pSocketList)
				{
						//Activate the Insight rule flag and handle in Insight rule thread for activating Insight rule.
						if (0x01 == pNowEntry->action)
						{
								APP_LOG("Rule", LOG_DEBUG, "#### ACTIVATING INSIGHT RULE");	
								lockRule();
								g_isInsightRuleActivated = 0x01;
								unlockRule();
								APP_LOG("Rule", LOG_DEBUG, "Insight rule is activated");	
						}
						else
						{
								APP_LOG("Rule", LOG_DEBUG, "Insight rule is deactivated");
								lockRule();
								g_isInsightRuleActivated = 0x00;
								g_pCurTimerControlEntry = 0x00;
								unlockRule();

								//- rule OFF noe
						}
				}
				else
				{
						APP_LOG("Rule", LOG_DEBUG, "Executing Simple Timer Rule For Insight");
						if(1 != g_isInsightRuleActivated)//This means that rule is simple timer rule and not insight rule.
						{
								setActuation(ACTUATION_TIME_RULE);
								int ret = ChangeBinaryState(pNowEntry->action);
								if (0x00 == ret)
								{
										RuleToggleLed(pNowEntry->action);
								}
						}
						else// Deactivate the Insight rule
						{
								APP_LOG("Rule", LOG_DEBUG, "Insight rule is deactivated");
								lockRule();
								g_isInsightRuleActivated = 0x00;
								g_pCurTimerControlEntry = 0x00;
								unlockRule();

						}
				}
		}

}

int CheckInsightRule(pTimerEntry pNowEntry)
{
		int TriggerValue=0x00;
		if (0x00 != pNowEntry)
		{
				if (0x00 != pNowEntry->pSocketList)
				{

						int loop = 0x00;
						for (; loop < pNowEntry->count; loop++)//check Insight condition for all the entry in this time.
						{
								//check if insight rule condition is true
								TriggerValue = CheckInsightTrigger(pNowEntry,loop);
								if(TriggerValue)
								{
										//If conditon is triggered Set the rule TriggerFlag
										pNowEntry->pSocketList[loop].InsightList.isTriggered = 1;
										TriggerValue = 0;
								}
						}
						loop = 0x00;
						for (; loop < pNowEntry->count; loop++)//check Insight condition for all the entry in this time.
						{
								if(pNowEntry->pSocketList[loop].InsightList.isTriggered == 1)
								{
										//If conditon is triggered ReSet the rule TriggerFlag
										pNowEntry->pSocketList[loop].InsightList.isTriggered = 0;
										//If conditon is triggered execute the rule
										ExecuteInsightRule(pNowEntry,loop);
								}
						}
				}
		}
		else
		{
				APP_LOG("Rule", LOG_DEBUG, "Insght Timer entry invalid, overrided or wrong");
		}
		return(0);

}

int ExecuteInsightRule(pTimerEntry pInsightNowEntry,int loop)
{
		int curState=0x00;
		//check if the UDN is for same Insight device or for some othe device in the home
		if(0 == strcmp(g_szUDN_1,pInsightNowEntry->pSocketList[loop].UND))
		{
				//check if the rule is for APNS
				if(pInsightNowEntry->pSocketList[loop].triggerType != 9)
				{
						LockLED();
						curState = GetCurBinaryState();
						UnlockLED();
						if(curState != pInsightNowEntry->pSocketList[loop].triggerType) 
						{
								setActuation(ACTUATION_TIME_RULE);
								int ret = ChangeBinaryState(pInsightNowEntry->pSocketList[loop].triggerType);
								if (0x00 == ret)
								{
										RuleToggleLed(pInsightNowEntry->pSocketList[loop].triggerType);
								}
						}
				}
				else
				{
						//check if earlier APNS notification is send to cloud or not.
						if(pInsightNowEntry->pSocketList[loop].InsightList.SendApnsFlag)
						{
								//send APNS Notification to the cloud using the RULEID at .cntDuration place holder of rule.txt
								g_DeferRuleID = 0;
								SendRuleNotification(pInsightNowEntry->pSocketList[loop].cntDuration);    
								if(g_DeferRuleID)
								{
										pInsightNowEntry->pSocketList[loop].InsightList.isActive=0;
								}
								pInsightNowEntry->pSocketList[loop].InsightList.SendApnsFlag = 0;
						}
				}

		}
		else
		{
				//Rule is for some other UDN in the home
				LockDeviceSync();
				int deviceIndex = GetDeviceIndexByUDN(pInsightNowEntry->pSocketList[loop].UND);
				UnlockDeviceSync();

				if (0x00 != deviceIndex)
				{
						int ret = PluginCtrlPointSensorEventInd(deviceIndex, 
										pInsightNowEntry->pSocketList[loop].triggerType,
										pInsightNowEntry->pSocketList[loop].cntDuration, 
										pInsightNowEntry->pSocketList[loop].endAction,
										pInsightNowEntry->pSocketList[loop].UND);

						if (ret != UPNP_E_SUCCESS)
						{
								APP_LOG("UPnPCtrPt", LOG_DEBUG, "Insight command sent unsuccess: %s",pInsightNowEntry->pSocketList[loop].UND);	
						}
				}
				else
				{
						APP_LOG("UPnPCtrPt", LOG_DEBUG, "Insight Device:%s  not online, command not sent", pInsightNowEntry->pSocketList[loop].UND);	
						CtrlPointDiscoverDevices();
				}
		}
		return(0);
}

int ValidateTrigger(int RuleState)
{
		int retVal=0;
		switch(RuleState)
		{
				case 0:
						if(g_APNSLastState == POWER_SBY)
								retVal = 0;
						else if (g_APNSLastState == POWER_ON)
								retVal = 1;
						break;
				case 8:
						if(g_APNSLastState == POWER_OFF)
								retVal = 0;
						else if (g_APNSLastState == POWER_ON)
								retVal = 1;
						break;
				case 1:
						retVal = 1;
						break;
				default:
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Wrong Rule State: %d",RuleState);

		}
		return retVal;
}

int FindRuleIndex(unsigned int APNSRuleId)
{
		int loopcntr=0;
		for(loopcntr=0;loopcntr < MAX_APNS_SUPPORTED; loopcntr++)
		{
				if(InsightActive[loopcntr].RuleId == APNSRuleId)
				{
						return loopcntr;
				}
		}
		return -1;
}

int FindEmptyRuleSlot()
{
		int loopcntr=0; 

		for(loopcntr=0;loopcntr < MAX_APNS_SUPPORTED; loopcntr++)
		{

				if(InsightActive[loopcntr].RuleId == 0)
				{
						return loopcntr;
				}
		}
		return -1;
}

int CheckInsightTrigger(pTimerEntry pInsightNowEntry,int loop)
{
		int TriggerFlag= 0x00;
		int RuleIdx = 0x00;
		//check the Insight parameter on which rule is to be checked
		switch(pInsightNowEntry->pSocketList[loop].InsightList.ParamCode)   
		{
				case E_COST:
						TriggerFlag = InsightCompareParam(g_Cost,pInsightNowEntry->pSocketList[loop].InsightList);
						break;
				case E_ON_DURATION:
						TriggerFlag = InsightCompareParam(g_RuleONFor,pInsightNowEntry->pSocketList[loop].InsightList);
						RuleIdx = FindRuleIndex(pInsightNowEntry->pSocketList[loop].cntDuration);//check idx of this ruleid
						if((RuleIdx == -1) && TriggerFlag)
						{
								RuleIdx = 0;
								RuleIdx = FindEmptyRuleSlot();
								if(-1 == RuleIdx)
								{
										APP_LOG("DEVICE:rule", LOG_CRIT, "--Rule Structure FULL");
										return 0;
								}
								InsightActive[RuleIdx].RuleId = pInsightNowEntry->pSocketList[loop].cntDuration;
								APP_LOG("DEVICE:rule", LOG_DEBUG, "RuleIDx=-1 InsightActive[%d].RuleId = %d and pInsightNowEntry->pSocketList[%d].cntDuration=%d\n",RuleIdx,InsightActive[RuleIdx].RuleId,loop,pInsightNowEntry->pSocketList[loop].cntDuration);
								InsightActive[RuleIdx].isActive = 0;
						}
						if((InsightActive[RuleIdx].isActive == 0) && (TriggerFlag))
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "@@@@@@@@@@@@@@@@@Send APNS AS g_RuleONFor: %d InsightList.SendApnsFlag: %d",g_RuleONFor,pInsightNowEntry->pSocketList[loop].InsightList.SendApnsFlag);
								pInsightNowEntry->pSocketList[loop].InsightList.SendApnsFlag = 1;
								InsightActive[RuleIdx].isActive = 1;
						}
						break;
				case E_SBY_DURATION:
						TriggerFlag = InsightCompareParam(g_InSBYSince,pInsightNowEntry->pSocketList[loop].InsightList);
						break;
				case E_STATE:
						LockLED();
						int s_CurState = GetCurBinaryState();
						UnlockLED();
						TriggerFlag = InsightCompareParam(s_CurState,pInsightNowEntry->pSocketList[loop].InsightList);
						if(TriggerFlag == 1)
						{
								TriggerFlag = ValidateTrigger(pInsightNowEntry->pSocketList[loop].InsightList.OPVal);
						}
						if((TriggerFlag == 0x00) && (pInsightNowEntry->pSocketList[loop].InsightList.OPVal == POWER_OFF))
						{
								pInsightNowEntry->pSocketList[loop].InsightList.OPVal = POWER_SBY;
								TriggerFlag = InsightCompareParam(s_CurState,pInsightNowEntry->pSocketList[loop].InsightList);
								if(TriggerFlag)
										TriggerFlag = ValidateTrigger(pInsightNowEntry->pSocketList[loop].InsightList.OPVal);
								pInsightNowEntry->pSocketList[loop].InsightList.OPVal = POWER_OFF;
						}
						RuleIdx = FindRuleIndex(pInsightNowEntry->pSocketList[loop].cntDuration);
						if((RuleIdx == -1) && TriggerFlag)
						{
								RuleIdx = 0;
								RuleIdx = FindEmptyRuleSlot();
								if(-1 == RuleIdx)
								{
										APP_LOG("DEVICE:rule", LOG_CRIT, "--Rule Structure FULL");
										return;
								}
								APP_LOG("DEVICE:rule", LOG_DEBUG, "State RuleIDx=-1 InsightActive[%d].RuleId = %d and pInsightNowEntry->pSocketList[%d].cntDuration=%d\n",RuleIdx,InsightActive[RuleIdx].RuleId,loop,pInsightNowEntry->pSocketList[loop].cntDuration);
								InsightActive[RuleIdx].RuleId = pInsightNowEntry->pSocketList[loop].cntDuration;
								InsightActive[RuleIdx].isActive = 0;
						}
						if((InsightActive[RuleIdx].isActive == 0) && (TriggerFlag))
						{
								APP_LOG("DEVICE:rule", LOG_DEBUG, "@@@@@@@@@@@@@@@@@Send APNS AS g_APNSLastState: %d g_PowerStatus: %d",g_APNSLastState,g_PowerStatus);
								pInsightNowEntry->pSocketList[loop].InsightList.SendApnsFlag = 1;
								InsightActive[RuleIdx].isActive = 1;
						}
						break;
				case E_POWER:
						TriggerFlag = InsightCompareParam(g_PowerNow,pInsightNowEntry->pSocketList[loop].InsightList);
						break;
				default:
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Wrong Insight Parameter: %d",pInsightNowEntry->pSocketList[loop].InsightList.ParamCode);
						break;
		}
		return(TriggerFlag);
}

int InsightCompareParam(int Parameter,InsightCondition IList)
{
		// compare the value of the parameter with condition value using the OPCode mentioned in the rule.
		switch(IList.OPCode)   
		{
				case E_EQUAL:
						if(Parameter == IList.OPVal)
						{
								return(1);
						}
						break;
				case E_LARGER:
						if(Parameter > IList.OPVal)
						{
								return(1);
						}
						break;
				case E_LESS:
						if(Parameter < IList.OPVal)
						{
								return(1);
						}
						break;
				case E_EQUAL_OR_LARGER:
						if(Parameter >= IList.OPVal)
						{
								return(1);
						}
						break;
				case E_EQUAL_OR_LESS:
						if(Parameter <= IList.OPVal)
						{
								return(1);
						}
						break;
				default:
						APP_LOG("DEVICE:rule", LOG_DEBUG, "Wrong Insight OPCode: %d",IList.OPCode);
						return (0);
						break;
		}
		return(0);
}
#endif

#if defined(PRODUCT_WeMo_Insight) || defined(PRODUCT_WeMo_SNS) || defined(PRODUCT_WeMo_NetCam)


void InitAPNS(void)
{
		struct stat FileInfo;
		g_APNSRuleDB = NULL;
		TableDetails RuleIDInfo[MAX_APNS_INFO_COLUMN] = {{"RULEID","int"},{"RULEMSG","char"},{"FREQ","int"}};

		//check if rule DB file already exists
		if(stat(RULEDBURL, &FileInfo) != -1)
		{
				APP_LOG("APP", LOG_DEBUG, "APNS Rule DB already Exists!");
				if(InitDB(RULEDBURL,&g_APNSRuleDB))
						goto INIT_FAIL;
				APP_LOG("APP", LOG_DEBUG, "APNS RULE DB Init done!");
				return;
		}
		else
		{
				if(!InitDB(RULEDBURL,&g_APNSRuleDB))
				{
						if(WeMoDBCreateTable(&g_APNSRuleDB,"RuleIDInfo", RuleIDInfo, 0, MAX_APNS_INFO_COLUMN))
								goto INIT_FAIL;
						APP_LOG("APP", LOG_DEBUG, "APNS RULE DB CREATED, Init Done!");
						return;
				}
				else
						goto INIT_FAIL;
		}

		//reboot on init fail
INIT_FAIL:
		APP_LOG("APP", LOG_ALERT, "APNS RULE DB Init Failed!");
		//system("reboot");
}

void  SendRuleNotification(unsigned int RuleID)  
{
		int s32NoOfRows=0,s32NoOfCols=0;
		char **s8ResultArray=NULL;
		char query[SIZE_256B];
		int retVal=0x00;
		APP_LOG("DEVICE:rule", LOG_DEBUG, "SendRuleNotification for RULEID: %d",RuleID);
#if defined(PRODUCT_WeMo_Insight)
		LockRuleID();
		retVal = GetRuleIDFlag();
		UnlockRuleID();
		if(retVal)
		{
				APP_LOG("DEVICE:rule", LOG_DEBUG, "SendRuleNotification for Another RuleID is in Progress defer RuleID %d",RuleID);
				g_DeferRuleID = 1;
				return;
		}
		retVal = 0;
#endif
		g_RuleID = RuleID;
		memset(g_RuleMSG, 0x00, sizeof(g_RuleMSG));
		memset(query, 0x00, sizeof(query));
		sprintf(query, "select RULEMSG,FREQ from RuleIDInfo where RULEID=%d;",g_RuleID);
		if(g_APNSRuleDB != NULL)
		{

				retVal= WeMoDBGetTableData(&g_APNSRuleDB,query,&s8ResultArray,&s32NoOfRows,&s32NoOfCols);
				if(!retVal)
				{
						if(0 != s32NoOfCols)
						{
								if(0 != strlen(s8ResultArray[2]))
								{
										sprintf((char *)g_RuleMSG,"%s", s8ResultArray[2]);
								}

								if(0 != strlen(s8ResultArray[3]))
								{
										g_RuleFreq = atoi(s8ResultArray[3]);
								}
								WeMoDBTableFreeResult(&s8ResultArray,&s32NoOfRows,&s32NoOfCols);
								APP_LOG("DEVICE:rule", LOG_DEBUG, "SendRuleNotification for RULEID: %d MSG: %s Freq: %d",RuleID,g_RuleMSG,g_RuleFreq);
								// Set the flag used in send notification API to check regular XML to be sent or APNS XML to be sent to cloud
								LockRuleID();
								SetRuleIDFlag(SET_RULE_FLAG);
								UnlockRuleID();
								g_RuleExecutionTS = (int) GetUTCTime();
								//use the exeisting send notification API with added TAG for APNS
								remoteAccessUpdateStatusTSParams (0xFF);
						}
				}
				else
				{
						WeMoDBTableFreeResult(&s8ResultArray,&s32NoOfRows,&s32NoOfCols);
						APP_LOG("DEVICE:rule", LOG_DEBUG, "SendRuleNotification No MSG DATA for RULEID: %d ",RuleID);

				}
		}
		else
		{
				APP_LOG("DEVICE:rule", LOG_DEBUG, "APNS Rule DB Not Initialized ");
		}

}
#endif



DailyTimerList* GetFirstSchedulerNode(int DayOfWeekIndex)
{
		DailyTimerList* pNode = 0x00;

		if (DayOfWeekIndex >= WEEK_DAYS_NO)
		{
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Day of week information wrong: %d", DayOfWeekIndex);
				return pNode;
		}

		if ((TimerCalender[DayOfWeekIndex].count > 0x00) && (TimerCalender[DayOfWeekIndex].pTimerList))
		{
				pNode = &TimerCalender[DayOfWeekIndex];
		}

		return pNode;

}


/* Open the rule file for further storage
 *
 *
 *
 *
 *
 *
 ********************************************/
void OpenRuleStorage()
{
		s_fpRuleHandle = fopen(RULE_FILE_PATH, "w");
		if (0x00 == s_fpRuleHandle)
		{
				APP_LOG("DEVICE:rule", LOG_ERR, "Rule file open failed: %s", RULE_FILE_PATH);
				return;
		}
}

/* Close the rule file when done
 *
 *
 *
 *
 *
 *
 ********************************************/
void CloseRuleStorage()
{
		if (0x00 != s_fpRuleHandle)
		{
				fclose(s_fpRuleHandle);
				s_fpRuleHandle = 0x00;
		}
}

/* Save rule entry
 *
 *
 *
 *
 *
 *
 ********************************************/
void SaveRuleEntry(const char* entry)
{
		if (s_fpRuleHandle)
		{
				fwrite(entry, 0x01, strlen(entry), s_fpRuleHandle);
		}
		else
		{
				APP_LOG("DEVICE:rule", LOG_DEBUG, "Rule file handle not found");
		}
}



/**
 * void OverrideRule(int userActionIndex)
 *
 *
 *	To override rule on relay
 *
 *
 *
 *
 *
 *
 ******************************************************************************************/
int OverrideRule(int userActionIndex)

{
		//- Comment out now for QA release since it is not fully tested

		APP_LOG("Rule", LOG_DEBUG, "Not over-riding rule: %d", userActionIndex);
		return 0;

		int isNotified = 0x00;
		StopPowerMonitorTimer();

		int startTime   = 0x00;
		int startAction = 0x00;

		if (DEVICE_SOCKET == g_eDeviceType)
		{
				lockRule();
				if (0x00 != curTimerEntry)
				{
						//- This one in default: socket?
						if (0x00 == curTimerEntry->isOverridable)
						{
								APP_LOG("Rule", LOG_DEBUG, "time %d, action: %d: Rule can not be override", curTimerEntry->time, 
												curTimerEntry->action);
								unlockRule();
								return 0x01;
						}

						if (0x00 == curTimerEntry->isExtraRule)
						{
								APP_LOG("Rule", LOG_DEBUG, "To use regular time for overidding");
								startTime  		= curTimerEntry->time;
						}
						else
						{
								//- Breaking rule is still using the broken value
								APP_LOG("Rule", LOG_DEBUG, "To use over-night time for overidding");
								startTime  		= curTimerEntry->isExtraRule;
						}

						startAction		= curTimerEntry->action;
						isOverriddenState = 0x1;
						curTimerEntry = 0x00;
						g_pCurTimerControlEntry = 0x00;
						unlockRule();


						APP_LOG("Rule", LOG_DEBUG, "Rule to be overrided: time: %d, action:%d", startTime, startAction);	

						//- Notify to application directly
						isNotified = 0x01;
						RuleOverrideNotify(userActionIndex, g_szUDN, startTime, startAction);
				}
				else
				{
						unlockRule();
				}


		}
		else if (DEVICE_SENSOR == g_eDeviceType)
		{
				//- Never happens here, so commented out
				//RuleOverrideNotify(userActionIndex, g_szUDN, 0x00, 0x00);
		}


		return isNotified;

}

void ReloadRuleFromFlash()
{	
		StopWeeklyScheduler();

		LoadRuleFromFlash();
}
