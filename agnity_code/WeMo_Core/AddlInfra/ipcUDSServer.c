/***************************************************************************
*
*
* ipcUDSServer.c
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
#ifdef PRODUCT_WeMo_Baby
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/un.h>
#include <sys/msg.h>
#include "types.h"
#include "global.h"
#include "logger.h"
#include "ipcUDS.h"
#include "thready_utils.h"

static pthread_attr_t IpcUdsServerth_attr;
static pthread_t IpcUdsServer_thread;
int gWatchDogStatus = -1;

int IpcUdsSend(pIPCCommand sa_resp, int aChildFD)
{
	if(sa_resp == NULL)
	{
		APP_LOG("IpcUdsSend:",LOG_DEBUG, "response structure is empty\n");
		return ERROR;
	}

	if (send(aChildFD, (IPCResp*)sa_resp, (IPC_CMD_HDR_LEN + sa_resp->hdr.arg_length), 0) < 0) 
	{
		APP_LOG("IpcUdsSend:",LOG_ERR, "Socket Sent error");
	}

	if(sa_resp!=NULL)
	{
		free(sa_resp);
		sa_resp=NULL;
	}

	return SUCCESS;
}

int StartIpcUdsServer()
{
	int retVal = 0;

	pthread_attr_init(&IpcUdsServerth_attr);
	pthread_attr_setdetachstate(&IpcUdsServerth_attr,PTHREAD_CREATE_DETACHED);
	retVal = pthread_create(&IpcUdsServer_thread,&IpcUdsServerth_attr,(void*)&CreateIpcUdsServer, NULL);
	if(retVal < 0)
	{
		APP_LOG("StartUdServer:",LOG_ERR, "Could not create unix domain server thread... ");
	}
	else
	{
		APP_LOG("StartUdServer:",LOG_ERR, "created IPC unix domain server thread... ");
	}
	return retVal;
}

int CreateIpcUdsServer(void *arg)
{
	int server_sock, *pClientFD, t_len, len;
 	struct sockaddr_un local, remote;
        int retVal = 0;
	pthread_attr_t IpcUdsChildth_attr;
	pthread_t IpcUdsChild_thread;

	tu_set_my_thread_name( __FUNCTION__ );

	if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == ERROR)
	{
		APP_LOG("CreateIpcUdsServer:", LOG_ERR, "Server Socket Creation Failed.");
		return ERROR;
	}

	local.sun_family = AF_UNIX;
	strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path)-1);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	if (bind(server_sock, (struct sockaddr *)&local, len) == ERROR) 
	{
		APP_LOG("CreateIpcUdsServer:", LOG_ERR, "bind failed");
		return ERROR;
	}

	if (listen(server_sock, 5) == ERROR)
	{
		APP_LOG("CreateIpcUdsServer:", LOG_ERR, "Listen Failed");
		return ERROR;
	}

	t_len = sizeof(remote);
	for(;;)
	{
		pClientFD = (int*)malloc(sizeof(int));
		if(pClientFD == NULL )
		{
			APP_LOG("CreateIpcUdsServer:", LOG_ERR, "FD buffer calloc failed");
			continue;
		}	
		*pClientFD = -1;
		if ((*pClientFD = accept(server_sock, (struct sockaddr *)&remote, &t_len)) == -1)
		{
			APP_LOG("CreateIpcUdsServer:", LOG_ERR, "Accept Failed.");
			if (NULL != pClientFD){free(pClientFD);pClientFD = NULL;}
			continue;
		}
		else
		{
			pthread_attr_init(&IpcUdsChildth_attr);
			pthread_attr_setdetachstate(&IpcUdsChildth_attr, PTHREAD_CREATE_DETACHED);
			retVal = pthread_create(&IpcUdsChild_thread, &IpcUdsChildth_attr, &ChildFDTask, (void*)pClientFD);
			if(retVal < 0)
			{
				APP_LOG("CreateIpcUdsServer:",LOG_ERR, "Could not create unix domain child server thread... ");
				if (NULL != pClientFD){free(pClientFD);pClientFD = NULL;}
			}
			else
			{
			}
		}

	}
	return SUCCESS;
}
void ChildFDTask(void *pClientFD)
{
	char *hdr_buffer = NULL;
	char *payload_buffer = NULL;
	IPCCommand buffer;
	int nbytes;
	pIPCHeader hdr;
	int ClientFD = -1;

	tu_set_my_thread_name( __FUNCTION__ );

	if (NULL == (int*)pClientFD)
	{
		APP_LOG("ChildFDTask:", LOG_ERR, "NULL FD Argumnet, exit thread!");
		return;
	}
	ClientFD = *(int *)pClientFD;
	free((int*)pClientFD);
	pClientFD = NULL;

	do
	{
		hdr_buffer= (char*)calloc(IPC_CMD_HDR_LEN, 1);
		if(hdr_buffer == NULL )
		{
			APP_LOG("ChildFDTask:", LOG_ERR, "header buffer calloc failed");
			break;
		}
		nbytes = recv(ClientFD, hdr_buffer, IPC_CMD_HDR_LEN, 0x0);
		if(nbytes < 0)
		{
			APP_LOG("ChildFDTask:", LOG_ERR, "header read error");
			break;
		}

		hdr = NULL;
		hdr = (IPCHeader*)hdr_buffer;

		memcpy(&(buffer.hdr),hdr_buffer,IPC_CMD_HDR_LEN);

		payload_buffer = NULL;
		if(hdr->arg_length > 0x0)
		{
			nbytes = 0;
			payload_buffer = (char*)calloc(hdr->arg_length, 1);

			if(payload_buffer == NULL )
			{
				APP_LOG("ChildFDTask:", LOG_ERR, "payload buffer calloc failed");
				break;		
			}
			nbytes = recv(ClientFD, payload_buffer, hdr->arg_length, 0x0);
			if(nbytes < 0)
			{
				APP_LOG("ChildFDTask:", LOG_ERR, "payload read error");
				break;
			}

			buffer.arg = (void *)payload_buffer;
			ProcessServerIpcCmd(&buffer, ClientFD);	
		}
		else
		{
			ProcessServerIpcCmd(&buffer, ClientFD);	
		}

	}
	while(0);

	if(hdr_buffer!=NULL){free(hdr_buffer); hdr_buffer=NULL;}
	if(payload_buffer!=NULL){free(payload_buffer); payload_buffer=NULL;}
	close(ClientFD);
        pthread_exit(0);
	return;
}


int ProcessServerIpcCmd(IPCCommand* s_cmd, int aClientFD)
{

	if (NULL == s_cmd)
		return FAILURE;

	switch(s_cmd->hdr.cmd)
	{
		case IPC_CMD_GET_NETWORK_STATE:
			SendNetworkState(s_cmd, aClientFD);
			break;
		case IPC_CMD_GET_SERVER_ENVIRONMENT:
			SendServerEnvironmnet(s_cmd, aClientFD);
			break;
		case IPC_CMD_GET_SIGNAL_STRENGTH:
			SendSignalStrength(s_cmd, aClientFD);
			break;
		case IPC_CMD_PUNCH_APP_WATCHDOG_STATE:
			SendWachDogStatus(s_cmd, aClientFD);
			break;

		default:
			APP_LOG("ProcessServerIpcCmd:", LOG_DEBUG, "\nUnknown command: %d", s_cmd->hdr.cmd);
			break;
	}

	return SUCCESS;
}

int SendNetworkState(IPCCommand *sa_cmd, int aClientFD)
{
	pIPCResp sa_resp = NULL;
	IPCNetworkStateResp getNetworkState;

	sa_resp = (pIPCResp)calloc((IPC_CMD_HDR_LEN + sizeof(IPCNetworkStateResp)), 1);
	if(sa_resp == NULL)
	{
		APP_LOG("SendNetworkState:", LOG_DEBUG, "sa_resp calloc failed");
		return ERROR;
	}

	//fill response header
	sa_resp->hdr.cmd = sa_cmd->hdr.cmd;
	sa_resp->hdr.arg_length = sizeof(IPCNetworkStateResp);

	//fill response payload
	getNetworkState.value = getCurrentClientState();
	memcpy((char *)sa_resp + IPC_CMD_HDR_LEN, &getNetworkState, sizeof(IPCNetworkStateResp));

	//send it on socket
	IpcUdsSend((pIPCCommand)sa_resp, aClientFD);

	return SUCCESS; 
}

int SendServerEnvironmnet(IPCCommand *sa_cmd, int aClientFD)
{
        pIPCResp sa_resp = NULL;
        IPCServerEnvResp getServerEnv;

        sa_resp = (pIPCResp)calloc((IPC_CMD_HDR_LEN + sizeof(IPCServerEnvResp)), 1);
        if(sa_resp == NULL)
        {
                APP_LOG("SendNetworkState:", LOG_DEBUG, "sa_resp calloc failed");
                return ERROR;
        }

        //fill response header
        sa_resp->hdr.cmd = sa_cmd->hdr.cmd;
        sa_resp->hdr.arg_length = sizeof(IPCServerEnvResp);

        getServerEnv.value = getCurrentServerEnv();
        memcpy((char *)sa_resp + IPC_CMD_HDR_LEN, &getServerEnv, sizeof(IPCServerEnvResp));

        //send it on socket
        IpcUdsSend((pIPCCommand)sa_resp, aClientFD);

        return SUCCESS;
}

int SendSignalStrength(IPCCommand *sa_cmd, int aClientFD)
{
        pIPCResp sa_resp = NULL;
        IPCSignalStrengthResp getSignalStrength;

        sa_resp = (pIPCResp)calloc((IPC_CMD_HDR_LEN + sizeof(IPCSignalStrengthResp)), 1);
        if(sa_resp == NULL)
        {
                APP_LOG("SendNetworkState:", LOG_DEBUG, "sa_resp calloc failed");
                return ERROR;
        }

        //fill response header
        sa_resp->hdr.cmd = sa_cmd->hdr.cmd;
        sa_resp->hdr.arg_length = sizeof(IPCSignalStrengthResp);

        getSignalStrength.value = getCurrentSignalStrength();
        memcpy((char *)sa_resp + IPC_CMD_HDR_LEN, &getSignalStrength, sizeof(IPCSignalStrengthResp));

        //send it on socket
        IpcUdsSend((pIPCCommand)sa_resp, aClientFD);

        return SUCCESS;
}

int SendWachDogStatus(IPCCommand *sa_cmd, int aClientFD)
{
        pIPCResp sa_resp = NULL;
        IPCWachDogStatusResp getWachDogStatus;

        sa_resp = (pIPCResp)calloc((IPC_CMD_HDR_LEN + sizeof(IPCWachDogStatusResp)), 1);
        if(sa_resp == NULL)
        {
                APP_LOG("SendWachDogStatus:", LOG_DEBUG, "sa_resp calloc failed");
                return ERROR;
        }

        //fill response header
        sa_resp->hdr.cmd = sa_cmd->hdr.cmd;
        sa_resp->hdr.arg_length = sizeof(IPCWachDogStatusResp);
	
	//set WatchDog Status
	gWatchDogStatus = 1;
        APP_LOG("SendWachDogStatus:", LOG_DEBUG, "Updated watchdog gWatchDogStatus = %d", gWatchDogStatus);
        getWachDogStatus.value = gWatchDogStatus;
        memcpy((char *)sa_resp + IPC_CMD_HDR_LEN, &getWachDogStatus, sizeof(IPCWachDogStatusResp));

        //send it on socket
        IpcUdsSend((pIPCCommand)sa_resp, aClientFD);

        return SUCCESS;
}

#endif /* #ifdef PRODUCT_WeMo_Baby */
