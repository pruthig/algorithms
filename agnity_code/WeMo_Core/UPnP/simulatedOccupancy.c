/***************************************************************************
 *
 *
 * simulatedOccupancy.c
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
#ifdef SIMULATED_OCCUPANCY
#include "simulatedOccupancy.h"
#include "utils.h"
#include "wifiHndlr.h"

pthread_mutex_t gSimulatedOccupancyLock;
int gSimulatedDeviceCount = 0;
SimulatedDevInfo *pgSimdevList = NULL;
SimulatedDevData *pgGlobalSimDevList = NULL;
int gSimulatedRuleRunning = 0;
int gSimulatedRuleScheduled= 0;
int gSimRuleFirstOccurenceOfDay= 0;
int gSimRuleLastOccurenceOfDay= 0;
int gSimRuleFirstOccurenceOfNextDay= 0;
int gSelfIndex = -1;
int gSimManualTrigger = 0;
int gSimulatedRuleEndTime = 0;
char gSimManualTriggerDate[SIZE_32B];
unsigned int gSimDevSelfOnTS = 0;
int gRemTimeToToggle = -1;  //(nextscheduletime - currenttime)
int gSimDevNumOff = 0;
int gSimDevNumOn = 0;
unsigned long gSimFirstTimer = 0;
int gDefaultSimFirstOnTime = 0;
int gDefaultSimMinOnTime = 0;
int gDefaultSimMaxOnTime = 0;
int gSimDevUpnpRespCount = 0;
int gSimRandomEndTime = 0;

void initSimulatedOccupancyLock(void)
{
    osUtilsCreateLock(&gSimulatedOccupancyLock);
}

void LockSimulatedOccupancy(void)
{
    osUtilsGetLock(&gSimulatedOccupancyLock);
}

void UnlockSimulatedOccupancy(void)
{
    osUtilsReleaseLock(&gSimulatedOccupancyLock);
}

int nowSimSeconds(void)
{
    time_t rawTime;
    struct tm * timeInfo;
    time(&rawTime);
    timeInfo = localtime (&rawTime);
    int seconds = timeInfo->tm_hour * 60 * 60 + timeInfo->tm_min * 60 + timeInfo->tm_sec;
    return seconds;
}

void resetManualToggle(void)
{
    /* unset manual trigger flag, if set */
    if(gSimManualTrigger)
    {
	gSimManualTrigger = 0x00;
	memset(gSimManualTriggerDate, 0, sizeof(gSimManualTriggerDate));
	APP_LOG("DEVICE:rule", LOG_DEBUG, "unsetting gSimManualTrigger= %d", gSimManualTrigger);
	LockLED();
	int curState = GetCurBinaryState();
	UnlockLED();
	//lastOccurenceToggle(curState);    /* to turn OFF light, in case of manual trigger and task is scheduled for next timer */
    }
}

void simulatedOccupancyInit(void)
{
    memset(gSimManualTriggerDate, 0 ,sizeof(gSimManualTriggerDate));

    char *pFirstTime = GetBelkinParameter (SIM_FIRST_ON_TIME);
    if (0x00 != pFirstTime && (0x00 != strlen(pFirstTime))){
	gDefaultSimFirstOnTime = atoi(pFirstTime);
    }else{
	gDefaultSimFirstOnTime = SIMULATED_FIRST_ON_TIME;
    }

    char *pMinTime = GetBelkinParameter (SIM_MIN_ON_TIME);
    if (0x00 != pMinTime && (0x00 != strlen(pMinTime))){
	gDefaultSimMinOnTime = atoi(pMinTime);
    }else{
	gDefaultSimMinOnTime = SIMULATED_MIN_ON_TIME;
    }

    char *pMaxTime = GetBelkinParameter (SIM_MAX_ON_TIME);
    if (0x00 != pMaxTime && (0x00 != strlen(pMaxTime))){
	gDefaultSimMaxOnTime = atoi(pMaxTime);
    }else{
	gDefaultSimMaxOnTime = SIMULATED_MAX_ON_TIME;
    }

    char *pDevCnt = GetBelkinParameter (SIM_DEVICE_COUNT);
    if (0x00 != pDevCnt && (0x00 != strlen(pDevCnt))){
	gSimulatedDeviceCount = atoi(pDevCnt);
    }else{
	gSimulatedDeviceCount = 0;
    }

    char *pManTrig = GetBelkinParameter (SIM_MANUAL_TRIGGER);
    if (0x00 != pManTrig && (0x00 != strlen(pManTrig))){
	gSimManualTrigger = atoi(pManTrig);
    }else{
	gSimManualTrigger = 0;
    }

    char *pManTrigDate = GetBelkinParameter (SIM_MANUAL_TRIGGER_DATE);
    if (0x00 != pManTrigDate && (0x00 != strlen(pManTrigDate))){
	strncpy(gSimManualTriggerDate, pManTrigDate, sizeof(gSimManualTriggerDate)-1);
    }

    APP_LOG("REMOTEACCESS", LOG_ERR, "Simulated: FirstOntime: %d, MinimumOnTime: %d, MaximumOnTime: %d, device count: %d, ManualTrigger: %d, ManualTriggerDate: %s", gDefaultSimFirstOnTime, gDefaultSimMinOnTime, gDefaultSimMaxOnTime, gSimulatedDeviceCount, gSimManualTrigger, gSimManualTriggerDate);

    initSimulatedOccupancyLock();
} 

void updateRuleStartEndTimers(int *startTimer, int *endTimer)
{
    int year = 0x00, monthIndex = 0x00, seconds = 0x00, dayIndex = 0x00;
    GetCalendarDayInfo(&dayIndex, &monthIndex, &year, &seconds);
    DailyTimerList* pNode = GetFirstSchedulerNode(dayIndex);
    *startTimer = 0, *endTimer = 0;

    APP_LOG("Rule", LOG_DEBUG, "update Rule Start End Time...");
    if (pNode)
    {
        int loop = 0x00;
        APP_LOG("Rule", LOG_DEBUG, "update Rule Start End Time...");

        for (loop = 0x00; loop < pNode->count; loop++)
        {
            /* Check whether simulated rule type timer */
            if ( (SIMULATED_OCCUPANCY_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNRISE_RULE == pNode->pTimerList[loop].sunrise_set) || (SIMULATED_SUNSET_RULE == pNode->pTimerList[loop].sunrise_set) )
            {
                if(!(*startTimer))
                {
                    *startTimer = pNode->pTimerList[loop].time;
                    APP_LOG("DEVICE:rule", LOG_DEBUG, "Start Timer: %d", *startTimer);
                    *endTimer = pNode->pTimerList[loop+1].time;
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
	    }
	}
    }
}

void simulatedStartControlPoint(void)
{
    char* ip_address = NULL;
    ip_address = wifiGetIP(INTERFACE_CLIENT);
    APP_LOG("DEVICE:rule", LOG_DEBUG, "Start control point: ip_address: %s", ip_address);
    if (ip_address && (0x00 != strcmp(ip_address, DEFAULT_INVALID_IP_ADDRESS)))
    {
	if(-1 != ctrlpt_handle)
	{
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "CONTROL POINT ALREADY RUNNING....");
	}
	else
	{
	    StartPluginCtrlPoint(ip_address, 0x00);
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Started control point");
	}
    }
}

int getRandomTime(int fromTime, int toTime, int ruleStartTime, int ruleEndTime)
{
    int startTime = 0x00, endTime = 0x00, randomTime = 0x00, curTime = 0x00;
    APP_LOG("DEVICE:rule", LOG_DEBUG, "Rule fromtime: %d, totime: %d, starttime: %d and endtime: %d", fromTime, toTime, ruleStartTime, ruleEndTime);

    if(gSimulatedRuleRunning)
    {
	curTime = nowSimSeconds();
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Current time: %d", curTime);
    }
    else
    {
	//curTime = gSimFirstTimer;
	curTime = ruleStartTime;
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Current time is rule first timer: %d", curTime);
    }
#if 0
    if(curTime > ruleEndTime)
    {
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Current time: %d is greater than rule end time: %d", curTime, ruleEndTime);
	return 0x00;
    }
#endif
    if((curTime + fromTime) >= ruleEndTime)
    {
	startTime = curTime;
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Start time is current time: %d", startTime);
    }
    else
    {
	startTime = (curTime + fromTime);
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Start time: %d", startTime);
    }

    if((curTime + toTime) >= ruleEndTime)
    {
	endTime = ruleEndTime;
	APP_LOG("DEVICE:rule", LOG_DEBUG, "End time is rule end time: %d", endTime);
    }
    else
    {
	endTime = (curTime + toTime);
	APP_LOG("DEVICE:rule", LOG_DEBUG, "End time: %d", endTime);
    }

    if((endTime-startTime) < gSimRandomEndTime)
    {
	APP_LOG("DEVICE:rule", LOG_DEBUG, "End - start time very near to rule end time", (endTime-startTime));
	gSimRuleLastOccurenceOfDay = 0x01;
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule last occurence of the day declared: %d", gSimRuleLastOccurenceOfDay);
	//return 0x00;
    }

    randomTime = SIMULATED_RANDOM_TIME(startTime, endTime);
    
    APP_LOG("DEVICE:rule", LOG_DEBUG, "Random time: %d", randomTime);
    return randomTime;
}

int adjustedFirstTimerofNextDay(int *duration_delta, int startTimer, int endTimer)
{
    //int duration = gSimFirstTimer;
    int duration = startTimer;
    int simulatedduration = 0;
    *duration_delta = 0;

    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule supported device... first duration: %d", duration);
    if(gSelfIndex == 0) /* for first device in rule */
    {
	//simulatedduration = SIMULATED_RANDOM_TIME((duration-gDefaultSimFirstOnTime), (duration+gDefaultSimFirstOnTime)); // + OR - 15mins of start time of next day
	simulatedduration = getRandomTime((-gDefaultSimFirstOnTime), gDefaultSimFirstOnTime, startTimer, endTimer); // + OR - 15mins of start time of next day
	APP_LOG("DEVICE:rule", LOG_DEBUG, "First device, next day first simulated duration = %d", simulatedduration);
	if(simulatedduration < duration)
	{
	    *duration_delta = duration - simulatedduration;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "updated delta= %d", *duration_delta);
	}
    }
    else      /* later devices */
    {
	//simulatedduration = SIMULATED_RANDOM_TIME((duration+gDefaultSimMinOnTime), (duration+gDefaultSimMaxOnTime)); // between 30mins - 180mins of first time of next day
	simulatedduration = getRandomTime(gDefaultSimMinOnTime, gDefaultSimMaxOnTime, startTimer, endTimer); // between 30mins - 180mins of first time of next day
	APP_LOG("DEVICE:rule", LOG_DEBUG, "First device, next day first simulated duration = %d", simulatedduration);
    }

    APP_LOG("DEVICE:rule", LOG_DEBUG, "duration delta: %d", *duration_delta);
    return simulatedduration; 
}

int adjustedFirstTimerofDay(int *duration_delta, int startTimer, int endTimer)
{
    //int duration = gSimFirstTimer;
    int duration = startTimer;
    int firstsimulatedduration = 0;
    int simulatedduration = 0;
    *duration_delta = 0;

    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule supported device... first duration: %d", duration);
    if(gSelfIndex == 0) /* for first device in rule */
    {
	//TODO:if duration == nowTime,then firstsimulatedduration = (duration)+(rand()%((duration+SIMULATED_FIRST_ON_TIME) - (duration))); //15mins
	int nowTime = nowSimSeconds();
	if (nowTime >= duration)
	{
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule: first duration: %d == nowTime: %d", duration, nowTime);
	    //firstsimulatedduration = SIMULATED_RANDOM_TIME(duration, (duration+gDefaultSimFirstOnTime)); // between 15mins
	    firstsimulatedduration = getRandomTime(0, gDefaultSimFirstOnTime, startTimer, endTimer); // between 15mins
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule: first simulated duration: %d", firstsimulatedduration);
	}
	else
	{ 
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule: first duration: %d != nowTime: %d", duration, nowTime);
	    //firstsimulatedduration = SIMULATED_RANDOM_TIME((duration-gDefaultSimFirstOnTime), (duration+gDefaultSimFirstOnTime)); // + OR - 15mins of start time
	    firstsimulatedduration = getRandomTime((-gDefaultSimFirstOnTime), gDefaultSimFirstOnTime, startTimer, endTimer); // + OR - 15mins of start time
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule: first simulated duration: %d", firstsimulatedduration);
	}

	int curTime = nowSimSeconds();
	if(firstsimulatedduration < curTime)	// case to handle overlapping -15 mins window with other rule
	{
	    simulatedduration = 0;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "First device, Overlapping so simulated duration = %d", simulatedduration);
	    /* compute delta */
	    if(firstsimulatedduration < duration)
	    {
		*duration_delta = duration - curTime;
		APP_LOG("DEVICE:rule", LOG_DEBUG, "updated delta= %d", *duration_delta);
	    }
	}
	else
	{
	    simulatedduration = firstsimulatedduration - curTime;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "First device, simulated duration = %d", simulatedduration);
	    /* compute delta */
	    if(firstsimulatedduration < duration)
	    {
		*duration_delta = duration - firstsimulatedduration;
		APP_LOG("DEVICE:rule", LOG_DEBUG, "updated delta= %d", *duration_delta);
	    }
	}
    }
    else      /* later devices */
    {
	//firstsimulatedduration = SIMULATED_RANDOM_TIME((duration+gDefaultSimMinOnTime), (duration+gDefaultSimMaxOnTime));     // between 30mins - 180mins
	firstsimulatedduration = getRandomTime(gDefaultSimMinOnTime, gDefaultSimMaxOnTime, startTimer, endTimer);     // between 30mins - 180mins
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule: first simulated duration: %d", firstsimulatedduration);
	int curTime = nowSimSeconds();
	simulatedduration = firstsimulatedduration - curTime;
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Later device, simulated duration = %d", simulatedduration);
    }

    APP_LOG("DEVICE:rule", LOG_DEBUG, "duration delta: %d", *duration_delta);
    return simulatedduration; 
}

int nextDayFirstSimDuration(int *delta, int startTimer, int endTimer)
{
    int firstsimulatedduration = 0x00;
    int duration_delta = 0x00;

    firstsimulatedduration = adjustedFirstTimerofNextDay(&duration_delta, startTimer, endTimer);
    *delta = duration_delta;
    APP_LOG("DEVICE:rule", LOG_DEBUG, "duration: %d and duration delta: %d", firstsimulatedduration, *delta);
    gSimRuleFirstOccurenceOfNextDay = 0x01;
    gSimulatedRuleRunning= 0x00;    //TODO: check
    APP_LOG("DEVICE:rule", LOG_DEBUG, "Sim Rule First Occurence Of Next Day flag set: %d, gSimulatedRuleRunning: %d", gSimRuleFirstOccurenceOfNextDay, gSimulatedRuleRunning);

    return firstsimulatedduration; 
}

int updateDuration(int simulatedduration, int duration, int startTimer, int endTimer, int *durationdelta)
{
    int firstsimulatedduration = 0x00;
    int duration_delta = 0x00;
    *durationdelta = 0;

    APP_LOG("Rule", LOG_DEBUG, "Simulated rule running: %d, simulatedduration: %d", gSimulatedRuleRunning, simulatedduration);
    if((DEVICE_SOCKET == g_eDeviceType) && (g_eDeviceTypeTemp != DEVICE_BABYMON))
    {
	/* First timer */
	if(gSimRuleFirstOccurenceOfDay)
	{
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule first timer...");
	    firstsimulatedduration = adjustedFirstTimerofDay(&duration_delta, startTimer, endTimer);	//TODO: rohit
	    duration = firstsimulatedduration;
	    *durationdelta = duration_delta;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "duration: %d and duration delta: %d", duration, *durationdelta);
	    gSimRuleFirstOccurenceOfDay = 0;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "simulated rule first occurence flag= %d", gSimRuleFirstOccurenceOfDay);
	}
	/* later timers */
	else
	{
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule later timer...");
	    if((simulatedduration != 0x00) && (simulatedduration < duration) && (!gSimManualTrigger))
	    {
		duration_delta = 0x00;
		duration = simulatedduration;
		*durationdelta = duration_delta;
		APP_LOG("DEVICE:rule", LOG_DEBUG, "later timer: simulated duration: %d, duration delta: %d", duration, *durationdelta);
	    }
	    else if((simulatedduration == 0x00) && (!gSimManualTrigger))  //was last occurence of day, moving to next day and adjust first timer of next day
	    {
		APP_LOG("DEVICE:rule", LOG_DEBUG, "was last occurence so simulated duration is 0, next day first timer= %d", duration);
		duration = nextDayFirstSimDuration(&duration_delta, startTimer, endTimer);
		*durationdelta = duration_delta;
		APP_LOG("DEVICE:rule", LOG_DEBUG, "duration: %d and duration delta: %d", duration, *durationdelta);
	    }
	    else  /* if simulatedduration > endtimerofday of simulated rule OR manual trigger happen so computed next day first occurence timer */
	    {
		if(!gSimManualTrigger)
		{
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "last occurence: simulated duration > endtimerofday = %d", duration); //TODO: rohit
		    gSimRuleLastOccurenceOfDay = 0x01;
		}
		else  //a manual trigger case, to defer to next day
		{
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "next day first occurence: simulated duration = %d", duration);
		    duration = nextDayFirstSimDuration(&duration_delta, startTimer, endTimer);
		    *durationdelta = duration_delta;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "duration: %d and duration delta: %d", duration, *durationdelta);
		}
	    }
	}

	/* compute device ON time from now time */
	time_t curTime;
	curTime = (int) GetUTCTime();
	//gRemTimeToToggle = ((curTime + duration) - curTime);
	gRemTimeToToggle = duration;
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Remaining time to toggle: %d", gRemTimeToToggle);
    }
    return duration;
}

int updateSimulatedDuration(int startTimer, int endTimer)
{
    int simulatedduration = 0x00;
    /* Run simulated rule specific actions */
    if(g_eDeviceTypeTemp != DEVICE_BABYMON)
    {
	if(gSimRuleFirstOccurenceOfNextDay)
	{
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "Simulated rule: first occurence of next day flag: %d... start control point", gSimRuleFirstOccurenceOfNextDay);
	    simulatedStartControlPoint();
	    gSimRuleFirstOccurenceOfNextDay = 0x00;
	    gSimulatedRuleRunning = 0x01;   //TODO: may not required now, as we set gSimulatedRuleScheduled
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "unset first occurence of next day flag: %d", gSimRuleFirstOccurenceOfNextDay);
	    updateRuleStartEndTimers(&startTimer, &endTimer);
	}
	simulatedduration = evaluateNextActions(startTimer, endTimer);
	APP_LOG("DEVICE:rule", LOG_DEBUG, "simulated duration = %d", simulatedduration);
	if(gSimRuleLastOccurenceOfDay)
	{
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "last occurence of day set= %d, unset gSimulatedRuleRunning: %d and gSimRuleLastOccurenceOfDay", gSimRuleLastOccurenceOfDay, gSimulatedRuleRunning);
	    //gSimulatedRuleRunning = 0x00;
	    //gSimRuleLastOccurenceOfDay = 0x00;
	    //gSimFirstTimer = 0x00;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimRuleLastOccurenceOfDay: %d and gSimulatedRuleRunning: %d", gSimRuleLastOccurenceOfDay, gSimulatedRuleRunning);

	    if(!gProcessData)
	    {
		StopPluginCtrlPoint();
		APP_LOG("Rule", LOG_DEBUG, "Control point stopped...");
	    }
	}
    }

    return simulatedduration;
}

void unsetSimulatedData(void)
{
    APP_LOG("Upnp", LOG_DEBUG,"unsetSimulatedData...");
    SimulatedDevData *headNode = NULL;
    SimulatedDevData *node= NULL;

    LockSimulatedOccupancy();
    gSimulatedRuleRunning = 0;
    gSimulatedRuleScheduled = 0;
    gSimRuleFirstOccurenceOfDay = 0;
    gSimRuleLastOccurenceOfDay = 0;
    gSimRuleFirstOccurenceOfNextDay = 0;
    gSelfIndex = -1;
    //gSimManualTrigger = 0;
    gSimDevSelfOnTS = 0;
    gRemTimeToToggle = -1;
    gSimDevNumOff = 0;
    gSimDevNumOn = 0;
    gSimFirstTimer = 0;
    gSimDevUpnpRespCount = 0;
    gSimRandomEndTime = 0;
    gSimulatedRuleEndTime = 0;

    if(pgSimdevList)	//free simulated device list 
    {
	free(pgSimdevList);
	pgSimdevList = NULL;
    }
    if(pgGlobalSimDevList) //free collected info of other devices 
    {
	headNode = pgGlobalSimDevList;
	while(headNode)
	{
	    node = headNode;
	    headNode = headNode->next;
	    free(node);
	}
	pgGlobalSimDevList = NULL; 
    }
    UnlockSimulatedOccupancy();		
    APP_LOG("Upnp", LOG_DEBUG,"unsetSimulatedData done...");
}

int setSimulatedRuleFile(void *deviceList)
{
    APP_LOG("Upnp", LOG_ERR,"simulated rule file write...");
    FILE *fps = NULL;
    int ret = PLUGIN_SUCCESS;
    char *deviceListXml = NULL;
    char *localdeviceListXml = NULL;

    deviceListXml = (char*)deviceList;
    if(deviceListXml)
    {
	APP_LOG("UPNP", LOG_DEBUG, "device list XML is: %s", deviceListXml);
	localdeviceListXml = (char*)calloc(1, (strlen(deviceListXml)+SIZE_64B));
	if (!localdeviceListXml) return PLUGIN_FAILURE;
	snprintf(localdeviceListXml, (strlen(deviceListXml)+SIZE_64B), "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>%s", deviceListXml);
	APP_LOG("UPNP", LOG_DEBUG, "XML modified is:\n %s\n", localdeviceListXml);
	fps = fopen(SIMULATED_RULE_FILE_PATH, "w");
	if (!fps)
	{
	    APP_LOG("Upnp", LOG_ERR,"simulated rule file error write");
	    ret = PLUGIN_FAILURE;
	    if (localdeviceListXml){free(localdeviceListXml); localdeviceListXml=NULL;} 
	    return ret;
	}
	fwrite(localdeviceListXml, sizeof(char), strlen(localdeviceListXml), fps);
	fclose(fps);
    }
    else
	ret = PLUGIN_FAILURE;

    APP_LOG("Upnp", LOG_ERR,"simulated rule file write done...");
    if (localdeviceListXml){free(localdeviceListXml); localdeviceListXml=NULL;} 
    return ret;
}

int parseSimulatedRuleFile(char *ruleBuf, SimulatedDevInfo *psDevList)
{
    int retVal = PLUGIN_SUCCESS, index = 0;
    mxml_node_t *tree;
    mxml_node_t *first_node, *chNode;
    mxml_node_t *top_node;
    mxml_index_t *node_index=NULL;
    char *simulatedRuleXml = NULL;
    int found = 0;

    APP_LOG("Upnp", LOG_DEBUG,"parseSimulatedRuleFile...");
    simulatedRuleXml = (char*)ruleBuf;
    if (!simulatedRuleXml)
    { 
	APP_LOG("Upnp", LOG_DEBUG,"simulatedRuleXml error");
	return PLUGIN_FAILURE;
    }

    APP_LOG("RULE", LOG_DEBUG, "XML received is:\n %s\n", simulatedRuleXml);

    tree = mxmlLoadString(NULL, simulatedRuleXml, MXML_OPAQUE_CALLBACK);
    if (tree)
    {
	APP_LOG("Upnp", LOG_DEBUG,"simulatedRuleXml tree");
	top_node = mxmlFindElement(tree, tree, "SimulatedRuleData", NULL, NULL, MXML_DESCEND);

	node_index = mxmlIndexNew(top_node,"Device", NULL);
	if (!node_index)
	{
	    APP_LOG("RULE", LOG_DEBUG, "node index error");
	    retVal = PLUGIN_FAILURE;
	    goto out;
	}

	first_node = mxmlIndexReset(node_index);
	if (!first_node)
	{
	    APP_LOG("RULE", LOG_DEBUG, "first node error");
	    retVal = PLUGIN_FAILURE;
	    goto out;
	}

	while (first_node != NULL)
	{
	    APP_LOG("RULE", LOG_DEBUG, "first node");
	    first_node = mxmlIndexFind(node_index,"Device", NULL);
	    if (first_node)
	    {
		APP_LOG("RULE", LOG_DEBUG, "first child node");
		chNode = mxmlFindElement(first_node, tree, "UDN", NULL, NULL, MXML_DESCEND);
		if (chNode && chNode->child)
		{
		    APP_LOG("RULE", LOG_DEBUG,"The node %s with value is %s\n", chNode->value.element.name, chNode->child->value.opaque);
		    if(!strcmp(chNode->child->value.opaque, g_szUDN_1))
		    {
			found = 1;
			APP_LOG("RULE", LOG_DEBUG,"!!!!!!!!! UDN MATCHED: %s|%s !!!!!!!!!", chNode->child->value.opaque, g_szUDN_1);
		    }
		    /* copy the data for caller */
		    strncpy(psDevList[index].UDN, chNode->child->value.opaque, sizeof(psDevList[index].UDN) - 1);
		}

		chNode = mxmlFindElement(first_node, tree, "index", NULL, NULL, MXML_DESCEND);
		if (chNode && chNode->child)
		{
		    APP_LOG("RULE", LOG_DEBUG,"The node %s with value is %s\n", chNode->value.element.name, chNode->child->value.opaque);
		    if(found)
		    {
			gSelfIndex = atoi(chNode->child->value.opaque);
			APP_LOG("RULE", LOG_DEBUG,"!!!!!!!!! SELF INDEX: %d !!!!!!!!!", gSelfIndex);
			found = 0;
		    }
		    /* copy the data for caller */
		    psDevList[index].devIndex = atoi(chNode->child->value.opaque);
		}
		APP_LOG("RULE", LOG_DEBUG, "psDevList[%d]: UDN: %s, device index: %d", index, psDevList[index].UDN, psDevList[index].devIndex);
	    }
	    /* advance to next memory location */
	    index++;
	}
	APP_LOG("RULE", LOG_DEBUG, "loop completed...");

	mxmlDelete(tree);
	APP_LOG("RULE", LOG_DEBUG, "tree deleted...");
    }
    else
	APP_LOG("RULE", LOG_ERR, "Could not load tree");
out:
    APP_LOG("RULE", LOG_DEBUG, "deleting index...");
    if (node_index) mxmlIndexDelete(node_index);
    APP_LOG("RULE", LOG_DEBUG, "returning...");
    return retVal;
}

char* getSimulatedRuleBuffer(void)
{
    FILE *fps = NULL;
    long lSize = 0;
    char *buffer = NULL;
    size_t result = 0;
    APP_LOG("Upnp", LOG_DEBUG,"getSimulatedRuleBuffer...");

    fps = fopen (SIMULATED_RULE_FILE_PATH, "r" );
    if (!fps)
    {
	APP_LOG("Simulated", LOG_ERR,"simulated rule file error");
	return 0x00;
    }

    fseek (fps, 0 , SEEK_END);
    lSize = ftell (fps);
    rewind (fps);

    buffer = (char*)calloc ( 1, (sizeof(char) * lSize) );
    if (!buffer)
    {
	APP_LOG("Simulated", LOG_ERR,"calloc error");
	fclose (fps);
	return 0x00;
    }

    result = fread (buffer, sizeof(char), lSize, fps);
    if (result != lSize)
    {
	APP_LOG("Simulated", LOG_ERR,"simulated rule file read error");
	free(buffer);
	fclose (fps);
	return 0x00;
    }

    APP_LOG("Simulated", LOG_DEBUG, "Simulated XML read from file is: %s", buffer);
    fclose (fps);
    return buffer;
}

int parseSimulatedRule(void)
{
    int retVal = FAILURE;
    FILE* fpSimulatedRuleHandle = NULL;
    SimulatedDevInfo *devList = NULL;
    char *ruleBuf = NULL;

    APP_LOG("Upnp", LOG_DEBUG,"parseSimulatedRule...");
    fpSimulatedRuleHandle = fopen(SIMULATED_RULE_FILE_PATH, "r");
    if(!fpSimulatedRuleHandle)
    {
	APP_LOG("Rule", LOG_ERR, "Simulated Rule file not found, returning");
	return retVal;
    }
    else
    {
	fclose(fpSimulatedRuleHandle);
	APP_LOG("Rule", LOG_DEBUG, "Simulated Rule file found");

	ruleBuf = getSimulatedRuleBuffer();
	if(!ruleBuf)
	{
	    APP_LOG("Rule", LOG_DEBUG, "rule buffer is null, returning");
	    return retVal;
	}

	int numDevices = gSimulatedDeviceCount;
	APP_LOG("Rule", LOG_DEBUG, "number of simulated rule device: %d", numDevices);

	devList = (SimulatedDevInfo *)calloc(numDevices, sizeof(SimulatedDevInfo));
	if(!devList)
	{
	    APP_LOG("Rule", LOG_DEBUG, "Memory allocation failed for Simulated Rule, returning");
	    free(ruleBuf);
	    return retVal;
	}

	if((retVal = parseSimulatedRuleFile(ruleBuf, devList)) != SUCCESS)
	{
	    APP_LOG("Rule", LOG_DEBUG, "parse Simulated Rule File failure, returning");
	    free(ruleBuf);
	    free(devList);
	    return retVal;
	}
	APP_LOG("Rule", LOG_DEBUG, "parse Simulated Rule File success...");

	LockSimulatedOccupancy();
	pgSimdevList = devList;
	UnlockSimulatedOccupancy();
	free(ruleBuf);
	APP_LOG("Rule", LOG_DEBUG, "parseSimulatedRule success...");
	retVal = SUCCESS;
    }

    APP_LOG("Rule", LOG_DEBUG, "parseSimulatedRule done...");
    return retVal;
}

int setSimDevicesBinaryStateCount(void)
{
    APP_LOG("Rule", LOG_DEBUG, "setSimDevicesBinaryStateCount...");
    gSimDevNumOff = 0;
    gSimDevNumOn = 0;

    LockLED();
    int curState = GetCurBinaryState();
    UnlockLED();

    if(curState)
    {
	gSimDevNumOn++;
	APP_LOG("Simulated",LOG_DEBUG, "BinaryState: %d, gSimDevNumOn: %d", curState, gSimDevNumOn);
    }
    else	
    {
	gSimDevNumOff++;
	APP_LOG("Simulated",LOG_DEBUG, "BinaryState: %d, gSimDevNumOff: %d", curState, gSimDevNumOff);
    }

    if (0x00 == pgGlobalSimDevList)
    {

	APP_LOG("Simulated",LOG_DEBUG, "No simulated devices data");
	return FAILURE;
    }

    LockSimulatedOccupancy();
    SimulatedDevData *tmpNode =  pgGlobalSimDevList;
    if (tmpNode)
    {
	while (tmpNode)
	{
	    APP_LOG("Simulated",LOG_DEBUG, "Device: %s", tmpNode->sDevInfo.UDN);
	    if(tmpNode->binaryState)
	    {
		gSimDevNumOn++;
		APP_LOG("Simulated",LOG_DEBUG, "BinaryState: %d, gSimDevNumOn: %d", tmpNode->binaryState, gSimDevNumOn);
	    }
	    else
	    {
		gSimDevNumOff++;
		APP_LOG("Simulated",LOG_DEBUG, "BinaryState: %d, gSimDevNumOff: %d", tmpNode->binaryState, gSimDevNumOff);
	    }
	    tmpNode = tmpNode->next;
	}
    }
    UnlockSimulatedOccupancy();

    APP_LOG("Rule", LOG_DEBUG, "setSimDevicesBinaryStateCount done...");
    return SUCCESS;
}

void unsetSimDevSkipFlag(void)
{
    if (0x00 == g_pGlobalPluginDeviceList)
    {
	APP_LOG("UPnPCtrPt",LOG_DEBUG, "No device in discovery list");
	return;
    }

    LockDeviceSync();
    pCtrlPluginDeviceNode tmpNode =  g_pGlobalPluginDeviceList;
    while (0x00 != tmpNode)
    {
	tmpNode->Skip = 0x00;
	APP_LOG("UPNP: Device", LOG_DEBUG, "Set device Skip flag to: %d", tmpNode->Skip); 
	tmpNode = tmpNode->next;
    }
    UnlockDeviceSync();

    return;
}

int isAllSimulatedDevicesFound(void)
{
    int index = 0x00;
    SimulatedDevInfo *simdeviceinf;
    int simdevicecnt = 0; 
    int i = 0;

    if (0x00 == g_pGlobalPluginDeviceList)
    {
	APP_LOG("UPnPCtrPt",LOG_DEBUG, "No device in discovery list");
	return index;
    }

    LockSimulatedOccupancy();
    simdeviceinf = pgSimdevList;
    simdevicecnt = gSimulatedDeviceCount;
    UnlockSimulatedOccupancy();

    pCtrlPluginDeviceNode tmpNode =  g_pGlobalPluginDeviceList;
    while (0x00 != tmpNode)
    {
	i = 0;
	for(i = 0; i < simdevicecnt; i++)
	{
	    if (strcmp(tmpNode->device.UDN, simdeviceinf[i].UDN) == 0)  //a simulated device
	    {
		index++;
		APP_LOG("UPNP: Device", LOG_DEBUG, "Simulated device...: %d", index); 
	    }
	}
	tmpNode = tmpNode->next;
    }

    if(index == simdevicecnt)
	return 0x01;

    return 0x00;
}

int updateDiscoveredDevicesListInfo(void)
{
	APP_LOG("UPNP: Device", LOG_DEBUG, "Simulated Rule: update Discovered Devices List Info"); 
	pCtrlPluginDeviceNode tmpdevnode;
	SimulatedDevInfo *simdeviceinf;
	char szUDN[SIZE_256B] = {'\0',};

	int simdevicecnt = -1, i = 0, index = 0, matched = 0; 

	if (0x00 == g_pGlobalPluginDeviceList)
	{
		APP_LOG("UPnPCtrPt",LOG_DEBUG, "No device in discovery list");
		return index;
	}

	LockSimulatedOccupancy();
	simdeviceinf = pgSimdevList;
	simdevicecnt = gSimulatedDeviceCount;
	UnlockSimulatedOccupancy();

	APP_LOG("Rule", LOG_DEBUG, "updateDiscoveredDevicesListInfo...: %d", simdevicecnt);
	for(i = 0; i < simdevicecnt; i++)
	{
		matched = 0x00;
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
				index++;
			}

			tmpdevnode = tmpdevnode->next;
		}

		UnlockDeviceSync();
	}

	if(index == (simdevicecnt-1))   //exclude self count
	{
		APP_LOG("UPNP: Device", LOG_DEBUG, "Simulated Rule: all simulated Discovered Devices - index: %d", index);
		return 0x01; 
	}

	APP_LOG("UPNP: Device", LOG_DEBUG, "Simulated Rule: update Discovered Devices List Info done...%d|%d", index, simdevicecnt); 
	return 0x00;
}

int simulatedToggleBinaryState(int new_state)
{
    APP_LOG("Rule", LOG_DEBUG, "simulatedToggleBinaryState...");
    setActuation("Automatic - AwayRule");
    int ret = ChangeBinaryState(new_state);
    if (0x00 == ret)
    {
	APP_LOG("Rule", LOG_DEBUG, "simulatedToggleBinary ret: %d", ret);
	RuleToggleLed(new_state);
    }

    APP_LOG("Rule", LOG_DEBUG, "simulatedToggleBinaryState done...");
    return ret;
}

void checkLastManualToggleState(void)
{
    /* check if manually toggled today */
    if(gSimManualTrigger)
    {
	APP_LOG("Rule", LOG_DEBUG, "manual trigger set: %d", gSimManualTrigger);
	char date[SIZE_16B];
	char savedDate[SIZE_16B];
	char savedSeconds[SIZE_16B];
	int reset=0;

	memset(date, 0, sizeof(date));
	memset(savedDate, 0, sizeof(savedDate));
	memset(savedSeconds, 0, sizeof(savedSeconds));

	todaysDate(date);
	sscanf(gSimManualTriggerDate,  "%[^:]:%s", savedDate, savedSeconds);
	APP_LOG("Rule", LOG_DEBUG, "Saved date: %s, saved seconds: %s", savedDate, savedSeconds);
	if(!strcmp(savedDate, date))
	{
	    APP_LOG("Rule", LOG_DEBUG, "manual trigger date is matching: %s with today's date: %s", savedDate, date);
	    if(atoi(savedSeconds) > nowSimSeconds())
	    {
		APP_LOG("Rule", LOG_DEBUG, "manual trigger time after current time ");
	    }
	    else
		reset=1;
	}
	else
	    reset=1;

	if(reset)
	{
	    APP_LOG("Rule", LOG_DEBUG, "manual trigger date is not matching: %s with today's date: %s", gSimManualTriggerDate, date);
	    gSimManualTrigger = 0;
	    memset(gSimManualTriggerDate, 0, sizeof(gSimManualTriggerDate));
	    UnSetBelkinParameter(SIM_MANUAL_TRIGGER);
	    UnSetBelkinParameter(SIM_MANUAL_TRIGGER_DATE);
	    AsyncSaveData();
	    APP_LOG("Rule", LOG_DEBUG, "unset gSimManualTrigger: %d and gSimManualTriggerDate: %s", gSimManualTrigger, gSimManualTriggerDate);
	}
    }
}

void todaysDate(char *date)
{
    time_t rawTime;
    struct tm * timeInfo;
    time(&rawTime);
    timeInfo = localtime (&rawTime);
    snprintf(date, SIZE_32B, "%d%d%d", timeInfo->tm_mday, timeInfo->tm_mon, (timeInfo->tm_year + 1900));
}

void saveManualTriggerData(void)
{
    char manualTrigger[SIZE_8B];
    char manualTriggerDate[SIZE_32B];
    int seconds = 0; 
    memset(manualTrigger, 0, sizeof(manualTrigger));
    memset(manualTriggerDate, 0, sizeof(manualTriggerDate));

    snprintf(manualTrigger, sizeof(manualTrigger), "%d", gSimManualTrigger);
    todaysDate(manualTriggerDate);
    seconds = gSimulatedRuleEndTime;
    snprintf(manualTriggerDate+strlen(manualTriggerDate), sizeof(manualTriggerDate)-strlen(manualTriggerDate),":%d", seconds);
    APP_LOG("Rule", LOG_DEBUG, "Manual trigger: %s and date is: %s", manualTrigger, manualTriggerDate);
    SetBelkinParameter(SIM_MANUAL_TRIGGER, manualTrigger);
    SetBelkinParameter(SIM_MANUAL_TRIGGER_DATE, manualTriggerDate);
    AsyncSaveData();
}

void notifyManualToggle(void)
{
    int	cntdiscoveryCounter = SIMULATED_DISCOVERY_RETRIES;
    int isAllSimDevFound = 0;

    APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle...");
    saveManualTriggerData();

    /* discovery loop */
    while(cntdiscoveryCounter)
    {
	APP_LOG("UPNP: Device", LOG_DEBUG, "##################### Simulated Rule: Control point to discover: %d|%d ###################", 
		cntdiscoveryCounter, SIMULATED_DISCOVERY_RETRIES);

	/* always discover in this case */
	CtrlPointDiscoverDevices();
	APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle: discovery done");

	/* sleep for a while to allow device discovery and creation of device list */
	pluginUsleep((MAX_DISCOVER_TIMEOUT + 1)*1000000);

	/* update Discover list to set skip 0 or 1
	   return 1 if all simulated devices found else 0 
	   and update gSimDevNumOff on last discovery retry */
	if(updateDiscoveredDevicesListInfo())
	{
	    isAllSimDevFound = 0x01;
	    APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle: all devices discovered: %d", isAllSimDevFound);
	}

	if(isAllSimDevFound || (cntdiscoveryCounter == 0x01))
	{
	    APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle: all devices discovered or all discovery retries done...");
	    break;
	}

	cntdiscoveryCounter--;
	APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle: discovery count: %d", cntdiscoveryCounter);
	pluginUsleep(SIMULATED_DISCOVERY_SLEEP*1000000);
    }//end of discovery loop 

    /* send action to the simluated device list: to notify about manual trigger */
    PluginCtrlPointSendActionSimulated(PLUGIN_E_EVENT_SERVICE, "NotifyManualToggle");
    APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle send action done...");
   
    //unsetSimDevSkipFlag();

    APP_LOG("Rule", LOG_DEBUG, "notify Manual Toggle done...");
}

void lastOccurenceToggle(int curState)
{
    // last occurence and ON, so OFF it, if OFF, then don't toggle to ON
    if(curState)
    {
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Day last timer and device is ON, so toggle to OFF");
	simulatedToggleBinaryState(!curState);
    }
    else
    {
	APP_LOG("DEVICE:rule", LOG_DEBUG, "Day last timer and device is OFF, DON'T toggle to ON otherwise it will remain ON till next day");
    }
}

int evaluateNextActions(int startTimer, int endTimer)
{
    APP_LOG("Rule", LOG_DEBUG, "evaluateNextActions...");
    /* get data from other devices in the rule */
    int	cntdiscoveryCounter = SIMULATED_DISCOVERY_RETRIES;
    int isAllSimDevFound = 0, upnpRespWait = SIMULATED_UPNP_RESP_WAIT;
    int deviceOnForTS = 0, simulatedduration = 0, tmpsimulatedduration = 0, duration = 0; 

    /* TODO: Walk through the discovered device list & for devices which are in SimulatedDevInfo list, set skip =0  else skip =1  
       - Enhance Plugin Ctrl Point Send Action All to send action whenever skip = 0 and skip otherwise 
       - Call Action Get Simulated Rule Data which will return index, BinaryState, RemTimeToToggle 
       - Fill the data in SimulatedDevData structure & keep reference to the head in some global variable gpsSimulatedDevData  
       - Traverse the list starting from gpsSimulatedDevData and count gNumOff and gNumOn devices including self
       - Maintain the gOnFor variable as is done for Insight right now
     */
    if(!gSimManualTrigger)
    {
	APP_LOG("Rule", LOG_DEBUG, "not manually toggled: %d", gSimManualTrigger);
	/* discovery loop */
	while(cntdiscoveryCounter)
	{
	    APP_LOG("UPNP: Device", LOG_DEBUG, "##################### Simulated Rule: Control point to discover: %d|%d ###################", 
		    cntdiscoveryCounter, SIMULATED_DISCOVERY_RETRIES);

	    /* always discover in this case */
	    CtrlPointDiscoverDevices();
	    APP_LOG("Rule", LOG_DEBUG, "discovery done");

	    /* sleep for a while to allow device discovery and creation of device list */
	    pluginUsleep((MAX_DISCOVER_TIMEOUT + 1)*1000000);

	    /* update Discover list to set skip 0 or 1
	       return 1 if all simulated devices found else 0 
	       and update gSimDevNumOff on last discovery retry 
	     */
	    if(updateDiscoveredDevicesListInfo())
	    {
		isAllSimDevFound = 0x01;
		APP_LOG("Rule", LOG_DEBUG, "all devices discovered: %d", isAllSimDevFound);
	    }

	    if(isAllSimDevFound || (cntdiscoveryCounter == 0x01))
	    {
		APP_LOG("Rule", LOG_DEBUG, "all devices discovered or all discovery retries done...");
		break;
	    }

	    cntdiscoveryCounter--;
	    APP_LOG("Rule", LOG_DEBUG, "discovery counter: %d", cntdiscoveryCounter);
	    pluginUsleep(SIMULATED_DISCOVERY_SLEEP*1000000);
	}//end of discovery loop 

	/* send action to all simulated devices list: to get index, BinaryState, RemTimeToToggle */
	PluginCtrlPointSendActionSimulated(PLUGIN_E_EVENT_SERVICE, "GetSimulatedRuleData");
	/* SimulatedDevData structure fills in GetSimulatedRuleDataResponse CallBack */
	APP_LOG("Rule", LOG_DEBUG, "send action done");

	/* wait, to make sure all upnp action responses received */ 
	while(upnpRespWait)
	{
	    pluginUsleep(1000000);
	    if(gSimDevUpnpRespCount == (gSimulatedDeviceCount-1))
	    {
		APP_LOG("Rule", LOG_DEBUG, "got upnp action responses from all devices");
		break;
	    }
	    upnpRespWait--;
	}
	gSimDevUpnpRespCount = 0x00;

	//unsetSimDevSkipFlag();

	/* set simulated devices binary state count: gSimDevNumOn and gSimDevNumOff */
	setSimDevicesBinaryStateCount();
	APP_LOG("Rule", LOG_DEBUG, "ON/OFF count inc done");

	if((gSimDevNumOn+gSimDevNumOff) < gSimulatedDeviceCount)
	{
	    gSimDevNumOff += (gSimulatedDeviceCount - (gSimDevNumOn+gSimDevNumOff));
	    APP_LOG("Rule", LOG_DEBUG, "gSimDevNumOff: %d", gSimDevNumOff);
	}	

	/* compute device ON time from now time */
	LockLED();
	int curState = GetCurBinaryState();
	UnlockLED();
	time_t curTime = (int) GetUTCTime();
	//int curTime = nowSimSeconds();
	if(curState)
	{ 
	    deviceOnForTS = (curTime - gSimDevSelfOnTS);
	    APP_LOG("Rule", LOG_DEBUG, "deviceOnForTS: %d, curTime: %d gSimDevSelfOnTS: %d", deviceOnForTS, curTime, gSimDevSelfOnTS);
	}
	duration = nowSimSeconds();
	APP_LOG("Rule", LOG_DEBUG, "duration: %d", duration);

	/* 
	   - Device woke up to take action but is ON for less than 30 mins, so don't toggle 
	   - Device woke up to take action but is OFF right not but already MAX allowed devices are ON, so don't toggle 
	   - ((GetCurBinaryState() == 0x1) && (gNumOn == 0)) is already taken care by gNumOn including this device
	   - TODO: Just schedule for the remaining time and enjoy
	 */
	if( ((deviceOnForTS < gDefaultSimMinOnTime) && (GetCurBinaryState() == 0x1)) ||
		((GetCurBinaryState() == 0x0) && (gSimDevNumOn == SIMULATED_MAX_ON_ALLOWED)))
	{
	    //tmpsimulatedduration = SIMULATED_RANDOM_TIME((duration+gDefaultSimMinOnTime), (duration+gDefaultSimMaxOnTime));     // between 30mins - 180mins
	    tmpsimulatedduration = getRandomTime(gDefaultSimMinOnTime, gDefaultSimMaxOnTime, startTimer, endTimer);     // between 30mins - 180mins
	    simulatedduration = (tmpsimulatedduration - duration) + SIMULATED_DURATION_ADDLTIME;
	    APP_LOG("DEVICE:rule", LOG_DEBUG, "updated simulated duration = %d", simulatedduration);

	    if(gSimRuleLastOccurenceOfDay)  //TODO: need to discuss this condition with team
	    {
		APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimRuleLastOccurenceOfDay flag: %d set", gSimRuleLastOccurenceOfDay);
		lastOccurenceToggle(curState);
	    }
	}
	/* 
	   - Only one device running within the rule set, keep ON 30 mins and then toggle to OFF (taken in 1st if)
	   - When OFF, schedule next toggle time b/w current and current + 15 mins (why not toggle right away?)
	   - So... this condition doesnt bring up anything special
	 */
	else
	{
	    APP_LOG("Rule", LOG_DEBUG, "curState: %d", curState);
	    if((gSimDevNumOn + gSimDevNumOff) == 1)
	    {

		APP_LOG("Rule", LOG_DEBUG, "(gSimDevNumOn + gSimDevNumOff): %d", (gSimDevNumOn + gSimDevNumOff));
		// ON
		if(curState)
		{
		    //tmpsimulatedduration = SIMULATED_RANDOM_TIME(duration, (duration+gDefaultSimFirstOnTime));	// between 15mins
		    tmpsimulatedduration = getRandomTime(0, gDefaultSimFirstOnTime, startTimer, endTimer);	// between 15mins
		    simulatedduration = (tmpsimulatedduration - duration) + SIMULATED_DURATION_ADDLTIME;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "updated simulated duration = %d", simulatedduration);
		}
		//OFF
		else
		{
		    tmpsimulatedduration = (duration + gDefaultSimMinOnTime);	// 30mins
		    simulatedduration = (tmpsimulatedduration - duration) + SIMULATED_DURATION_ADDLTIME;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "updated simulated duration = %d", simulatedduration);
		}
	    }
	    else
	    {
		if(curState && (gSimDevNumOff == (gSimulatedDeviceCount - 1)))	// all devices OFF, this can be OFF but ON should be in 15 mins
		{
		    //tmpsimulatedduration = SIMULATED_RANDOM_TIME(duration, (duration+gDefaultSimFirstOnTime));	// between 15mins
		    tmpsimulatedduration = getRandomTime(0, gDefaultSimFirstOnTime, startTimer, endTimer);	// between 15mins
		    simulatedduration = (tmpsimulatedduration - duration) + SIMULATED_DURATION_ADDLTIME;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "updated simulated duration = %d", simulatedduration);
		}
		else
		{
		    //tmpsimulatedduration = SIMULATED_RANDOM_TIME((duration+gDefaultSimMinOnTime), (duration+gDefaultSimMaxOnTime));     // between 30mins - 180mins
		    tmpsimulatedduration = getRandomTime(gDefaultSimMinOnTime, gDefaultSimMaxOnTime, startTimer, endTimer);     // between 30mins - 180mins
		    simulatedduration = (tmpsimulatedduration - duration) + SIMULATED_DURATION_ADDLTIME;
		    APP_LOG("DEVICE:rule", LOG_DEBUG, "updated simulated duration = %d", simulatedduration);
		}
	    }

	    /* all set to toggle */
	    if(gSimRuleLastOccurenceOfDay)
	    {
		APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimRuleLastOccurenceOfDay flag: %d set", gSimRuleLastOccurenceOfDay);
		lastOccurenceToggle(curState);
	    }
	    else
	    {
		APP_LOG("DEVICE:rule", LOG_DEBUG, "gSimRuleLastOccurenceOfDay flag: %d not set", gSimRuleLastOccurenceOfDay);
		simulatedToggleBinaryState(!curState);
	    }
	}

	if(gSimRuleLastOccurenceOfDay)
	{
	    simulatedduration = 0x00;
	    APP_LOG("Rule", LOG_DEBUG, "gSimRuleLastOccurenceOfDay: %d, simulatedduration: %d", gSimRuleLastOccurenceOfDay, simulatedduration);
	}
    }

    APP_LOG("Rule", LOG_DEBUG, "evaluateNextActions done...");
    return simulatedduration;
}

#endif
