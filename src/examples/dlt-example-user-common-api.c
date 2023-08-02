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
 * \file dlt-example-user-common-api.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-example-common-api.c                                      **
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

#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#include "dlt_common_api.h"

DLT_DECLARE_CONTEXT(mycontext)

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-example-common-api [options] message\n");
    printf("Generate DLT messages and store them to file or send them to daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -d delay      Milliseconds to wait between sending messages (Default: 500)\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -n count      Number of messages to be generated (Default: 10)\n");
    printf("  -g            Switch to non-verbose mode (Default: verbose mode)\n");
    printf("  -a            Enable local printing of DLT messages (Default: disabled)\n");
    printf("  -m mode       Set log mode 0=off,1=external,2=internal,3=both\n");
#ifdef DLT_TEST_ENABLE
    printf("  -c               Corrupt user header\n");
    printf("  -s size       Corrupt message size\n");
    printf("  -z size          Size of message\n");
#endif /* DLT_TEST_ENABLE */
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
#ifdef DLT_TEST_ENABLE
    int cflag = 0;
    char *svalue = 0;
    char *zvalue = 0;
#endif /* DLT_TEST_ENABLE */
    int gflag = 0;
    char *dvalue = 0;
    char *nvalue = 0;
    char *message = 0;

    int index;
    int c;

    char *text;
    int num, maxnum;
    int delay;
    struct timespec ts;

    int state = -1, newstate;

    opterr = 0;
#ifdef DLT_TEST_ENABLE

    while ((c = getopt (argc, argv, "vgcd:n:z:s:")) != -1)
#else

    while ((c = getopt (argc, argv, "vgd:n:")) != -1)
#endif /* DLT_TEST_ENABLE */
    {
        switch (c) {
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
        case 'g':
        {
            gflag = 1;
            break;
        }
        case 'd':
        {
            dvalue = optarg;
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
            break;/*for parasoft */
        }
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

    DLT_REGISTER_APP("LOG", "Test Application for Logging");
    DLT_REGISTER_CONTEXT_APP(mycontext, "TEST", "LOG", "Test Context for Logging");

    text = message;

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
        DLT_LOG_ID0(mycontext, DLT_LOG_INFO, 10);
        DLT_LOG_ID1(mycontext, DLT_LOG_INFO, 11, DLT_UINT16(1011));
        DLT_LOG_ID2(mycontext, DLT_LOG_INFO, 12, DLT_UINT32(1012), DLT_UINT32(1013));
        DLT_LOG_ID2(mycontext, DLT_LOG_INFO, 13, DLT_UINT8(123), DLT_FLOAT32(1.12));
        DLT_LOG_ID1(mycontext, DLT_LOG_INFO, 14, DLT_STRING("DEAD BEEF"));
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

        DLT_LOG2(mycontext, DLT_LOG_WARN, DLT_STRING(text), DLT_RAW(buffer, atoi(zvalue)));
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

        if (gflag)
            /* Non-verbose mode */
            DLT_LOG_ID2(mycontext, DLT_LOG_WARN, num, DLT_INT(num), DLT_STRING(text));
        else
            /* Verbose mode */
            DLT_LOG2(mycontext, DLT_LOG_WARN, DLT_INT(num), DLT_STRING(text));

        if (delay > 0) {
            ts.tv_sec = delay / 1000000000;
            ts.tv_nsec = delay % 1000000000;
            nanosleep(&ts, NULL);
        }
    }

    sleep(1);

    DLT_UNREGISTER_CONTEXT(mycontext);

    DLT_UNREGISTER_APP();

    return 0;

}


