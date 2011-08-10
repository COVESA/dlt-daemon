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
**  SRC-MODULE: dlt_daemon_common_cfg.h                                       **
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

#ifndef DLT_DAEMON_COMMON_CFG_H
#define DLT_DAEMON_COMMON_CFG_H

/*************/
/* Changable */
/*************/

/* Default Path for runtime configuration */
#define DLT_RUNTIME_DEFAULT_DIRECTORY "/tmp"
/* Path and filename for runtime configuration (applications) */
#define DLT_RUNTIME_APPLICATION_CFG "/dlt-runtime-application.cfg"
/* Path and filename for runtime configuration (contexts) */
#define DLT_RUNTIME_CONTEXT_CFG     "/dlt-runtime-context.cfg"

/* Size of text buffer */
#define DLT_DAEMON_TEXTBUFSIZE          255   

/* Initial log level */
#define DLT_DAEMON_INITIAL_LOG_LEVEL    DLT_LOG_INFO
/* Initial trace status */
#define DLT_DAEMON_INITIAL_TRACE_STATUS DLT_TRACE_STATUS_OFF

/* Application ID used when the dlt daemon creates a control message */
#define DLT_DAEMON_CTRL_APID 		 "DA1"
/* Context ID used when the dlt daemon creates a control message */
#define DLT_DAEMON_CTRL_CTID 		 "DC1"

/* Number of entries to be allocated at one in application table, 
   when no more entries are available */
#define DLT_DAEMON_APPL_ALLOC_SIZE      500
/* Number of entries to be allocated at one in context table, 
   when no more entries are available */
#define DLT_DAEMON_CONTEXT_ALLOC_SIZE  1000

/* Debug get log info function, 
   set to 1 to enable, 0 to disable debugging */
#define DLT_DEBUG_GETLOGINFO 0

/************************/
/* Don't change please! */
/************************/

/* Minimum ID for an injection message */
#define DLT_DAEMON_INJECTION_MIN      0xFFF
/* Maximum ID for an injection message */
#define DLT_DAEMON_INJECTION_MAX 0xFFFFFFFF

/* Remote interface identifier */
#define DLT_DAEMON_REMO_STRING "remo"

#endif /* DLT_DAEMON_COMMON_CFG_H */

