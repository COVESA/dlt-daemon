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
 * \file dlt_user_macros.h
*/

/*******************************************************************************
 **                                                                            **
 **  SRC-MODULE: dlt_user_macros.h                                             **
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
 **                      Revision Control History                              **
 *******************************************************************************/

/*
 * $LastChangedRevision: 1515 $
 * $LastChangedDate: 2010-12-13 09:18:54 +0100 (Mon, 13 Dec 2010) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#ifndef DLT_USER_MACROS_H
#define DLT_USER_MACROS_H

#include "dlt_version.h"

/**
  \defgroup userapi DLT User API
  \addtogroup userapi
  \{
*/

/**************************************************************************************************
 * The folowing macros define a macro interface for DLT
 **************************************************************************************************/

/**
 * Create an object for a new context.
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 * @note To avoid the MISRA warning "Null statement is located close to other code or comments"
 *       remove the semicolon when using the macro.
 *       Example: DLT_DECLARE_CONTEXT(hContext)
 */
#define DLT_DECLARE_CONTEXT(CONTEXT) \
    DltContext CONTEXT;

/**
 * Use an object of a new context created in another module.
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 * @note To avoid the MISRA warning "Null statement is located close to other code or comments"
 *       remove the semicolon when using the macro.
 *       Example: DLT_IMPORT_CONTEXT(hContext)
 */
#define DLT_IMPORT_CONTEXT(CONTEXT) \
    extern DltContext CONTEXT;

/**
 * Register application.
 * @param APPID application id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_APP(APPID,DESCRIPTION) do {\
    (void)dlt_check_library_version(_DLT_PACKAGE_MAJOR_VERSION, _DLT_PACKAGE_MINOR_VERSION ); \
    (void)dlt_register_app( APPID, DESCRIPTION);} while(0)


/**
 * Unregister application.
 */
#define DLT_UNREGISTER_APP() do{\
    (void)dlt_unregister_app();} while(0)

/**
 * Register context (with default log level and default trace status)
 * @param CONTEXT object containing information about one special logging context
 * @param CONTEXTID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_CONTEXT(CONTEXT,CONTEXTID,DESCRIPTION) do{\
    (void)dlt_register_context(&(CONTEXT), CONTEXTID, DESCRIPTION);} while(0)

/**
 * Register context with pre-defined log level and pre-defined trace status.
 * @param CONTEXT object containing information about one special logging context
 * @param CONTEXTID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 * @param LOGLEVEL log level to be pre-set for this context
 (DLT_LOG_DEFAULT is not allowed here)
 * @param TRACESTATUS trace status to be pre-set for this context
 (DLT_TRACE_STATUS_DEFAULT is not allowed here)
 */
#define DLT_REGISTER_CONTEXT_LL_TS(CONTEXT,CONTEXTID,DESCRIPTION,LOGLEVEL,TRACESTATUS) do{\
    (void)dlt_register_context_ll_ts(&(CONTEXT), CONTEXTID, DESCRIPTION, LOGLEVEL, TRACESTATUS);} while(0)

/**
 * Unregister context.
 * @param CONTEXT object containing information about one special logging context
 */
#define DLT_UNREGISTER_CONTEXT(CONTEXT) do{\
    (void)dlt_unregister_context(&(CONTEXT));} while(0)

/**
 * Register callback function called when injection message was received
 * @param CONTEXT object containing information about one special logging context
 * @param SERVICEID service id of the injection message
 * @param CALLBACK function pointer to callback function
 */
#define DLT_REGISTER_INJECTION_CALLBACK(CONTEXT, SERVICEID, CALLBACK) do{\
    (void)dlt_register_injection_callback(&(CONTEXT),SERVICEID, CALLBACK);} while(0)

/**
 * Register callback function called when log level of context was changed
 * @param CONTEXT object containing information about one special logging context
 * @param CALLBACK function pointer to callback function
 */
#define DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK(CONTEXT, CALLBACK) do{\
    (void)dlt_register_log_level_changed_callback(&(CONTEXT),CALLBACK);} while(0)

/**
 * Send log message with variable list of messages (intended for verbose mode)
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param ARGS variable list of arguments
 * @note To avoid the MISRA warning "The comma operator has been used outside a for statement"
 *       use a semicolon instead of a comma to separate the ARGS.
 *       Example: DLT_LOG(hContext, DLT_LOG_INFO, DLT_STRING("Hello world"); DLT_INT(123));
 */
#ifdef _MSC_VER
/* DLT_LOG is not supported by MS Visual C++ */
/* use function interface instead            */
#else
#define DLT_LOG(CONTEXT,LOGLEVEL,ARGS...) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            DltContextData log_local; \
            int dlt_local; \
            dlt_local = dlt_user_log_write_start(&CONTEXT,&log_local,LOGLEVEL); \
            if (dlt_local > 0) \
            { \
                ARGS; \
                (void)dlt_user_log_write_finish(&log_local); \
            } \
        } \
    } while(0)
#endif

/**
 * Send log message with variable list of messages (intended for non-verbose mode)
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param MSGID the message id of log message
 * @param ARGS variable list of arguments:
 calls to DLT_STRING(), DLT_BOOL(), DLT_FLOAT32(), DLT_FLOAT64(),
 DLT_INT(), DLT_UINT(), DLT_RAW()
 * @note To avoid the MISRA warning "The comma operator has been used outside a for statement"
 *       use a semicolon instead of a comma to separate the ARGS.
 *       Example: DLT_LOG_ID(hContext, DLT_LOG_INFO, 0x1234, DLT_STRING("Hello world"); DLT_INT(123));
 */
#ifdef _MSC_VER
/* DLT_LOG_ID is not supported by MS Visual C++ */
/* use function interface instead               */
#else
#define DLT_LOG_ID(CONTEXT,LOGLEVEL,MSGID,ARGS...) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            DltContextData log_local; \
            int dlt_local; \
            dlt_local = dlt_user_log_write_start_id(&CONTEXT,&log_local,LOGLEVEL,MSGID); \
            if (dlt_local > 0) \
            { \
                ARGS; \
                (void)dlt_user_log_write_finish(&log_local); \
            } \
        } \
    } while(0)
#endif

/**
 * Add string parameter to the log messsage.
 * @param TEXT ASCII string
 */
#define DLT_STRING(TEXT) \
    (void)dlt_user_log_write_string(&log_local,TEXT)

/**
 * Add constant string parameter to the log messsage.
 * @param TEXT Constant ASCII string
 */
#define DLT_CSTRING(TEXT) \
    (void)dlt_user_log_write_constant_string(&log_local,TEXT)

/**
 * Add utf8-encoded string parameter to the log messsage.
 * @param TEXT UTF8-encoded string
 */
#define DLT_UTF8(TEXT) \
    (void)dlt_user_log_write_utf8_string(&log_local,TEXT)

/**
 * Add boolean parameter to the log messsage.
 * @param BOOL_VAR Boolean value (mapped to uint8)
 */
#define DLT_BOOL(BOOL_VAR) \
    (void)dlt_user_log_write_bool(&log_local,BOOL_VAR)

/**
 * Add float32 parameter to the log messsage.
 * @param FLOAT32_VAR Float32 value (mapped to float)
 */
#define DLT_FLOAT32(FLOAT32_VAR) \
    (void)dlt_user_log_write_float32(&log_local,FLOAT32_VAR)

/**
 * Add float64 parameter to the log messsage.
 * @param FLOAT64_VAR Float64 value (mapped to double)
 */
#define DLT_FLOAT64(FLOAT64_VAR) \
    (void)dlt_user_log_write_float64(&log_local,FLOAT64_VAR)

/**
 * Add integer parameter to the log messsage.
 * @param INT_VAR integer value
 */
#define DLT_INT(INT_VAR) \
    (void)dlt_user_log_write_int(&log_local,INT_VAR)

#define DLT_INT8(INT_VAR) \
    (void)dlt_user_log_write_int8(&log_local,INT_VAR)

#define DLT_INT16(INT_VAR) \
    (void)dlt_user_log_write_int16(&log_local,INT_VAR)

#define DLT_INT32(INT_VAR) \
    (void)dlt_user_log_write_int32(&log_local,INT_VAR)

#define DLT_INT64(INT_VAR) \
    (void)dlt_user_log_write_int64(&log_local,INT_VAR)

/**
 * Add unsigned integer parameter to the log messsage.
 * @param UINT_VAR unsigned integer value
 */
#define DLT_UINT(UINT_VAR) \
    (void)dlt_user_log_write_uint(&log_local,UINT_VAR)

#define DLT_UINT8(UINT_VAR) \
    (void)dlt_user_log_write_uint8(&log_local,UINT_VAR)

#define DLT_UINT16(UINT_VAR) \
    (void)dlt_user_log_write_uint16(&log_local,UINT_VAR)

#define DLT_UINT32(UINT_VAR) \
    (void)dlt_user_log_write_uint32(&log_local,UINT_VAR)

#define DLT_UINT64(UINT_VAR) \
    (void)dlt_user_log_write_uint64(&log_local,UINT_VAR)

/**
 * Add binary memory block to the log messages.
 * @param BUF pointer to memory block
 * @param LEN length of memory block
 */
#define DLT_RAW(BUF,LEN) \
    (void)dlt_user_log_write_raw(&log_local,BUF,LEN)
#define DLT_HEX8(UINT_VAR) \
    (void)dlt_user_log_write_uint8_formatted(&log_local,UINT_VAR,DLT_FORMAT_HEX8)
#define DLT_HEX16(UINT_VAR) \
    (void)dlt_user_log_write_uint16_formatted(&log_local,UINT_VAR,DLT_FORMAT_HEX16)
#define DLT_HEX32(UINT_VAR) \
    (void)dlt_user_log_write_uint32_formatted(&log_local,UINT_VAR,DLT_FORMAT_HEX32)
#define DLT_HEX64(UINT_VAR) \
    (void)dlt_user_log_write_uint64_formatted(&log_local,UINT_VAR,DLT_FORMAT_HEX64)
#define DLT_BIN8(UINT_VAR) \
    (void)dlt_user_log_write_uint8_formatted(&log_local,UINT_VAR,DLT_FORMAT_BIN8)
#define DLT_BIN16(UINT_VAR) \
    (void)dlt_user_log_write_uint16_formatted(&log_local,UINT_VAR,DLT_FORMAT_BIN16)

/**
 * Architecture independent macro to print pointers
 */
#define DLT_PTR(PTR_VAR) \
    do { \
        if (sizeof(void *) < 8) { \
            DLT_HEX32((uintptr_t)PTR_VAR); \
        } else { \
            DLT_HEX64((uintptr_t)PTR_VAR); \
        } \
    } while(0)

/**
 * Trace network message
 * @param CONTEXT object containing information about one special logging context
 * @param TYPE type of network trace message
 * @param HEADERLEN length of network message header
 * @param HEADER pointer to network message header
 * @param PAYLOADLEN length of network message payload
 * @param PAYLOAD pointer to network message payload
 */
#define DLT_TRACE_NETWORK(CONTEXT,TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD) \
    do { \
        if ((CONTEXT).trace_status_ptr && *((CONTEXT).trace_status_ptr)==DLT_TRACE_STATUS_ON) \
        { \
            (void)dlt_user_trace_network(&(CONTEXT),TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD); \
        } \
    }while(0)

/**
 * Trace network message, allow truncation
 * @param CONTEXT object containing information about one special logging context
 * @param TYPE type of network trace message
 * @param HEADERLEN length of network message header
 * @param HEADER pointer to network message header
 * @param PAYLOADLEN length of network message payload
 * @param PAYLOAD pointer to network message payload
 */
#define DLT_TRACE_NETWORK_TRUNCATED(CONTEXT,TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD) \
    do { \
        if ((CONTEXT).trace_status_ptr && *((CONTEXT).trace_status_ptr)==DLT_TRACE_STATUS_ON) \
        { \
            (void)dlt_user_trace_network_truncated(&(CONTEXT),TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD, 1); \
        } \
    }while(0)

/**
 * Trace network message, segment large messages
 * @param CONTEXT object containing information about one special logging context
 * @param TYPE type of network trace message
 * @param HEADERLEN length of network message header
 * @param HEADER pointer to network message header
 * @param PAYLOADLEN length of network message payload
 * @param PAYLOAD pointer to network message payload
 */
#define DLT_TRACE_NETWORK_SEGMENTED(CONTEXT,TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD) \
    do { \
        if ((CONTEXT).trace_status_ptr && *((CONTEXT).trace_status_ptr)==DLT_TRACE_STATUS_ON) \
        { \
            (void)dlt_user_trace_network_segmented(&(CONTEXT),TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD); \
        } \
    }while(0)

/**
 * Send log message with string parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param TEXT ASCII string
 */
#define DLT_LOG_STRING(CONTEXT,LOGLEVEL,TEXT) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            (void)dlt_log_string(&(CONTEXT), LOGLEVEL, TEXT); \
        } \
    } while(0)

/**
 * Send log message with string parameter and integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log messages
 * @param TEXT ASCII string
 * @param INT_VAR integer value
 */
#define DLT_LOG_STRING_INT(CONTEXT,LOGLEVEL,TEXT,INT_VAR) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            (void)dlt_log_string_int(&(CONTEXT), LOGLEVEL, TEXT, INT_VAR); \
        } \
    } while(0)

/**
 * Send log message with string parameter and unsigned integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param TEXT ASCII string
 * @param UINT_VAR unsigned integer value
 */
#define DLT_LOG_STRING_UINT(CONTEXT,LOGLEVEL,TEXT,UINT_VAR) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            (void)dlt_log_string_uint(&(CONTEXT),LOGLEVEL,TEXT,UINT_VAR); \
        } \
    } while(0)

/**
 * Send log message with unsigned integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param UINT_VAR unsigned integer value
 */
#define DLT_LOG_UINT(CONTEXT,LOGLEVEL,UINT_VAR) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            (void)dlt_log_uint(&(CONTEXT),LOGLEVEL,UINT_VAR); \
        } \
    } while(0)

/**
 * Send log message with integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param INT_VAR integer value
 */
#define DLT_LOG_INT(CONTEXT,LOGLEVEL,INT_VAR) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            (void)dlt_log_int(&(CONTEXT),LOGLEVEL,INT_VAR); \
        } \
    } while(0)

/**
 * Send log message with binary memory block.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param BUF pointer to memory block
 * @param LEN length of memory block
 */
#define DLT_LOG_RAW(CONTEXT,LOGLEVEL,BUF,LEN) \
    do { \
        if((CONTEXT).log_level_ptr && ((LOGLEVEL)<=(int)*((CONTEXT).log_level_ptr) ) && ((LOGLEVEL)!=0)) \
        { \
            (void)dlt_log_raw(&(CONTEXT),LOGLEVEL,BUF,LEN); \
        } \
    } while(0)

/**
 * Send log message with marker.
 */
#define DLT_LOG_MARKER() \
    do { \
        (void)dlt_log_marker(); \
    } while(0)

/**
 * Switch to verbose mode
 *
 */
#define DLT_VERBOSE_MODE() do { \
    (void)dlt_verbose_mode();} while(0)

/**
 * Switch to non-verbose mode
 *
 */
#define DLT_NONVERBOSE_MODE() do {\
    (void)dlt_nonverbose_mode();} while(0)

/**
 * Set maximum logged log level and trace status of application
 *
 * @param LOGLEVEL This is the log level to be set for the whole application
 * @param TRACESTATUS This is the trace status to be set for the whole application
 */
#define DLT_SET_APPLICATION_LL_TS_LIMIT(LOGLEVEL, TRACESTATUS) do {\
    (void)dlt_set_application_ll_ts_limit(LOGLEVEL, TRACESTATUS);} while(0)

/**
 * Enable local printing of messages
 *
 */
#define DLT_ENABLE_LOCAL_PRINT() do {\
    (void)dlt_enable_local_print();} while(0)

/**
 * Disable local printing of messages
 *
 */
#define DLT_DISABLE_LOCAL_PRINT() do {\
    (void)dlt_disable_local_print();} while(0)

/**
  \}
*/

#endif /* DLT_USER_MACROS_H */
