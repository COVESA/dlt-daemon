/*
 * Dlt Client test utilities - Diagnostic Log and Trace
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
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
 Initials    Date         Comment
 aw          13.01.2010   initial
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
void thread_function(void *ptr);

void stress3(void);

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version);

    printf("Usage: dlt-test-stress [options]\n");
    printf("Test application executing several stress tests.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -1            Execute test 1 (register/unregister many contexts)\n");
    printf("  -2            Execute test 2 (multiple threads logging data)\n");
    printf("  -3            Execute test 3 (logging much data)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    int  vflag = 0;
    char *fvalue = 0;
    int test[MAX_TESTS];

    int i,c,help;

    for (i=0;i<MAX_TESTS;i++)
    {
        test[i]=0;
    }

    opterr = 0;

    while ((c = getopt (argc, argv, "vf:123")) != -1)
    {
        switch (c)
        {
        case 'v':
        {
            vflag = 1;
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

    help=0;
    for (i=0;i<MAX_TESTS;i++)
    {
        if (test[i]==1)
        {
            help=1;
            break;
        }
    }

    if (help==0)
    {
        usage();
        return -1;
    }

    DLT_REGISTER_APP("DSTS","DLT daemon stress tests");

    if (test[0])
    {
    	stress1();
    }
    if (test[1])
    {
    	stress2();
    }
    if (test[2])
    {
    	stress3();
    }

    DLT_UNREGISTER_APP();

    sleep(1);

    return 0;
}

void stress1(void)
{
    int i,c;
    char ctid[5];

    printf("Starting stress test1... (press \"Enter\" to terminate test) \n");

    printf("* Register   %d contexts...\n",STRESS1_NUM_CONTEXTS);

    for (i=0; i<STRESS1_NUM_CONTEXTS; i++)
    {
        /* Generate id */
        memset(ctid,0,5);
        sprintf(ctid,"%d",i);

        //printf("%i: '%s' \n",i,ctid);

        dlt_register_context(&(mycontext[i]),ctid,ctid);
        usleep(500);
    }

    while (1)
    {
        c=getchar();
        /* if "Return" is pressed, exit loop; */
        if (c==10)
        {
            break;
        }
    }

    printf("* Unregister %d contexts...\n",STRESS1_NUM_CONTEXTS);

    for (i=0; i<STRESS1_NUM_CONTEXTS; i++)
    {
        DLT_UNREGISTER_CONTEXT(mycontext[i]);
        usleep(500);
    }

    printf("Finished stress test1 \n\n");
}

void stress2(void)
{
    int ret,index;

    pthread_t thread[STRESS2_MAX_NUM_THREADS];
    thread_data_t thread_data[STRESS2_MAX_NUM_THREADS];

    printf("Starting stress test2... \n");

    srand(time(NULL));

    printf("* Creating %d Threads, each of them registers one context,\n",STRESS2_MAX_NUM_THREADS);
    printf("  sending one log message, then unregisters the context\n");

    for (index=0;index<STRESS2_MAX_NUM_THREADS;index++)
    {
        thread_data[index].num = index;
        ret=pthread_create(&(thread[index]), NULL, (void *) &thread_function, (void *) &(thread_data[index]));
        if (ret!=0)
        {
			printf("Error creating thread %d: %s \n", index, strerror(errno));
        }

        usleep(1000);
    }

    for (index=0;index<STRESS2_MAX_NUM_THREADS;index++)
    {
		pthread_join(thread[index], NULL);
    }

    printf("Finished stress test2 \n\n");
}

void thread_function(void *ptr)
{
    thread_data_t *data;
    DLT_DECLARE_CONTEXT(context_thread1);
    char ctid[5];

    data = (thread_data_t *) ptr;

    memset(ctid,0,5);

    /* Create random context id */
    sprintf(ctid,"%.2x", rand() & 0x0000ffff);

    usleep(rand()/1000);

    DLT_REGISTER_CONTEXT(context_thread1,ctid,ctid);

    DLT_LOG(context_thread1,DLT_LOG_INFO,DLT_STRING(ctid));

    DLT_UNREGISTER_CONTEXT(context_thread1);
}

void stress3(void)
{
    DLT_DECLARE_CONTEXT(context_stress3);
    char buffer[STRESS3_MAX_NUM_MESSAGES];
    int num;

    /* Performance test */
    DLT_REGISTER_CONTEXT(context_stress3,"TST3","Stress Test 3 - Performance");

    printf("Starting stress test3... \n");
    printf("* Logging raw data, up to a size of %d\n",STRESS3_MAX_NUM_MESSAGES);

    for (num=0;num<STRESS3_MAX_NUM_MESSAGES;num++)
    {
        buffer[num] = num;
        DLT_LOG(context_stress3,DLT_LOG_INFO,DLT_INT(num),DLT_RAW(buffer,num));
        usleep(10000);
    }

    printf("Finished stress test3 \n\n");
}
