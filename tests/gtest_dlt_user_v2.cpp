/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, V2 - Volvo Group
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
 * \author
 * Shivam Goel <shivam.goel@volvo.com>
 *
 * \copyright Copyright © 2011-2015 V2 - Volvo Group. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_user_v2.cpp
 */

/*******************************************************************************
**                                                                            **
**  FILE      : gtest_dlt_user_v2.cpp                                       **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Shivam Goel shivam.goel@volvo.com                             **
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
**   sg         Shivam Goel                V2 - Volvo Group                   **
*******************************************************************************/

#include <stdio.h>
#include "gtest/gtest.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include <chrono>

extern "C" {
#include "dlt_user.h"
#include "dlt_user_cfg.h"
}

/* TEST COMMENTED OUT WITH */
/* TODO: */
/* DO FAIL! */


//TBD: Update
/* tested functions */
/*
 * int dlt_user_log_write_start(DltContext *handle, DltContextData *log, DltLogLevelType loglevel);
 * int dlt_user_log_write_start_id(DltContext *handle, DltContextData *log, DltLogLevelType loglevel, uint32_t messageid);
 * int dlt_user_log_write_finish(DltContextData *log);
 * int dlt_user_log_write_bool(DltContextData *log, uint8_t data);
 * int dlt_user_log_write_bool_attr(DltContextData *log, uint8_t data, const char *name);
 * int dlt_user_log_write_float32(DltContextData *log, float32_t data);
 * int dlt_user_log_write_float32_attr(DltContextData *log, float32_t data, const char *name, const char *unit);
 * int dlt_user_log_write_float64(DltContextData *log, double data);
 * int dlt_user_log_write_float64_attr(DltContextData *log, double data, const char *name, const char *unit);
 * int dlt_user_log_write_uint(DltContextData *log, unsigned int data);
 * int dlt_user_log_write_uint_attr(DltContextData *log, unsigned int data, const char *name, const char *unit);
 * int dlt_user_log_write_uint8(DltContextData *log, uint8_t data);
 * int dlt_user_log_write_uint8_attr(DltContextData *log, uint8_t data, const char *name, const char *unit);
 * int dlt_user_log_write_uint16(DltContextData *log, uint16_t data);
 * int dlt_user_log_write_uint16_attr(DltContextData *log, uint16_t data, const char *name, const char *unit);
 * int dlt_user_log_write_uint32(DltContextData *log, uint32_t data);
 * int dlt_user_log_write_uint32_attr(DltContextData *log, uint32_t data, const char *name, const char *unit);
 * int dlt_user_log_write_uint64(DltContextData *log, uint64_t data);
 * int dlt_user_log_write_uint64_attr(DltContextData *log, uint64_t data, const char *name, const char *unit);
 * int dlt_user_log_write_uint8_formatted(DltContextData *log, uint8_t data, DltFormatType type);
 * int dlt_user_log_write_uint16_formatted(DltContextData *log, uint16_t data, DltFormatType type);
 * int dlt_user_log_write_uint32_formatted(DltContextData *log, uint32_t data, DltFormatType type);
 * int dlt_user_log_write_uint64_formatted(DltContextData *log, uint64_t data, DltFormatType type);
 * int dlt_user_log_write_int(DltContextData *log, int data);
 * int dlt_user_log_write_int_attr(DltContextData *log, int data, const char *name, const char *unit);
 * int dlt_user_log_write_int8(DltContextData *log, int8_t data);
 * int dlt_user_log_write_int8_attr(DltContextData *log, int8_t data, const char *name, const char *unit);
 * int dlt_user_log_write_int16(DltContextData *log, int16_t data);
 * int dlt_user_log_write_int16_attr(DltContextData *log, int16_t data, const char *name, const char *unit);
 * int dlt_user_log_write_int32(DltContextData *log, int32_t data);
 * int dlt_user_log_write_int32_attr(DltContextData *log, int32_t data, const char *name, const char *unit);
 * int dlt_user_log_write_int64(DltContextData *log, int64_t data);
 * int dlt_user_log_write_int64_attr(DltContextData *log, int64_t data, const char *name, const char *unit);
 * int dlt_user_log_write_string( DltContextData *log, const char *text);
 * int dlt_user_log_write_string_attr(DltContextData *log, const char *text, const char *name);
 * int dlt_user_log_write_sized_string(DltContextData *log, const char *text, uint16_t length);
 * int dlt_user_log_write_sized_string_attr(DltContextData *log, const char *text, uint16_t length, const char *name);
 * int dlt_user_log_write_constant_string( DltContextData *log, const char *text);
 * int dlt_user_log_write_constant_string_attr(DltContextData *log, const char *text, const char *name);
 * int dlt_user_log_write_sized_constant_string(DltContextData *log, const char *text, uint16_t length);
 * int dlt_user_log_write_sized_constant_string_attr(DltContextData *log, const char *text, uint16_t length, const char *name);
 * int dlt_user_log_write_utf8_string(DltContextData *log, const char *text);
 * int dlt_user_log_write_utf8_string_attr(DltContextData *log, const char *text, const char *name);
 * int dlt_user_log_write_sized_utf8_string(DltContextData *log, const char *text, uint16_t length);
 * int dlt_user_log_write_sized_utf8_string_attr(DltContextData *log, const char *text, uint16_t length, const char *name);
 * int dlt_user_log_write_constant_utf8_string(DltContextData *log, const char *text);
 * int dlt_user_log_write_constant_utf8_string_attr(DltContextData *log, const char *text, const char *name);
 * int dlt_user_log_write_sized_constant_utf8_string(DltContextData *log, const char *text);
 * int dlt_user_log_write_sized_constant_utf8_string_attr(DltContextData *log, const char *text, const char *name);
 * int dlt_user_log_write_raw(DltContextData *log,void *data,uint16_t length);
 * int dlt_user_log_write_raw_attr(DltContextData *log,void *data,uint16_t length, const char *name);
 * int dlt_user_log_write_raw_formatted(DltContextData *log,void *data,uint16_t length,DltFormatType type);
 * int dlt_user_log_write_raw_formatted_attr(DltContextData *log,void *data,uint16_t length,DltFormatType type, const char *name);
 */

/*
 * int dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text);
 * int dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data);
 * int dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data);
 * int dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data);
 * int dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data);
 * int dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length);
 * int dlt_log_marker();
 */

/*
 * int dlt_register_app(const char *apid, const char * description);
 * int dlt_unregister_app(void);
 * int dlt_register_context(DltContext *handle, const char *contextid, const char * description);
 * int dlt_register_context_ll_ts(DltContext *handle, const char *contextid, const char * description, int loglevel, int tracestatus);
 * int dlt_unregister_context(DltContext *handle);
 * int dlt_register_injection_callback(DltContext *handle, uint32_t service_id, int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length));
 * int dlt_register_log_level_changed_callback(DltContext *handle, void (*dlt_log_level_changed_callback)(char context_id[DLT_ID_SIZE],uint8_t log_level, uint8_t trace_status));
 */

/*
 * int dlt_user_trace_network(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);
 * int dlt_user_trace_network_truncated(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload, int allow_truncate);
 * int dlt_user_trace_network_segmented(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);
 */

/*
 * int dlt_set_log_mode(DltUserLogMode mode);
 * int dlt_get_log_state();
 */

/*
 * int dlt_verbose_mode(void);
 * int dlt_nonverbose_mode(void);
 */


/*/////////////////////////////////////// */
/* start initial dlt */
TEST(t_dlt_init, onetime)
{
    /**
     * Unset DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to make sure the dlt user buffer initialized with default value
     */
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/*/////////////////////////////////////// */

/* t_dlt_user_log_write_start */
TEST(t_dlt_user_log_write_start, normal)
{
    DltContext context;
    DltContextData contextData;


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start normal"));

    /* the defined enum values for log level */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_FATAL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_ERROR));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_WARN));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_INFO));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    /* To test the default behaviour and the default log level set to DLT_LOG_INFO */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_OFF));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEBUG));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_VERBOSE));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}


TEST(t_dlt_user_log_write_start_v2, startstartfinish)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start startstartfinish"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* shouldn't it return -1, because it is already started? */
    /* EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_log_write_start_v2, nullpointer)
{
    DltContext context;
    DltContextData contextData;


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start nullpointer"));
    /* NULL's */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start(NULL, &contextData, DLT_LOG_DEFAULT));
    /*EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(&contextData)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start(NULL, NULL, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start(&context, NULL, DLT_LOG_DEFAULT));
    /*EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(&contextData)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_start_id */
TEST(t_dlt_user_log_write_start_id_v2, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id normal"));

    /* the defined enum values for log level */
    messageid = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    /* To test the default behaviour and the default log level set to DLT_LOG_INFO */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));

    messageid = UINT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));
    /* To test the default behaviour and the default log level set to DLT_LOG_INFO */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_log_write_start_id_v2, startstartfinish)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id startstartfinish"));

    messageid = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    /* shouldn't it return -1, because it is already started? */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start_id_v2(&context, &contextData, DLT_LOG_DEFAULT, messageid)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_log_write_start_id_v2, nullpointer)
{
    DltContext context;
    uint32_t messageid;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id_v2 nullpointer"));

    /* NULL's */
    messageid = 0;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start_id(NULL, &contextData, DLT_LOG_DEFAULT, messageid));
    /*EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(&contextData)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start_id(NULL, NULL, DLT_LOG_DEFAULT, messageid));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start_id(&context, NULL, DLT_LOG_DEFAULT, messageid));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_finish_v2 */
TEST(t_dlt_user_log_write_finish_v2, finish)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start finish"));

    /* finish without start */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(NULL)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(&contextData)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_finish_v2 finish"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(&contextData)); */

    /* finish with start and initialized context */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));

    /* 2nd finish */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish_v2(&contextData)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_finish_v2 */
TEST(t_dlt_user_log_write_finish_v2, finish_with_timestamp)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_finish_v2 finish"));

    /* finish with start and initialized context */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    contextData.use_timestamp = DLT_USER_TIMESTAMP;
    contextData.user_timestamp = UINT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_bool */
TEST(t_dlt_user_log_write_bool_v2, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = true;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool(&contextData, data));
    data = false;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_log_write_bool_v2, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: uint8_t data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = 2; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_bool_v2(&contextData, data)); */
    /* TODO: data = 100; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_bool_v2(&contextData, data)); */
    /* TODO: data = UINT8_MAX; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_bool_v2(&contextData, data)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish_v2(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_log_write_bool_v2, nullpointer)
{
    DltContext context;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool nullpointer"));

    /* NULL */
    data = true;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_bool(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_register_context_ll_ts */
TEST(t_dlt_register_context_ll_ts_v2, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_FATAL,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_ERROR,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_WARN,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_INFO,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_DEBUG,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_VERBOSE,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_FATAL,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_ERROR,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_WARN,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_INFO,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_DEBUG,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_VERBOSE,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}


TEST(t_dlt_register_context_ll_ts_v2, abnormal)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts_v2(&context, "", "d", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "T", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts_v2(&context, "", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST1", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST1", "1", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST1234567890", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST1234567890", "1", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */

    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_ON));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));

    /* DLT_LOG_DEFAULT and DLT_TRACE_STATUS_DEFAULT not allowed */
    /* TODO: Why not? */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEFAULT, DLT_TRACE_STATUS_OFF)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_DEFAULT)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */

    /* abnormal values for loglevel and tracestatus */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal", -3,
                                         DLT_TRACE_STATUS_OFF));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal", 100,
                                         DLT_TRACE_STATUS_OFF));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_OFF, -3));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts_v2(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts_v2 normal",
                                         DLT_LOG_OFF, 100));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts_v2(&context, "TEST", NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_register_context_ll_ts_v2, nullpointer)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_register_context_ll_ts_v2(&context, NULL, "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts_v2(&context, NULL, NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts_v2(NULL, "TEST", NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_register_context_ll_ts_v2(NULL, NULL, "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts_v2(NULL, NULL, NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_unregister_context_v2 */
TEST(t_dlt_unregister_context_v2, normal)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));

    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_unregister_context_v2 normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_unregister_context_v2, abnormal)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(&context, "", "d"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "T", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(&context, "", ""));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST1", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST1", "1")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST1234567890", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context_v2(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST1234567890", "1")); */

    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_unregister_context_v2 normal"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_unregister_context_v2 normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_unregister_context_v2 normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_unregister_context_v2 normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_unregister_context_v2 normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_v2(&context, "TEST", NULL)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}


TEST(t_dlt_unregister_context_v2, nullpointer)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(&context, NULL, "dlt_user.c t_dlt_unregister_context_v2 normal"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(&context, NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(NULL, "TEST", NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(NULL, NULL, "dlt_user.c t_dlt_unregister_context_v2 normal"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_v2(NULL, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

#ifdef DLT_NETWORK_TRACE_ENABLE
/*/////////////////////////////////////// */
/* t_dlt_user_trace_network */
TEST(t_dlt_user_trace_network, nullpointer)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_trace_network nullpointer"));

    char header[16];

    for (char i = 0; i < 16; ++i)
        header[(int)i] = i;

    char payload[32];

    for (char i = 0; i < 32; ++i)
        payload[(int)i] = i;

    /* what to expect when giving in NULL pointer? */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network(&context, DLT_NW_TRACE_IPC, 16, NULL, 32, payload));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_trace_network(&context, DLT_NW_TRACE_CAN, 16, header, 32, NULL));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_trace_network(&context, DLT_NW_TRACE_FLEXRAY, 16, NULL, 32, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

/*/////////////////////////////////////// */
/* t_dlt_user_trace_network_truncated_v2 */
TEST(t_dlt_user_trace_network_truncated, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated normal"));

    char header[16];

    for (char i = 0; i < 16; ++i)
        header[(int)i] = i;

    char payload[32];

    for (char i = 0; i < 32; ++i)
        payload[(int)i] = i;

    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_IPC, 16, header, 32, payload, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_CAN, 16, header, 32, payload, 1));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 32, payload, -1));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_MOST, 16, header, 32, payload, 10));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}


TEST(t_dlt_user_trace_network_truncated, abnormal)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated abnormal"));

    /* TODO: char header[16]; */
    /* TODO: for(char i = 0; i < 16; ++i) */
    /* TODO: { */
    /* TODO:     header[(int)i] = i; */
    /* TODO: } */
    /* TODO: char payload[32]; */
    /* TODO: for(char i = 0; i < 32; ++i) */
    /* TODO: { */
    /* TODO:     payload[(int)i] = i; */
    /* TODO: } */

    /* data length = 0. Does this make sense? */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload, 0)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload, 0)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload, 0)); */

    /* invalid DltNetworkTraceType value */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)-100, 16, header, 32, payload, 0)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)-10, 16, header, 32, payload, 0)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)10, 16, header, 32, payload, 0)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)100, 16, header, 32, payload, 0)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}


TEST(t_dlt_user_trace_network_truncated, nullpointer)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated nullpointer"));

    char header[16];

    for (char i = 0; i < 16; ++i)
        header[(int)i] = i;

    char payload[32];

    for (char i = 0; i < 32; ++i)
        payload[(int)i] = i;

    /* what to expect when giving in NULL pointer? */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_IPC, 16, NULL, 32, payload, 0));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER,
              dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_CAN, 16, header, 32, NULL, 0));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER,
              dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_FLEXRAY, 16, NULL, 32, NULL, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_trace_network_segmented, abnormal)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented_v2 abnormal"));

    /* TODO: char header[16]; */
    /* TODO: for(char i = 0; i < 16; ++i) */
    /* TODO: { */
    /* TODO:     header[(int)i] = i; */
    /* TODO: } */
    /* TODO: char payload[32]; */
    /* TODO: for(char i = 0; i < 32; ++i) */
    /* TODO: { */
    /* TODO:     payload[(int)i] = i; */
    /* TODO: } */

    /* data length = 0. Does this make sense? */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload)); */

    /* invalid DltNetworkTraceType value */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, (DltNetworkTraceType)-100, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, (DltNetworkTraceType)-10, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, (DltNetworkTraceType)10, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented_v2(&context, (DltNetworkTraceType)100, 16, header, 32, payload)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}

TEST(t_dlt_user_trace_network_segmented, nullpointer)
{
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented_v2 nullpointer"));

    char header[16];

    for (char i = 0; i < 16; ++i)
        header[(int)i] = i;

    char payload[32];

    for (char i = 0; i < 32; ++i)
        payload[(int)i] = i;

    /* what to expect when giving in NULL pointer? */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_IPC, 16, NULL, 32, payload));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER,
              dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_CAN, 16, header, 32, NULL));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER,
              dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_FLEXRAY, 16, NULL, 32, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}
#endif /* DLT_NETWORK_TRACE_ENABLE */

/*/////////////////////////////////////// */
/* dlt_user_is_logLevel_enabled */
TEST(t_dlt_user_is_logLevel_enabled, normal)
{
    DltContext context;
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_ll_ts_v2(&context, "ILLE",
                                                        "t_dlt_user_is_logLevel_enabled context",
                                                        DLT_LOG_INFO,
                                                        -2)); /* DLT_USER_TRACE_STATUS_NOT_SET */

    EXPECT_LE(DLT_RETURN_TRUE, dlt_user_is_logLevel_enabled(&context, DLT_LOG_FATAL));
    EXPECT_LE(DLT_RETURN_TRUE, dlt_user_is_logLevel_enabled(&context, DLT_LOG_ERROR));
    EXPECT_LE(DLT_RETURN_TRUE, dlt_user_is_logLevel_enabled(&context, DLT_LOG_WARN));
    EXPECT_LE(DLT_RETURN_TRUE, dlt_user_is_logLevel_enabled(&context, DLT_LOG_INFO));
    EXPECT_LE(DLT_RETURN_LOGGING_DISABLED, dlt_user_is_logLevel_enabled(&context, DLT_LOG_DEBUG));
    EXPECT_LE(DLT_RETURN_LOGGING_DISABLED, dlt_user_is_logLevel_enabled(&context, DLT_LOG_VERBOSE));
    EXPECT_LE(DLT_RETURN_LOGGING_DISABLED, dlt_user_is_logLevel_enabled(&context, DLT_LOG_OFF));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context_v2(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app_v2());
}


#ifdef DLT_TRACE_LOAD_CTRL_ENABLE

/*/////////////////////////////////////// */
/* t_dlt_user_run_into_trace_limit */
TEST(t_dlt_user_run_into_trace_limit, normal)
{
    DltContext context;
    DltContextData contextData;
    unsigned int data;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app_v2("TLMT", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_v2(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint normal"));

    auto loadExceededReceived = false;

    for (int i = 0; i < DLT_TRACE_LOAD_CLIENT_HARD_LIMIT_DEFAULT;++i) {
        /* normal values */
        EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
        data = 0;
        EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));
        data = 1;
        EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));
        data = 2;
        EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));
        data = UINT_MAX;
        EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));

        loadExceededReceived = dlt_user_log_write_finish_v2(&contextData) == DLT_RETURN_LOAD_EXCEEDED;
        if (loadExceededReceived) {
            // values are averaged over a minute, therefore a spike in load also triggers the limit
            EXPECT_LE(i, DLT_TRACE_LOAD_CLIENT_HARD_LIMIT_DEFAULT);
            break;
        }
    }

    EXPECT_TRUE(loadExceededReceived);

    // unregister return values are not checked because error is returned due to full local buffers
    dlt_unregister_context_v2(&context);
    dlt_unregister_app_v2();
}

#endif

/*/////////////////////////////////////// */
/* main */
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    /* Reduce atexit resend wait during unit tests to avoid long teardown delays */
    dlt_set_resend_timeout_atexit(0);
    return RUN_ALL_TESTS();
}
