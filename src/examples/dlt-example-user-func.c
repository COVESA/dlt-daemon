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
 * \file dlt-example-user-func.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-example-user-func.cpp                                     **
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

DltContext mycontext;
DltContextData mycontextdata;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-example-user-func [options] message\n");
    printf("Generate DLT messages and store them to file or send them to daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -d delay      Milliseconds to wait between sending messages (Default: 500)\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -n count      Number of messages to be generated (Default: 10)\n");
    printf("  -g            Switch to non-verbose mode (Default: verbose mode)\n");
    printf("  -a            Enable local printing of DLT messages (Default: disabled)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    int gflag = 0;
    int aflag = 0;
    char *dvalue = 0;
    char *fvalue = 0;
    char *nvalue = 0;
    char *message = 0;

    int index;
    int c;
    char *text;
    int num, maxnum;
    int delay;
    struct timespec ts;

    opterr = 0;

    while ((c = getopt (argc, argv, "vgad:f:n:")) != -1)
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
        case 'n':
        {
            nvalue = optarg;
            break;
        }
        case '?':
        {
            if ((optopt == 'd') || (optopt == 'f') || (optopt == 'n'))
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
            return -1;/*for parasoft */
        }
        }



    for (index = optind; index < argc; index++)
        message = argv[index];

    if (message == 0) {
        /* no message, show usage and terminate */
        fprintf(stderr, "ERROR: No message selected\n");
        usage();
        return -1;
    }

    if (fvalue) {
        /* DLT is intialised automatically, except another output target will be used */
        if (dlt_init_file(fvalue) < 0) /* log to file */
            return -1;
    }

    dlt_register_app("LOG", "Test Application for Logging");

    dlt_register_context(&mycontext, "TEST", "Test Context for Logging");

    dlt_register_injection_callback(&mycontext, 0xFFF, dlt_user_injection_callback);

    text = message;

    if (gflag)
        dlt_nonverbose_mode();

    if (aflag)
        dlt_enable_local_print();

    if (nvalue)
        maxnum = atoi(nvalue);
    else
        maxnum = 10;

    if (dvalue)
        delay = atoi(dvalue) * 1000000;
    else
        delay = 500 * 1000000;

    if (gflag) {
        /* DLT messages to test Fibex non-verbose description: dlt-example-non-verbose.xml */
        if (dlt_user_log_write_start_id(&mycontext, &mycontextdata, DLT_LOG_INFO, 10) > 0)
            dlt_user_log_write_finish(&mycontextdata);

        if (dlt_user_log_write_start_id(&mycontext, &mycontextdata, DLT_LOG_INFO, 11) > 0) {
            dlt_user_log_write_uint16(&mycontextdata, 1011);
            dlt_user_log_write_finish(&mycontextdata);
        }

        if (dlt_user_log_write_start_id(&mycontext, &mycontextdata, DLT_LOG_INFO, 12) > 0) {
            dlt_user_log_write_uint32(&mycontextdata, 1012);
            dlt_user_log_write_uint32(&mycontextdata, 1013);
            dlt_user_log_write_finish(&mycontextdata);
        }

        if (dlt_user_log_write_start_id(&mycontext, &mycontextdata, DLT_LOG_INFO, 13) > 0) {
            dlt_user_log_write_uint8(&mycontextdata, 123);
            dlt_user_log_write_float32(&mycontextdata, 1.12);
            dlt_user_log_write_finish(&mycontextdata);
        }

        if (dlt_user_log_write_start_id(&mycontext, &mycontextdata, DLT_LOG_INFO, 14) > 0) {
            dlt_user_log_write_string(&mycontextdata, "DEAD BEEF");
            dlt_user_log_write_finish(&mycontextdata);
        }
    }

    for (num = 0; num < maxnum; num++) {
        printf("Send %d %s\n", num, text);

        if (gflag) {
            /* Non-verbose mode */
            if (dlt_user_log_write_start_id(&mycontext, &mycontextdata, DLT_LOG_WARN, num) > 0) {
                dlt_user_log_write_int(&mycontextdata, num);
                dlt_user_log_write_string(&mycontextdata, text);
                dlt_user_log_write_finish(&mycontextdata);
            }
        }
        else
        /* Verbose mode */
        if (dlt_user_log_write_start(&mycontext, &mycontextdata, DLT_LOG_WARN) > 0) {
            dlt_user_log_write_int(&mycontextdata, num);
            dlt_user_log_write_string(&mycontextdata, text);
            dlt_user_log_write_finish(&mycontextdata);
        }

        if (delay > 0) {
            ts.tv_sec = delay / 1000000000;
            ts.tv_nsec = delay % 1000000000;
            nanosleep(&ts, NULL);
        }
    }

    dlt_unregister_context(&mycontext);

    dlt_unregister_app();

    return 0;
}

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    printf("Injection %d, Length=%d \n", service_id, length);

    if (length > 0) {
        dlt_print_mixed_string(text, 1024, data, length, 0);
        printf("%s \n", text);
    }

    return 0;
}
