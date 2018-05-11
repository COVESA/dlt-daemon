/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_user_cfg.h
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

/* Size of ring buffer */
#define DLT_USER_RINGBUFFER_MIN_SIZE   50000
#define DLT_USER_RINGBUFFER_MAX_SIZE  500000
#define DLT_USER_RINGBUFFER_STEP_SIZE  50000

/* Name of environment variable for ringbuffer configuration */
#define DLT_USER_ENV_BUFFER_MIN_SIZE  "DLT_USER_BUFFER_MIN"
#define DLT_USER_ENV_BUFFER_MAX_SIZE  "DLT_USER_BUFFER_MAX"
#define DLT_USER_ENV_BUFFER_STEP_SIZE "DLT_USER_BUFFER_STEP"

/* Temporary buffer length */
#define DLT_USER_BUFFER_LENGTH               255

/* Number of context entries, which will be allocated,
   if no more context entries are available */
#define DLT_USER_CONTEXT_ALLOC_SIZE          500

/* Maximu length of a filename string */
#define DLT_USER_MAX_FILENAME_LENGTH         255

/* Maximum length of a single version number */
#define DLT_USER_MAX_LIB_VERSION_LENGTH        3

/* Length of buffer for constructing text output */
#define DLT_USER_TEXT_LENGTH                10024

/* Stack size of receiver thread */
#define DLT_USER_RECEIVERTHREAD_STACKSIZE 100000

/* default value for storage to file, not used in daemon connection */
#define DLT_USER_DEFAULT_ECU_ID "ECU1"

/* Initial log level */
#define DLT_USER_INITIAL_LOG_LEVEL    DLT_LOG_INFO

/* Initial trace status */
#define DLT_USER_INITIAL_TRACE_STATUS DLT_TRACE_STATUS_OFF

/* use extended header for non-verbose mode: 0 - don't use, 1 - use */
#define DLT_USER_USE_EXTENDED_HEADER_FOR_NONVERBOSE 1

/* send always session id: 0 - don't use, 1 - use */
#define DLT_USER_WITH_SESSION_ID 1

/* send always timestamp: 0 - don't use, 1 - use */
#define DLT_USER_WITH_TIMESTAMP 1

/* send always ecu id: 0 - don't use, 1 - use */
#define DLT_USER_WITH_ECU_ID 1

/* default message id for non-verbose mode, if no message id was provided */
#define DLT_USER_DEFAULT_MSGID 0xffff

/* delay in receiver routine in usec (100000 usec = 100ms) */
#define DLT_USER_RECEIVE_DELAY 100000

/* Name of environment variable for local print mode */
#define DLT_USER_ENV_LOCAL_PRINT_MODE "DLT_LOCAL_PRINT_MODE"

/* Timeout offset for resending user buffer at exit in 10th milliseconds (10000 = 1s)*/
#define DLT_USER_ATEXIT_RESEND_BUFFER_EXIT_TIMEOUT 100000

/* Sleeps between resending user buffer at exit in usec (1000 usec = 1ms)*/
#define DLT_USER_ATEXIT_RESEND_BUFFER_SLEEP 100000

/* Retry interval for mq error in usec */
#define DLT_USER_MQ_ERROR_RETRY_INTERVAL 100000


/************************/
/* Don't change please! */
/************************/

/* Minimum valid ID of an injection message */
#define DLT_USER_INJECTION_MIN      0xFFF

/* Defines of the different local print modes */
#define DLT_PM_UNSET     0
#define DLT_PM_AUTOMATIC 1
#define    DLT_PM_FORCE_ON  2
#define    DLT_PM_FORCE_OFF 3

#endif /* DLT_USER_CFG_H */
