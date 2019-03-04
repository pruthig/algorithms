/***************************************************************************
*
*
* common.h
*
* Created by Belkin International, Software Engineering on May 27, 2011
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
#ifndef _UPNP_COMMON_H_
#define _UPNP_COMMON_H_

#include "ithread.h"
#include "ixml.h" /* for IXML_Document, IXML_Element */
#include "upnp.h" /* for Upnp_EventType */
#include "upnptools.h"

ithread_mutex_t display_mutex;

typedef enum {LOG_E_INFO, LOG_E_WARNING, LOG_E_ERROR, LOG_E_EXCEPTION} LOG_E_LEVEL;

char *Util_GetElementValue(
        /*! [in] The DOM node from which to extract the value. */
        IXML_Element *element);

/*!
 * \brief Given a DOM node representing a UPnP Device Description Document,
 * this routine parses the document and finds the first service list
 * (i.e., the service list for the root device).  The service list
 * is returned as a DOM node list. The NodeList must be freed using
 * NodeList_free.
 *
 * \return The service list is returned as a DOM node list.
 */
IXML_NodeList *Util_GetFirstServiceList(
        /*! [in] The DOM node from which to extract the service list. */
        IXML_Document *doc);

/*!
 * \brief Given a document node, this routine searches for the first element
 * named by the input string item, and returns its value as a string.
 * String must be freed by caller using free.
 */
char *Util_GetFirstDocumentItem(
        /*! [in] The DOM document from which to extract the value. */
        IXML_Document *doc,
        /*! [in] The item to search for. */
        const char *item);

/*!
 * \brief Given a DOM element, this routine searches for the first element
 * named by the input string item, and returns its value as a string.
 * The string must be freed using free.
 */
char *Util_GetFirstElementItem(
        /*! [in] The DOM element from which to extract the value. */
        IXML_Element *element,
        /*! [in] The item to search for. */
        const char *item);

/*!
 * \brief Prints a callback event type as a string.
 */
void Util_PrintEventType(
        /*! [in] The callback event. */
        Upnp_EventType S);

/*!
 * \brief Prints callback event structure details.
 */
int Util_PrintEvent(
        /*! [in] The type of callback event. */
        Upnp_EventType EventType,
        /*! [in] The callback event structure. */
        void *Event);

/*!
 * \brief This routine finds the first occurance of a service in a DOM
 * representation of a description document and parses it.  Note that this
 * function currently assumes that the eventURL and controlURL values in
 * the service definitions are full URLs.  Relative URLs are not handled here.
 */
int Util_FindAndParseService (
        /*! [in] The DOM description document. */
        IXML_Document *DescDoc,
        /*! [in] The location of the description document. */
        const char *location,
        /*! [in] The type of service to search for. */
        const char *serviceType,
        /*! [out] The service ID. */
        char **serviceId,
        /*! [out] The event URL for the service. */
        char **eventURL,
        /*! [out] The control URL for the service. */
        char **controlURL);

/*!
 * \brief Prototype for displaying strings. All printing done by the device,
 * control point, and sample util, ultimately use this to display strings
 * to the user.
 */

/*!
 * \brief Releases Resources held by sample util.
 */




void FreeXmlSource(char* obj);
void FreeResource(void* obj);

typedef struct Upnp_Action_Request *pUPnPActionRequest;


#endif	//_COMMON_UTILS_H_
