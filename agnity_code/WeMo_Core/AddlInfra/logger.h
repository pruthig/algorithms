/***************************************************************************
*
*
* logger.h
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
#ifndef __PLUGIN_LOGGER_H
#define __PLUGIN_LOGGER_H

#include <syslog.h>

//The below details taken from syslog.h for reference
/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 *
 * #define LOG_PID         0x01    log the pid with each message
 * #define LOG_CONS        0x02    log on the console if errors in sending
 * #define LOG_ODELAY      0x04    delay open until first syslog() (default)
 * #define LOG_NDELAY      0x08    don't delay open
 * #define LOG_NOWAIT      0x10    don't wait for console forks: DEPRECATED
 * #define LOG_PERROR      0x20    log to stderr as well
 *
 * Log Levels 
 * #define	LOG_EMERG	0
 * #define	LOG_ALERT	1
 * #define	LOG_CRIT	2
 * #define	LOG_ERR		3
 * #define	LOG_WARNING	4
 * #define	LOG_NOTICE	5
 * #define	LOG_INFO	6
 * #define	LOG_DEBUG	7
 		
 CODE prioritynames[] = {
  { "alert",	LOG_ALERT },
  { "crit",	LOG_CRIT },
  { "debug",	LOG_DEBUG },
  { "emerg",	LOG_EMERG },
  { "err",	LOG_ERR },
  { "error",	LOG_ERR },		 Deprecated
  { "info",	LOG_INFO },
  { "none",	INTERNAL_NOPRI },
  { "notice",	LOG_NOTICE },
  { "panic",	LOG_EMERG },		 Deprecated 
  { "warn",	LOG_WARNING },		 Deprecated 
  { "warning",	LOG_WARNING },
  { NULL, -1 }
 };

  //Facility Codes
  * #define	LOG_KERN	(0<<3)
	* #define	LOG_USER	(1<<3)
	* #define	LOG_MAIL	(2<<3)
	* #define	LOG_DAEMON	(3<<3)
	* #define	LOG_AUTH	(4<<3)
	* #define	LOG_SYSLOG	(5<<3)
	* #define	LOG_LPR		(6<<3)
	* #define	LOG_NEWS	(7<<3)
	* #define	LOG_UUCP	(8<<3)
	* #define	LOG_CRON	(9<<3)
	* #define	LOG_AUTHPRIV	(10<<3)
	* #define LOG_FTP		(11<<3)
	* 
	* Codes through 15 are reserved for system use
	* #define LOG_LOCAL0	(16<<3)
	* #define LOG_LOCAL1	(17<<3)
	* #define LOG_LOCAL2	(18<<3)
	* #define LOG_LOCAL3	(19<<3)
	* #define LOG_LOCAL4	(20<<3)
	* #define LOG_LOCAL5	(21<<3)
	* #define LOG_LOCAL6	(22<<3)
	* #define LOG_LOCAL7	(23<<3)
	*
	CODE facilitynames[] = {
	  { "auth",	LOG_AUTH },
	  { "authpriv",	LOG_AUTHPRIV },
	  { "cron",	LOG_CRON },
	  { "daemon",	LOG_DAEMON },
	  { "ftp",	LOG_FTP },
	  { "kern",	LOG_KERN },
	  { "lpr",	LOG_LPR },
	  { "mail",	LOG_MAIL },
	  { "mark",	INTERNAL_MARK },
	  { "news",	LOG_NEWS },
	  { "security",	LOG_AUTH },		 Deprecated 
	  { "syslog",	LOG_SYSLOG },
	  { "user",	LOG_USER },
	  { "uucp",	LOG_UUCP },
	  { "local0",	LOG_LOCAL0 },
	  { "local1",	LOG_LOCAL1 },
	  { "local2",	LOG_LOCAL2 },
	  { "local3",	LOG_LOCAL3 },
	  { "local4",	LOG_LOCAL4 },
	  { "local5",	LOG_LOCAL5 },
	  { "local6",	LOG_LOCAL6 },
	  { "local7",	LOG_LOCAL7 },
	  { NULL,	-1 }
	};
*/

#define LOG_HIDE 111

//Interface to be used by other modules to log text string into syslog file
void pluginLog(char* logIdentifier, int logLevel, const char* pLogStr, ...);
void wdLog(int logLevel, const char* pLogStr, ...);

//Severity
typedef enum {
   CRITICAL=0,
   NORMAL
}severityLevel;

#define LOG_HDR "<%s:%s:%d> [0x%x:\"%s\"] "
#define LOG_FTR __FILE__, __FUNCTION__, __LINE__, pthread_self(), tu_get_my_thread_name()
#define LOG_HDR_WD "%s|"
#define LOG_FTR_WD __FUNCTION__
#define ELOG(X) pluginLog X
#define APP_LOG(source, level, format, ...) \
{   \
    if(level <= LOG_CRIT)   \
    {	\
	pluginLog(source, level, LOG_HDR format, LOG_FTR, ## __VA_ARGS__); \
	wdLog(level, LOG_HDR_WD format, LOG_FTR_WD, ## __VA_ARGS__); \
    }	\
    else    \
    {	\
	pluginLog(source, level, LOG_HDR format, LOG_FTR, ## __VA_ARGS__); \
    }	\
}

#define BUFFSIZE        (4*1024)
#define FILESIZE        (10*1024)
#define MAX_ROLL_OVER 2048
#define FILE_WRITE_TIMER 90
#define CONSOLE_LOGS_TIME   15*8 //15*8 mins
#define CONSOLE_LOGS_SIZE   10*SIZE_1024B  //10KB
#define PLUGIN_LOGS_FILE    "/tmp/PluginLogs"

pthread_mutex_t logMutex;

//Some new functions
int pluginOpenLog(char *BaseLogName,int MaxLogSize);
void pluginPrintBuf(char *title, char *buf, int len);
int loggerSetLogLevel (int lvl, int option);
int loggerGetLogLevel ();
void writeLogs(severityLevel severity, char *eventType, char *buff);
void rolloverCheck();
char rollOverBuffer[MAX_ROLL_OVER];
FILE *openFile(int flag );
int closeWDFile(int flag);
FILE *openWDFile(int flag);
void initLogger(void);
void deInitLogger(void);

struct console_logs_info
{
    int timePeriod;
    int fileSize;
		int timed;
};
typedef struct console_logs_info ConsoleLogsInfo;
#endif
