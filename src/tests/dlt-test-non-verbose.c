/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
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
 *author
 * Onkar Palkar <onkar.palkar@wipro.com>
 *
 *copyright Copyright © 2015 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 *file dlt-test-non-verbose.c
 */

#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dlt.h"

/* data for real life message test */
#define DLT_COMMON_BUFFER_LENGTH 255
#define MAXSTRLEN 1024
#define DLT_MODULE_ID 0x0154
#define DLT_MSG_ID_1 0x0032
#define DLT_MSG_ID_2 0x0033
#define DLT_MSG_ID_3 0x0009
#define DLT_MSG_ID_4 0x000c
#define DLT_MSG_ID_5 0x0004
#define DLT_MSG_ID_6 0x0009

#define LOG_DELAY       200 * 1000
#define NUM_LOG_MSGS    10

#define DEFAULT_WAIT_TIMEOUT 1000

DLT_DECLARE_CONTEXT(context_info)
DLT_DECLARE_CONTEXT(context_log);
DLT_DECLARE_CONTEXT(context_macro_test)
DltContext context_function_test;

DltContextData context_data;

void dlt_user_log_level_changed_callback(char context_id[DLT_ID_SIZE],uint8_t log_level,uint8_t trace_status);

void usage()
{
    char version[DLT_COMMON_BUFFER_LENGTH];

    dlt_get_version(version, DLT_COMMON_BUFFER_LENGTH);

    printf("Usage: dlt-test-non-verbose [options]\n");
    printf("Test user application providing several Tests.\n");
    printf("%s\n", version);
    printf("Options:\n");
    printf("  -a               run all tests \n");
    printf("  -i               test all types (macro interface and functional interface)\n");
    printf("  -l               message for log storage test\n");
    printf("  -r               real data\n");
    printf(" -o               Log level test \n");
    printf("  -h               this help\n");
    printf("\nTests:\n");
    printf("  01: (Macro IF)    Test all variable types (non-verbose)\n");
    printf("  02: (Function IF) Test all variable types (non-verbose)\n");
    printf("  03: Test Logstorage messages (non-verbose)\n");
    printf("  04: Test real life messages (non-verbose)\n");
}

/******************/
/* The test cases */
/******************/
int test_logstorage()
{
    int delay = LOG_DELAY;
    int i;

    DLT_LOG(context_info, DLT_LOG_INFO, DLT_STRING("Test Logstorage messages"));

    printf("Test01: Sending log messages with level :\n");
    printf("         FATAL\n");
    printf("         ERROR\n");
    printf("         WARN\n");

    printf("Test01: Check DLT viewer\n");
    printf("Test01: Log messages with FATAL,"
                                           "ERROR should be seen\n");
    printf("Test01: Connect USB to TARGET\n");

    for(i = 1 ; i <= NUM_LOG_MSGS ; i++)
    {
        printf("Send log message  %d\n", i);
        DLT_LOG_ID(context_log,DLT_LOG_FATAL, 1000,
                   DLT_CSTRING("DLT Log Storage Test"), DLT_INT(i));
        DLT_LOG_ID(context_log,DLT_LOG_ERROR, 1001,
                   DLT_CSTRING("DLT Log Storage Test"), DLT_INT(i));
        DLT_LOG_ID(context_log,DLT_LOG_WARN, 1002,
                   DLT_CSTRING("DLT Log Storage Test"), DLT_INT(i));

        usleep(delay);
    }

    printf("Test01: Remove  USB from TARGET\n");
    printf("Test01: Open log file stored in USB using DLT viewer\n");

    DLT_LOG(context_info, DLT_LOG_INFO, DLT_STRING("Test Logstorage messages finished"));

    return 0;
}

int test_loglevel(int wait_duration)
{
    DLT_LOG(context_info, DLT_LOG_INFO, DLT_STRING("Test log level"));

    sleep(wait_duration);
    return 0;
}

int test_macro_interface(void)
{
    char buffer[10];
    int num2;

    printf("Test02: (Macro IF) Test all variable types (non-verbose)\n");

    DLT_LOG(context_info, DLT_LOG_INFO,DLT_STRING
           ("Test02: (Macro IF) Test all variable types (non-verbose)"));

    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 1,
                             DLT_STRING("string"), DLT_STRING("Hello world"));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 2,
                             DLT_STRING("utf8"), DLT_UTF8("Hello world"));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 3,
                             DLT_STRING("bool"), DLT_BOOL(1));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 4,
                             DLT_STRING("int"), DLT_INT(INT32_MIN));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 5,
                             DLT_STRING("int8"), DLT_INT8(INT8_MIN));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 6,
                             DLT_STRING("int16"), DLT_INT16(INT16_MIN));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 7,
                             DLT_STRING("int32"), DLT_INT32(INT32_MIN));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 8,
                             DLT_STRING("int64"), DLT_INT64(INT64_MIN));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 9,
                             DLT_STRING("uint"), DLT_UINT(UINT32_MAX));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 10,
                             DLT_STRING("uint8"), DLT_UINT8(UINT8_MAX));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 11,
                             DLT_STRING("uint16"), DLT_UINT16(UINT16_MAX));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 12,
                             DLT_STRING("uint32"), DLT_UINT32(UINT32_MAX));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 13,
                             DLT_STRING("uint64"), DLT_UINT64(UINT64_MAX));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 14, DLT_STRING("float32"),
                             DLT_FLOAT32(FLT_MIN), DLT_FLOAT32(FLT_MAX));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 15, DLT_STRING("float64"),
                             DLT_FLOAT64(DBL_MIN), DLT_FLOAT64(DBL_MAX));

    for(num2 = 0 ; num2 < 10 ; num2++)
    {
        buffer[num2] = (char) num2;
    }
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, 14,
                             DLT_STRING("raw"), DLT_RAW(buffer, 10));

    sleep(2);
    DLT_LOG(context_info, DLT_LOG_INFO,
                             DLT_STRING("Test02: (Macro IF) finished"));

    return 0;
}

int test_function_interface(void)
{
    char buffer[10];
    int num2;

    printf("Test03: (Function IF) Test all variable types (non-verbose)\n");
    if (dlt_user_log_write_start(&context_info,
                     &context_data,DLT_LOG_INFO) > 0)
    {
        dlt_user_log_write_string(&context_data,
             "Test03: (Function IF) Test all variable types (non-verbose)");
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data,DLT_LOG_INFO, 1) > 0)
    {
        dlt_user_log_write_string(&context_data, "bool");
        dlt_user_log_write_bool(&context_data, 1);
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 2) > 0)
    {
        dlt_user_log_write_string(&context_data, "int");
        dlt_user_log_write_int(&context_data, INT32_MIN);/* (-2147483647-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 3) > 0)
    {
        dlt_user_log_write_string(&context_data, "int8");
        dlt_user_log_write_int8(&context_data, INT8_MIN); /*         (-128) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO,4) > 0)
    {
        dlt_user_log_write_string(&context_data, "int16");
        dlt_user_log_write_int16(&context_data, INT16_MIN);/*    (-32767-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 5) > 0)
    {
        dlt_user_log_write_string(&context_data, "int32");
        dlt_user_log_write_int32(&context_data, INT32_MIN);/*(-2147483647-1)*/
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 6) > 0)
    {
        dlt_user_log_write_string(&context_data, "int64");
        dlt_user_log_write_int64(&context_data, INT64_MIN);
        /* (-__INT64_C(9223372036854775807)-1) */
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 7) > 0)
    {
        dlt_user_log_write_string(&context_data, "uint");
        dlt_user_log_write_uint(&context_data, UINT32_MAX);/*  (4294967295U)*/
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 8) > 0)
    {
        dlt_user_log_write_string(&context_data, "uint8");
        dlt_user_log_write_uint8(&context_data, UINT8_MAX);/*         (255) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 9) > 0)
    {
        dlt_user_log_write_string(&context_data, "uint16");
        dlt_user_log_write_uint16(&context_data, UINT16_MAX);/*    (65535)  */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                      &context_data, DLT_LOG_INFO, 10) > 0)
    {
        dlt_user_log_write_string(&context_data,"uint32");
        dlt_user_log_write_uint32(&context_data,UINT32_MAX);/* (4294967295U)*/
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                             &context_data, DLT_LOG_INFO, 11) > 0)
    {
        dlt_user_log_write_string(&context_data,"uint64");
        dlt_user_log_write_uint64(&context_data,UINT64_MAX);
        /* (__UINT64_C(18446744073709551615)) */
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test),
                             &context_data, DLT_LOG_INFO, 12) > 0)
    {
        dlt_user_log_write_string(&context_data,"float32");
        dlt_user_log_write_float32(&context_data,FLT_MIN);
        dlt_user_log_write_float32(&context_data,FLT_MAX);
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test),
                             &context_data, DLT_LOG_INFO, 13) > 0)
    {
        dlt_user_log_write_string(&context_data,"float64");
        dlt_user_log_write_float64(&context_data,DBL_MIN);
        dlt_user_log_write_float64(&context_data,DBL_MAX);
        dlt_user_log_write_finish(&context_data);
    }

    for(num2 = 0 ; num2 < 10 ; num2++)
    {
        buffer[num2] = (char) num2;
    }

    if (dlt_user_log_write_start_id(&(context_function_test),
                             &context_data, DLT_LOG_INFO, 14) > 0)
    {
        dlt_user_log_write_string(&context_data, "raw");
        dlt_user_log_write_raw(&context_data,buffer, 10);
        dlt_user_log_write_finish(&context_data);
    }

    sleep(2);
    if (dlt_user_log_write_start(&context_info,
                           &context_data, DLT_LOG_INFO) > 0)
    {
        dlt_user_log_write_string(&context_data,
                          "Test03: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    return 0;
}

int test_real_data(void)
{
    printf("Test04: Real data test (non-verbose)\n");
    DLT_LOG(context_info, DLT_LOG_INFO,DLT_STRING("Test04: real life mesages"
                 " (non-verbose)"));

    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, DLT_MSG_ID_1,
               DLT_UINT16(DLT_MODULE_ID), DLT_UINT8(0x98), DLT_UINT8(0x01));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, DLT_MSG_ID_2,
               DLT_UINT16(DLT_MODULE_ID), DLT_UINT8(0x64), DLT_UINT8(0x0));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, DLT_MSG_ID_3,
               DLT_UINT16(DLT_MODULE_ID), DLT_UINT8(0x31), DLT_UINT8(0x28));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, DLT_MSG_ID_4,
               DLT_UINT16(DLT_MODULE_ID), DLT_UINT8(0x30));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, DLT_MSG_ID_5,
               DLT_UINT16(DLT_MODULE_ID), DLT_UINT8(0x31),
               DLT_UINT8(0x02), DLT_UINT8(0x2c), DLT_UINT8(0x0f),
               DLT_UINT8(0x08), DLT_UINT8(0x01), DLT_UINT8(0x11));
    DLT_LOG_ID(context_macro_test, DLT_LOG_INFO, DLT_MSG_ID_6,
               DLT_UINT16(DLT_MODULE_ID), DLT_UINT8(0x31), DLT_UINT8(0x28));

    sleep(2);

    DLT_LOG(context_info, DLT_LOG_INFO, DLT_STRING
           (" Test04: real life mesages (non-verbose) finished"));

    return 0;
}

void dlt_user_log_level_changed_callback(char context_id[DLT_ID_SIZE],uint8_t log_level,uint8_t trace_status)
{
    char text[5];
    text[4]=0;

    memcpy(text,context_id,DLT_ID_SIZE);

    printf("Log level changed of context %s, LogLevel=%u, TraceState=%u \n",text,log_level,trace_status);
}
/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    int avalue = 0;
    int ivalue = 0;
    int lvalue = 0;
    int rvalue = 0;
    int ovalue = 0;
    int wait_timeout = DEFAULT_WAIT_TIMEOUT;
    int c;

    if(argc < 2)
    {
        printf("\nPlease enter valid option\n\n");
        usage();
        return -1;
    }

    while ((c = getopt (argc, argv, "ailrho:")) != -1)
    {
        switch (c)
        {
        case 'a':
        {
            avalue = 1;
            break;
        }
        case 'i':
        {
            ivalue = 1;
            break;
        }
        case 'l':
        {
            lvalue = 1;
            break;
        }
        case 'r':
        {
            rvalue = 1;
            break;
        }
        case 'h':
        {
            usage();
            return 0;
        }
        case 'o':
        {
            ovalue = 1;
            wait_timeout = atoi(optarg);
            break;
        }
        case '?':
        {
            if (isprint (optopt))
            {
                fprintf (stderr, "\nUnknown option `-%c'.\n\n", optopt);
            }
            else
            {
                fprintf (stderr, "\nUnknown option character `\\x%x'.\n\n",
                                                                   optopt);
            }
            usage();
            return -1;
        }
        default:
        {
            abort ();
            return -1;
        }
        }
    }

    DLT_REGISTER_APP("DINT", "DLT Non-Verbose Interface Test");
    DLT_REGISTER_CONTEXT(context_info, "INFO","Information context");
    DLT_REGISTER_CONTEXT(context_log, "LOG", "Log Context");
    DLT_REGISTER_CONTEXT(context_macro_test, "MACR", "Macro Test Context");
    dlt_register_context(&(context_function_test), "FUNC", "Function Test Context");

    DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK(context_log, dlt_user_log_level_changed_callback);
    DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK(context_macro_test, dlt_user_log_level_changed_callback);

    DLT_NONVERBOSE_MODE();

    printf("Tests starting\n");

    DLT_LOG(context_info, DLT_LOG_INFO,DLT_STRING("Tests starting"));

    if(avalue)
    {
        printf("Execute all tests\n");
        test_logstorage();
        sleep(1);
        test_macro_interface();
        sleep(1);
        test_function_interface();
        sleep(1);
        test_real_data();
    }
    else if(ivalue)
    {
        printf("Test all different log interface types\n");
        test_macro_interface();
        sleep(1);
        test_function_interface();
    }
    else if(lvalue)
    {
        printf("Log storage test\n");
        test_logstorage();
    }
    else if(rvalue)
    {
        printf("Real data test\n");
        test_real_data();
    }
    else if(ovalue)
    {
        printf("Log level test\n");
        test_loglevel(wait_timeout);
    }

    printf("Tests finished\n");
    DLT_LOG(context_info, DLT_LOG_INFO, DLT_STRING("Tests finished"));

    DLT_UNREGISTER_CONTEXT(context_info);
    DLT_UNREGISTER_CONTEXT(context_log);
    DLT_UNREGISTER_CONTEXT(context_macro_test);
    dlt_unregister_context(&(context_function_test));

    DLT_UNREGISTER_APP();

    return 0;
}
