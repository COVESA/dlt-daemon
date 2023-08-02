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
 * \file dlt_types.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_types.h                                                   **
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

#ifndef DLT_TYPES_H
#define DLT_TYPES_H

#ifdef _MSC_VER
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8 int8_t;

typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;

typedef int pid_t;
typedef unsigned int speed_t;

#   define UINT16_MAX 0xFFFF

#   include <varargs.h>
#else
#   include <stdint.h>
#endif

/**
 * Definitions of DLT return values
 */
typedef enum
{
    DLT_RETURN_FILESZERR = -8,
    DLT_RETURN_LOGGING_DISABLED = -7,
    DLT_RETURN_USER_BUFFER_FULL = -6,
    DLT_RETURN_WRONG_PARAMETER = -5,
    DLT_RETURN_BUFFER_FULL = -4,
    DLT_RETURN_PIPE_FULL = -3,
    DLT_RETURN_PIPE_ERROR = -2,
    DLT_RETURN_ERROR = -1,
    DLT_RETURN_OK = 0,
    DLT_RETURN_TRUE = 1
} DltReturnValue;

/**
 * Definitions of DLT log level
 */
typedef enum
{
    DLT_LOG_DEFAULT = -1,               /**< Default log level */
    DLT_LOG_OFF = 0x00,                 /**< Log level off */
    DLT_LOG_FATAL = 0x01,               /**< fatal system error */
    DLT_LOG_ERROR = 0x02,               /**< error with impact to correct functionality */
    DLT_LOG_WARN = 0x03,                /**< warning, correct behaviour could not be ensured */
    DLT_LOG_INFO = 0x04,                /**< informational */
    DLT_LOG_DEBUG = 0x05,               /**< debug  */
    DLT_LOG_VERBOSE = 0x06,             /**< highest grade of information */
    DLT_LOG_MAX                         /**< maximum value, used for range check */
} DltLogLevelType;

/**
 * Definitions of DLT Format
 */
typedef enum
{
    DLT_FORMAT_DEFAULT = 0x00,          /**< no sepecial format */
    DLT_FORMAT_HEX8 = 0x01,             /**< Hex 8 */
    DLT_FORMAT_HEX16 = 0x02,            /**< Hex 16 */
    DLT_FORMAT_HEX32 = 0x03,            /**< Hex 32 */
    DLT_FORMAT_HEX64 = 0x04,            /**< Hex 64 */
    DLT_FORMAT_BIN8 = 0x05,             /**< Binary 8 */
    DLT_FORMAT_BIN16 = 0x06,            /**< Binary 16  */
    DLT_FORMAT_MAX                      /**< maximum value, used for range check */
} DltFormatType;

/**
 * Definitions of DLT trace status
 */
typedef enum
{
    DLT_TRACE_STATUS_DEFAULT = -1,         /**< Default trace status */
    DLT_TRACE_STATUS_OFF = 0x00,           /**< Trace status: Off */
    DLT_TRACE_STATUS_ON = 0x01,            /**< Trace status: On */
    DLT_TRACE_STATUS_MAX                   /**< maximum value, used for range check */
} DltTraceStatusType;

/**
 * Definitions for  dlt_user_trace_network/DLT_TRACE_NETWORK()
 * as defined in the DLT protocol
 */
typedef enum
{
    DLT_NW_TRACE_IPC = 0x01,                /**< Interprocess communication */
    DLT_NW_TRACE_CAN = 0x02,                /**< Controller Area Network Bus */
    DLT_NW_TRACE_FLEXRAY = 0x03,            /**< Flexray Bus */
    DLT_NW_TRACE_MOST = 0x04,               /**< Media Oriented System Transport Bus */
    DLT_NW_TRACE_RESERVED0 = 0x05,
    DLT_NW_TRACE_RESERVED1 = 0x06,
    DLT_NW_TRACE_RESERVED2 = 0x07,
    DLT_NW_TRACE_USER_DEFINED0 = 0x08,
    DLT_NW_TRACE_USER_DEFINED1 = 0x09,
    DLT_NW_TRACE_USER_DEFINED2 = 0x0A,
    DLT_NW_TRACE_USER_DEFINED3 = 0x0B,
    DLT_NW_TRACE_USER_DEFINED4 = 0x0C,
    DLT_NW_TRACE_USER_DEFINED5 = 0x0D,
    DLT_NW_TRACE_USER_DEFINED6 = 0x0E,
    DLT_NW_TRACE_RESEND = 0x0F,             /**< Mark a resend */
    DLT_NW_TRACE_MAX                        /**< maximum value, used for range check */
} DltNetworkTraceType;

/**
 * This are the log modes.
 */
typedef enum
{
    DLT_USER_MODE_UNDEFINED = -1,
    DLT_USER_MODE_OFF = 0,
    DLT_USER_MODE_EXTERNAL,
    DLT_USER_MODE_INTERNAL,
    DLT_USER_MODE_BOTH,
    DLT_USER_MODE_MAX                       /**< maximum value, used for range check */
} DltUserLogMode;

/**
 * Definition of Maintain Logstorage Loglevel modes
 */
#define DLT_MAINTAIN_LOGSTORAGE_LOGLEVEL_UNDEF -1
#define DLT_MAINTAIN_LOGSTORAGE_LOGLEVEL_OFF    0
#define DLT_MAINTAIN_LOGSTORAGE_LOGLEVEL_ON     1

typedef float float32_t;
typedef double float64_t;

#if defined DLT_LIB_USE_UNIX_SOCKET_IPC || defined DLT_LIB_USE_VSOCK_IPC
/**
 * Definition Library connection state
 */
typedef enum
{
    DLT_USER_NOT_CONNECTED = 0,
    DLT_USER_CONNECTED,
    DLT_USER_RETRY_CONNECT
} DltUserConnectionState;
#endif

/**
 * Definition of timestamp types
 */
typedef enum
{
	DLT_AUTO_TIMESTAMP = 0,
	DLT_USER_TIMESTAMP
} DltTimestampType;

#endif  /* DLT_TYPES_H */
