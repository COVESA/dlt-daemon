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
 * \file dlt_common_api.h
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_commpn_api.h                                              **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
*******************************************************************************/

#ifndef DLT_COMMON_API_H
#define DLT_COMMON_API_H

#include "dlt.h"

/**
 * Create an object for a new context.
 * Common API with DLT Embedded
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 */
/* #define DLT_DECLARE_CONTEXT(CONTEXT) */
/* UNCHANGED */

/**
 * Use an object of a new context created in another module.
 * Common API with DLT Embedded
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 */
/* #define DLT_IMPORT_CONTEXT(CONTEXT) */
/* UNCHANGED */

/**
 * Register application.
 * Common API with DLT Embedded
 * @param APPID application id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
/* #define DLT_REGISTER_APP(APPID,DESCRIPTION) */
/* UNCHANGED */

/**
 * Register context including application (with default log level and default trace status)
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 * @param CONTEXTID context id with maximal four characters
 * @param APPID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_CONTEXT_APP(CONTEXT, CONTEXTID, APPID, DESCRIPTION) \
    DLT_REGISTER_CONTEXT(CONTEXT, CONTEXTID, DESCRIPTION)

/**
 * Send log message with variable list of messages (intended for verbose mode)
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param ARGS variable list of arguments
 */
/*****************************************/
#define DLT_LOG0(CONTEXT, LOGLEVEL) \
    DLT_LOG(CONTEXT, LOGLEVEL)
/*****************************************/
#define DLT_LOG1(CONTEXT, LOGLEVEL, ARGS1) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1)
/*****************************************/
#define DLT_LOG2(CONTEXT, LOGLEVEL, ARGS1, ARGS2) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2)
/*****************************************/
#define DLT_LOG3(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3)
/*****************************************/
#define DLT_LOG4(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4)
/*****************************************/
#define DLT_LOG5(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5)
/*****************************************/
#define DLT_LOG6(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6)
/*****************************************/
#define DLT_LOG7(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7)
/*****************************************/
#define DLT_LOG8(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8)
/*****************************************/
#define DLT_LOG9(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9)
/*****************************************/
#define DLT_LOG10(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10)
/*****************************************/
#define DLT_LOG11(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10, ARGS11) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10, ARGS11)
/*****************************************/
#define DLT_LOG12(CONTEXT, \
                  LOGLEVEL, \
                  ARGS1, \
                  ARGS2, \
                  ARGS3, \
                  ARGS4, \
                  ARGS5, \
                  ARGS6, \
                  ARGS7, \
                  ARGS8, \
                  ARGS9, \
                  ARGS10, \
                  ARGS11, \
                  ARGS12) \
    DLT_LOG(CONTEXT, LOGLEVEL, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10, ARGS11, ARGS12)
/*****************************************/
#define DLT_LOG13(CONTEXT, \
                  LOGLEVEL, \
                  ARGS1, \
                  ARGS2, \
                  ARGS3, \
                  ARGS4, \
                  ARGS5, \
                  ARGS6, \
                  ARGS7, \
                  ARGS8, \
                  ARGS9, \
                  ARGS10, \
                  ARGS11, \
                  ARGS12, \
                  ARGS13) \
    DLT_LOG(CONTEXT, \
            LOGLEVEL, \
            ARGS1, \
            ARGS2, \
            ARGS3, \
            ARGS4, \
            ARGS5, \
            ARGS6, \
            ARGS7, \
            ARGS8, \
            ARGS9, \
            ARGS10, \
            ARGS11, \
            ARGS12, \
            ARGS13)
/*****************************************/
#define DLT_LOG14(CONTEXT, \
                  LOGLEVEL, \
                  ARGS1, \
                  ARGS2, \
                  ARGS3, \
                  ARGS4, \
                  ARGS5, \
                  ARGS6, \
                  ARGS7, \
                  ARGS8, \
                  ARGS9, \
                  ARGS10, \
                  ARGS11, \
                  ARGS12, \
                  ARGS13, \
                  ARGS14) \
    DLT_LOG(CONTEXT, \
            LOGLEVEL, \
            ARGS1, \
            ARGS2, \
            ARGS3, \
            ARGS4, \
            ARGS5, \
            ARGS6, \
            ARGS7, \
            ARGS8, \
            ARGS9, \
            ARGS10, \
            ARGS11, \
            ARGS12, \
            ARGS13, \
            ARGS14)
/*****************************************/
#define DLT_LOG15(CONTEXT, \
                  LOGLEVEL, \
                  ARGS1, \
                  ARGS2, \
                  ARGS3, \
                  ARGS4, \
                  ARGS5, \
                  ARGS6, \
                  ARGS7, \
                  ARGS8, \
                  ARGS9, \
                  ARGS10, \
                  ARGS11, \
                  ARGS12, \
                  ARGS13, \
                  ARGS14, \
                  ARGS15) \
    DLT_LOG(CONTEXT, \
            LOGLEVEL, \
            ARGS1, \
            ARGS2, \
            ARGS3, \
            ARGS4, \
            ARGS5, \
            ARGS6, \
            ARGS7, \
            ARGS8, \
            ARGS9, \
            ARGS10, \
            ARGS11, \
            ARGS12, \
            ARGS13, \
            ARGS14, \
            ARGS15)
/*****************************************/
#define DLT_LOG16(CONTEXT, \
                  LOGLEVEL, \
                  ARGS1, \
                  ARGS2, \
                  ARGS3, \
                  ARGS4, \
                  ARGS5, \
                  ARGS6, \
                  ARGS7, \
                  ARGS8, \
                  ARGS9, \
                  ARGS10, \
                  ARGS11, \
                  ARGS12, \
                  ARGS13, \
                  ARGS14, \
                  ARGS15, \
                  ARGS16) \
    DLT_LOG(CONTEXT, \
            LOGLEVEL, \
            ARGS1, \
            ARGS2, \
            ARGS3, \
            ARGS4, \
            ARGS5, \
            ARGS6, \
            ARGS7, \
            ARGS8, \
            ARGS9, \
            ARGS10, \
            ARGS11, \
            ARGS12, \
            ARGS13, \
            ARGS14, \
            ARGS15, \
            ARGS16)

/**
 * Send log message with variable list of messages (intended for non-verbose mode)
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param MSGID the message id of log message
 * @param ARGS variable list of arguments:
 * calls to DLT_STRING(), DLT_BOOL(), DLT_FLOAT32(), DLT_FLOAT64(),
 * DLT_INT(), DLT_UINT(), DLT_RAW()
 */
/*****************************************/
#define DLT_LOG_ID0(CONTEXT, LOGLEVEL, MSGID) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID)
/*****************************************/
#define DLT_LOG_ID1(CONTEXT, LOGLEVEL, MSGID, ARGS1) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1)
/*****************************************/
#define DLT_LOG_ID2(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2)
/*****************************************/
#define DLT_LOG_ID3(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3)
/*****************************************/
#define DLT_LOG_ID4(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4)
/*****************************************/
#define DLT_LOG_ID5(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5)
/*****************************************/
#define DLT_LOG_ID6(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6)
/*****************************************/
#define DLT_LOG_ID7(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7)
/*****************************************/
#define DLT_LOG_ID8(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8)
/*****************************************/
#define DLT_LOG_ID9(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9)
/*****************************************/
#define DLT_LOG_ID10(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10)
/*****************************************/
#define DLT_LOG_ID11(CONTEXT, \
                     LOGLEVEL, \
                     MSGID, \
                     ARGS1, \
                     ARGS2, \
                     ARGS3, \
                     ARGS4, \
                     ARGS5, \
                     ARGS6, \
                     ARGS7, \
                     ARGS8, \
                     ARGS9, \
                     ARGS10, \
                     ARGS11) \
    DLT_LOG_ID(CONTEXT, LOGLEVEL, MSGID, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7, ARGS8, ARGS9, ARGS10, ARGS11)
/*****************************************/
#define DLT_LOG_ID12(CONTEXT, \
                     LOGLEVEL, \
                     MSGID, \
                     ARGS1, \
                     ARGS2, \
                     ARGS3, \
                     ARGS4, \
                     ARGS5, \
                     ARGS6, \
                     ARGS7, \
                     ARGS8, \
                     ARGS9, \
                     ARGS10, \
                     ARGS11, \
                     ARGS12) \
    DLT_LOG_ID(CONTEXT, \
               LOGLEVEL, \
               MSGID, \
               ARGS1, \
               ARGS2, \
               ARGS3, \
               ARGS4, \
               ARGS5, \
               ARGS6, \
               ARGS7, \
               ARGS8, \
               ARGS9, \
               ARGS10, \
               ARGS11, \
               ARGS12)
/*****************************************/
#define DLT_LOG_ID13(CONTEXT, \
                     LOGLEVEL, \
                     MSGID, \
                     ARGS1, \
                     ARGS2, \
                     ARGS3, \
                     ARGS4, \
                     ARGS5, \
                     ARGS6, \
                     ARGS7, \
                     ARGS8, \
                     ARGS9, \
                     ARGS10, \
                     ARGS11, \
                     ARGS12, \
                     ARGS13) \
    DLT_LOG_ID(CONTEXT, \
               LOGLEVEL, \
               MSGID, \
               ARGS1, \
               ARGS2, \
               ARGS3, \
               ARGS4, \
               ARGS5, \
               ARGS6, \
               ARGS7, \
               ARGS8, \
               ARGS9, \
               ARGS10, \
               ARGS11, \
               ARGS12, \
               ARGS13)
/*****************************************/
#define DLT_LOG_ID14(CONTEXT, \
                     LOGLEVEL, \
                     MSGID, \
                     ARGS1, \
                     ARGS2, \
                     ARGS3, \
                     ARGS4, \
                     ARGS5, \
                     ARGS6, \
                     ARGS7, \
                     ARGS8, \
                     ARGS9, \
                     ARGS10, \
                     ARGS11, \
                     ARGS12, \
                     ARGS13, \
                     ARGS14) \
    DLT_LOG_ID(CONTEXT, \
               LOGLEVEL, \
               MSGID, \
               ARGS1, \
               ARGS2, \
               ARGS3, \
               ARGS4, \
               ARGS5, \
               ARGS6, \
               ARGS7, \
               ARGS8, \
               ARGS9, \
               ARGS10, \
               ARGS11, \
               ARGS12, \
               ARGS13, \
               ARGS14)
/*****************************************/
#define DLT_LOG_ID15(CONTEXT, \
                     LOGLEVEL, \
                     MSGID, \
                     ARGS1, \
                     ARGS2, \
                     ARGS3, \
                     ARGS4, \
                     ARGS5, \
                     ARGS6, \
                     ARGS7, \
                     ARGS8, \
                     ARGS9, \
                     ARGS10, \
                     ARGS11, \
                     ARGS12, \
                     ARGS13, \
                     ARGS14, \
                     ARGS15) \
    DLT_LOG_ID(CONTEXT, \
               LOGLEVEL, \
               MSGID, \
               ARGS1, \
               ARGS2, \
               ARGS3, \
               ARGS4, \
               ARGS5, \
               ARGS6, \
               ARGS7, \
               ARGS8, \
               ARGS9, \
               ARGS10, \
               ARGS11, \
               ARGS12, \
               ARGS13, \
               ARGS14, \
               ARGS15)
/*****************************************/
#define DLT_LOG_ID16(CONTEXT, \
                     LOGLEVEL, \
                     MSGID, \
                     ARGS1, \
                     ARGS2, \
                     ARGS3, \
                     ARGS4, \
                     ARGS5, \
                     ARGS6, \
                     ARGS7, \
                     ARGS8, \
                     ARGS9, \
                     ARGS10, \
                     ARGS11, \
                     ARGS12, \
                     ARGS13, \
                     ARGS14, \
                     ARGS15, \
                     ARGS16) \
    DLT_LOG_ID(CONTEXT, \
               LOGLEVEL, \
               MSGID, \
               ARGS1, \
               ARGS2, \
               ARGS3, \
               ARGS4, \
               ARGS5, \
               ARGS6, \
               ARGS7, \
               ARGS8, \
               ARGS9, \
               ARGS10, \
               ARGS11, \
               ARGS12, \
               ARGS13, \
               ARGS14, \
               ARGS15, \
               ARGS16)

/**
 * Unregister context.
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 */
/* #define DLT_UNREGISTER_CONTEXT(CONTEXT) */
/* UNCHANGED */

/**
 * Unregister application.
 * Common API with DLT Embedded
 */
/* #define DLT_UNREGISTER_APP() */
/* UNCHANGED */

/**
 * Add string parameter to the log messsage.
 * Common API with DLT Embedded
 * In the future in none verbose mode the string will not be sent via DLT message.
 * @param TEXT ASCII string
 */
/* #define DLT_CSTRING(TEXT) */
/* UNCHANGED */

#endif /* DLT_COMMON_API_H */

