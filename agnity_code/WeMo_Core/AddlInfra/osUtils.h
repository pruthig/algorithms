/***************************************************************************
*
*
* osUtils.h
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
#ifndef __PLUGIN_OSUTILS_H
#define __PLUGIN_OSUTILS_H


/*
 * NOTE:::: The below interfaces currently supports operations on UDP 
 * sockets only
*/

//Global or local sockets
#define GLOBAL 0
#define OTHER 1

//Connection Modes
#define BROADCAST 0
#define UNICAST 1
#define MULTICAST 2 /* We won't need this rightnow. just a place holder*/

//Connection Types, will use UDP here, TCP is a place holder for future
#define TCP_CONN 0
#define UDP_CONN 1

/* 
 *creates socket and return socket fd if successfull or some error code
 * if failed
*/
int osUtilsCreateSocket(int connMode, int connType, int fdType);

//Socket binds with the provided ipAddr and port
int osUtilsBindSocket(int sockFd, int connMode, char* ipAddr, int ipPort);

//Read from the created and bind socket
int	osUtilsReadSocket(int sockFd, char* dataBuffer, int timeout);

//write to the remote socket
int osUtilsWriteSocket(int sockFd, int connMode, char* remoteAddr, int remotePort, char* writeBuffer, int len);

//Macro to check mutex operation results for debugging
#define checkLockResults(string, val) {         \
 if (val) {                                     \
   printf("Failed with %d at %s", val, string); \
   exit(1);                                     \
 }                                              \
}
//Create lock
int osUtilsCreateLock(pthread_mutex_t *lock);
//Destroy lock
int osUtilsDestroyLock(pthread_mutex_t *lock);	
//Get lock
int osUtilsGetLock(pthread_mutex_t *lock);
//Try lock
int osUtilsTryLock(pthread_mutex_t *theLock);
//Release lock
int osUtilsReleaseLock (pthread_mutex_t *lock);

#endif
