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
 * \file dlt-test-user.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-user.c                                               **
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

#include <stdio.h>      /* for printf() and fprintf() */
#include <float.h>
#include <stdlib.h>     /* for atoi(), abort() */
#include <string.h>     /* for memset() */
#include <ctype.h>      /* for isprint() */

#include "dlt.h"

#define DLT_TEST_NUM_CONTEXT 9

/* Test functions... */

/* for macro interface */
int test1m(void);
int test2m(void);
int test3m(void);
int test4m(void);
int test5m(void);
int test6m(void);
int test7m(void);
int test8m(void);
int test9m(void);

/* for function interface */
int test1f(void);
int test2f(void);
int test3f(void);
int test4f(void);
int test5f(void);
int test6f(void);
int test7f(void);
int test8f(void);
int test9f(void);

/* Declaration of callback functions */
int test_injection_macro_callback(uint32_t service_id, void *data, uint32_t length);
int test_injection_function_callback(uint32_t service_id, void *data, uint32_t length);

/* Context declaration.. */
DLT_DECLARE_CONTEXT(context_info)

/* for macro interface */
DLT_DECLARE_CONTEXT(context_macro_callback)
DLT_DECLARE_CONTEXT(context_macro_test[DLT_TEST_NUM_CONTEXT])

/* for function interface */
DltContext context_function_callback;
DltContext context_function_test[DLT_TEST_NUM_CONTEXT];

DltContextData context_data;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version,255);

    printf("Usage: dlt-test-user [options]\n");
    printf("Test user application providing several Tests.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -n count      Repeats of tests (Default: 1)\n");
    printf("Tests:\n");
    printf("  1m: (Macro IF)    Test all log levels\n");
    printf("  2m: (Macro IF)    Test all variable types (verbose) \n");
    printf("  3m: (Macro IF)    Test all variable types (non-verbose) \n");
    printf("  4m: (Macro IF)    Test different message sizes\n");
    printf("  5m: (Macro IF)    Test high-level API\n");
    printf("  6m: (Macro IF)    Test local printing\n");
    printf("  7m: (Macro IF)    Test network trace\n");
    printf("  8m: (Macro IF)    Test truncated network trace\n");
    printf("  9m: (Macro IF)    Test segmented network trace\n");
    printf("  1f: (Function IF) Test all log levels\n");
    printf("  2f: (Function IF) Test all variable types (verbose) \n");
    printf("  3f: (Function IF) Test all variable types (non-verbose) \n");
    printf("  4f: (Function IF) Test different message sizes\n");
    printf("  5f: (Function IF) Test high-level API\n");
    printf("  6f: (Function IF) Test local printing\n");
    printf("  7f: (Function IF) Test network trace\n");
    printf("  8f: (Function IF) Test truncated network trace\n");
    printf("  9f: (Function IF) Test segmented network trace\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    //int vflag = 0;
    char *fvalue = 0;
    char *nvalue = 0;

    int c;

    int i;
    char ctid[4], ctdesc[255];

    int num,maxnum;

    opterr = 0;

    while ((c = getopt (argc, argv, "vf:n:")) != -1)
    {
        switch (c)
        {
        case 'v':
        {
            //vflag = 1;
            break;
        }
        case 'f':
        {
            fvalue = optarg;
            break;
        }
        case 'n':
        {
            nvalue = optarg;
            break;
        }
        case '?':
        {
            if (optopt == 'd' || optopt == 'f' || optopt == 'n')
            {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
            }
            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            abort ();
            return -1;//for parasoft
        }
        }
    }

    if (fvalue)
    {
        /* DLT is intialised automatically, except another output target will be used */
        if (dlt_init_file(fvalue)<0) /* log to file */
        {
            return -1;
        }
    }

    if (nvalue)
    {
        maxnum = atoi(nvalue);
    }
    else
    {
        maxnum = 1;
    }

    /* Register APP */
    DLT_REGISTER_APP("DIFT","DLT Interface Test");

    /* Register CONTEXTS... */
    DLT_REGISTER_CONTEXT(context_info,"INFO","Information context");

    /* used for macro interface tests */
    DLT_REGISTER_CONTEXT(context_macro_callback,"CBM","Callback Test context for macro interface");
    for (i=0;i<DLT_TEST_NUM_CONTEXT;i++)
    {
        snprintf(ctid,4,"TM%d",i+1);
        snprintf(ctdesc,255,"Test %d context for macro interface",i+1);
        DLT_REGISTER_CONTEXT(context_macro_test[i],ctid,ctdesc);
    }

    /* used for function interface tests */
    dlt_register_context(&context_function_callback,"CBF","Callback Test context for function interface");
    for (i=0;i<DLT_TEST_NUM_CONTEXT;i++)
    {
        snprintf(ctid,4,"TF%d",i+1);
        snprintf(ctdesc,255,"Test %d context for function interface",i+1);
        dlt_register_context(&(context_function_test[i]),ctid,ctdesc);
    }

    /* Register callbacks... */

    /* with macro interface */
    DLT_LOG(context_macro_callback,DLT_LOG_INFO,DLT_STRING("Register callback (Macro Interface) for Injection ID: 0xFFF"));
    DLT_REGISTER_INJECTION_CALLBACK(context_macro_callback, 0xFFF, test_injection_macro_callback);

    /* with function interface */
    if (dlt_user_log_write_start(&context_function_callback,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Register callback (Function Interface) for Injection ID: 0xFFF");
        dlt_user_log_write_finish(&context_data);
    }
    dlt_register_injection_callback(&context_function_callback, 0xFFF, test_injection_function_callback);

    /* Tests starting */
    printf("Tests starting\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Tests starting"));

    /* wait 3 seconds before starting */
    sleep(3);

    for (num=0;num<maxnum;num++)
    {
        /* Execute tests... */

        /* with macro interface */
        test1m();
        test2m();
        test3m();
        test4m();
        test5m();
        test6m();
        test7m();
        test8m();
        test9m();

        /* with function interface */
        test1f();
        test2f();
        test3f();
        test4f();
        test5f();
        test6f();
        test7f();
        test8f();
        test9f();

        /* wait 1 second before next repeat of tests */
        sleep(1);
    }

    /* Tests finished */
    printf("Tests finished\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Tests finished"));

    /* wait 3 seconds before terminating application */
    sleep(3);

    /* Unregister CONTEXTS... */
    DLT_UNREGISTER_CONTEXT(context_info);

    /* used for macro interface tests */
    for (i=0;i<DLT_TEST_NUM_CONTEXT;i++)
    {
        DLT_UNREGISTER_CONTEXT(context_macro_test[i]);
    }
    DLT_UNREGISTER_CONTEXT(context_macro_callback);

    /* used for function interface tests */
    for (i=0;i<DLT_TEST_NUM_CONTEXT;i++)
    {
        dlt_unregister_context(&(context_function_test[i]));
    }
    dlt_unregister_context(&context_function_callback);

    /* Unregister APP */
    DLT_UNREGISTER_APP();

    return 0;
}

/******************/
/* The test cases */
/******************/

int test1m(void)
{
    /* Test 1: (Macro IF) Test all log levels */
    printf("Test1m: (Macro IF) Test all log levels\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test1: (Macro IF) Test all log levels"));

    DLT_LOG(context_macro_test[0],DLT_LOG_FATAL,DLT_STRING("fatal"));
    DLT_LOG(context_macro_test[0],DLT_LOG_ERROR,DLT_STRING("error"));
    DLT_LOG(context_macro_test[0],DLT_LOG_WARN,DLT_STRING("warn"));
    DLT_LOG(context_macro_test[0],DLT_LOG_INFO,DLT_STRING("info"));
    DLT_LOG(context_macro_test[0],DLT_LOG_DEBUG,DLT_STRING("debug"));
    DLT_LOG(context_macro_test[0],DLT_LOG_VERBOSE,DLT_STRING("verbose"));

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test1: (Macro IF) finished"));

    return 0;
}

int test2m(void)
{
    char buffer[10];
    int num2;

    /* Test 2: (Macro IF) Test all variable types (verbose) */
    printf("Test2m: (Macro IF) Test all variable types (verbose)\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test2: (Macro IF) Test all variable types (verbose)"));

    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("string"),DLT_STRING("Hello world"));
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("utf8"),DLT_UTF8("Hello world"));
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("bool"),DLT_BOOL(1));
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("int"),DLT_INT(INT32_MIN));         /* (-2147483647-1) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("int8"),DLT_INT8(INT8_MIN));        /*          (-128) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("int16"),DLT_INT16(INT16_MIN));     /*      (-32767-1) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("int32"),DLT_INT32(INT32_MIN));     /* (-2147483647-1) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("int64"),DLT_INT64(INT64_MIN));     /* (-__INT64_C(9223372036854775807)-1) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("uint"),DLT_UINT(UINT32_MAX));      /*   (4294967295U) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("uint8"),DLT_UINT8(UINT8_MAX));     /*           (255) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("uint16"),DLT_UINT16(UINT16_MAX));  /*         (65535) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("uint32"),DLT_UINT32(UINT32_MAX));  /*   (4294967295U) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("uint64"),DLT_UINT64(UINT64_MAX));  /* (__UINT64_C(18446744073709551615)) */
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("float32"),DLT_FLOAT32(FLT_MIN),DLT_FLOAT32(FLT_MAX));
    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("float64"),DLT_FLOAT64(DBL_MIN),DLT_FLOAT64(DBL_MAX));

    for(num2=0;num2<10;num2++)
    {
        buffer[num2] = num2;
    }

    DLT_LOG(context_macro_test[1],DLT_LOG_INFO,DLT_STRING("raw"),DLT_RAW(buffer,10));

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test2: (Macro IF) finished"));

    return 0;
}

int test3m(void)
{
    char buffer[10];
    int num2;

    /* Test 3: (Macro IF) Test all variable types (non-verbose) */
    printf("Test3m: (Macro IF) Test all variable types (non-verbose)\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test3: (Macro IF) Test all variable types (non-verbose)"));

    DLT_NONVERBOSE_MODE();

    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,1,DLT_STRING("string"),DLT_STRING("Hello world"));
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,2,DLT_STRING("utf8"),DLT_UTF8("Hello world"));
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,3,DLT_STRING("bool"),DLT_BOOL(1));
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,4,DLT_STRING("int"),DLT_INT(INT32_MIN));         /* (-2147483647-1) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,5,DLT_STRING("int8"),DLT_INT8(INT8_MIN));        /*          (-128) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,6,DLT_STRING("int16"),DLT_INT16(INT16_MIN));     /*      (-32767-1) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,7,DLT_STRING("int32"),DLT_INT32(INT32_MIN));     /* (-2147483647-1) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,8,DLT_STRING("int64"),DLT_INT64(INT64_MIN));     /* (-__INT64_C(9223372036854775807)-1) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,9,DLT_STRING("uint"),DLT_UINT(UINT32_MAX));      /*   (4294967295U) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,10,DLT_STRING("uint8"),DLT_UINT8(UINT8_MAX));     /*           (255) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,11,DLT_STRING("uint16"),DLT_UINT16(UINT16_MAX));  /*         (65535) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,12,DLT_STRING("uint32"),DLT_UINT32(UINT32_MAX));  /*   (4294967295U) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,13,DLT_STRING("uint64"),DLT_UINT64(UINT64_MAX));  /* (__UINT64_C(18446744073709551615)) */
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,14,DLT_STRING("float32"),DLT_FLOAT32(FLT_MIN),DLT_FLOAT32(FLT_MAX));
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,15,DLT_STRING("float64"),DLT_FLOAT64(DBL_MIN),DLT_FLOAT64(DBL_MAX));


    for(num2=0;num2<10;num2++)
    {
        buffer[num2] = num2;
    }
    DLT_LOG_ID(context_macro_test[2],DLT_LOG_INFO,14,DLT_STRING("raw"),DLT_RAW(buffer,10));

    DLT_VERBOSE_MODE();

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test3: (Macro IF) finished"));

    return 0;
}

int test4m(void)
{
    char buffer[1024];
    int num;

    for(num=0;num<1024;num++)
    {
        buffer[num] = num;
    }

    /* Test 4: (Macro IF) Message size test */
    printf("Test4m: (Macro IF) Test different message sizes\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test4: (Macro IF) Test different message sizes"));

    DLT_LOG(context_macro_test[3],DLT_LOG_INFO,DLT_STRING("1"),DLT_RAW(buffer,1));
    DLT_LOG(context_macro_test[3],DLT_LOG_INFO,DLT_STRING("16"),DLT_RAW(buffer,16));
    DLT_LOG(context_macro_test[3],DLT_LOG_INFO,DLT_STRING("256"),DLT_RAW(buffer,256));
    DLT_LOG(context_macro_test[3],DLT_LOG_INFO,DLT_STRING("1024"),DLT_RAW(buffer,1024));

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test4: (Macro IF) finished"));

    return 0;
}

int test5m(void)
{
    char buffer[32];
    int num;

    void *ptr = malloc(sizeof(int));

    for(num=0;num<32;num++)
    {
        buffer[num] = num;
    }

    /* Test 5: (Macro IF) Test high-level API */
    printf("Test5m: (Macro IF) Test high-level API\n");
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test5: (Macro IF) Test high-level API"));

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_INT"));
    DLT_LOG_INT(context_macro_test[4], DLT_LOG_INFO, -42);

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_UINT"));
    DLT_LOG_UINT(context_macro_test[4], DLT_LOG_INFO, 42);

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_STRING"));
    DLT_LOG_STRING(context_macro_test[4], DLT_LOG_INFO, "String output");

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_RAW"));
    DLT_LOG_RAW(context_macro_test[4],DLT_LOG_INFO, buffer,16);

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_STRING_INT"));
    DLT_LOG_STRING_INT(context_macro_test[4], DLT_LOG_INFO, "String output: ", -42);

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_STRING_UINT"));
    DLT_LOG_STRING_UINT(context_macro_test[4], DLT_LOG_INFO,"String output: ", 42);

    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_STRING("Next line: DLT_LOG_PTR"));
    DLT_LOG(context_macro_test[4],DLT_LOG_INFO,DLT_PTR(ptr));
    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test5: (Macro IF) finished"));

    free(ptr);
    return 0;
}

int test6m(void)
{
    /* Test 6: (Macro IF) Test local printing */
    printf("Test6m: (Macro IF) Test local printing\n");
    DLT_LOG_STRING(context_info, DLT_LOG_INFO, "Test 6: (Macro IF) Test local printing");

    DLT_ENABLE_LOCAL_PRINT();
    DLT_LOG_STRING(context_macro_test[5], DLT_LOG_INFO, "Message (visible: locally printed)");

    DLT_DISABLE_LOCAL_PRINT();
    DLT_LOG_STRING(context_macro_test[5], DLT_LOG_INFO, "Message (invisible: not locally printed)");

     /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test6: (Macro IF) finished"));

    return 0;
}

int test7m(void)
{
    char buffer[32];
    int num;

    for(num=0;num<32;num++)
    {
        buffer[num] = num;
    }

    /* Show all log messages and traces */
    DLT_SET_APPLICATION_LL_TS_LIMIT(DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);

    /* Test 7: (Macro IF) Test network trace */
    printf("Test7m: (Macro IF) Test network trace\n");
    DLT_LOG_STRING(context_info, DLT_LOG_INFO, "Test 7: (Macro IF) Test network trace");

    /* Dummy messages: 16 byte header, 32 byte payload */
    DLT_TRACE_NETWORK(context_macro_test[6], DLT_NW_TRACE_IPC, 16, buffer, 32, buffer);
    DLT_TRACE_NETWORK(context_macro_test[6], DLT_NW_TRACE_CAN, 16, buffer, 32, buffer);
    DLT_TRACE_NETWORK(context_macro_test[6], DLT_NW_TRACE_FLEXRAY, 16, buffer, 32, buffer);
    DLT_TRACE_NETWORK(context_macro_test[6], DLT_NW_TRACE_MOST, 16, buffer, 32, buffer);

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test7: (Macro IF) finished"));

    DLT_SET_APPLICATION_LL_TS_LIMIT(DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT);
    sleep(2);

    return 0;
}

int test8m(void)
{
    char buffer[1024*5];
    int num;

    for(num=0;num<1024*5;num++)
    {
        buffer[num] = num;
    }

    /* Show all log messages and traces */
    DLT_SET_APPLICATION_LL_TS_LIMIT(DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);

    /* Test 8: (Macro IF) Test truncated network trace*/
    printf("Test8m: (Macro IF) Test truncated network trace\n");
    DLT_LOG_STRING(context_info, DLT_LOG_INFO, "Test 8: (Macro IF) Test truncated network trace");

    /* Dummy messages: 16 byte header, 5k payload */
    DLT_TRACE_NETWORK_TRUNCATED(context_macro_test[7], DLT_NW_TRACE_IPC, 16, buffer, 1024*5, buffer);
    DLT_TRACE_NETWORK_TRUNCATED(context_macro_test[7], DLT_NW_TRACE_CAN, 16, buffer, 1024*5, buffer);
    DLT_TRACE_NETWORK_TRUNCATED(context_macro_test[7], DLT_NW_TRACE_FLEXRAY, 16, buffer, 1024*5, buffer);
    DLT_TRACE_NETWORK_TRUNCATED(context_macro_test[7], DLT_NW_TRACE_MOST, 16, buffer, 1024*5, buffer);

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test8: (Macro IF) finished"));

    DLT_SET_APPLICATION_LL_TS_LIMIT(DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT);
    sleep(2);

    return 0;
}

int test9m(void)
{
    char buffer[1024*5];
    int num;

    for(num=0;num<1024*5;num++)
    {
        buffer[num] = num;
    }

    /* Show all log messages and traces */
    DLT_SET_APPLICATION_LL_TS_LIMIT(DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);

    /* Test 9: (Macro IF) Test segmented network trace*/
    printf("Test9m: (Macro IF) Test segmented  network trace\n");
    DLT_LOG_STRING(context_info, DLT_LOG_INFO, "Test 9: (Macro IF) Test segmented network trace");

    /* Dummy messages: 16 byte header, 5k payload */
    DLT_TRACE_NETWORK_SEGMENTED(context_macro_test[8], DLT_NW_TRACE_IPC, 16, buffer, 1024*5, buffer);
    DLT_TRACE_NETWORK_SEGMENTED(context_macro_test[8], DLT_NW_TRACE_CAN, 16, buffer, 1024*5, buffer);
    DLT_TRACE_NETWORK_SEGMENTED(context_macro_test[8], DLT_NW_TRACE_FLEXRAY, 16, buffer, 1024*5, buffer);
    DLT_TRACE_NETWORK_SEGMENTED(context_macro_test[8], DLT_NW_TRACE_MOST, 16, buffer, 1024*5, buffer);

    /* wait 2 second before next test */
    sleep(2);
    DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test9: (Macro IF) finished"));

    DLT_SET_APPLICATION_LL_TS_LIMIT(DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT);
    sleep(2);

    return 0;
}

int test1f(void)
{
    /* Test 1: (Function IF) Test all log levels */
    printf("Test1f: (Function IF) Test all log levels\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test1: (Function IF) Test all log levels");
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start(&(context_function_test[0]),&context_data,DLT_LOG_FATAL)>0)
    {
        dlt_user_log_write_string(&context_data,"fatal");
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[0]),&context_data,DLT_LOG_ERROR)>0)
    {
        dlt_user_log_write_string(&context_data,"error");
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[0]),&context_data,DLT_LOG_WARN)>0)
    {
        dlt_user_log_write_string(&context_data,"warn");
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[0]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"info");
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[0]),&context_data,DLT_LOG_DEBUG)>0)
    {
        dlt_user_log_write_string(&context_data,"debug");
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[0]),&context_data,DLT_LOG_VERBOSE)>0)
    {
        dlt_user_log_write_string(&context_data,"verbose");
        dlt_user_log_write_finish(&context_data);
    }

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test1: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    return 0;
}

int test2f(void)
{
    char buffer[10];
    int num2;

    /* Test 2: (Function IF) Test all variable types (verbose) */
    printf("Test2f: (Function IF) Test all variable types (verbose)\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test2: (Function IF) Test all variable types (verbose)");
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"bool");
        dlt_user_log_write_bool(&context_data,1);
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"int");
        dlt_user_log_write_int(&context_data,INT32_MIN);        /* (-2147483647-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"int8");
        dlt_user_log_write_int8(&context_data,INT8_MIN);        /*          (-128) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"int16");
        dlt_user_log_write_int16(&context_data,INT16_MIN);     /*      (-32767-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"int32");
        dlt_user_log_write_int32(&context_data,INT32_MIN);     /* (-2147483647-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"int64");
        dlt_user_log_write_int64(&context_data,INT64_MIN);     /* (-__INT64_C(9223372036854775807)-1) */
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"uint");
        dlt_user_log_write_uint(&context_data,UINT32_MAX);     /*   (4294967295U) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"uint8");
        dlt_user_log_write_uint8(&context_data,UINT8_MAX);     /*           (255) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"uint16");
        dlt_user_log_write_uint16(&context_data,UINT16_MAX);   /*         (65535) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"uint32");
        dlt_user_log_write_uint32(&context_data,UINT32_MAX);   /*   (4294967295U) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"uint64");
        dlt_user_log_write_uint64(&context_data,UINT64_MAX);   /* (__UINT64_C(18446744073709551615)) */
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"float32");
        dlt_user_log_write_float32(&context_data,FLT_MIN);
        dlt_user_log_write_float32(&context_data,FLT_MAX);
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"float64");
        dlt_user_log_write_float64(&context_data,DBL_MIN);
        dlt_user_log_write_float64(&context_data,DBL_MAX);
        dlt_user_log_write_finish(&context_data);
    }

    for(num2=0;num2<10;num2++)
    {
        buffer[num2] = num2;
    }

    if (dlt_user_log_write_start(&(context_function_test[1]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"raw");
        dlt_user_log_write_raw(&context_data,buffer,10);
        dlt_user_log_write_finish(&context_data);
    }

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test2: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    return 0;
}

int test3f(void)
{
    char buffer[10];
    int num2;

    /* Test 3: (Function IF) Test all variable types (non-verbose) */
    printf("Test3f: (Function IF) Test all variable types (non-verbose)\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test3: (Function IF) Test all variable types (non-verbose)");
        dlt_user_log_write_finish(&context_data);
    }

    dlt_nonverbose_mode();

    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,1)>0) // bug mb: we have to compare against >0. in case of error -1 is returned!
    {
        dlt_user_log_write_string(&context_data,"bool");
        dlt_user_log_write_bool(&context_data,1);
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,2)>0)
    {
        dlt_user_log_write_string(&context_data,"int");
        dlt_user_log_write_int(&context_data,INT32_MIN);        /* (-2147483647-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,3)>0)
    {
        dlt_user_log_write_string(&context_data,"int8");
        dlt_user_log_write_int8(&context_data,INT8_MIN);        /*          (-128) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,4)>0)
    {
        dlt_user_log_write_string(&context_data,"int16");
        dlt_user_log_write_int16(&context_data,INT16_MIN);     /*      (-32767-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,5)>0)
    {
        dlt_user_log_write_string(&context_data,"int32");
        dlt_user_log_write_int32(&context_data,INT32_MIN);     /* (-2147483647-1) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,6)>0)
    {
        dlt_user_log_write_string(&context_data,"int64");
        dlt_user_log_write_int64(&context_data,INT64_MIN);     /* (-__INT64_C(9223372036854775807)-1) */
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,7)>0)
    {
        dlt_user_log_write_string(&context_data,"uint");
        dlt_user_log_write_uint(&context_data,UINT32_MAX);     /*   (4294967295U) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,8)>0)
    {
        dlt_user_log_write_string(&context_data,"uint8");
        dlt_user_log_write_uint8(&context_data,UINT8_MAX);     /*           (255) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,9)>0)
    {
        dlt_user_log_write_string(&context_data,"uint16");
        dlt_user_log_write_uint16(&context_data,UINT16_MAX);   /*         (65535) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,10)>0)
    {
        dlt_user_log_write_string(&context_data,"uint32");
        dlt_user_log_write_uint32(&context_data,UINT32_MAX);   /*   (4294967295U) */
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,11)>0)
    {
        dlt_user_log_write_string(&context_data,"uint64");
        dlt_user_log_write_uint64(&context_data,UINT64_MAX);   /* (__UINT64_C(18446744073709551615)) */
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,12)>0)
    {
        dlt_user_log_write_string(&context_data,"float32");
        dlt_user_log_write_float32(&context_data,FLT_MIN);
        dlt_user_log_write_float32(&context_data,FLT_MAX);
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,13)>0)
    {
        dlt_user_log_write_string(&context_data,"float64");
        dlt_user_log_write_float64(&context_data,DBL_MIN);
        dlt_user_log_write_float64(&context_data,DBL_MAX);
        dlt_user_log_write_finish(&context_data);
    }

    for(num2=0;num2<10;num2++)
    {
        buffer[num2] = num2;
    }

    if (dlt_user_log_write_start_id(&(context_function_test[2]),&context_data,DLT_LOG_INFO,14)>0)
    {
        dlt_user_log_write_string(&context_data,"raw");
        dlt_user_log_write_raw(&context_data,buffer,10);
        dlt_user_log_write_finish(&context_data);
    }

    dlt_verbose_mode();

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test3: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    return 0;
}

int test4f(void)
{
    char buffer[1024];
    int num;

    for(num=0;num<1024;num++)
    {
        buffer[num] = num;
    }

    /* Test 4: (Function IF) Message size test */
    printf("Test4f: (Function IF) Test different message sizes\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test4: (Function IF) Test different message sizes");
        dlt_user_log_write_finish(&context_data);
    }

    if (dlt_user_log_write_start(&(context_function_test[3]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"1");
        dlt_user_log_write_raw(&context_data,buffer,1);
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[3]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"16");
        dlt_user_log_write_raw(&context_data,buffer,16);
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[3]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"256");
        dlt_user_log_write_raw(&context_data,buffer,256);
        dlt_user_log_write_finish(&context_data);
    }
    if (dlt_user_log_write_start(&(context_function_test[3]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"1024");
        dlt_user_log_write_raw(&context_data,buffer,1024);
        dlt_user_log_write_finish(&context_data);
    }

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test4: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    return 0;
}

int test5f(void)
{
    char buffer[32];
    int num;

    for(num=0;num<32;num++)
    {
        buffer[num] = num;
    }

    /* Test 5: (Function IF) Test high-level API */
    printf("Test5f: (Function IF) Test high-level API\n");
    dlt_log_string(&context_info,DLT_LOG_INFO,"Test5: (Function IF) Test high-level API");

    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO,"Next line: dlt_log_int()");
    dlt_log_int(&(context_function_test[4]),DLT_LOG_INFO, -42);

    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO,"Next line: dlt_log_uint()");
    dlt_log_uint(&(context_function_test[4]),DLT_LOG_INFO, 42);

    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO,"Next line: dlt_log_string()");
    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO, "String output");

    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO,"Next line: dlt_log_raw()");
    dlt_log_raw(&(context_function_test[4]),DLT_LOG_INFO, buffer,16);

    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO,"Next line: dlt_log_string_int()");
    dlt_log_string_int(&(context_function_test[4]), DLT_LOG_INFO, "String output: ", -42);

    dlt_log_string(&(context_function_test[4]),DLT_LOG_INFO,"Next line: dlt_log_string_uint()");
    dlt_log_string_uint(&(context_function_test[4]), DLT_LOG_INFO,"String output: ", 42);

    /* wait 2 second before next test */
    sleep(2);
    dlt_log_string(&context_info,DLT_LOG_INFO,"Test5: (Function IF) finished");

    return 0;
}

int test6f(void)
{
    /* Test 6: (Function IF) Test local printing */
    printf("Test6f: (Function IF) Test local printing\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test 6: (Function IF) Test local printing");
        dlt_user_log_write_finish(&context_data);
    }

    dlt_enable_local_print();
    if (dlt_user_log_write_start(&(context_function_test[5]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Message (visible: locally printed)");
        dlt_user_log_write_finish(&context_data);
    }

    dlt_disable_local_print();
    if (dlt_user_log_write_start(&(context_function_test[5]),&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Message (invisible: not locally printed)");
        dlt_user_log_write_finish(&context_data);
    }

     /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test6: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    return 0;
}

int test7f(void)
{
    char buffer[32];
    int num;

    for(num=0;num<32;num++)
    {
        buffer[num] = num;
    }

    /* Show all log messages and traces */
    dlt_set_application_ll_ts_limit(DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);

    /* Test 7: (Function IF) Test network trace */
    printf("Test7f: (Function IF) Test network trace\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test 7: (Function IF) Test network trace");
        dlt_user_log_write_finish(&context_data);
    }

    /* Dummy message: 16 byte header, 32 byte payload */
    dlt_user_trace_network(&(context_function_test[6]), DLT_NW_TRACE_IPC, 16, buffer, 32, buffer);
    dlt_user_trace_network(&(context_function_test[6]), DLT_NW_TRACE_CAN, 16, buffer, 32, buffer);
    dlt_user_trace_network(&(context_function_test[6]), DLT_NW_TRACE_FLEXRAY, 16, buffer, 32, buffer);
    dlt_user_trace_network(&(context_function_test[6]), DLT_NW_TRACE_MOST, 16, buffer, 32, buffer);

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test7: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    dlt_set_application_ll_ts_limit(DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT);
    sleep(2);

    return 0;
}

int test8f(void)
{
    char buffer[1024*5];
    int num;

    for(num=0;num<1024*5;num++)
    {
        buffer[num] = num;
    }

    /* Show all log messages and traces */
    dlt_set_application_ll_ts_limit(DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);

    /* Test 8: (Function IF) Test truncated network trace */
    printf("Test8f: (Function IF) Test truncated network trace\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test 8: (Function IF) Test truncated network trace");
        dlt_user_log_write_finish(&context_data);
    }

    /* Dummy message: 16 byte header, 32 byte payload */
    dlt_user_trace_network_truncated(&(context_function_test[7]), DLT_NW_TRACE_IPC, 16, buffer, 1024*5, buffer, 1);
    dlt_user_trace_network_truncated(&(context_function_test[7]), DLT_NW_TRACE_CAN, 16, buffer, 1024*5, buffer, 1);
    dlt_user_trace_network_truncated(&(context_function_test[7]), DLT_NW_TRACE_FLEXRAY, 16, buffer, 1024*5, buffer, 1);
    dlt_user_trace_network_truncated(&(context_function_test[7]), DLT_NW_TRACE_MOST, 16, buffer, 1024*5, buffer, 1);

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test8: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    dlt_set_application_ll_ts_limit(DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT);
    sleep(2);

    return 0;
}

int test9f(void)
{
    char buffer[1024*5];
    int num;

    for(num=0;num<1024*5;num++)
    {
        buffer[num] = num;
    }

    /* Show all log messages and traces */
    dlt_set_application_ll_ts_limit(DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON);

    /* Test 9: (Function IF) Test segmented network trace */
    printf("Test9f: (Function IF) Test segmented network trace\n");
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test 9: (Function IF) Test segmented network trace");
        dlt_user_log_write_finish(&context_data);
    }

    /* Dummy message: 16 byte header, 5k payload */
    dlt_user_trace_network_segmented(&(context_function_test[8]), DLT_NW_TRACE_IPC, 16, buffer, 1024*5, buffer);
    dlt_user_trace_network_segmented(&(context_function_test[8]), DLT_NW_TRACE_CAN, 16, buffer, 1024*5, buffer);
    dlt_user_trace_network_segmented(&(context_function_test[8]), DLT_NW_TRACE_FLEXRAY, 16, buffer, 1024*5, buffer);
    dlt_user_trace_network_segmented(&(context_function_test[8]), DLT_NW_TRACE_MOST, 16, buffer, 1024*5, buffer);

    /* wait 2 second before next test */
    sleep(2);
    if (dlt_user_log_write_start(&context_info,&context_data,DLT_LOG_INFO)>0)
    {
        dlt_user_log_write_string(&context_data,"Test9: (Function IF) finished");
        dlt_user_log_write_finish(&context_data);
    }

    dlt_set_application_ll_ts_limit(DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT);
    sleep(2);

    return 0;
}

int test_injection_macro_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    memset(text,0,1024);
    snprintf(text,1024,"Injection received (macro IF). ID: 0x%.4x, Length: %d",service_id,length);
    printf("%s \n", text);
    DLT_LOG(context_macro_callback, DLT_LOG_INFO, DLT_STRING("Injection received (macro IF). ID: "), DLT_UINT32(service_id),DLT_STRING("Data:"),DLT_STRING(text));
    memset(text,0,1024);

    if (length>0)
    {
        dlt_print_mixed_string(text,1024,data,length,0);
        printf("%s \n", text);
    }

    return 0;
}

int test_injection_function_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    memset(text,0,1024);

    snprintf(text,1024,"Injection received (function IF). ID: 0x%.4x, Length: %d",service_id,length);
    printf("%s \n", text);
    DLT_LOG(context_function_callback, DLT_LOG_INFO, DLT_STRING("Injection received (function IF). ID: "), DLT_UINT32(service_id),DLT_STRING("Data:"),DLT_STRING(text));
    memset(text,0,1024);

    if (length>0)
    {
        dlt_print_mixed_string(text,1024,data,length,0);
        printf("%s \n", text);
    }

    return 0;
}


