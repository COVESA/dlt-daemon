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
 * \file dlt_user_shared.h
 */



/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_shared.h                                             **
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

#ifndef DLT_USER_SHARED_H
#define DLT_USER_SHARED_H

#include "dlt_types.h"
#include "dlt_user.h"

#include <sys/types.h>

#ifdef DLT_LIB_USE_QNX_MESSAGE_IPC
#    ifdef __QNX__
#        include <sys/iofunc.h>
#        include <sys/dispatch.h>
#    else
#        error DLT_IPC="QNX_MESSAGE" can only be built for QNX
#    endif /* __QNX__ */
#endif /* DLT_DAEMON_USE_QNX_MESSAGE_IPC */

/**
 * This is the header of each message to be exchanged between application and daemon.
 */
typedef struct
{
    char pattern[DLT_ID_SIZE];      /**< This pattern should be DUH0x01 */
    uint32_t message;               /**< messsage info */
} DLT_PACKED DltUserHeader;

/**
 * This is the internal message content to exchange control msg register app information between application and daemon.
 */
typedef struct
{
    char apid[DLT_ID_SIZE];          /**< application id */
    pid_t pid;                       /**< process id of user application */
    uint32_t description_length;     /**< length of description */
} DLT_PACKED DltUserControlMsgRegisterApplication;

/**
 * This is the internal message content to exchange control msg unregister app information between application and daemon.
 */
typedef struct
{
    char apid[DLT_ID_SIZE];         /**< application id */
    pid_t pid;                      /**< process id of user application */
} DLT_PACKED DltUserControlMsgUnregisterApplication;

/**
 * This is the internal message content to exchange control msg register information between application and daemon.
 */
typedef struct
{
    char apid[DLT_ID_SIZE];          /**< application id */
    char ctid[DLT_ID_SIZE];          /**< context id */
    int32_t log_level_pos;           /**< offset in management structure on user-application side */
    int8_t log_level;                /**< log level */
    int8_t trace_status;             /**< trace status */
    pid_t pid;                       /**< process id of user application */
    uint32_t description_length;     /**< length of description */
} DLT_PACKED DltUserControlMsgRegisterContext;

/**
 * This is the internal message content to exchange control msg unregister information between application and daemon.
 */
typedef struct
{
    char apid[DLT_ID_SIZE];         /**< application id */
    char ctid[DLT_ID_SIZE];         /**< context id */
    pid_t pid;                      /**< process id of user application */
} DLT_PACKED DltUserControlMsgUnregisterContext;

/**
 * This is the internal message content to exchange control msg log level information between application and daemon.
 */
typedef struct
{
    uint8_t log_level;             /**< log level */
    uint8_t trace_status;          /**< trace status */
    int32_t log_level_pos;          /**< offset in management structure on user-application side */
} DLT_PACKED DltUserControlMsgLogLevel;

/**
 * This is the internal message content to exchange control msg injection information between application and daemon.
 */
typedef struct
{
    int32_t log_level_pos;          /**< offset in management structure on user-application side */
    uint32_t service_id;            /**< service id of injection */
    uint32_t data_length_inject;    /**< length of injection message data field */
} DLT_PACKED DltUserControlMsgInjection;

/**
 * This is the internal message content to exchange information about application log level and trace stats between
 * application and daemon.
 */
typedef struct
{
    char apid[DLT_ID_SIZE];        /**< application id */
    uint8_t log_level;             /**< log level */
    uint8_t trace_status;          /**< trace status */
} DLT_PACKED DltUserControlMsgAppLogLevelTraceStatus;

/**
 * This is the internal message content to set the logging mode: off, external, internal, both.
 */
typedef struct
{
    int8_t log_mode;          /**< the mode to be used for logging: off, external, internal, both */
} DLT_PACKED DltUserControlMsgLogMode;

/**
 * This is the internal message content to get the logging state: 0 = off, 1 = external client connected.
 */
typedef struct
{
    int8_t log_state;          /**< the state to be used for logging state: 0 = off, 1 = external client connected */
} DLT_PACKED DltUserControlMsgLogState;

/**
 * This is the internal message content to get the number of lost messages reported to the daemon.
 */
typedef struct
{
    uint32_t overflow_counter;          /**< counts the number of lost messages */
    char apid[4];                        /**< application which lost messages */
} DLT_PACKED DltUserControlMsgBufferOverflow;

/**************************************************************************************************
* The folowing functions are used shared between the user lib and the daemon implementation
**************************************************************************************************/

/**
 * Set user header marker and store message type in user header
 * @param userheader pointer to the userheader
 * @param mtype user message type of internal message
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_set_userheader(DltUserHeader *userheader, uint32_t mtype);

/**
 * Check if user header contains its marker
 * @param userheader pointer to the userheader
 * @return 0 no, 1 yes, negative value if there was an error
 */
int dlt_user_check_userheader(DltUserHeader *userheader);

/**
 * Atomic write to file descriptor, using vector of 2 elements
 * @param handle file descriptor
 * @param ptr1 generic pointer to first segment of data to be written
 * @param len1 length of first segment of data to be written
 * @param ptr2 generic pointer to second segment of data to be written
 * @param len2 length of second segment of data to be written
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_out2(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2);

/**
 * Atomic write to file descriptor, using vector of 3 elements
 * @param handle file descriptor
 * @param ptr1 generic pointer to first segment of data to be written
 * @param len1 length of first segment of data to be written
 * @param ptr2 generic pointer to second segment of data to be written
 * @param len2 length of second segment of data to be written
 * @param ptr3 generic pointer to third segment of data to be written
 * @param len3 length of third segment of data to be written
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_out3(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3);

#ifdef DLT_DAEMON_USE_QNX_MESSAGE_IPC
/** @brief Create QNX messaging channel for receiving new log levels from dlt-daemon.
 *
 * This function registers a name in the pathname space and create a channel
 * @param attach - pointer to a name_attach_t structure that looks like this:
 * typedef struct _name_attach {
 *     dispatch_t* dpp;
 *     int         chid;
 *     int         mntid;
 *     int         zero[2];
 * } name_attach_t;
 * @return negative value if there was an error
 */
DltReturnValue dlt_attach_channel(name_attach_t **attach);

/**
 * Atomic write to file descriptor, using vector of 2 elements
 * @param handle file descriptor
 * @param ptr1 generic pointer to first segment of data to be written
 * @param len1 length of first segment of data to be written
 * @param ptr2 generic pointer to second segment of data to be written
 * @param len2 length of second segment of data to be written
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_out2_qnx_msg(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2);

/**
 * Atomic write to file descriptor, using vector of 3 elements
 * @param handle file descriptor
 * @param ptr1 generic pointer to first segment of data to be written
 * @param len1 length of first segment of data to be written
 * @param ptr2 generic pointer to second segment of data to be written
 * @param len2 length of second segment of data to be written
 * @param ptr3 generic pointer to third segment of data to be written
 * @param len3 length of third segment of data to be written
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_out3_qnx_msg(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3);
#endif /* DLT_DAEMON_USE_QNX_MESSAGE_IPC */

#endif /* DLT_USER_SHARED_H */
