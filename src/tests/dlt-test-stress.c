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
 * \file dlt-test-stress.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-stress.c                                             **
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
#include <ctype.h>      /* for isprint() */
#include <errno.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>    /* POSIX Threads */

#include "dlt.h"
#include "dlt_common.h" /* for dlt_get_version() */

DltContext mycontext[9999];

typedef struct
{
    int num;
} thread_data_t;

#define STRESS1_NUM_CONTEXTS    3000
#define STRESS2_MAX_NUM_THREADS  64
#define STRESS3_MAX_NUM_MESSAGES 512

#define MAX_TESTS 3

void stress1(void);

void stress2(void);
void thread_function(void);

void stress3(void);

/*
 * This environment variable is used when developer wants to interrupt program manually
 */
char *env_manual_interruption = 0;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-test-stress [options]\n");
    printf("Test application executing several stress tests.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -1            Execute test 1 (register/unregister many contexts)\n");
    printf("                In order to interrupt test manually (e.g: wait for ENTER key),\n");
    printf("                set environment variable DLT_TEST_MANUAL_INTERRUPTION=1\n");
    printf("  -2            Execute test 2 (multiple threads logging data)\n");
    printf("  -3            Execute test 3 (logging much data)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    /*int  vflag = 0; */
    char *fvalue = 0;
    int test[MAX_TESTS];

    int i, c, help;

    for (i = 0; i < MAX_TESTS; i++)
        test[i] = 0;

    opterr = 0;

    while ((c = getopt (argc, argv, "vf:123")) != -1)
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
        case '1':
        {
            test[0] = 1;
            env_manual_interruption = getenv("DLT_TEST_MANUAL_INTERRUPTION");
            break;
        }
        case '2':
        {
            test[1] = 1;
            break;
        }
        case '3':
        {
            test[2] = 1;
            break;
        }
        case '?':
        {
            if (optopt == 'f')
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

    help = 0;

    for (i = 0; i < MAX_TESTS; i++)
        if (test[i] == 1) {
            help = 1;
            break;
        }



    if (help == 0) {
        usage();
        return -1;
    }

    dlt_register_app("DSTS", "DLT daemon stress tests");

    if (test[0])
        stress1();

    if (test[1])
        stress2();

    if (test[2])
        stress3();

    dlt_unregister_app();

    sleep(1);

    return 0;
}

void stress1(void)
{
    int i, c;
    char ctid[5];
    struct timespec ts;

    printf("Starting stress test1...\n");

    printf("* Register   %d contexts...\n", STRESS1_NUM_CONTEXTS);

    for (i = 0; i < STRESS1_NUM_CONTEXTS; i++) {
        /* Generate id */
        memset(ctid, 0, 5);
        snprintf(ctid, 5, "%d", i);

        /*printf("%i: '%s' \n",i,ctid); */

        dlt_register_context(&(mycontext[i]), ctid, ctid);
        ts.tv_sec = 0;
        ts.tv_nsec = 500 * 1000;
        nanosleep(&ts, NULL);
    }

    if (env_manual_interruption && (strcmp(env_manual_interruption, "1") == 0))
    {
        printf("press \"Enter\" to terminate test");
        while (1)
        {
            c=getchar();
            /* if "Return" is pressed, exit loop; */
            if (c==10)
            {
                break;
            }
        }
    }

    printf("* Unregister %d contexts...\n", STRESS1_NUM_CONTEXTS);

    for (i = 0; i < STRESS1_NUM_CONTEXTS; i++) {
        dlt_unregister_context(&(mycontext[i]));
        ts.tv_sec = 0;
        ts.tv_nsec = 500 * 1000;
        nanosleep(&ts, NULL);
    }

    printf("Finished stress test1 \n\n");
}

void stress2(void)
{
    int ret, index;
    struct timespec ts;

    pthread_t thread[STRESS2_MAX_NUM_THREADS];
    thread_data_t thread_data[STRESS2_MAX_NUM_THREADS];

    printf("Starting stress test2... \n");

    srand((unsigned int) time(NULL));

    printf("* Creating %d Threads, each of them registers one context,\n", STRESS2_MAX_NUM_THREADS);
    printf("  sending one log message, then unregisters the context\n");

    for (index = 0; index < STRESS2_MAX_NUM_THREADS; index++) {
        thread_data[index].num = index;
        ret = pthread_create(&(thread[index]), NULL, (void *)&thread_function, (void *)&(thread_data[index]));

        if (ret != 0)
            printf("Error creating thread %d: %s \n", index, strerror(errno));

        ts.tv_sec = 0;
        ts.tv_nsec = 1000 * 1000;
        nanosleep(&ts, NULL);
    }

    for (index = 0; index < STRESS2_MAX_NUM_THREADS; index++)
        pthread_join(thread[index], NULL);

    printf("Finished stress test2 \n\n");
}

void thread_function(void)
{
    /*thread_data_t *data; */
    DltContext context_thread1;
    DltContextData context_thread1_data;
    char ctid[5];
    struct timespec ts;

    /*data = (thread_data_t *) ptr; */

    memset(ctid, 0, 5);

    /* Create random context id */
    snprintf(ctid, 5, "%.2x", rand() & 0x0000ffff);

    ts.tv_sec = 0;
    ts.tv_nsec = rand();
    nanosleep(&ts, NULL);

    dlt_register_context(&context_thread1, ctid, ctid);

    if (dlt_user_log_write_start(&context_thread1, &context_thread1_data, DLT_LOG_INFO) > 0) {
        dlt_user_log_write_string(&context_thread1_data, ctid);
        dlt_user_log_write_finish(&context_thread1_data);
    }

    dlt_unregister_context(&context_thread1);
}

void stress3(void)
{
    DltContext context_stress3;
    DltContextData context_stress3_data;
    char buffer[STRESS3_MAX_NUM_MESSAGES];
    int num;
    struct timespec ts;

    /* Performance test */
    dlt_register_context(&context_stress3, "TST3", "Stress Test 3 - Performance");

    printf("Starting stress test3... \n");
    printf("* Logging raw data, up to a size of %d\n", STRESS3_MAX_NUM_MESSAGES);

    for (num = 0; num < STRESS3_MAX_NUM_MESSAGES; num++) {
        buffer[num] = (char) num;
        if (dlt_user_log_write_start(&context_stress3, &context_stress3_data, DLT_LOG_INFO) > 0) {
            dlt_user_log_write_int(&context_stress3_data, num);
            dlt_user_log_write_raw(&context_stress3_data, buffer, (uint16_t) num);
            dlt_user_log_write_finish(&context_stress3_data);
        }
        ts.tv_sec = 0;
        ts.tv_nsec = 10000 * 1000;
        nanosleep(&ts, NULL);
    }

    printf("Finished stress test3 \n\n");
}
