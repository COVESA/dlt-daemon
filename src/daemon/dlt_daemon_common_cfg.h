/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_common_cfg.h
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
/* Path and filename for runtime configuration */
#define DLT_RUNTIME_CONFIGURATION     "/dlt-runtime.cfg"

/* Default Path for control socket */
#define DLT_DAEMON_DEFAULT_CTRL_SOCK_PATH DLT_RUNTIME_DEFAULT_DIRECTORY \
    "/dlt-ctrl.sock"

#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
#define DLT_DAEMON_DEFAULT_APP_SOCK_PATH DLT_RUNTIME_DEFAULT_DIRECTORY \
    "/dlt-app.sock"
#endif

/* Size of text buffer */
#define DLT_DAEMON_COMMON_TEXTBUFSIZE          255

/* Application ID used when the dlt daemon creates a control message */
#define DLT_DAEMON_CTRL_APID         "DA1"
/* Context ID used when the dlt daemon creates a control message */
#define DLT_DAEMON_CTRL_CTID         "DC1"

/* Number of entries to be allocated at one in application table,
 * when no more entries are available */
#define DLT_DAEMON_APPL_ALLOC_SIZE      500
/* Number of entries to be allocated at one in context table,
 * when no more entries are available */
#define DLT_DAEMON_CONTEXT_ALLOC_SIZE  1000

/* Debug get log info function,
 * set to 1 to enable, 0 to disable debugging */
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
