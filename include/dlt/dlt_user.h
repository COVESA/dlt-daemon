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
 * \file dlt_user.h
*/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user.h                                                    **
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
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#ifndef DLT_USER_H
#define DLT_USER_H

/**
  \defgroup userapi DLT User API
  \addtogroup userapi
  \{
*/
#include <mqueue.h>

#if !defined (__WIN32__)
#include <semaphore.h>
#endif

#include "dlt_types.h"
#include "dlt_user_macros.h"
#include "dlt_shm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DLT_USER_BUF_MAX_SIZE 1390               /**< maximum size of each user buffer, also used for injection buffer */

#define DLT_USER_RESENDBUF_MAX_SIZE (DLT_USER_BUF_MAX_SIZE + 100)       /**< Size of resend buffer; Max DLT message size is 1390 bytes plus some extra header space  */

/* Use a semaphore or mutex from your OS to prevent concurrent access to the DLT buffer. */
#define DLT_SEM_LOCK() { sem_wait(&dlt_mutex); }
#define DLT_SEM_FREE() { sem_post(&dlt_mutex); }

/**
 * This structure is used for every context used in an application.
 */
typedef struct
{
    char contextID[DLT_ID_SIZE];                  /**< context id */
    int32_t log_level_pos;                        /**< offset in user-application context field */
    int8_t *log_level_ptr;                        /**< pointer to the log level */
    int8_t *trace_status_ptr;                     /**< pointer to the trace status */
    uint8_t mcnt;                                 /**< message counter */
} DltContext;

/**
 * This structure is used for context data used in an application.
 */
typedef struct
{
    DltContext *handle;                           /**< pointer to DltContext */
    unsigned char buffer[DLT_USER_BUF_MAX_SIZE];  /**< buffer for building log message*/
    int32_t size;                                 /**< payload size */
    int32_t log_level;                            /**< log level */
    int32_t trace_status;                         /**< trace status */
    int32_t args_num;                             /**< number of arguments for extended header*/
    char* context_description;                    /**< description of context */
} DltContextData;

typedef struct
{
    uint32_t service_id;
    int (*injection_callback)(uint32_t service_id, void *data, uint32_t length);
} DltUserInjectionCallback;

typedef struct
{
    char contextID[DLT_ID_SIZE];      /**< Context ID */
    int8_t log_level;                 /**< Log level */
    int8_t trace_status;              /**< Trace status */
    void (*log_level_changed_callback) (char context_id[DLT_ID_SIZE],uint8_t log_level,uint8_t trace_status);
} DltUserLogLevelChangedCallback;

/**
 * This structure is used in a table managing all contexts and the corresponding log levels in an application.
 */
typedef struct
{
    char contextID[DLT_ID_SIZE];      /**< Context ID */
    int8_t log_level;                 /**< Log level */
    int8_t *log_level_ptr;             /**< Ptr to the log level */
    int8_t trace_status;              /**< Trace status */
    int8_t *trace_status_ptr;             /**< Ptr to the trace status */
    char *context_description;        /**< description of context */
    DltUserInjectionCallback *injection_table; /**< Table with pointer to injection functions and service ids */
    uint32_t nrcallbacks;

    /* Log Level changed callback */
    void (*log_level_changed_callback) (char context_id[DLT_ID_SIZE],uint8_t log_level,uint8_t trace_status);

} dlt_ll_ts_type;

/**
 * @brief holds initial log-level for given appId:ctxId pair
 */
typedef struct
{
    char appId[DLT_ID_SIZE];
    char ctxId[DLT_ID_SIZE];
    int8_t ll;
} dlt_env_ll_item;


/**
 * @brief holds all initial log-levels given via environment variable DLT_INITIAL_LOG_LEVEL
 */
typedef struct
{
    dlt_env_ll_item * item;
    size_t array_size;
    size_t num_elem;
} dlt_env_ll_set;


/**
 * This structure is used once for one application.
 */
typedef struct
{
    char ecuID[DLT_ID_SIZE];                   /**< ECU ID */
    char appID[DLT_ID_SIZE];                   /**< Application ID */
    int dlt_log_handle;                        /**< Handle to fifo of dlt daemon */
    int dlt_user_handle;                       /**< Handle to own fifo */
    mqd_t dlt_segmented_queue_read_handle;     /**< Handle message queue */
    mqd_t dlt_segmented_queue_write_handle;    /**< Handle message queue */
    pthread_t dlt_segmented_nwt_handle;        /**< thread handle of segmented sending */

    int8_t dlt_is_file;                        /**< Target of logging: 1 to file, 0 to daemon */

    dlt_ll_ts_type *dlt_ll_ts;                 /** [MAX_DLT_LL_TS_ENTRIES]; < Internal management struct for all
                                                   contexts */
    uint32_t dlt_ll_ts_max_num_entries;        /**< Maximum number of contexts */

    uint32_t dlt_ll_ts_num_entries;            /**< Number of used contexts */

    int8_t overflow;                           /**< Overflow marker, set to 1 on overflow, 0 otherwise */
    uint32_t overflow_counter;                 /**< Counts the number of lost messages */

    char *application_description;             /**< description of application */

    DltReceiver receiver;                      /**< Receiver for internal user-defined messages from daemon */

    int8_t verbose_mode;                       /**< Verbose mode enabled: 1 enabled, 0 disabled */
    int8_t use_extende_header_for_non_verbose; /**< Use extended header for non verbose: 1 enabled, 0 disabled */
    int8_t with_session_id;                    /**< Send always session id: 1 enabled, 0 disabled */
    int8_t with_timestamp;                     /**< Send always timestamp: 1 enabled, 0 disabled */
    int8_t with_ecu_id;                        /**< Send always ecu id: 1 enabled, 0 disabled */

    int8_t enable_local_print;                 /**< Local printing of log messages: 1 enabled, 0 disabled */
    int8_t local_print_mode;                   /**< Local print mode, controlled by environment variable */

    int8_t log_state;                          /**< Log state of external connection:
                                                  1 client connected,
                                                  0 not connected,
                                                 -1 unknown */

    DltBuffer startup_buffer; /**< Ring-buffer for buffering messages during startup and missing connection */
    /* Buffer used for resending, locked by DLT semaphore */
    uint8_t resend_buffer[DLT_USER_RESENDBUF_MAX_SIZE];

    uint32_t timeout_at_exit_handler; /**< timeout used in dlt_user_atexit_blow_out_user_buffer, in 0.1 milliseconds */
    dlt_env_ll_set initial_ll_set;

#ifdef DLT_SHM_ENABLE
    DltShm dlt_shm;
#endif
#ifdef DLT_TEST_ENABLE
    int corrupt_user_header;
    int corrupt_message_size;
    int16_t corrupt_message_size_size;
#endif
} DltUser;

/**************************************************************************************************
* The following API functions define a low level function interface for DLT
**************************************************************************************************/

/**
 * Initialize the generation of a DLT log message (intended for usage in verbose mode)
 * This function has to be called first, when an application wants to send a new log messages.
 * Following functions like dlt_user_log_write_string and dlt_user_log_write_finish must only be called,
 * when return value is bigger than zero.
 * @param handle pointer to an object containing information about one special logging context
 * @param log pointer to an object containing information about logging context data
 * @param loglevel this is the current log level of the log message to be sent
 * @return Value from DltReturnValue enum, DLT_RETURN_TRUE if log level is matching
 */
DltReturnValue dlt_user_log_write_start(DltContext *handle, DltContextData *log, DltLogLevelType loglevel);

/**
 * Initialize the generation of a DLT log message (intended for usage in non-verbose mode)
 * This function has to be called first, when an application wants to send a new log messages.
 * Following functions like dlt_user_log_write_string and dlt_user_log_write_finish must only be called,
 * when return value is bigger than zero.
 * @param handle pointer to an object containing information about one special logging context
 * @param log pointer to an object containing information about logging context data
 * @param loglevel this is the current log level of the log message to be sent
 * @param messageid message id of message
 * @return Value from DltReturnValue enum, DLT_RETURN_TRUE if log level is matching
 */
DltReturnValue dlt_user_log_write_start_id(DltContext *handle, DltContextData *log, DltLogLevelType loglevel, uint32_t messageid);

/**
 * Finishing the generation of a DLT log message and sending it to the DLT daemon.
 * This function has to be called after writing all the log attributes of a log message.
 * @param log pointer to an object containing information about logging context data
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_finish(DltContextData *log);

/**
 * Write a boolean parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data boolean parameter written into log message (mapped to uint8)
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_bool(DltContextData *log, uint8_t data);

/**
 * Write a float parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data float32_t parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_float32(DltContextData *log, float32_t data);

/**
 * Write a double parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data float64_t parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_float64(DltContextData *log, double data);

/**
 * Write a uint parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data unsigned int parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_uint(DltContextData *log, unsigned int data);
DltReturnValue dlt_user_log_write_uint8(DltContextData *log, uint8_t data);
DltReturnValue dlt_user_log_write_uint16(DltContextData *log, uint16_t data);
DltReturnValue dlt_user_log_write_uint32(DltContextData *log, uint32_t data);
DltReturnValue dlt_user_log_write_uint64(DltContextData *log, uint64_t data);

/**
 * Write a uint parameter into a DLT log message. The output will be formatted as given by the parameter type.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data unsigned int parameter written into log message.
 * @param type The formatting type of the string output.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_uint8_formatted(DltContextData *log, uint8_t data, DltFormatType type);
DltReturnValue dlt_user_log_write_uint16_formatted(DltContextData *log, uint16_t data, DltFormatType type);
DltReturnValue dlt_user_log_write_uint32_formatted(DltContextData *log, uint32_t data, DltFormatType type);
DltReturnValue dlt_user_log_write_uint64_formatted(DltContextData *log, uint64_t data, DltFormatType type);

/**
 * Write a pointer value architecture independent.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data void* parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_ptr(DltContextData *log, void *data);

/**
 * Write a int parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data int parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_int(DltContextData *log, int data);
DltReturnValue dlt_user_log_write_int8(DltContextData *log, int8_t data);
DltReturnValue dlt_user_log_write_int16(DltContextData *log, int16_t data);
DltReturnValue dlt_user_log_write_int32(DltContextData *log, int32_t data);
DltReturnValue dlt_user_log_write_int64(DltContextData *log, int64_t data);
/**
 * Write a null terminated ASCII string into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param text pointer to the parameter written into log message containing null termination.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_string( DltContextData *log, const char *text);

/**
 * Write a constant null terminated ASCII string into a DLT log message.
 * In non verbose mode DLT parameter will not be send at all.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param text pointer to the parameter written into log message containing null termination.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_constant_string( DltContextData *log, const char *text);

/**
 * Write a null terminated UTF8 string into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param text pointer to the parameter written into log message containing null termination.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_utf8_string(DltContextData *log, const char *text);

/**
 * Write a binary memory block into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data pointer to the parameter written into log message.
 * @param length length in bytes of the parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_raw(DltContextData *log,void *data,uint16_t length);

/**
 * Write a binary memory block into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data pointer to the parameter written into log message.
 * @param length length in bytes of the parameter written into log message.
 * @param type the format information.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_write_raw_formatted(DltContextData *log,void *data,uint16_t length,DltFormatType type);

/**
 * Trace network message
 * @param handle pointer to an object containing information about one special logging context
 * @param nw_trace_type type of network trace (DLT_NW_TRACE_IPC, DLT_NW_TRACE_CAN, DLT_NW_TRACE_FLEXRAY, or DLT_NW_TRACE_MOST)
 * @param header_len length of network message header
 * @param header pointer to network message header
 * @param payload_len length of network message payload
 * @param payload pointer to network message payload
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_trace_network(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);

/**
 * Trace network message, truncated if necessary.
 * @param handle pointer to an object containing information about logging context
 * @param nw_trace_type type of network trace (DLT_NW_TRACE_IPC, DLT_NW_TRACE_CAN, DLT_NW_TRACE_FLEXRAY, or DLT_NW_TRACE_MOST)
 * @param header_len length of network message header
 * @param header pointer to network message header
 * @param payload_len length of network message payload
 * @param payload pointer to network message payload
 * @param allow_truncate Set to > 0 to allow truncating of the message if it is too large.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_trace_network_truncated(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload, int allow_truncate);

/**
 * Trace network message in segmented asynchronous mode.
 * The sending of the data is done in a separate thread.
 * Please note that handle must exist for the lifetime of the application, because
 * data chunks are sent asynchronously in undetermined future time.
 * @param handle pointer to an object containing information about logging context
 * @param nw_trace_type type of network trace (DLT_NW_TRACE_IPC, DLT_NW_TRACE_CAN, DLT_NW_TRACE_FLEXRAY, or DLT_NW_TRACE_MOST)
 * @param header_len length of network message header
 * @param header pointer to network message header
 * @param payload_len length of network message payload
 * @param payload pointer to network message payload
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_trace_network_segmented(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);

/**************************************************************************************************
 * The following API functions define a high level function interface for DLT
 **************************************************************************************************/

/**
 * Initialize the user lib communication with daemon.
 * This function has to be called first, before using any DLT user lib functions.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_init();

/**
 * Initialize the user lib writing only to file.
 * This function has to be called first, before using any DLT user lib functions.
 * @param name name of an optional log file
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_init_file(const char *name);

/**
 * Terminate the user lib.
 * This function has to be called when finishing using the DLT user lib.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_free();

/**
 * Check the library version of DLT library.
 * @param user_major_version the major version to be compared
 * @param user_minor_version the minor version to be compared
 * @return Value from DltReturnValue enum, DLT_RETURN_ERROR if there is a mismatch
 */
DltReturnValue dlt_check_library_version(const char * user_major_version, const char * user_minor_version);

/**
 * Register an application in the daemon.
 * @param appid four byte long character array with the application id
 * @param description long name of the application
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_register_app(const char *appid, const char * description);

/**
 * Unregister an application in the daemon.
 * This function has to be called when finishing using an application.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_unregister_app(void);

/**
 * Register a context in the daemon.
 * This function has to be called before first usage of the context.
 * @param handle pointer to an object containing information about one special logging context
 * @param contextid four byte long character array with the context id
 * @param description long name of the context
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_register_context(DltContext *handle, const char *contextid, const char * description);

/**
 * Register a context in the daemon with pre-defined log level and pre-defined trace status.
 * This function has to be called before first usage of the context.
 * @param handle pointer to an object containing information about one special logging context
 * @param contextid four byte long character array with the context id
 * @param description long name of the context
 * @param loglevel This is the log level to be pre-set for this context
          (DLT_LOG_DEFAULT is not allowed here)
 * @param tracestatus This is the trace status to be pre-set for this context
          (DLT_TRACE_STATUS_DEFAULT is not allowed here)
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_register_context_ll_ts(DltContext *handle, const char *contextid, const char * description, int loglevel, int tracestatus);

/**
 * Unregister a context in the DLT daemon.
 * This function has to be called when finishing using a context.
 * @param handle pointer to an object containing information about one special logging context
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_unregister_context(DltContext *handle);


/**
 * Set maximum timeout for re-sending at exit
 * @param timeout_in_milliseconds maximum time to wait until giving up re-sending, default 10000 (equals to 10 seconds)
 */
int dlt_set_resend_timeout_atexit(uint32_t timeout_in_milliseconds);

/**
 * Set the logging mode used by the daemon.
 * The logging mode is stored persistantly by the daemon.
 * @see DltUserLogMode
 * @param mode the new logging mode used by the daemon: off, extern, internal, both.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_set_log_mode(DltUserLogMode mode);

/**
 * Get the state of the connected client to the daemon.
 * The user application gets a message, when client is connected or disconnected.
 * This value contains the last state.
 * It needs some time until the application gets state from the daemon.
 * Until then the state is "unknown state".
 * @return -1 = unknown state, 0 = client not connected, 1 = client connected
 */
int dlt_get_log_state();

/**
 * Register callback function called when injection message was received
 * @param handle pointer to an object containing information about one special logging context
 * @param service_id the service id to be waited for
 * @param (*dlt_injection_callback) function pointer to callback function
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_register_injection_callback(DltContext *handle, uint32_t service_id,
        int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length));

/**
 * Register callback function called when log level of context was changed
 * @param handle pointer to an object containing information about one special logging context
 * @param (*dlt_log_level_changed_callback) function pointer to callback function
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_register_log_level_changed_callback(DltContext *handle,
        void (*dlt_log_level_changed_callback)(char context_id[DLT_ID_SIZE],uint8_t log_level, uint8_t trace_status));

/**
 * Switch to verbose mode
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_verbose_mode(void);

/**
 * Check the version of dlt library with library version used of the application.
 * @param Major version number of application - see dlt_version.h
 * @param Minor version number of application - see dlt_version.h
 *  @return Value from DltReturnValue enum, DLT_RETURN_ERROR if there is a mismatch
 */
DltReturnValue dlt_user_check_library_version(const char *user_major_version,const char *user_minor_version);

/**
 * Switch to non-verbose mode
 *
 */
DltReturnValue dlt_nonverbose_mode(void);

/**
 * Use extended header in non verbose mode.
 * Enabled by default.
 * @param use_extende_header_for_non_verbose Use extended header for non verbose mode if true
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_use_extended_header_for_non_verbose(int8_t use_extende_header_for_non_verbose);

/**
 * Send session id configuration.
 * Enabled by default.
 * @param with_session_id Send session id in each message if enabled
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_with_session_id(int8_t with_session_id);

/**
 * Send timestamp configuration.
 * Enabled by default.
 * @param with_timestamp Send timestamp id in each message if enabled
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_with_timestamp(int8_t with_timestamp);

/**
 * Send ecu id configuration.
 * Enabled by default.
 * @param with_ecu_id Send ecu id in each message if enabled
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_with_ecu_id(int8_t with_ecu_id);

/**
 * Set maximum logged log level and trace status of application
 *
 * @param loglevel This is the log level to be set for the whole application
 * @param tracestatus This is the trace status to be set for the whole application
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_set_application_ll_ts_limit(DltLogLevelType loglevel, DltTraceStatusType tracestatus);


/**
 * @brief adjust log-level based on values given through environment
 *
 * Iterate over the set of items, and find the best match.
 * For any item that matches, the one with the highest priority is selected and that
 * log-level is returned.
 *
 * Priorities are determined as follows:
 * - no apid, no ctid only ll given in item: use ll with prio 1
 * - no apid, ctid matches: use ll with prio 2
 * - no ctid, apid matches: use ll with prio 3
 * - apid, ctid matches: use ll with prio 4
 *
 * If no item matches or in case of error, the original log-level (\param ll) is returned
 */
int dlt_env_adjust_ll_from_env(dlt_env_ll_set const * const ll_set, char const * const apid, char const * const ctid, int const ll);

/**
 * @brief extract log-level settings from given string
 *
 * Scan \param env for setttings like apid:ctid:log-level and store them
 * in given \param ll_set
 *
 * @param env reference to a string to be parsed, after parsing env will point after the last parse character
 * @param ll_set set of log-level extracted from given string
 *
 * @return 0 on success
 * @return -1 on failure
 */
int dlt_env_extract_ll_set(char ** const env, dlt_env_ll_set * const ll_set);

void dlt_env_free_ll_set(dlt_env_ll_set * const ll_set);

/**
 * Enable local printing of messages
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_enable_local_print(void);

/**
 * Disable local printing of messages
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_disable_local_print(void);

/**
 * Write a null terminated ASCII string into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param text pointer to the ASCII string written into log message containing null termination.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text);

/**
 * Write a null terminated ASCII string and an integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param text pointer to the ASCII string written into log message containing null termination.
 * @param data integer value written into the log message
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data);

/**
 * Write a null terminated ASCII string and an unsigned integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param text pointer to the ASCII string written into log message containing null termination.
 * @param data unsigned integer value written into the log message
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data);

/**
 * Write an integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param data integer value written into the log message
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data);

/**
 * Write an unsigned integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param data unsigned integer value written into the log message
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data);

/**
 * Write an unsigned integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param data pointer to the parameter written into log message.
 * @param length length in bytes of the parameter written into log message.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length);

/**
 * Write marker message to DLT.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_log_marker();

/**
 * Forward a complete DLT message to the DLT daemon
 * @param msgdata Message data of DLT message
 * @param size Size of DLT message
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_forward_msg(void *msgdata,size_t size);

/**
 * Get the total size and available size of the shared memory buffer between daemon and applications.
 * This information is useful to control the flow control between applications and daemon.
 * For example only 50% of the buffer should be used for file transfer.
 * @param total_size total size of buffer in bytes
 * @param used_size used size of buffer in bytes
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_check_buffer(int *total_size, int *used_size);

/**
 * Try to resend log message in the user buffer. Stops if the dlt_uptime is bigger than
 * dlt_uptime() + DLT_USER_ATEXIT_RESEND_BUFFER_EXIT_TIMEOUT. A pause between the resending
 * attempts can be defined with DLT_USER_ATEXIT_RESEND_BUFFER_SLEEP
 * @return number of messages in the user buffer
 */
int dlt_user_atexit_blow_out_user_buffer(void);

/**
 * Try to resend log message in the user buffer.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_resend_buffer(void);

/**
 * Checks the log level passed by the log function if enabled for that context or not.
 * This function can be called by applications before generating their logs.
 * Also called before writing new log messages.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @return Value from DltReturnValue enum, DLT_RETURN_TRUE if log level is enabled
 */
static inline DltReturnValue dlt_user_is_logLevel_enabled(DltContext *handle,DltLogLevelType loglevel)
{
    if (handle == NULL || handle->log_level_ptr == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (loglevel <= (DltLogLevelType)(*(handle->log_level_ptr)) && loglevel != DLT_LOG_OFF)
    {
        return DLT_RETURN_TRUE;
    }

    return DLT_RETURN_LOGGING_DISABLED;
}

#ifdef DLT_TEST_ENABLE
    void dlt_user_test_corrupt_user_header(int enable);
    void dlt_user_test_corrupt_message_size(int enable,int16_t size);
#endif /* DLT_TEST_ENABLE */

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_USER_H */
