/***************************************************************************
*
*
* ipcUDS.h
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
#ifndef __IPC_UDS__
#define __IPC_UDS__

#ifdef PRODUCT_WeMo_Baby

#include "types.h"

#define ERROR 			-1
#define SOCK_PATH 		"/tmp/ipcudssocket.sock"

/** Enumerated type of end node command type*/
typedef enum{
	IPC_CMD_INVALID = -1,
	IPC_CMD_GET_NETWORK_STATE,	
	IPC_CMD_GET_SERVER_ENVIRONMENT,	
	IPC_CMD_GET_SIGNAL_STRENGTH,
	IPC_CMD_PUNCH_APP_WATCHDOG_STATE
}IPCCOMMAND;

typedef struct IPC_Header {
	IPCCOMMAND cmd;					/* command name*/
	UINT32 arg_length;				/* length of payload*/
} IPCHeader, *pIPCHeader;

// GENERIC DATA STRUCTURE
#define IPC_CMD_HDR_LEN     sizeof(IPCHeader)

/** Structure representing IPC action command request */
typedef struct IPC_command{
	IPCHeader hdr;
	PVOID arg;					/* Pointer to the actual data of command */
}IPCCommand, *pIPCCommand;


/** Structure representing IPC action command response */
typedef struct IPC_response{
	IPCHeader hdr;
	PVOID arg;						
}IPCResp, *pIPCResp;

/** Structure representing IPC response for Network state */
typedef struct Network_State_response{
	INT32 value;
}IPCNetworkStateResp, *pIPCNetworkStateResp;

/** Structure representing IPC response for Server Environment */
typedef struct Server_EVN_response{
	INT32 value;
}IPCServerEnvResp, *pIPCServerEnvResp;

/** Structure representing IPC response for Signal strength */
typedef struct Signal_Strenght_response{
        INT32 value;
}IPCSignalStrengthResp, *pIPCSignalStrengthResp;

/** Structure representing IPC response for WachDog Status */
typedef struct WachDog_Status_response{
        INT32 value;
}IPCWachDogStatusResp, *pIPCWachDogStatusResp;

/*IPC UDS Server API's*/
int ProcessServerIpcCmd(IPCCommand* s_cmd, int aChildFD);
int CreateIpcUdsServer(void *arg);
void ChildFDTask(void *pClientFD);
int StartIpcUdsServer();
int IpcUdsSend(pIPCCommand sa_resp, int aChildFD);
int SendNetworkState(IPCCommand *sa_cmd, int aChildFD);
int SendServerEnvironmnet(IPCCommand *sa_cmd, int aChildFD);
int SendSignalStrength(IPCCommand *sa_cmd, int aChildFD);
int SendWachDogStatus(IPCCommand *sa_cmd, int aChildFD);

/*IPC UDS Client API's*/
pIPCCommand ProcessClientIPCCommand(IPCCOMMAND cmd);
int SendIPCCommand(pIPCCommand psCmd, int aClientFD);
int GetIPCResponse(pIPCCommand buffer, int aClientFD);
int StartIPCClient();

/*API For Evos*/
int GetNetworkState();
int GetServerEnv();
int GetSignalStrength();
int PunchAppWatchDogState();

#endif /*PRODUCT_WeMo_Baby*/
#endif /*__IPC_UDS__*/



