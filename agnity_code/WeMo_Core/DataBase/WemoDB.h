/***************************************************************************
*
*
* WemoDB.h
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
#ifndef _WEMODB_H_
#define _WEMODB_H_

#include "sqlite3.h"
#include "defines.h"
#define DB_SUCCESS	0
#define DB_ERROR	1
#define DB_NOT_OPEN	2
#define MAX_NAME_SIZE	SIZE_512B		//MAX Char 155, Localization Size, One Chinese = 155*3=465

typedef struct 
{
char *ParamName,*ParamDataType;
} TableDetails;

typedef struct 
{
char *ColName,*ColValue;
} ColDetails;

int InitDB(const char *ps8DBURL,sqlite3 **db);
void CloseDB(void);
int ExecuteStatement(char *ps8Statement,sqlite3 **db);
void DeleteDB(char *ps8DBURL);
int WeMoDBCreateTable(sqlite3 **DBHandle,char *TableName,TableDetails TableParams[],int PrimaryKeyEnable,int ParametersCount);
int WeMoDBUpdateTable(sqlite3 **DBHandle,char *TableName,ColDetails ColParams[],int ParametersCount,char *Condition);
int WeMoDBInsertInTable(sqlite3 **DBHandle,char *TableName,ColDetails ColParams[],int ParametersCount);
int WeMoDBDropTable(sqlite3 **DBHandle,char *TableName);
int WeMoDBGetTableData(sqlite3 **DBHandle,char *s8Query,char ***s8Result,int *s32NumofRows, int *s32NumofCols);
void WeMoDBTableFreeResult(char ***s8Result,int *s32NumofRows,int *s32NumofCols);
int WeMoDBDeleteEntry(sqlite3 **DBHandle,char *TableName,char *Condition);
#endif
