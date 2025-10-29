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
 * \file dlt-test-stress-user.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-stress-user.c                                        **
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

#include <stdio.h>      /* for printf() and fprintf() */
#include <float.h>
#include <stdlib.h>     /* for atoi(), abort() */
#include <string.h>     /* for memset() */
#include <ctype.h>      /* for isprint() */

#include "dlt.h"

#define DLT_TEST_NUM_CONTEXT 7

/* Test functions... */

int testall(int count, int repeat, int delay, int size);

/* Context declaration.. */
DltContext context_info;

DltContextData context_data;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-test-stress-user [options]\n");
    printf("Test user application providing Test messages.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -n count      Number of messages to be sent per test (Default: 10000)\n");
    printf("  -r repeat     How often test is repeated (Default: 100)\n");
    printf("  -d delay      Delay between sent messages in uSec (Default: 1000)\n");
    printf("  -s size       Size of extra message data in bytes (Default: 100)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    /*int vflag = 0; */
    char *fvalue = 0;
    int nvalue = 10000;
    int rvalue = 100;
    int dvalue = 1000;
    int svalue = 100;

    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "vf:n:r:d:s:")) != -1)
        switch (c) {
        case 'v':
        {
            /*vflag = 1; */
            break;
        }
        case 'f':
        {
            fvalue = optarg;
            break;
        }
        case 'n':
        {
            nvalue = atoi(optarg);
            break;
        }
        case 'r':
        {
            rvalue = atoi(optarg);
            break;
        }
        case 'd':
        {
            dvalue = atoi(optarg);
            break;
        }
        case 's':
        {
            svalue = atoi(optarg);
            break;
        }
        case '?':
        {
            if ((optopt == 'f') || (optopt == 'n') || (optopt == 'r') ||
                (optopt == 'd') || (optopt == 's'))
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



    if (fvalue) {
        /* DLT is intialised automatically, except another output target will be used */
        if (dlt_init_file(fvalue) < 0) /* log to file */
            return -1;
    }

    /* Register APP */
    dlt_register_app("DIFT", "DLT Interface Test");

    /* Register CONTEXTS... */
    dlt_register_context(&context_info, "INFO", "Information context");

    /* Tests starting */
    printf("Tests starting\n");
    /*DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Tests starting")); */

    /* wait 3 seconds before starting */
    /*sleep(3); */

    testall(nvalue, rvalue, dvalue, svalue);

    /* Tests finished */
    printf("Tests finished\n");
    /*DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Tests finished")); */

    /* wait 3 seconds before terminating application */
    /*sleep(3); */

    /* Unregister CONTEXTS... */
    dlt_unregister_context(&context_info);

    /* Unregister APP */
    dlt_unregister_app();

    return 0;
}

/******************/
/* The test cases */
/******************/

int testall(int count, int repeat, int delay, int size)
{
    char buffer[size];
    int num, rnum;
    struct timespec ts;

    for (num = 0; num < size; num++)
        buffer[num] = (char) num;

    /* Test All: Test all start */
    /*printf("Test1: Test all\n"); */
    /*DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test1: Test all")); */

    for (rnum = 0; rnum < repeat; rnum++)
        for (num = 1; num <= count; num++) {
            if (dlt_user_log_write_start(&context_info, &context_data, DLT_LOG_INFO) > 0) {
                dlt_user_log_write_int(&context_data, num);
                dlt_user_log_write_raw(&context_data, buffer, size);
                dlt_user_log_write_finish(&context_data);
            }
            ts.tv_sec = (delay * 1000) / 1000000000;
            ts.tv_nsec = (delay * 1000) % 1000000000;
            nanosleep(&ts, NULL);
        }

    /* wait 5 seconds after test */
    /*sleep(5); */
    /*DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test1: finished")); */

    return 0;
}



