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
 * \author Jens Bocklage <jens_bocklage@mentor.com>
 * \author Stefan Held <stefan_held@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_common.cpp
 */

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

static const char *STR_TRUNCATED_MESSAGE = "... <<Message truncated, too long>>";

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


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start normal"));

    /* the defined enum values for log level */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_FATAL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_ERROR));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_WARN));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_INFO));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    /* To test the default behaviour and the default log level set to DLT_LOG_INFO */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_OFF));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEBUG));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_VERBOSE));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_start, abnormal)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start abnormal"));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_start(&context, &contextData, (DltLogLevelType) - 100));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_start(&context, &contextData, (DltLogLevelType) - 10));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)10));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)100));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_start, startstartfinish)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start startstartfinish"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* shouldn't it return -1, because it is already started? */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_start, nullpointer)
{
    DltContext context;
    DltContextData contextData;


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start nullpointer"));
    /* NULL's */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start(NULL, &contextData, DLT_LOG_DEFAULT));
    /*EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(&contextData)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start(NULL, NULL, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start(&context, NULL, DLT_LOG_DEFAULT));
    /*EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(&contextData)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_start_id */
TEST(t_dlt_user_log_write_start_id, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id normal"));

    /* the defined enum values for log level */
    messageid = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    /* To test the default behaviour and the default log level set to DLT_LOG_INFO */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));

    messageid = UINT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    /* To test the default behaviour and the default log level set to DLT_LOG_INFO */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_start_id, abnormal)
{
    DltContext context;
    /* TODO: DltContextData contextData; */
    /* TODO: uint32_t messageid; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id abnormal"));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    /* TODO: messageid = 0; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)-100, messageid)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)-10, messageid)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)10, messageid)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)100, messageid)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_start_id, startstartfinish)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id startstartfinish"));

    messageid = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    /* shouldn't it return -1, because it is already started? */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_start_id, nullpointer)
{
    DltContext context;
    uint32_t messageid;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id nullpointer"));

    /* NULL's */
    messageid = 0;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start_id(NULL, &contextData, DLT_LOG_DEFAULT, messageid));
    /*EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(&contextData)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start_id(NULL, NULL, DLT_LOG_DEFAULT, messageid));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_start_id(&context, NULL, DLT_LOG_DEFAULT, messageid));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_finish */
TEST(t_dlt_user_log_write_finish, finish)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start finish"));

    /* finish without start */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(NULL)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(&contextData)); */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_finish finish"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(&contextData)); */

    /* finish with start and initialized context */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    /* 2nd finish */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_finish(&contextData)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_finish */
TEST(t_dlt_user_log_write_finish, finish_with_timestamp)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_finish finish"));

    /* finish with start and initialized context */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    contextData.use_timestamp = DLT_USER_TIMESTAMP;
    contextData.user_timestamp = UINT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_bool */
TEST(t_dlt_user_log_write_bool, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = true;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool(&contextData, data));
    data = false;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_bool, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: uint8_t data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = 2; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_bool(&contextData, data)); */
    /* TODO: data = 100; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_bool(&contextData, data)); */
    /* TODO: data = UINT8_MAX; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_bool(&contextData, data)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_bool, nullpointer)
{
    DltContext context;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool nullpointer"));

    /* NULL */
    data = true;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_bool(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_bool_attr */
TEST(t_dlt_user_log_write_bool_attr, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    data = true;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool_attr(&contextData, data, "state"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool_attr(&contextData, data, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_bool_attr(&contextData, data, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_float32 */
TEST(t_dlt_user_log_write_float32, normal)
{
    DltContext context;
    DltContextData contextData;
    float32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 3.141592653589793238;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32(&contextData, data));
    data = -3.141592653589793238;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32(&contextData, data));
    data = 0.;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32(&contextData, data));
    data = -0.;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32(&contextData, data));
    data = FLT_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32(&contextData, data));
    data = FLT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_float32, nullpointer)
{
    DltContext context;
    float32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32 nullpointer"));

    /* NULL */
    data = 1.;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_float32(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_float32_attr */
TEST(t_dlt_user_log_write_float32_attr, normal)
{
    DltContext context;
    DltContextData contextData;
    float32_t data;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    data = 3.141592653589793238f;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float32_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_float64 */
TEST(t_dlt_user_log_write_float64, normal)
{
    DltContext context;
    DltContextData contextData;
    double data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 3.14159265358979323846;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64(&contextData, data));
    data = -3.14159265358979323846;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64(&contextData, data));
    data = 0.;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64(&contextData, data));
    data = -0.;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64(&contextData, data));
    data = DBL_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64(&contextData, data));
    data = DBL_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_float64, nullpointer)
{
    DltContext context;
    double data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64 nullpointer"));

    /* NULL */
    data = 1.;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_float64(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_float64_attr */
TEST(t_dlt_user_log_write_float64_attr, normal)
{
    DltContext context;
    DltContextData contextData;
    double data;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    data = 3.14159265358979323846;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_float64_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint */
TEST(t_dlt_user_log_write_uint, normal)
{
    DltContext context;
    DltContextData contextData;
    unsigned int data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));
    data = UINT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: unsigned int data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = -1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint(&contextData, data)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint, nullpointer)
{
    DltContext context;
    unsigned int data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint_attr */
TEST(t_dlt_user_log_write_uint_attr, normal)
{
    DltContext context;
    DltContextData contextData;
    unsigned int data;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    data = 42;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint8 */
TEST(t_dlt_user_log_write_uint8, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8(&contextData, data));
    data = UINT8_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint8, nullpointer)
{
    DltContext context;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint8_attr */
TEST(t_dlt_user_log_write_uint8_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    uint8_t data = 0xaa;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint16 */
TEST(t_dlt_user_log_write_uint16, normal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16(&contextData, data));
    data = UINT16_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint16, nullpointer)
{
    DltContext context;
    uint16_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint16_attr */
TEST(t_dlt_user_log_write_uint16_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    uint16_t data = 0xaa55;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint32 */
TEST(t_dlt_user_log_write_uint32, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32(&contextData, data));
    data = UINT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint32, nullpointer)
{
    DltContext context;
    uint32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint32_attr */
TEST(t_dlt_user_log_write_uint32_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    uint32_t data = 0xaabbccdd;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint64 */
TEST(t_dlt_user_log_write_uint64, normal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64(&contextData, data));
    data = UINT64_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint64, nullpointer)
{
    DltContext context;
    uint64_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint64_attr */
TEST(t_dlt_user_log_write_uint64_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    uint64_t data = 0x11223344aabbccddULL;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint8_formatted */
TEST(t_dlt_user_log_write_uint8_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT8_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint8_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: uint8_t data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)-100)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)-10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)100)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint8_formatted, nullpointer)
{
    DltContext context;
    uint8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_BIN16));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint16_formatted */
TEST(t_dlt_user_log_write_uint16_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT16_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint16_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: uint16_t data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)-100)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)-10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)100)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint16_formatted, nullpointer)
{
    DltContext context;
    uint16_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_BIN16));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint32_formatted */
TEST(t_dlt_user_log_write_uint32_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint32_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: uint32_t data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)-100)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)-10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)100)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint32_formatted, nullpointer)
{
    DltContext context;
    uint32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_BIN16));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_uint64_formatted */
TEST(t_dlt_user_log_write_uint64_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT64_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint64_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    /* TODO: uint64_t data; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted abnormal"));

    /* abnormal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    /* TODO: data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)-100)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)-10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)100)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_uint64_formatted, nullpointer)
{
    DltContext context;
    uint64_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_BIN16));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int */
TEST(t_dlt_user_log_write_int, normal)
{
    DltContext context;
    DltContextData contextData;
    int data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int(&contextData, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int(&contextData, data));
    data = INT_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int(&contextData, data));
    data = INT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_int, nullpointer)
{
    DltContext context;
    int data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_int(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int_attr */
TEST(t_dlt_user_log_write_int_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    int data = -42;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int8 */
TEST(t_dlt_user_log_write_int8, normal)
{
    DltContext context;
    DltContextData contextData;
    int8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8(&contextData, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8(&contextData, data));
    data = INT8_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8(&contextData, data));
    data = INT8_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_int8, nullpointer)
{
    DltContext context;
    int8_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_int8(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int8_attr */
TEST(t_dlt_user_log_write_int8_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    int8_t data = 0xaa;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int8_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int16 */
TEST(t_dlt_user_log_write_int16, normal)
{
    DltContext context;
    DltContextData contextData;
    int16_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16(&contextData, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16(&contextData, data));
    data = INT16_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16(&contextData, data));
    data = INT16_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_int16, nullpointer)
{
    DltContext context;
    int16_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_int16(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int16_attr */
TEST(t_dlt_user_log_write_int16_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    int16_t data = 0xaa55;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int16_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int32 */
TEST(t_dlt_user_log_write_int32, normal)
{
    DltContext context;
    DltContextData contextData;
    int32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32(&contextData, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32(&contextData, data));
    data = INT32_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32(&contextData, data));
    data = INT32_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_int32, nullpointer)
{
    DltContext context;
    int32_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_int32(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int32_attr */
TEST(t_dlt_user_log_write_int32_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    int32_t data = 0xffeeddcc;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int32_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int64 */
TEST(t_dlt_user_log_write_int64, normal)
{
    DltContext context;
    DltContextData contextData;
    int64_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64 normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64(&contextData, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64(&contextData, data));
    data = 1;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64(&contextData, data));
    data = INT64_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64(&contextData, data));
    data = INT64_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64(&contextData, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_int64, nullpointer)
{
    DltContext context;
    int64_t data;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64 nullpointer"));

    /* NULL */
    data = 1;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_int64(NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_int64_attr */
TEST(t_dlt_user_log_write_int64_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    int64_t data = 0xffeeddcc44332211LL;
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, "name", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, "", "unit"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, "name", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, "", ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, NULL, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, "", NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_int64_attr(&contextData, data, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_string */
TEST(t_dlt_user_log_write_string, normal)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_string(&contextData, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  Send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated and appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_string, normal_dlt_log_msg_truncated_because_exceed_the_buffer_length_in_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    char *message = NULL;
    char *expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal_dlt_log_msg_truncated_because_exceed_the_buffer_length_in_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message exceed buffer length 10 bytes */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  In Non-Verbose mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated and appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_string, normal_dlt_log_msg_truncated_because_exceed_the_buffer_length_in_non_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    char *message = NULL;
    char *expected_message = NULL;

    dlt_nonverbose_mode();

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal_dlt_log_msg_truncated_because_exceed_the_buffer_length_in_non_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message exceed buffer length 10 bytes */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Non-Verbose Mode:
     * package_description_size = Message ID (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore verbose mode */
    dlt_verbose_mode();
}

/**
 *  Set the DLT_USER_ENV_LOG_MSG_BUF_LEN to 46, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated and appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other test cases
 */
TEST(t_dlt_user_log_write_string, normal_message_truncated_because_exceed_buffer_length_and_reduce_msg_buf_len_by_env_variable)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t user_message_after_truncated_size = 0;
    const char *message = "$$$$###############################################";
    char *expected_message = NULL;

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size only available 4 bytes for store user message.
     * Note: 46 bytes = package_description_size (6 bytes) + str_truncated_message_length (35 bytes) + 4 bytes user message + 1 byte NULL terminator.
     */
    user_message_after_truncated_size = 4;
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "46", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal_message_truncated_because_exceed_buffer_length_and_reduce_msg_buf_len_by_env_variable"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    expected_message_length = user_message_after_truncated_size + str_truncate_message_length;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '$';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));
    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/**
 *  Set DLT_USER_ENV_LOG_MSG_BUF_LEN to 35 bytes and send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_string() will be returned DLT_RETURN_USER_BUFFER_FULL because the DLT_USER_ENV_LOG_MSG_BUF_LEN
 *               does not have enough space to store truncate message STR_TRUNCATED_MESSAGE
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other test cases
 */
TEST(t_dlt_user_log_write_string, normal_DLT_USER_ENV_LOG_MSG_BUF_LEN_does_not_enough_space_for_truncated_message)
{
    DltContext context;
    DltContextData contextData;
    uint16_t package_description_size = 0;
    const char *message = "################################################################################";

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size not enough minimum space to store data even the truncate notice message.
     * Note: The minimum buffer to store the truncate notice message is 42 bytes.
     */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "35", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal_DLT_USER_ENV_LOG_MSG_BUF_LEN_does_not_enough_space_for_truncated_message"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    ASSERT_STREQ("", (char *)(contextData.buffer + package_description_size));

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/**
 *  Set DLT_USER_ENV_LOG_MSG_BUF_LEN to 42 bytes and send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               receive message will be STR_TRUNCATED_MESSAGE
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other test cases
 */
TEST(t_dlt_user_log_write_string, normal_DLT_USER_ENV_LOG_MSG_BUF_LEN_fix_truncate_message)
{
    DltContext context;
    DltContextData contextData;
    uint16_t package_description_size = 0;
    const char *message = "################################################################################";

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size just fixed to truncate message STR_TRUNCATED_MESSAGE
     * Note: 42 bytes = package_description_size (6 bytes) + str_truncated_message_length (35 bytes) + 1 byte NULL terminator.
     */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "42", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c normal_DLT_USER_ENV_LOG_MSG_BUF_LEN_fix_truncate_message"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    ASSERT_STREQ(STR_TRUNCATED_MESSAGE, (char *)(contextData.buffer + package_description_size));

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

TEST(t_dlt_user_log_write_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string nullpointer"));

    /* NULL */
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_string(NULL, text1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_string(NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_string(&contextData, NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_string */

TEST(t_dlt_user_log_write_sized_string, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_string normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char text1[] = "TheQuickBrownFox";
    const char *arg1_start = strchr(text1, 'Q');
    const size_t arg1_len = 5;
    /* from the above string, send only the substring "Quick" */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_string(&contextData, arg1_start, arg1_len));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_constant_string */
TEST(t_dlt_user_log_write_constant_string, normal)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string(&contextData, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  Send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_constant_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated and appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_constant_string, normal_too_long_message_is_truncated_and_appended_notice_message_in_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    char *message = NULL;
    char *expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string normal_too_long_message_is_truncated_and_appended_notice_message_in_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the message exceed DLT_USER_ENV_LOG_MSG_BUF_LEN 10 bytes */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_constant_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 * In Non-Verbose Mode
 * Expectation: dlt_user_log_write_constant_string() will not package and send message. Return DLT_RETURN_OK
 */
TEST(t_dlt_user_log_write_constant_string, normal_do_nothing_in_non_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    const char *message = "message";

    dlt_nonverbose_mode();

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string normal_do_nothing_in_non_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_constant_string(&contextData, message));
    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore verbose mode */
    dlt_verbose_mode();
}

TEST(t_dlt_user_log_write_constant_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string nullpointer"));

    /* NULL */
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_constant_string(NULL, text1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_constant_string(NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_constant_string(&contextData, NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_constant_string */

TEST(t_dlt_user_log_write_sized_constant_string, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_string normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char text1[] = "TheQuickBrownFox";
    const char *arg1_start = strchr(text1, 'Q');
    const size_t arg1_len = 5;
    /* from the above string, send only the substring "Quick" */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_string(&contextData, arg1_start, arg1_len));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_utf8_string */
TEST(t_dlt_user_log_write_utf8_string, normal)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_utf8_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_utf8_string(&contextData, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  Send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at one byte utf-8 and appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_1byte_in_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    char *message = NULL;
    char *expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_1byte_in_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message exceed buffer length 10 bytes which have '$' character (utf-8 1 byte) right before truncate position */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /* Fill '$' before truncate position */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - 1);
    message[index] = '$';

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length;

    for (index = 0; index < (user_message_after_truncated_size - 1); index++)
    {
        expected_message[index] = '#';
    }
    expected_message[user_message_after_truncated_size - 1] = '$';
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at one byte utf-8 and appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_1byte_in_non_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    char *message = NULL;
    char *expected_message = NULL;

    dlt_nonverbose_mode();

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_1byte_in_non_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message exceed buffer length 10 bytes which have '$' character (utf-8 1 byte) right before truncate position */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /**
     * In Non-Verbose Mode:
     * package_description_size = Message ID (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /* Fill '$' before truncate position */
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - 1);
    message[index] = '$';

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length;

    for (index = 0; index < (user_message_after_truncated_size - 1); index++)
    {
        expected_message[index] = '#';
    }
    expected_message[user_message_after_truncated_size - 1] = '$';
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore verbose mode */
    dlt_verbose_mode();
}

/**
 *  Set the DLT_USER_ENV_LOG_MSG_BUF_LEN to 46, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the whole utf-8 1 bytes and appended
 *               STR_TRUNCATED_MESSAGE at the end of received message.
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other testcases
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_1bytes_and_reduce_msg_buf_len_by_env_variable)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t expected_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t user_message_after_truncated_size = 0;
    const char *message = "$$$$###############################################";
    char *expected_message = NULL;

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size only available 4 bytes for store user message.
     * Note: 46 bytes = package_description_size (6 bytes) + str_truncated_message_length (35 bytes) + 4 bytes user message + 1 byte NULL terminator.
     */
    user_message_after_truncated_size = 4;
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "46", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_1bytes_and_reduce_msg_buf_len_by_env_variable"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    expected_message_length = user_message_after_truncated_size + str_truncate_message_length;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '$';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));
    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 2 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_2bytes_in_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *utf8_2byte_character = "¢";
    char *message = NULL;
    char *expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_2bytes_in_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message contain the '¢' (2 bytes utf-8 character) and last byte of this character is exceed buffer length */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /**
     * Fill the "¢" character at the position which the last byte of this character is exceed the buffer length and
     * expectation is it will be truncated 1 more bytes in the character sequence
     */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    remaining_byte_truncated_utf8_character = 1;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - remaining_byte_truncated_utf8_character);
    strncpy(message + index, utf8_2byte_character, strlen(utf8_2byte_character));

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length - remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 2 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_2bytes_in_non_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *utf8_2byte_character = "¢";
    char *message = NULL;
    char *expected_message = NULL;

    dlt_nonverbose_mode();

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_2bytes_in_non_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message contain the '¢' (2 bytes utf-8 character) and last byte of this character is exceed buffer length */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Non-Verbose Mode:
     * package_description_size = Message ID (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /**
     * Fill the "¢" character at the position which the last byte of this character is exceed the buffer length and
     * expectation is it will be truncated 1 more bytes in the character sequence
     */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    remaining_byte_truncated_utf8_character = 1;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - remaining_byte_truncated_utf8_character);
    strncpy(message + index, utf8_2byte_character, strlen(utf8_2byte_character));

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(DLT_USER_BUF_MAX_SIZE));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length - remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore verbose mode */
    dlt_verbose_mode();
}

/**
 *  Set the DLT_USER_ENV_LOG_MSG_BUF_LEN to 46, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 2 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other testcases
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_2bytes_and_reduce_msg_buf_len_by_env_variable)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *message = "$$$¢###############################################";
    char *expected_message = NULL;

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size only available 4 bytes for store user message.
     * Note: 46 bytes = package_description_size (6 bytes) + str_truncated_message_length (35 bytes) + 4 bytes user message + 1 byte NULL terminator.
     */
    user_message_after_truncated_size = 4;
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "46", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_2bytes_and_reduce_msg_buf_len_by_env_variable"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    remaining_byte_truncated_utf8_character = 1;
    expected_message_length = user_message_after_truncated_size - remaining_byte_truncated_utf8_character + str_truncate_message_length;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size -= remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '$';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));
    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 3 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_3bytes_in_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *utf8_3byte_character = "€";
    char *message = NULL;
    char *expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_3bytes_in_verbose_mode"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message contain the '€' (3 bytes utf-8 character) and last byte of this character is exceed buffer length */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /**
     * Fill the "€" character at the position which the last byte of this character is exceed the buffer length and
     * expectation is it will be truncated 2 more bytes in the character sequence
     */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    remaining_byte_truncated_utf8_character = 2;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - remaining_byte_truncated_utf8_character);
    strncpy(message + index, utf8_3byte_character, strlen(utf8_3byte_character));

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length - remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 3 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_3bytes_in_non_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *utf8_3byte_character = "€";
    char *message = NULL;
    char *expected_message = NULL;

    dlt_nonverbose_mode();

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_3bytes_in_non_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message contain the '€' (3 bytes utf-8 character) and last byte of this character is exceed buffer length */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /**
     * Fill the "€" character at the position which the last byte of this character is exceed the buffer length and
     * expectation is it will be truncated 2 more bytes in the character sequence
     */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    remaining_byte_truncated_utf8_character = 2;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - remaining_byte_truncated_utf8_character);
    strncpy(message + index, utf8_3byte_character, strlen(utf8_3byte_character));

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length - remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore verbose mode */
    dlt_verbose_mode();
}

/**
 *  Set the DLT_USER_ENV_LOG_MSG_BUF_LEN to 46, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 3 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other testcases
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_3bytes_and_reduce_msg_buf_len_by_env_variable)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *message = "$$€###############################################";
    char *expected_message = NULL;

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size only available 4 bytes for store user message.
     * Note: 46 bytes = package_description_size (6 bytes) + str_truncated_message_length (35 bytes) + 4 bytes user message + 1 byte NULL terminator.
     */
    user_message_after_truncated_size = 4;
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "46", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_3bytes_and_reduce_msg_buf_len_by_env_variable"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    remaining_byte_truncated_utf8_character = 2;
    expected_message_length = user_message_after_truncated_size - remaining_byte_truncated_utf8_character + str_truncate_message_length;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size -= remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '$';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));
    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 4 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_4bytes_in_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *utf8_4byte_character = "𐍈";
    char *message = NULL;
    char *expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_4bytes_in_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message contain the '𐍈' (4 bytes utf-8 character) and last byte of this character is exceed buffer length */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /**
     * Fill the "𐍈" character at the position which the last byte of this character is exceed the buffer length and
     * expectation is it will be truncated 3 more bytes in the character sequence
     */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    remaining_byte_truncated_utf8_character = 3;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - remaining_byte_truncated_utf8_character);
    strncpy(message + index, utf8_4byte_character, strlen(utf8_4byte_character));

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length - remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

/**
 *  In Non-Verbose Mode, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 4 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_4bytes_in_non_verbose_mode)
{
    DltContext context;
    DltContextData contextData;
    uint16_t index = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t send_message_length = 0;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *utf8_4byte_character = "𐍈";
    char *message = NULL;
    char *expected_message = NULL;

    dlt_nonverbose_mode();

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_4bytes_in_non_verbose_mode"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    /* Create the message contain the '𐍈' (4 bytes utf-8 character) and last byte of this character is exceed buffer length */
    send_message_length = DLT_USER_BUF_MAX_SIZE + 10;
    message = (char *)(malloc(send_message_length));
    ASSERT_TRUE(message != NULL) << "Failed to allocate memory.";

    for (index = 0; index < send_message_length; index++)
    {
        message[index] = '#';
    }
    message[send_message_length - 1] = '\0';

    /**
     * In Non-Verbose Mode:
     * package_description_size = Message ID (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);

    /**
     * Fill the "𐍈" character at the position which the last byte of this character is exceed the buffer length and
     * expectation is it will be truncated 3 more bytes in the character sequence
     */
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;
    remaining_byte_truncated_utf8_character = 3;
    index = (DLT_USER_BUF_MAX_SIZE - package_description_size - str_truncate_message_length - remaining_byte_truncated_utf8_character);
    strncpy(message + index, utf8_4byte_character, strlen(utf8_4byte_character));

    /* Create the expected message */
    expected_message_length = DLT_USER_BUF_MAX_SIZE - package_description_size;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";
    user_message_after_truncated_size = expected_message_length - str_truncate_message_length - remaining_byte_truncated_utf8_character;

    for (index = 0; index < user_message_after_truncated_size; index++)
    {
        expected_message[index] = '#';
    }
    strncpy(expected_message + user_message_after_truncated_size, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));
    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));

    free(message);
    message = NULL;
    free(expected_message);
    expected_message = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore verbose mode */
    dlt_verbose_mode();
}

/**
 *  Set the DLT_USER_ENV_LOG_MSG_BUF_LEN to 46, send a message which has the length exceed DLT_USER_ENV_LOG_MSG_BUF_LEN
 *  Expectation: dlt_user_log_write_utf8_string() will be returned DLT_RETURN_USER_BUFFER_FULL and
 *               message will be truncated at the middle of utf-8 4 bytes, the rest of this utf-8 character will
 *               be removed completely, after that appended STR_TRUNCATED_MESSAGE at
 *               the end of received message.
 *  Note: dlt_init() will be called after testcase is finished to restore environment for other testcases
 */
TEST(t_dlt_user_log_write_utf8_string, normal_message_truncated_at_utf8_4bytes_and_reduce_msg_buf_len_by_env_variable)
{
    DltContext context;
    DltContextData contextData;
    uint16_t str_truncate_message_length = 0;
    uint16_t expected_message_length = 0;
    uint16_t package_description_size = 0;
    uint16_t user_message_after_truncated_size = 0;
    uint16_t remaining_byte_truncated_utf8_character = 0;
    const char *message = "$𐍈###############################################";
    char *expected_message = NULL;

    /**
     * Re-initialize the dlt with dlt user buffer size from DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to simulate use case the dlt user buffer size only available 4 bytes for store user message.
     * Note: 46 bytes = package_description_size (6 bytes) + str_truncated_message_length (35 bytes) + 4 bytes user message + 1 byte NULL terminator.
     */
    user_message_after_truncated_size = 4;
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    setenv(DLT_USER_ENV_LOG_MSG_BUF_LEN, "46", 1);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal_message_truncated_at_utf8_4bytes_and_reduce_msg_buf_len_by_env_variable"));

    /* Normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_user_log_write_utf8_string(&contextData, message));

    /**
     * In Verbose Mode:
     * package_description_size = Type info (32 bits) + Description of data payload of type string (16 bits)
     */
    package_description_size = sizeof(uint32_t) + sizeof(uint16_t);
    str_truncate_message_length = strlen(STR_TRUNCATED_MESSAGE) + 1;

    /* Create the expected message */
    remaining_byte_truncated_utf8_character = 3;
    expected_message_length = user_message_after_truncated_size - remaining_byte_truncated_utf8_character + str_truncate_message_length;
    expected_message = (char *)(malloc(expected_message_length));
    ASSERT_TRUE(expected_message != NULL) << "Failed to allocate memory.";

    expected_message[0] = '$';
    strncpy(expected_message + 1, STR_TRUNCATED_MESSAGE, str_truncate_message_length);

    ASSERT_STREQ(expected_message, (char *)(contextData.buffer + package_description_size));
    EXPECT_EQ(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());

    /* Restore the dlt with dlt user buffer size as default */
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

TEST(t_dlt_user_log_write_utf8_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string nullpointer"));

    /* NULL */
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_utf8_string(NULL, text1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_utf8_string(NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_utf8_string(&contextData, NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_utf8_string */

TEST(t_dlt_user_log_write_sized_utf8_string, partial_string)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_utf8_string normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char text1[] = "TheQuickBrownFox";
    const char* arg1_start = strchr(text1, 'Q');
    const size_t arg1_len = 5;
    /* from the above string, send only the substring "Quick" */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_utf8_string(&contextData, arg1_start, arg1_len));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_sized_utf8_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_utf8_string nullpointer"));

    /* NULL */
    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_sized_utf8_string(NULL, text1, 5));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_sized_utf8_string(NULL, NULL, 5));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_sized_utf8_string(&contextData, NULL, 5));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_constant_utf8_string*/
TEST(t_dlt_user_log_write_constant_utf8_string, verbose)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_utf8_string verbose"));

    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string(&contextData, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_constant_utf8_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_utf8_string nullpointer"));

    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_TRUE, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string(NULL, text1));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string(NULL, NULL));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string(&contextData, NULL));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_constant_utf8_string, nonverbose)
{
    dlt_nonverbose_mode();

    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_utf8_string nonverbose"));

    const char *text2 = "test2";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, 42));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string(&contextData, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());

    dlt_verbose_mode();
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_constant_utf8_string */
TEST(t_dlt_user_log_write_sized_constant_utf8_string, verbose)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_utf8_string verbose"));

    const char *text1 = "test1text";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string(&contextData, text1, 5));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_sized_constant_utf8_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_utf8_string nullpointer"));

    const char *text1 = "test1text";
    EXPECT_LE(DLT_RETURN_TRUE, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string(NULL, text1, 5));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string(NULL, NULL, 5));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string(&contextData, NULL, 5));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_sized_constant_utf8_string, nonverbose)
{
    dlt_nonverbose_mode();

    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_utf8_string nonverbose"));

    const char *text2 = "test2text";
    size_t text2len = 5;  // only use a (non-null-terminated) substring
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, 42));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string(&contextData, text2, text2len));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());

    dlt_verbose_mode();
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_string_attr */
TEST(t_dlt_user_log_write_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_string_attr(&contextData, data, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_string_attr(&contextData, data, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_string_attr(&contextData, data, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_string_attr */
TEST(t_dlt_user_log_write_sized_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_string_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456789";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_string_attr(&contextData, data, 6, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_string_attr(&contextData, data, 6, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_string_attr(&contextData, data, 6, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_constant_string_attr */
TEST(t_dlt_user_log_write_constant_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string_attr(&contextData, data, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string_attr(&contextData, data, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string_attr(&contextData, data, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_constant_string_attr */
TEST(t_dlt_user_log_write_sized_constant_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_string_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456789";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_string_attr(&contextData, data, 6, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_string_attr(&contextData, data, 6, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_string_attr(&contextData, data, 6, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_utf8_string_attr */
TEST(t_dlt_user_log_write_utf8_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_utf8_string_attr(&contextData, data, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_utf8_string_attr(&contextData, data, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_utf8_string_attr(&contextData, data, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_utf8_string_attr */
TEST(t_dlt_user_log_write_sized_utf8_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_utf8_string_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456789";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_utf8_string_attr(&contextData, data, 6, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_utf8_string_attr(&contextData, data, 6, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_utf8_string_attr(&contextData, data, 6, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_constant_utf8_string_attr */
TEST(t_dlt_user_log_write_constant_utf8_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_utf8_string_attr normal"));

    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string_attr(&contextData, text1, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_constant_utf8_string_attr, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_utf8_string_attr nullpointer"));

    const char *text1 = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string_attr(NULL, text1, "name"));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string_attr(NULL, NULL, "name"));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_constant_utf8_string_attr(&contextData, NULL, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_sized_constant_utf8_string_attr */
TEST(t_dlt_user_log_write_sized_constant_utf8_string_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_utf8_string_attr normal"));

    const char *text1 = "test1text";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string_attr(&contextData, text1, 5, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_sized_constant_utf8_string_attr, nullpointer)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_sized_constant_utf8_string_attr nullpointer"));

    const char *text1 = "test1text";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string_attr(NULL, text1, 5, "name"));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string_attr(NULL, NULL, 5, "name"));
    EXPECT_GT(DLT_RETURN_OK, dlt_user_log_write_sized_constant_utf8_string_attr(&contextData, NULL, 5, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_raw */
TEST(t_dlt_user_log_write_raw, normal)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    char text1[6] = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw(&contextData, text1, 6));
    char text2[1] = "";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw(&contextData, text2, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_raw, nullpointer)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw nullpointer"));

    /* NULL */
    char text1[6] = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_raw(NULL, text1, 6));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_raw(NULL, NULL, 0));
    EXPECT_GE(DLT_RETURN_OK, dlt_user_log_write_raw(&contextData, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_raw(&contextData, NULL, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_raw_attr */
TEST(t_dlt_user_log_write_raw_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_attr(&contextData, data, 6, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_attr(&contextData, data, 6, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_attr(&contextData, data, 6, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_raw_formatted */
TEST(t_dlt_user_log_write_raw_formatted, normal)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted normal"));

    /* normal values */
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    char text1[6] = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_BIN16));
    char text2[1] = "";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX32));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX64));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_BIN8));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_BIN16));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_raw_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;

    uint16_t length = DLT_USER_BUF_MAX_SIZE + 10;

    char buffer[length];
    memset(buffer, '\000', length);

    for (int i = 0; i < length; i++)
        buffer[i] = 'X';

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted abnormal"));

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL,
              dlt_user_log_write_raw_formatted(&contextData, buffer, length, DLT_FORMAT_DEFAULT));

/*     undefined values for DltFormatType */
/*     shouldn't it return -1? */
/*    char text1[6] = "test1"; */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)-100)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)-10)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)10)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)100)); */

    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_log_write_raw_formatted, nullpointer)
{
    DltContext context;
    DltContextData contextData;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted nullpointer"));

    /* NULL */
    char text1[6] = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_raw_formatted(NULL, text1, 6, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_raw_formatted(NULL, NULL, 0, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted(&contextData, NULL, 0, DLT_FORMAT_DEFAULT));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_user_log_write_raw_formatted(&contextData, NULL, 1, DLT_FORMAT_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_user_log_write_raw_formatted_attr */
TEST(t_dlt_user_log_write_raw_formatted_attr, normal)
{
    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted_attr normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));

    char data[] = "123456";
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted_attr(&contextData, data, 6, DLT_FORMAT_DEFAULT, "name"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted_attr(&contextData, data, 6, DLT_FORMAT_DEFAULT, ""));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_raw_formatted_attr(&contextData, data, 6, DLT_FORMAT_DEFAULT, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/*
 * Test sending Verbose and Non-Verbose messages in the same session.
 */

/*/////////////////////////////////////// */
/* t_dlt_user_nonverbose*/
TEST(t_dlt_user_nonverbose, nonverbosemode)
{
    dlt_nonverbose_mode();
    dlt_use_extended_header_for_non_verbose(false);

    DltContext context;
    DltContextData contextData;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_message_modes"));

    // Send a Verbose message
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string_attr(&contextData, "hello", "msg"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, 0x01020304, "val1", "unit1"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, 0x04030201, "val2", "unit2"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    // Send a Non-Verbose message
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, 42));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_constant_string_attr(&contextData, "hello", "msg"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, 0x01020304, "val1", "unit1"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_uint32_attr(&contextData, 0x04030201, "val2", "unit2"));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_log_write_finish(&contextData));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());

    dlt_use_extended_header_for_non_verbose(true);
    dlt_verbose_mode();
}

/*/////////////////////////////////////// */
/*
 * int dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text);
 * int dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data);
 * int dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data);
 * int dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data);
 * int dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data);
 * int dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length);
 * int dlt_log_marker();
 */

/*/////////////////////////////////////// */
/* t_dlt_log_string */
TEST(t_dlt_log_string, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string normal"));

    /* normal values */
    const char text1[6] = "test1";
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_DEFAULT, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_OFF, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_FATAL, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_ERROR, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_WARN, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_INFO, text1));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_VERBOSE, text1));
    const char text2[1] = "";
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_DEFAULT, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_OFF, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_FATAL, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_ERROR, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_WARN, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_INFO, text2));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string(&context, DLT_LOG_VERBOSE, text2));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_string, abnormal)
{
    DltContext context;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string abnormal"));

    uint16_t length = DLT_USER_BUF_MAX_SIZE + 10;
    char buffer[length];
    memset(buffer, '\000', length);

    for (int i = 0; i < length - 1; i++)
        buffer[i] = 'X';

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_log_string(&context, DLT_LOG_INFO, buffer));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    /* TODO: const char text1[6] = "test1"; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string(&context, (DltLogLevelType)-100, text1)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string(&context, (DltLogLevelType)-10, text1)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string(&context, (DltLogLevelType)10, text1)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string(&context, (DltLogLevelType)100, text1)); */

    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_string, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string nullpointer"));

    /* NULL */
    char text1[6] = "test1";
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string(NULL, DLT_LOG_DEFAULT, text1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string(NULL, DLT_LOG_DEFAULT, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string(&context, DLT_LOG_DEFAULT, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_log_string_int */
TEST(t_dlt_log_string_int, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int normal"));

    /* normal values */
    const char text1[6] = "test1";
    int data = INT_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_DEFAULT, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_OFF, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_FATAL, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_ERROR, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_WARN, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_INFO, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_VERBOSE, text1, data));
    const char text2[1] = "";
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_VERBOSE, text2, data));
    data = INT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_int(&context, DLT_LOG_VERBOSE, text2, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_string_int, abnormal)
{
    DltContext context;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int abnormal"));

    uint16_t length = DLT_USER_BUF_MAX_SIZE + 10;
    char buffer[length];
    memset(buffer, '\000', length);

    for (int i = 0; i < length - 1; i++)
        buffer[i] = 'X';

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_log_string_int(&context, DLT_LOG_INFO, buffer, 1));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    /* TODO: const char text1[6] = "test1"; */
    /* TODO: int data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_int(&context, (DltLogLevelType)-100, text1, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_int(&context, (DltLogLevelType)-10, text1, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_int(&context, (DltLogLevelType)10, text1, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_int(&context, (DltLogLevelType)100, text1, data)); */

    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_string_int, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int nullpointer"));

    /* NULL */
    char text1[6] = "test1";
    int data = 0;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string_int(NULL, DLT_LOG_DEFAULT, text1, data));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string_int(NULL, DLT_LOG_DEFAULT, NULL, data));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string_int(&context, DLT_LOG_DEFAULT, NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_log_string_uint */
TEST(t_dlt_log_string_uint, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint normal"));

    /* normal values */
    const char text1[6] = "test1";
    unsigned int data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_OFF, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_FATAL, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_ERROR, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_WARN, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_INFO, text1, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text1, data));
    const char text2[1] = "";
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text2, data));
    data = UINT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text2, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_string_uint, abnormal)
{
    DltContext context;

    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint abnormal"));

    uint16_t length = DLT_USER_BUF_MAX_SIZE + 10;
    char buffer[length];
    memset(buffer, '\000', DLT_USER_BUF_MAX_SIZE + 10);

    for (int i = 0; i < length - 1; i++)
        buffer[i] = 'X';

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_log_string_uint(&context, DLT_LOG_INFO, buffer, 1));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    /* TODO: const char text1[6] = "test1"; */
    /* TODO: unsigned int data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_uint(&context, (DltLogLevelType)-100, text1, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_uint(&context, (DltLogLevelType)-10, text1, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_uint(&context, (DltLogLevelType)10, text1, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_string_uint(&context, (DltLogLevelType)100, text1, data)); */

    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_string_uint, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint nullpointer"));

    /* NULL */
    char text1[6] = "test1";
    unsigned int data = 0;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string_uint(NULL, DLT_LOG_DEFAULT, text1, data));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string_uint(NULL, DLT_LOG_DEFAULT, NULL, data));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_string_uint(&context, DLT_LOG_DEFAULT, NULL, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_log_int */
TEST(t_dlt_log_int, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int normal"));

    /* normal values */
    int data = INT_MIN;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_VERBOSE, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_VERBOSE, data));
    data = INT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_int(&context, DLT_LOG_VERBOSE, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_int, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int abnormal"));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    /* TODO: int data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_int(&context, (DltLogLevelType)-100, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_int(&context, (DltLogLevelType)-10, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_int(&context, (DltLogLevelType)10, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_int(&context, (DltLogLevelType)100, data)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_int, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int nullpointer"));

    /* NULL */
    int data = 0;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_int(NULL, DLT_LOG_DEFAULT, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_log_uint */
TEST(t_dlt_log_uint, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint normal"));

    /* normal values */
    unsigned int data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_VERBOSE, data));
    data = 0;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_VERBOSE, data));
    data = UINT_MAX;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_uint(&context, DLT_LOG_VERBOSE, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_uint, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint abnormal"));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
    /* TODO: unsigned int data = 1; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_uint(&context, (DltLogLevelType)-100, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_uint(&context, (DltLogLevelType)-10, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_uint(&context, (DltLogLevelType)10, data)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_uint(&context, (DltLogLevelType)100, data)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_uint, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint nullpointer"));

    /* NULL */
    unsigned int data = 0;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_uint(NULL, DLT_LOG_DEFAULT, data));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_log_raw */
TEST(t_dlt_log_raw, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw normal"));

    /* normal values */
    char data[5] = "test";
    uint16_t length = 4;
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_DEFAULT, data, length));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_OFF, data, length));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_FATAL, data, length));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_ERROR, data, length));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_WARN, data, length));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_INFO, data, length));
    EXPECT_LE(DLT_RETURN_OK, dlt_log_raw(&context, DLT_LOG_VERBOSE, data, length));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_raw, abnormal)
{
    DltContext context;



    EXPECT_EQ(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw abnormal"));

    uint16_t length = DLT_USER_BUF_MAX_SIZE + 10;

    char buffer[length];
    memset(buffer, '\000', length);

    for (int i = 0; i < length; i++)
        buffer[i] = 'X';

    EXPECT_EQ(DLT_RETURN_USER_BUFFER_FULL, dlt_log_raw(&context, DLT_LOG_INFO, buffer, length));

    /* undefined values for DltLogLevelType */
    /* shouldn't it return -1? */
/*    char data[5] = "test"; */
    /* TODO: uint16_t length = 4; */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, (DltLogLevelType)-100, data, length)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, (DltLogLevelType)-10, data, length)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, (DltLogLevelType)10, data, length)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, (DltLogLevelType)100, data, length)); */

    /* zero length */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, 0)); */

    /* negative length */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, -1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, -100)); */

    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_EQ(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_log_raw, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw nullpointer"));

    /* NULL */
    char data[5] = "test";
    uint16_t length = 4;
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_raw(NULL, DLT_LOG_DEFAULT, data, length));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_raw(NULL, DLT_LOG_DEFAULT, NULL, length));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log_raw(&context, DLT_LOG_DEFAULT, NULL, length));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_log_marker */
TEST(t_dlt_log_marker, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_marker normal"));

    /* normal */
    EXPECT_LE(DLT_RETURN_OK, dlt_log_marker());

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_register_app */
TEST(t_dlt_register_app, normal)
{


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("T", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TU", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUS", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_register_app, abnormal)
{


    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_app("", "dlt_user.c tests"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_unregister_app());
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR1", "dlt_user.c tests")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_app()); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR123445667", "dlt_user.c tests")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_app()); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_app()); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR", NULL)); */
}

TEST(t_dlt_register_app, nullpointer)
{


    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_app(NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_app(NULL, "dlt_user.c tests"));

}

/*/////////////////////////////////////// */
/* t_dlt_unregister_app */
TEST(t_dlt_unregister_app, normal)
{


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("T", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TU", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUS", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_unregister_app, abnormal)
{


    EXPECT_GE(DLT_RETURN_ERROR, dlt_unregister_app());
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_app("", "dlt_user.c tests"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_unregister_app());
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR1", "dlt_user.c tests")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_app()); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR123445667", "dlt_user.c tests")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_app()); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_app("TUSR", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_app()); */
}

/*/////////////////////////////////////// */
/* t_dlt_register_context */
TEST(t_dlt_register_context, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_register_context, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, "", "d"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "T", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, "", ""));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1", "1")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1234567890", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1234567890", "1")); */

    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", NULL)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_register_context, nullpointer)
{
    DltContext context;


    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, NULL, "dlt_user.c t_dlt_register_context normal"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(NULL, "TEST", NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(NULL, NULL, "dlt_user.c t_dlt_register_context normal"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(NULL, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}


/*/////////////////////////////////////// */
/* t_dlt_register_context_ll_ts */
TEST(t_dlt_register_context_ll_ts, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_FATAL,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_ERROR,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_WARN,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_INFO,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_DEBUG,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_VERBOSE,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_FATAL,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_ERROR,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_WARN,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_INFO,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_DEBUG,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_VERBOSE,
                                         DLT_TRACE_STATUS_ON));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_register_context_ll_ts, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts(&context, "", "d", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "T", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts(&context, "", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST1", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST1", "1", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST1234567890", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST1234567890", "1", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */

    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_ON));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));

    /* DLT_LOG_DEFAULT and DLT_TRACE_STATUS_DEFAULT not allowed */
    /* TODO: Why not? */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEFAULT, DLT_TRACE_STATUS_OFF)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
/*    EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_DEFAULT)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */

    /* abnormal values for loglevel and tracestatus */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", -3,
                                         DLT_TRACE_STATUS_OFF));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", 100,
                                         DLT_TRACE_STATUS_OFF));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_OFF, -3));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER,
              dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal",
                                         DLT_LOG_OFF, 100));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context_ll_ts(&context, "TEST", NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_register_context_ll_ts, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_register_context_ll_ts(&context, NULL, "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts(&context, NULL, NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts(NULL, "TEST", NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_register_context_ll_ts(NULL, NULL, "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF,
                                         DLT_TRACE_STATUS_OFF));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context_ll_ts(NULL, NULL, NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

/*/////////////////////////////////////// */
/* t_dlt_unregister_context */
TEST(t_dlt_unregister_context, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_unregister_context, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, "", "d"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "T", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, "", ""));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1", "1")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1234567890", "")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_unregister_context(&context)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST1234567890", "1")); */

    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal")); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_register_context(&context, "TEST", NULL)); */
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_unregister_context, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, NULL, "dlt_user.c t_dlt_unregister_context normal"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(&context, NULL, NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(NULL, "TEST", NULL));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(NULL, NULL, "dlt_user.c t_dlt_unregister_context normal"));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_register_context(NULL, NULL, NULL));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}


/*/////////////////////////////////////// */
/* t_dlt_register_injection_callback */
int dlt_user_injection_callback(uint32_t /*service_id*/, void */*data*/, uint32_t /*length*/)
{
    return 0;
}

TEST(t_dlt_register_injection_callback, normal)
{
    DltContext context;
    /* TODO: uint32_t service_id; */



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_injection_callback normal"));

    /* TODO: service_id = 0x123; */
    /* TODO: EXPECT_LE(DLT_RETURN_OK,dlt_register_injection_callback(&context, service_id, dlt_user_injection_callback)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());

}


/*/////////////////////////////////////// */
/* t_dlt_register_log_level_changed_callback */
void dlt_user_log_level_changed_callback(char /*context_id*/[DLT_ID_SIZE], uint8_t /*log_level*/,
                                         uint8_t /*trace_status*/)
{}

TEST(t_dlt_register_log_level_changed_callback, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_log_level_changed_callback normal"));

    EXPECT_LE(DLT_RETURN_OK, dlt_register_log_level_changed_callback(&context, dlt_user_log_level_changed_callback));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());

}

#ifdef DLT_NETWORK_TRACE_ENABLE
/*/////////////////////////////////////// */
/* t_dlt_user_trace_network */
TEST(t_dlt_user_trace_network, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network normal"));

    char header[16];

    for (char i = 0; i < 16; ++i)
        header[(int)i] = i;

    char payload[32];

    for (char i = 0; i < 32; ++i)
        payload[(int)i] = i;

    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network(&context, DLT_NW_TRACE_IPC, 16, header, 32, payload));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network(&context, DLT_NW_TRACE_CAN, 16, header, 32, payload));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 32, payload));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network(&context, DLT_NW_TRACE_MOST, 16, header, 32, payload));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_trace_network, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network abnormal"));

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
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload)); */

    /* invalid DltNetworkTraceType value */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, (DltNetworkTraceType)-100, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, (DltNetworkTraceType)-10, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, (DltNetworkTraceType)10, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network(&context, (DltNetworkTraceType)100, 16, header, 32, payload)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_trace_network, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network nullpointer"));

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

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}


/*/////////////////////////////////////// */
/* t_dlt_user_trace_network_truncated */
TEST(t_dlt_user_trace_network_truncated, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated normal"));

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

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_trace_network_truncated, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated abnormal"));

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

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_trace_network_truncated, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated nullpointer"));

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

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}


/*/////////////////////////////////////// */
/* t_dlt_user_trace_network_segmented */
TEST(t_dlt_user_trace_network_segmented, normal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented normal"));

    char header[16];

    for (char i = 0; i < 16; ++i)
        header[(int)i] = i;

    char payload[32];

    for (char i = 0; i < 32; ++i)
        payload[(int)i] = i;

    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_IPC, 16, header, 32, payload));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_CAN, 16, header, 32, payload));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 32, payload));
    EXPECT_LE(DLT_RETURN_OK, dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_MOST, 16, header, 32, payload));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_trace_network_segmented, abnormal)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented abnormal"));

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
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload)); */

    /* invalid DltNetworkTraceType value */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)-100, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)-10, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)10, 16, header, 32, payload)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)100, 16, header, 32, payload)); */

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_trace_network_segmented, nullpointer)
{
    DltContext context;



    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented nullpointer"));

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

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}
#endif /* DLT_NETWORK_TRACE_ENABLE */

/*/////////////////////////////////////// */
/* t_dlt_set_log_mode */
TEST(t_dlt_set_log_mode, normal)
{



    EXPECT_LE(DLT_RETURN_OK, dlt_set_log_mode(DLT_USER_MODE_OFF));
    EXPECT_LE(DLT_RETURN_OK, dlt_set_log_mode(DLT_USER_MODE_EXTERNAL));
    EXPECT_LE(DLT_RETURN_OK, dlt_set_log_mode(DLT_USER_MODE_INTERNAL));
    EXPECT_LE(DLT_RETURN_OK, dlt_set_log_mode(DLT_USER_MODE_BOTH));

}

TEST(t_dlt_set_log_mode, abnormal)
{



    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_set_log_mode(DLT_USER_MODE_UNDEFINED)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_set_log_mode((DltUserLogMode)-100)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_set_log_mode((DltUserLogMode)-10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_set_log_mode((DltUserLogMode)10)); */
    /* TODO: EXPECT_GE(DLT_RETURN_ERROR,dlt_set_log_mode((DltUserLogMode)100)); */

}


/*/////////////////////////////////////// */
/* t_dlt_get_log_state */
TEST(t_dlt_get_log_state, normal)
{


    sleep(1);
    dlt_init_common();
    EXPECT_EQ(-1, dlt_get_log_state());

}


/*/////////////////////////////////////// */
/* t_dlt_verbose_mode */
TEST(t_dlt_verbose_mode, normal)
{



    EXPECT_LE(DLT_RETURN_OK, dlt_verbose_mode());

}


/*/////////////////////////////////////// */
/* t_dlt_nonverbose_mode */
TEST(t_dlt_nonverbose_mode, normal)
{



    EXPECT_LE(DLT_RETURN_OK, dlt_nonverbose_mode());

}

/*/////////////////////////////////////// */
/* free dlt */
TEST(t_dlt_free, onetime)
{
    EXPECT_EQ(DLT_RETURN_OK, dlt_free());
}

/*/////////////////////////////////////// */
/* dlt_user_is_logLevel_enabled */
TEST(t_dlt_user_is_logLevel_enabled, normal)
{
    DltContext context;
    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(DLT_RETURN_OK, dlt_register_context_ll_ts(&context, "ILLE",
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

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}

TEST(t_dlt_user_is_logLevel_enabled, nullpointer)
{
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_user_is_logLevel_enabled(NULL, DLT_LOG_FATAL));
}

/*/////////////////////////////////////// */
/* t_dlt_user_shutdown_while_init_is_running */

struct ShutdownWhileInitParams {
    ShutdownWhileInitParams() = default;
    // delete copy constructor
    ShutdownWhileInitParams(const ShutdownWhileInitParams&) = delete;

    std::chrono::time_point<std::chrono::steady_clock> stop_time;

    pthread_cond_t dlt_free_done = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t dlt_free_mtx = PTHREAD_MUTEX_INITIALIZER;
    bool has_error = false;

};

void* dlt_free_call_and_deadlock_detection(void *arg) {
    auto *params = static_cast<ShutdownWhileInitParams *>(arg);

    // allow thread to be canceled
    int old_thread_type;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_thread_type);

    dlt_free();

    // signal that we are done
    pthread_mutex_lock(&params->dlt_free_mtx);
    pthread_cond_signal(&params->dlt_free_done);
    pthread_mutex_unlock(&params->dlt_free_mtx);
    return nullptr;
}

void *dlt_free_thread(void *arg) {
    auto *params = static_cast<ShutdownWhileInitParams *>(arg);
    while (std::chrono::steady_clock::now() < params->stop_time && !params->has_error) {

        // pthread cond_timedwait expects an absolute time to wait
        struct timespec abs_time{};
        clock_gettime(CLOCK_REALTIME, &abs_time);
        abs_time.tv_sec += 3; // wait at most 3 seconds

        pthread_t dlt_free_deadlock_detection_thread_id;

        pthread_mutex_lock(&params->dlt_free_mtx);
        pthread_create(&dlt_free_deadlock_detection_thread_id, nullptr, dlt_free_call_and_deadlock_detection, params);
        const auto err = pthread_cond_timedwait(&params->dlt_free_done, &params->dlt_free_mtx, &abs_time);
        pthread_mutex_unlock(&params->dlt_free_mtx);

        if (err == ETIMEDOUT) {
            fprintf(stderr, "\n%s: detected DLT-deadlock!\n", __func__);
            params->has_error = true;

            // cancel thread after timeout, so join won't block forever.
            pthread_cancel(dlt_free_deadlock_detection_thread_id);
        }

        pthread_join(dlt_free_deadlock_detection_thread_id, nullptr);
    }

    return nullptr;
}

TEST(t_dlt_user_shutdown_while_init_is_running, normal) {
    const auto max_runtime = std::chrono::seconds(5);
    const auto stop_time = std::chrono::steady_clock::now() + max_runtime;

    struct ShutdownWhileInitParams args{};
    args.stop_time = stop_time;

    pthread_t dlt_free_thread_id;
    pthread_create(&dlt_free_thread_id, nullptr, dlt_free_thread, &args);

    while (std::chrono::steady_clock::now() < stop_time && !args.has_error) {
        dlt_init();
    }

    pthread_join(dlt_free_thread_id, nullptr);
    EXPECT_FALSE(args.has_error);

    const auto last_init = dlt_init();
    const auto last_free = dlt_free();

    EXPECT_EQ(last_init, DLT_RETURN_OK);
    EXPECT_EQ(last_free, DLT_RETURN_OK);
}

/*/////////////////////////////////////// */
/* main */
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

