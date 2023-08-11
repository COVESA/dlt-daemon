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
 * \file dlt-example-user.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-example-user.c                                            **
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
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 */
#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#include "dlt.h"
#include "dlt_common.h" /* for dlt_get_version() */

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length);
int dlt_user_injection_callback_with_specific_data(uint32_t service_id, void *data, uint32_t length, void *priv_data);

void dlt_user_log_level_changed_callback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t trace_status);

DLT_DECLARE_CONTEXT(mycontext1)
DLT_DECLARE_CONTEXT(mycontext2)
DLT_DECLARE_CONTEXT(mycontext3)

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-example-user [options] message\n");
    printf("Generate DLT messages and store them to file or send them to daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -d delay      Milliseconds to wait between sending messages (Default: 500)\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -S filesize   Set maximum size of local log file (Default: UINT_MAX)\n");
    printf("  -n count      Number of messages to be generated (Default: 10)\n");
    printf("  -g            Switch to non-verbose mode (Default: verbose mode)\n");
    printf("  -a            Enable local printing of DLT messages (Default: disabled)\n");
    printf("  -k            Send marker message\n");
    printf("  -m mode       Set log mode 0=off, 1=external, 2=internal, 3=both\n");
    printf("  -l level      Set log level to <level>, level=-1..6\n");
    printf("  -C ContextID  Set context ID for send message (Default: TEST)\n");
    printf("  -A AppID      Set app ID for send message (Default: LOG)\n");
    printf("  -t timeout    Set timeout when sending messages at exit, in ms (Default: 10000 = 10sec)\n");
    printf("  -r size       Send raw data with specified size instead of string\n");
#ifdef DLT_TEST_ENABLE
    printf("  -c            Corrupt user header\n");
    printf("  -s size       Corrupt message size\n");
    printf("  -z size          Size of message\n");
#endif /* DLT_TEST_ENABLE */
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    int gflag = 0;
    int aflag = 0;
    int kflag = 0;
#ifdef DLT_TEST_ENABLE
    int cflag = 0;
    char *svalue = 0;
    char *zvalue = 0;
#endif /* DLT_TEST_ENABLE */
    char *dvalue = 0;
    char *fvalue = 0;
    unsigned int filesize = 0;
    char *nvalue = 0;
    char *mvalue = 0;
    char *message = 0;
    int lvalue = DLT_LOG_WARN;
    char *tvalue = 0;
    int rvalue = -1;
    int index;
    int c;

    char *appID = "LOG";
    char *contextID = "TEST";

    char *text;
    int num, maxnum;
    int delay;
    struct timespec ts;

    int state = -1, newstate;

    opterr = 0;
#ifdef DLT_TEST_ENABLE

    while ((c = getopt (argc, argv, "vgakcd:f:S:n:m:z:r:s:l:t:A:C:")) != -1)
#else

    while ((c = getopt (argc, argv, "vgakd:f:S:n:m:l:r:t:A:C:")) != -1)
#endif /* DLT_TEST_ENABLE */
    {
        switch (c) {
        case 'g':
        {
            gflag = 1;
            break;
        }
        case 'a':
        {
            aflag = 1;
            break;
        }
        case 'k':
        {
            kflag = 1;
            break;
        }
#ifdef DLT_TEST_ENABLE
        case 'c':
        {
            cflag = 1;
            break;
        }
        case 's':
        {
            svalue = optarg;
            break;
        }
        case 'z':
        {
            zvalue = optarg;
            break;
        }
#endif /* DLT_TEST_ENABLE */
        case 'd':
        {
            dvalue = optarg;
            break;
        }
        case 'f':
        {
            fvalue = optarg;
            break;
        }
        case 'S':
        {
            filesize = atoi(optarg);
            break;
        }
        case 'n':
        {
            nvalue = optarg;
            break;
        }
        case 'm':
        {
            mvalue = optarg;
            break;
        }
        case 'l':
        {
            lvalue = atoi(optarg);
            break;
        }
        case 'A':
        {
            appID = optarg;
            break;
        }
        case 'C':
        {
            contextID = optarg;
            break;
        }
        case 't':
        {
            tvalue = optarg;
            break;
        }
        case 'r':
        {
            rvalue = atoi(optarg);
            break;
        }
        case '?':
        {
            if ((optopt == 'd') || (optopt == 'f') || (optopt == 'n') ||
                (optopt == 'l') || (optopt == 't') || (optopt == 'S'))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            abort ();
            break;/*for parasoft */
        }
        }
    }

    if (rvalue == -1) {
        for (index = optind; index < argc; index++)
            message = argv[index];
    }
    else { /* allocate raw buffer */
        message = calloc(sizeof(char), rvalue);
        memset(message, 'X', rvalue - 1);
    }

    if (message == 0) {
        /* no message, show usage and terminate */
        fprintf(stderr, "ERROR: No message selected\n");
        usage();
        return -1;
    }

    if (fvalue) {
        /* DLT is initialized automatically, except another output target will be used */
        if (dlt_init_file(fvalue) < 0) /* log to file */
            return -1;
    }

    if (filesize != 0) {
        if (dlt_set_filesize_max(filesize) < 0)
            return -1;
    }

    dlt_with_session_id(1);
    dlt_with_timestamp(1);
    dlt_with_ecu_id(1);
    dlt_verbose_mode();

    DLT_REGISTER_APP(appID, "Test Application for Logging");
    DLT_REGISTER_CONTEXT(mycontext1, contextID, "Test Context for Logging");
    DLT_REGISTER_CONTEXT_LLCCB(mycontext2, "TS1", "Test Context1 for injection", dlt_user_log_level_changed_callback);
    DLT_REGISTER_CONTEXT_LLCCB(mycontext3, "TS2", "Test Context2 for injection", dlt_user_log_level_changed_callback);


    DLT_REGISTER_INJECTION_CALLBACK(mycontext1, 0x1000, dlt_user_injection_callback);
    DLT_REGISTER_INJECTION_CALLBACK_WITH_ID(mycontext2,
                                            0x1000,
                                            dlt_user_injection_callback_with_specific_data,
                                            (void *)"TS1 context");
    DLT_REGISTER_INJECTION_CALLBACK(mycontext2, 0x1001, dlt_user_injection_callback);
    DLT_REGISTER_INJECTION_CALLBACK_WITH_ID(mycontext3,
                                            0x1000,
                                            dlt_user_injection_callback_with_specific_data,
                                            (void *)"TS2 context");
    DLT_REGISTER_INJECTION_CALLBACK(mycontext3, 0x1001, dlt_user_injection_callback);
    DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK(mycontext1, dlt_user_log_level_changed_callback);

    text = message;

    if (mvalue) {
        printf("Set log mode to %d\n", atoi(mvalue));
        dlt_set_log_mode(atoi(mvalue));
    }

    if (gflag)
        DLT_NONVERBOSE_MODE();

    if (aflag)
        DLT_ENABLE_LOCAL_PRINT();

    if (kflag)
        DLT_LOG_MARKER();

    if (nvalue)
        maxnum = atoi(nvalue);
    else
        maxnum = 10;

    if (dvalue)
        delay = atoi(dvalue);
    else
        delay = 500;

    if (tvalue)
        dlt_set_resend_timeout_atexit(atoi(tvalue));

    if (gflag) {
        /* DLT messages to test Fibex non-verbose description: dlt-example-non-verbose.xml */
        DLT_LOG_ID(mycontext1, DLT_LOG_INFO, 10);
        DLT_LOG_ID(mycontext1, DLT_LOG_INFO, 11, DLT_UINT16(1011));
        DLT_LOG_ID(mycontext1, DLT_LOG_INFO, 12, DLT_UINT32(1012), DLT_UINT32(1013));
        DLT_LOG_ID(mycontext1, DLT_LOG_INFO, 13, DLT_UINT8(123), DLT_FLOAT32(1.12));
        DLT_LOG_ID(mycontext1, DLT_LOG_INFO, 14, DLT_STRING("DEAD BEEF"));
    }

#ifdef DLT_TEST_ENABLE

    if (cflag)
        dlt_user_test_corrupt_user_header(1);

    if (svalue)
        dlt_user_test_corrupt_message_size(1, atoi(svalue));

    if (zvalue) {
        char *buffer = malloc(atoi(zvalue));

        if (buffer == 0) {
            /* no message, show usage and terminate */
            fprintf(stderr, "Cannot allocate buffer memory!\n");
            return -1;
        }

        DLT_LOG(mycontext1, DLT_LOG_WARN, DLT_STRING(text), DLT_RAW(buffer, atoi(zvalue)));
        free(buffer);
    }

#endif /* DLT_TEST_ENABLE */

    for (num = 0; num < maxnum; num++) {
        printf("Send %d %s\n", num, text);

        newstate = dlt_get_log_state();

        if (state != newstate) {
            state = newstate;

            if (state == -1)
                printf("Client unknown state!\n");
            else if (state == 0)
                printf("Client disconnected!\n");
            else if (state == 1)
                printf("Client connected!\n");
        }

        if (gflag) {
            /* Non-verbose mode */
            DLT_LOG_ID(mycontext1, lvalue, num, DLT_INT(num), DLT_STRING(text));
        }
        else {
            if (rvalue == -1)
                /* Verbose mode */
                DLT_LOG(mycontext1, lvalue, DLT_INT(num), DLT_STRING(text));
            else
                DLT_LOG(mycontext1, lvalue, DLT_RAW(text, rvalue));
        }

        if (delay > 0) {
            ts.tv_sec = delay / 1000;
            ts.tv_nsec = (delay % 1000) * 1000000;
            nanosleep(&ts, NULL);
        }
    }

    sleep(1);

    DLT_UNREGISTER_CONTEXT(mycontext1);

    DLT_UNREGISTER_APP();

    return 0;

}

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    DLT_LOG(mycontext1, DLT_LOG_INFO, DLT_STRING("Injection: "), DLT_UINT32(service_id));
    printf("Injection %d, Length=%d \n", service_id, length);

    if (length > 0) {
        dlt_print_mixed_string(text, 1024, data, length, 0);
        DLT_LOG(mycontext1, DLT_LOG_INFO, DLT_STRING("Data: "), DLT_STRING(text));
        printf("%s \n", text);
    }

    return 0;
}

int dlt_user_injection_callback_with_specific_data(uint32_t service_id, void *data, uint32_t length, void *priv_data)
{
    char text[1024];

    DLT_LOG(mycontext1, DLT_LOG_INFO, DLT_STRING("Injection: "), DLT_UINT32(service_id));
    printf("Injection %d, Length=%d \n", service_id, length);

    if (length > 0) {
        dlt_print_mixed_string(text, 1024, data, length, 0);
        DLT_LOG(mycontext1, DLT_LOG_INFO, DLT_STRING("Data: "), DLT_STRING(text), DLT_STRING(priv_data));
        printf("%s \n", text);
    }

    return 0;
}

void dlt_user_log_level_changed_callback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t trace_status)
{
    char text[5];
    text[4] = 0;

    memcpy(text, context_id, DLT_ID_SIZE);

    printf("Log level changed of context %s, LogLevel=%u, TraceState=%u \n", text, log_level, trace_status);
}

