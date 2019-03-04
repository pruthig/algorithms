/***************************************************************************
*
*
* utils.h
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
#ifndef __PLUGIN_UTILS_H
#define __PLUGIN_UTILS_H

//convert ":" separated hex string to byte array of size 8
unsigned char* utilsStrToByte8(const char *pstrId, unsigned char* pbyId);
//coverts 8 byte hex array to string
unsigned char *utilsByteToStr(unsigned char *pbyId, char* pstrId);
//convert to integer, similar to atoi
int utilsConvertToInt(char* rValue);
//convert to double, similar to atof
double utilsConvertToDouble(char* rValue);
//converting byte array to a double taking care of denominations
double utilsBytesToDouble(unsigned char* pbyId, unsigned char size);
//coverts hex array to string
unsigned char* utilsByteToStrG(unsigned char *bytebuf, int len, char *s);
//convert ":" separated hex string to byte array of size 16
unsigned char* utilsStrToByte16(const char *pstrId, unsigned char* pbyId);
//coverts 8 byte hex array of any length to string
unsigned char* utilsByteToStrGS(unsigned char *bytebuf, int len, char *s);
unsigned char* utilsByteToStrAS(unsigned char *bytebuf, int len, char *s);
//BCD to decimal format
int BCDtoDecimal(int valBCD);
unsigned char* HexStrToBytesStr(char *str, int len, unsigned char *buffer);
char *utilsRemDelimitStr(char *src, char *key);
void* saveSettingThread(void *arg);
int computeDstToggleTime(int updateDstime);
int pluginUsleep (unsigned int delay);
void remoteParseDomainLkup(char *dname, char ipArr[][64], int *num);
int isStrPrintAble(char *pInPutStr, int strLen);
char convert(unsigned char ch);
int convertInRawBytes(char *ssidStr, char *str);
char *convertSSID(char *ssid);
char *convertMonth(int month);
int utilsReplaceFileString(FILE* updfp, FILE* origfp, const char *str, char *value);
int utilsReplaceString(char* fName, const char *str,char *value);
#endif
