/***************************************************************************
*
*
* mxml-private.c
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
/*
 * Include necessary headers...
 */

#include "mxml-private.h"


/*
 * 'mxml_error()' - Display an error message.
 */

void
mxml_error(const char *format,		/* I - Printf-style format string */
           ...)				/* I - Additional arguments as needed */
{
  va_list	ap;			/* Pointer to arguments */
  char		s[1024];		/* Message string */
  _mxml_global_t *global = _mxml_global();
					/* Global data */


 /*
  * Range check input...
  */

  if (!format)
    return;

 /*
  * Format the error message string...
  */

  va_start(ap, format);

  vsnprintf(s, sizeof(s), format, ap);

  va_end(ap);

 /*
  * And then display the error message...
  */

  if (global->error_cb)
    (*global->error_cb)(s);
  else
    fprintf(stderr, "mxml: %s\n", s);
}


/*
 * 'mxml_ignore_cb()' - Default callback for ignored values.
 */

mxml_type_t				/* O - Node type */
mxml_ignore_cb(mxml_node_t *node)	/* I - Current node */
{
  (void)node;

  return (MXML_IGNORE);
}


/*
 * 'mxml_integer_cb()' - Default callback for integer values.
 */

mxml_type_t				/* O - Node type */
mxml_integer_cb(mxml_node_t *node)	/* I - Current node */
{
  (void)node;

  return (MXML_INTEGER);
}


/*
 * 'mxml_opaque_cb()' - Default callback for opaque values.
 */

mxml_type_t				/* O - Node type */
mxml_opaque_cb(mxml_node_t *node)	/* I - Current node */
{
  (void)node;

  return (MXML_OPAQUE);
}


/*
 * 'mxml_real_cb()' - Default callback for real number values.
 */

mxml_type_t				/* O - Node type */
mxml_real_cb(mxml_node_t *node)		/* I - Current node */
{
  (void)node;

  return (MXML_REAL);
}


#ifdef HAVE_PTHREAD_H			/**** POSIX threading ****/
#  include <pthread.h>

static pthread_key_t	_mxml_key = -1;	/* Thread local storage key */
static pthread_once_t	_mxml_key_once = PTHREAD_ONCE_INIT;
					/* One-time initialization object */
static void		_mxml_init(void);
static void		_mxml_destructor(void *g);


/*
 * '_mxml_global()' - Get global data.
 */

_mxml_global_t *			/* O - Global data */
_mxml_global(void)
{
  _mxml_global_t	*global;	/* Global data */


  pthread_once(&_mxml_key_once, _mxml_init);

  if ((global = (_mxml_global_t *)pthread_getspecific(_mxml_key)) == NULL)
  {
    global = (_mxml_global_t *)calloc(1, sizeof(_mxml_global_t));
    pthread_setspecific(_mxml_key, global);

    global->num_entity_cbs = 1;
    global->entity_cbs[0]  = _mxml_entity_cb;
    global->wrap           = 72;
  }

  return (global);
}


/*
 * '_mxml_init()' - Initialize global data...
 */

static void
_mxml_init(void)
{
  pthread_key_create(&_mxml_key, _mxml_destructor);
}


/*
 * '_mxml_destructor()' - Free memory used for globals...
 */

static void
_mxml_destructor(void *g)		/* I - Global data */
{
  free(g);
}


#elif defined(WIN32)			/**** WIN32 threading ****/
#  include <windows.h>

static DWORD _mxml_tls_index;		/* Index for global storage */


/*
 * 'DllMain()' - Main entry for library.
 */
 
BOOL WINAPI				/* O - Success/failure */
DllMain(HINSTANCE hinst,		/* I - DLL module handle */
        DWORD     reason,		/* I - Reason */
        LPVOID    reserved)		/* I - Unused */
{
  _mxml_global_t	*global;	/* Global data */


  (void)hinst;
  (void)reserved;

  switch (reason) 
  { 
    case DLL_PROCESS_ATTACH :		/* Called on library initialization */
        if ((_mxml_tls_index = TlsAlloc()) == TLS_OUT_OF_INDEXES) 
          return (FALSE); 
        break; 

    case DLL_THREAD_DETACH :		/* Called when a thread terminates */
        if ((global = (_mxml_global_t *)TlsGetValue(_mxml_tls_index)) != NULL)
          free(global);
        break; 

    case DLL_PROCESS_DETACH :		/* Called when library is unloaded */
        if ((global = (_mxml_global_t *)TlsGetValue(_mxml_tls_index)) != NULL)
          free(global);

        TlsFree(_mxml_tls_index); 
        break; 

    default: 
        break; 
  } 

  return (TRUE);
}


/*
 * '_mxml_global()' - Get global data.
 */

_mxml_global_t *			/* O - Global data */
_mxml_global(void)
{
  _mxml_global_t	*global;	/* Global data */


  if ((global = (_mxml_global_t *)TlsGetValue(_mxml_tls_index)) == NULL)
  {
    global = (_mxml_global_t *)calloc(1, sizeof(_mxml_global_t));

    global->num_entity_cbs = 1;
    global->entity_cbs[0]  = _mxml_entity_cb;
    global->wrap           = 72;

    TlsSetValue(_mxml_tls_index, (LPVOID)global); 
  }

  return (global);
}


#else					/**** No threading ****/
/*
 * '_mxml_global()' - Get global data.
 */

_mxml_global_t *			/* O - Global data */
_mxml_global(void)
{
  static _mxml_global_t	global =	/* Global data */
  {
    NULL,				/* error_cb */
    1,					/* num_entity_cbs */
    { _mxml_entity_cb },		/* entity_cbs */
    72,					/* wrap */
    NULL,				/* custom_load_cb */
    NULL				/* custom_save_cb */
  };


  return (&global);
}
#endif /* HAVE_PTHREAD_H */


/*
 * End of "$Id: mxml-private.c 315 2007-11-22 18:01:52Z mike $".
 */
