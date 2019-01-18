/***************************************************************************
*
*
* ipcUDSClient.c
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
#include <sys/un.h>
#include <semaphore.h>
#include "types.h"
#include "global.h"
#include "logger.h"

#include "ipcUDS.h"

int GetNetworkState()
{
	int NetworkState = 0; 
	pIPCCommand response = NULL;
	pIPCNetworkStateResp status = NULL;

	response = ProcessClientIPCCommand(IPC_CMD_GET_NETWORK_STATE);
	if (response)
	{
		status = (pIPCNetworkStateResp)(response->arg);
		if (status)
		{
			NetworkState = status->value;
			free(status);status = NULL;
		}
		free(response);response = NULL;
	}

	APP_LOG("GetNetworkState:", LOG_DEBUG, "*****NetworkState = %d\n", NetworkState);
	return NetworkState;
}

int GetServerEnv()
{
        int ServerEnv = 0;
        pIPCCommand response = NULL;
        pIPCServerEnvResp status = NULL;

        response = ProcessClientIPCCommand(IPC_CMD_GET_SERVER_ENVIRONMENT);
        if (response)
        {
                status = (pIPCServerEnvResp)(response->arg);
                if (status)
                {
                        ServerEnv = status->value;
			free(status);status = NULL;
                }
                free(response);response = NULL;
        }

        APP_LOG("GetServerEnv:", LOG_DEBUG, "*****ServerEnv = %d\n", ServerEnv);
	return ServerEnv;
}

int GetSignalStrength()
{
        int SignalStrength = 0;
        pIPCCommand response = NULL;
        pIPCSignalStrengthResp status = NULL;

        response = ProcessClientIPCCommand(IPC_CMD_GET_SIGNAL_STRENGTH);
        if (response)
        {
                status = (pIPCSignalStrengthResp)(response->arg);
                if (status)
                {
                        SignalStrength = status->value;
                        free(status);status = NULL;
                }
                free(response);response = NULL;
        }

        APP_LOG("GetSignalStrength:", LOG_DEBUG, "*****SignalStrength = %d\n", SignalStrength);
        return SignalStrength;
}

int PunchAppWatchDogState()
{
        int WatchdogStatus = 0;
        pIPCCommand response = NULL;
        pIPCSignalStrengthResp status = NULL;

        response = ProcessClientIPCCommand(IPC_CMD_PUNCH_APP_WATCHDOG_STATE);
        if (response)
        {
                status = (pIPCSignalStrengthResp)(response->arg);
                if (status)
                {
                        WatchdogStatus = status->value;
                        free(status);status = NULL;
                }
		free(response);response = NULL;
	}
	if (WatchdogStatus)
	{
	}
	else
	{
		APP_LOG("WatchdogStatus:", LOG_DEBUG, "*****WatchdogStatus punch Fail  = %d", WatchdogStatus);
	}
        return WatchdogStatus;
}

pIPCCommand ProcessClientIPCCommand(IPCCOMMAND cmd)
{
	pIPCCommand pgetResp = NULL;
	pIPCCommand pcmd = NULL;
	int retVal = ERROR,  clientFd = ERROR;
	do
	{
		pcmd = (IPCCommand*)calloc(sizeof(pIPCCommand), 1);
		pgetResp = (IPCCommand*)calloc(sizeof(pIPCCommand), 1);
		if(pcmd == NULL || pgetResp == NULL)
		{
			APP_LOG("ProcessClientIPCCommand:", LOG_ERR, "calloc failed\n");
			if(pcmd) free (pgetResp);
			if(pgetResp) free(pcmd);
			return NULL;;
		}
		pcmd->hdr.cmd = cmd;
		pcmd->hdr.arg_length = 0;

		retVal = clientFd = StartIPCClient();
		if(ERROR == retVal)
			break;

		if(retVal == SendIPCCommand(pcmd, clientFd))
			break;

		free(pcmd);pcmd = NULL;
		retVal = GetIPCResponse(pgetResp, clientFd);
	}while(0);	
	if (ERROR == retVal)
	{
		free(pcmd);pcmd = NULL;
		free(pgetResp->arg);pgetResp->arg = NULL;
		free(pgetResp);pgetResp = NULL;
	}
	/*close UDS client socket*/
	close(clientFd);
	clientFd = ERROR;
	return pgetResp;
}

int StartIPCClient()
{
	int clientSock, len;
	struct sockaddr_un remote;

	if ((clientSock = socket(AF_UNIX, SOCK_STREAM, 0)) == ERROR) 
	{
		APP_LOG("StartIPCClient:", LOG_ERR, "client socket creation error!");
		return ERROR;
	}

	remote.sun_family = AF_UNIX;
	strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path)-1);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(clientSock, (struct sockaddr *)&remote, len) == ERROR) 
	{
		APP_LOG("StartIPCClient:", LOG_ERR, "Connected to IPC server failed");
		close(clientSock);
		return ERROR;
	}

	return clientSock;
}



int SendIPCCommand(pIPCCommand psCmd, int aClientFd)
{
	char *buf = NULL;
	int len;

	/* Send the message on client fd */
	if(!(psCmd->hdr.arg_length))
	{
		len = IPC_CMD_HDR_LEN;
	}
	else
	{
		len = IPC_CMD_HDR_LEN + psCmd->hdr.arg_length;
	}

	buf = calloc(len,1);
	if (NULL == buf)
	{
		return ERROR;
	}
	memcpy(buf, &(psCmd->hdr),IPC_CMD_HDR_LEN);
	if(psCmd->hdr.arg_length)
	{
		memcpy(buf + IPC_CMD_HDR_LEN, (char *)psCmd->arg, len - IPC_CMD_HDR_LEN);
	}

	/* buffer now ready to send */
	if (send(aClientFd, buf, len, 0) == ERROR) 
	{
		APP_LOG("SendIPCCommand:", LOG_ERR, "Data send Fail.");
		free(buf);
		return ERROR;
	}
	free(buf);
	return SUCCESS;
}


int GetIPCResponse(pIPCCommand pRespbuff, int aClientFd)
{
	char *payload_buffer = NULL;
	int nbytes;

	nbytes = recv(aClientFd, (char*)&(pRespbuff->hdr), IPC_CMD_HDR_LEN, 0x0);
	if(nbytes < 0)
	{
		APP_LOG("GetIPCResponse:", LOG_ERR, "header read error");
		return ERROR;
	}

	payload_buffer = NULL;
	if(pRespbuff->hdr.arg_length > 0x0)
	{
		nbytes = 0;
		payload_buffer = (char*)calloc(pRespbuff->hdr.arg_length, 1);

		if(payload_buffer == NULL )
		{
			APP_LOG("GetIPCResponse:", LOG_ERR, " payload buffer calloc failed");
			return ERROR;
		}
		nbytes = recv(aClientFd, payload_buffer, pRespbuff->hdr.arg_length, 0x0);
		if(nbytes < 0)
		{
			APP_LOG("GetIPCResponse:", LOG_ERR, "payload read error");
			free(payload_buffer);
			payload_buffer = NULL;
			return ERROR;
		}
	}
	else
	{
	}
	pRespbuff->arg = (void *)payload_buffer;
	return SUCCESS;
}

#endif /* #ifdef PRODUCT_WeMo_Baby */
