/*
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
 * if no more context entries are available */
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

/* delay for receiver thread (nsec) */
#define DLT_USER_RECEIVE_NDELAY (100000000)

/* Name of environment variable for local print mode */
#define DLT_USER_ENV_LOCAL_PRINT_MODE "DLT_LOCAL_PRINT_MODE"

/* Timeout offset for resending user buffer at exit in 10th milliseconds (10000 = 1s)*/
#define DLT_USER_ATEXIT_RESEND_BUFFER_EXIT_TIMEOUT 100000

/* Sleeps between resending user buffer at exit in nsec (1000000 nsec = 1ms)*/
#define DLT_USER_ATEXIT_RESEND_BUFFER_SLEEP 100000000

/* Retry interval for mq error in usec */
#define DLT_USER_MQ_ERROR_RETRY_INTERVAL 100000


/* Name of environment variable to change the dlt log message buffer size */
#define DLT_USER_ENV_LOG_MSG_BUF_LEN "DLT_LOG_MSG_BUF_LEN"

/* Maximum msg size as per autosar standard */
#define DLT_LOG_MSG_BUF_MAX_SIZE 65535


/* For trace load control */
#ifdef DLT_TRACE_LOAD_CTRL_ENABLE

/* Default value of trace load budget in bytes/sec
 *  (Default: 83333 byte/sec = 5 MB/min,  0 - No budget)
 */
#define DLT_USER_TRACE_LOAD_BUDGET_DEFAULT     (83333)

/* Default value of trace load limit in bytes/sec (Default: 0 - No limit) */
#define DLT_USER_TRACE_LOAD_LIMIT_DEFAULT      (0)

/* Number of slots in window for recording trace load (Default: 60)
 * Average trace load in this window will be used as trace load
 * Older time data than this size will be removed from trace load
 */
#define DLT_USER_TRACE_LOAD_WINDOW_SIZE        (60)

/* Window resolution in unit of timestamp (Default: 10000 x 0.1 msec = 1 sec)
 * This value is same as size of 1 slot of window.
 * Actual window size in sec can be calculated by
 * DLT_USER_TRACE_LOAD_WINDOW_SIZE x DLT_USER_TRACE_LOAD_WINDOW_RESOLUTION / DLT_USER_TIMESTAMP_RESOLUTION.
 * (Default: 60 x 10000 / 10000 = 60 sec)
 * NOTE: When timestamp resolution of DLT is changed from 0.1 msec,
 * then DLT_USER_TRACE_LOAD_WINDOW_RESOLUTION value also has to be updated accordingly.
 */
#define DLT_USER_TRACE_LOAD_WINDOW_RESOLUTION  (10000)

/* Special Context ID for output budget/limit over warning message */
#define DLT_USER_INTERNAL_CONTEXT_ID           ("DLTL")

/* Interval of warning message for trace load budget over.
 * Unit of this value is Number of slot of window.
 * NOTE: Size of the slot depends on value of DLT_USER_TRACE_LOAD_WINDOW_RESOLUTION
 * (Default: 1 slot = 10000 x 0.1 msec = 1 sec)
 */
#define DLT_USER_BUDGET_OVER_MSG_INTERVAL      (1)

/* Interval of warning message for trace load limit over.
 * Unit of this value is Number of slot of window.
 * NOTE: Size of the slot depends on value of DLT_USER_TRACE_LOAD_WINDOW_RESOLUTION
 * (Default: 1 slot = 10000 x 0.1 msec = 1 sec)
 */
#define DLT_USER_LIMIT_OVER_MSG_INTERVAL       (1)

/* Timestamp resolution of 1 second (Default: 10000 -> 1/10000 = 0.0001sec = 0.1msec)
 * This value is defined as reciprocal of the resolution (1 / DLT_USER_TIMESTAMP_RESOLUTION)
 * NOTE: When timestamp resolution of DLT is changed from 0.1 msec,
 * then DLT_USER_TIMESTAMP_RESOLUTION value also has to be updated accordingly.
 */
#define DLT_USER_TIMESTAMP_RESOLUTION          (10000)

#endif

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
