/***************************************************************************
*
*
* DebugFrameWork.h
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
#ifndef __DEBUG_FRMWK__
#define __DEBUG_FRMWK__

#include "defines.h"

enum E_COMMAND{
	CMD_ICE=1,
	CMD_RELAY,
	CMD_REGISTER,
	CMD_NAT,
	CMD_SYSTEM,
	CMD_CONSOLELOGS,
	CMD_NONE
};

struct CommandInfo{
	unsigned int index;
	char 	cmd_value[SIZE_256B];
	int          cmd_length;
	unsigned int cmd_prio;
};


struct Command{
	struct CommandInfo  Info;
	struct Command      *next ;
};

void trigger_logging(char *cmd);
void enqueue(struct Command *tmp);

#define SLEEP_SEC_DEQUEUE                6
#define SLEEP_SEC_ENQUEUE                6

#endif
