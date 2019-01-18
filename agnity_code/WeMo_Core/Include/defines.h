/***************************************************************************
*
*
* defines.h
*
* Created by Belkin International, Software Engineering on Dec 19, 2012
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
#ifndef __DEFINES_H
#define __DEFINES_H

/* */
#define SIZE_2B		    2
#define SIZE_4B		    4
#define SIZE_8B		    8
#define SIZE_10B	    10
#define SIZE_16B	    16
#define SIZE_20B	    20
#define SIZE_32B	    32
#define SIZE_50B	    50
#define SIZE_64B	    64
#define SIZE_100B	    100
#define SIZE_128B	    128
#define SIZE_256B	    256
#define SIZE_512B	    512
#define SIZE_768B	    768
#define SIZE_1024B	    1024
#define SIZE_2048B	    2048
#define SIZE_4096B          4096
#define SIZE_8192B	    8192

/* Macros for delay in seconds */
#define DELAY_1SEC	    1
#define DELAY_3SEC	    3
#define DELAY_5SEC	    5
#define DELAY_10SEC	    10
#define DELAY_20SEC	    20
#define DELAY_60SEC	    60

//
#define P_MIN_THREADS			2
#define P_MAX_THREADS 			6 
#define P_THREAD_STACK_SIZE 		0
#define P_JOBS_PER_THREAD 		10
#define P_THREAD_IDLE_TIME 		5000
#define P_MAX_JOBS_TOTAL 		20

#define P_THREAD_LOW_PRIORITY		0
#define P_THREAD_MED_PRIORITY		1
#define	P_THREAD_HIGH_PRIORITY		2

//
#define MAX_RVAL_LEN			SIZE_16B
#define MAX_DVAL_LEN 			SIZE_32B
#define MAX_LVALUE_LEN 			SIZE_64B
#define MAX_RVALUE_LEN 			SIZE_64B
#define MAX_DBVAL_LEN			SIZE_128B
#define MAX_FILE_LINE			SIZE_256B
#define MAX_KEY_LEN			SIZE_256B
#define MAX_BUF_LEN   			SIZE_1024B
#define MAX_DATA_LEN			SIZE_1024B
#define MAX_MMSG_LEN                    SIZE_1024B
#define MAX_PKEY_LEN			SIZE_50B
#define MAX_RES_LEN			SIZE_2B
#define MAX_MAC_LEN			SIZE_20B
#define MAX_ESSID_LEN			SIZE_64B
#define MAX_ESSID_SPLCHAR_LEN	SIZE_128B
#define MAX_IP_COUNT			SIZE_10B
#define MAX_DESC_LEN			180
#define	MAX_SKEY_LEN			100
#define MAX_MIN_LEN			1
#define MAX_OCT_LEN			SIZE_8B
#define MAX_RESP_LEN			SIZE_512B
#define MAX_FW_URL_SIGN_LEN		SIZE_50B
#define LOGS_BUFF_LEN			SIZE_256B
#define PASSWORD_MAX_LEN		SIZE_128B
#define MAX_FW_URL_LEN			SIZE_256B	

#define THREAD_WAIT_TIME		10

#define PLUGIN_SUCCESS 			0
#define PLUGIN_FAILURE 			(-1)
#define ERROR_BASE 			(-10)
#define ERROR_NO_HANDLER 		(ERROR_BASE-2)

#define	MAX_DEV_UDID_LEN		SIZE_64B

/* Product Serial Number Schema
 * Serial Number    - Val-Index- Description
 *
 * 221238K01FFFFF   - 22 - 0,1 - Supplier Id
 *                  - 12 - 2,3 - Year of Mfg
 *                  - 38 - 4,5 - Week of Mfg
 *                  - K  - 6   - Product Code
 *                  - 01 - 7,8 - Product sub code
 *                  - FFFFF - 9,10,11,12,13 - Unique Sequence Identifier
 */

#define SUPPLIER_ID_INDEX                           0   /*length=2 digits*/
#define YEAR_OF_MFG_INDEX                           2   /*length=2 digits*/
#define WEEK_OF_MFG_INDEX                           4   /*length=2 digits*/
#define PRODCT_TYPE_INDEX                           6   /*length=1 digits*/
#define PRODCT_SUB_TYPE_INDEX                       7   /*length=2 digits*/
#define UNIQUE_SEQ_NO_INDEX                         9   /*length=5 digits*/
#endif
