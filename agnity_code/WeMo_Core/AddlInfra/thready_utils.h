/***************************************************************************
* thread_utils.h
*
* Created by Belkin International, Software Engineering on 2014-06-24
* Copyright (c) 2014 Belkin International, Inc. and/or its
* affiliates. All rights reserved.
*
* Belkin International, Inc. retains all right, title and interest
* (including all intellectual property rights) in and to this computer
* program, which is protected by applicable intellectual property
* laws.  Unless you have obtained a separate written license from
* Belkin International, Inc., you are not authorized to utilize all or
* a part of this computer program for any purpose (including
* reproduction, distribution, modification, and compilation into
* object code) and you must immediately destroy or return to Belkin
* International, Inc all copies of this computer program.  If you are
* licensed by Belkin International, Inc., your rights to utilize this
* computer program are limited by the terms of that license.
*
* To obtain a license, please contact Belkin International, Inc.
*
* This computer program contains trade secrets owned by Belkin
* International, Inc.  and, unless unauthorized by Belkin
* International, Inc. in writing, you agree to maintain the
* confidentiality of this computer program and related information and
* to not disclose this computer program and related information to any
* other person or entity.
*
* THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
* BELKIN INTERNATIONAL, INC.  EXPRESSLY DISCLAIMS ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING THE WARRANTIES OF MERCHANTIBILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND NON-INFRINGEMENT.
*
*
***************************************************************************/
#ifndef __THREAD_UTILS_H
#define __THREAD_UTILS_H

/** The pthread standard states that thread names are never more than
 * 16 characters long.  Oddly, there is no pre-defined constant with
 * this value so we make our own here.
 */
#define PTHREAD_NAME_SIZE (16)

/** @brief Set calling thread's name.
 * @param name Null-terminated string holding name.  Only
 * PTHREAD_NAME_SIZE bytes will be stored.  Longer strings will be
 * truncated.  Even when truncated there will always be a terminating
 * NULL.
 */
extern void tu_set_my_thread_name( const char* name );

/** @brief Get name of calling thread.
 * @return NULL terminated string containing thread name.  Storage is
 * managed automaticall.  DO NOT FREE.
 */
extern const char *tu_get_my_thread_name( void );

#endif

/* -------------------- End of file -------------------- */
