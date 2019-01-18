/***************************************************************************
*
*
* httpsWrapper.h
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
#ifndef __HTTPS_WRAPPER_H
#define __HTTPS_WRAPPER_H

#include <curl/curl.h>
#include "defines.h"

#define DATA_BUF_LEN	SIZE_8192B
#define KEY_BUF_LEN	SIZE_64B
#define KEY_VAL_LEN	SIZE_512B

#define ERROR_BASE_APP 		  -100  // starts with -100 to avoid overlap with internal curl error codes
#define ERROR_INVALID_SESSION_ID  ERROR_BASE_APP-1
#define ERROR_INVALID_FILE	  ERROR_BASE_APP-2
#define ERROR_TRANSACTION	  ERROR_BASE_APP-3
#define ERROR_CLOUD_RESPONSE	  ERROR_BASE_APP-4

//Connection error codes for internal handling 
typedef	enum {
	RETRY_NO,
	RETRY_TIMEOUT,
	RETRY_HTTP,
	RETRY_LAST /* not used */
}retryCode;

typedef struct hdrKeyValue {
	char key[KEY_BUF_LEN]; // Key paramater eg Content-Type 
	char value[KEY_VAL_LEN]; // Key value ag application/xml
}KeyValue;

// user Application data
typedef struct userAppData {
  char url[SIZE_256B]; // url to which the session needs to be established
  KeyValue keyVal[SIZE_32B]; // html content heaer specifying the key and value
  int keyValLen;			//Number of pairs in keyValue
  char inData[DATA_BUF_LEN]; // file path or data which needs to be sent 
  int inDataLength; // length of data which needs to be sent, if <=0 then indata contains file path
										// or if it is >0 then it should contain length of data in indata
  char *outData; // pointer to the data which is received in response from the server
  //char outData[DATA_BUF_LEN]; // pointer to the data which is received in response from the server
  int outDataLength; // data length of the out data
  char outHeader[DATA_BUF_LEN]; // pointer to the header data which is received in response from the server
  int outHeaderLength; // data length of the out header data
  int httpsFlag; //Flag to identify https or http, should be set to 1 for https and 0 for http
  int disableFlag; //Flag to identify whether error handling is to be enabled or disabled 
  int partNumber; //Specifies the current file part 
  char eTag[SIZE_4B][SIZE_128B]; // eTag for file uploads
  char mac[KEY_BUF_LEN]; //max address used internally for posting transaction info for PUT case
	char cookie_data[KEY_VAL_LEN];
	int outResp; //Response value in outHeader
}UserAppData;

// user session data
typedef struct userAppSessionData {
  int sessionId;  // id corresponding to the session created
  CURL * curl; // pointer to curl SessionHandle structure 
}UserAppSessionData;

typedef struct webSessionListNode{
  UserAppData *pUsrAppData;
  UserAppSessionData *pUsrAppSsnData;
}WebSessionListNode;

typedef struct transactionInfo{
  char data[SIZE_1024B];
  int dataSize;
}TransactionInfo;

int webAppInit(int flag);
int webAppSendData( UserAppSessionData *pSsnData, UserAppData *pUsrAppData, int flag /*GET(1) or POST(0) or PUT or else*/);
UserAppSessionData* webAppCreateSession ( int flag );
int webAppDestroySession ( UserAppSessionData *session );
int webAppPostTransactionInfo( WebSessionListNode *pWebSsnListNode );
int webAppSendDataMultiPartAmazonUploadParallelThreaded1Step( WebSessionListNode *pWebSsnListNode);
int webAppFileDownload(char *url, char *outfilename);
void StopDownloadRequest(void);

#endif /* _HTTPS_WRAPPER_H  */
