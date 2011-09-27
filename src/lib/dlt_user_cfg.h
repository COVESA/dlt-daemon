/*
 * Dlt- Diagnostic Log and Trace daemon
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_cfg.h                                                **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#ifndef DLT_USER_CFG_H
#define DLT_USER_CFG_H

/*************/
/* Changable */
/*************/

/* Size of receive buffer */
#define DLT_USER_RCVBUF_MAX_SIZE 10024 

/* Temporary buffer length */
#define DLT_USER_BUFFER_LENGTH               255

/* Number of context entries, which will be allocated, 
   if no more context entries are available */
#define DLT_USER_CONTEXT_ALLOC_SIZE          500

/* Maximu length of a filename string */
#define DLT_USER_MAX_FILENAME_LENGTH         255

/* Maximum length of a single version number */
#define DLT_USER_MAX_LIB_VERSION_LENGTH		3

/* Length of buffer for constructing text output */
#define DLT_USER_TEXT_LENGTH         	   10024

/* Stack size of receiver thread */
#define DLT_USER_RECEIVERTHREAD_STACKSIZE 100000

/* default value for storage to file, not used in daemon connection */
#define DLT_USER_DEFAULT_ECU_ID "ECU1"

/* Initial log level */
#define DLT_USER_INITIAL_LOG_LEVEL    DLT_LOG_INFO

/* Initial trace status */
#define DLT_USER_INITIAL_TRACE_STATUS DLT_TRACE_STATUS_OFF

/* use extended header for non-verbose mode: 0 - don't use, 1 - use */
#define DLT_USER_USE_EXTENDED_HEADER_FOR_NONVERBOSE 0

/* default message id for non-verbose mode, if no message id was provided */
#define DLT_USER_DEFAULT_MSGID 0xffff

/* delay in receiver routine in usec (100000 usec = 100ms) */
#define DLT_USER_RECEIVE_DELAY 100000 

/* Name of environment variable for local print mode */
#define DLT_USER_ENV_LOCAL_PRINT_MODE "DLT_LOCAL_PRINT_MODE"

/************************/
/* Don't change please! */
/************************/

/* Minimum valid ID of an injection message */
#define DLT_USER_INJECTION_MIN      0xFFF

/* Defines of the different local print modes */
#define DLT_PM_UNSET     0
#define DLT_PM_AUTOMATIC 1
#define	DLT_PM_FORCE_ON  2
#define	DLT_PM_FORCE_OFF 3

#endif /* DLT_USER_CFG_H */
