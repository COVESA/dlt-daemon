/*
 * Dlt Test Stress user - Diagnostic Log and Trace
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
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#include <stdio.h>      /* for printf() and fprintf() */
#include <float.h>
#include <stdlib.h>     /* for atoi(), abort() */
#include <string.h>     /* for memset() */
#include <ctype.h>      /* for isprint() */

#include "dlt.h"

#define DLT_TEST_NUM_CONTEXT 7

/* Test functions... */

int testall(int count,int repeat,int delay,int size);

/* Context declaration.. */
DLT_DECLARE_CONTEXT(context_info);

/* for macro interface */
DLT_DECLARE_CONTEXT(context_macro_callback);
DLT_DECLARE_CONTEXT(context_macro_test[DLT_TEST_NUM_CONTEXT]);

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

	dlt_get_version(version);

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
int main(int argc, char* argv[])
{
    int vflag = 0;
    char *fvalue = 0;
    int nvalue = 10000;
    int rvalue = 100;
    int dvalue = 1000;
    int svalue = 100;

    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "vf:n:r:d:s:")) != -1)
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
            if (optopt == 'd' || optopt == 'f' || optopt == 'n' || optopt == 'r' || optopt == 'd' || optopt == 's')
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

	/* Register APP */
	DLT_REGISTER_APP("DIFT","DLT Interface Test");

	/* Register CONTEXTS... */
	DLT_REGISTER_CONTEXT(context_info,"INFO","Information context");

    /* Tests starting */
    printf("Tests starting\n");
    //DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Tests starting"));

    /* wait 3 seconds before starting */
    //sleep(3);

	testall(nvalue,rvalue,dvalue,svalue);

    /* Tests finished */
    printf("Tests finished\n");
    //DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Tests finished"));

    /* wait 3 seconds before terminating application */
    //sleep(3);

	/* Unregister CONTEXTS... */
	DLT_UNREGISTER_CONTEXT(context_info);

	/* Unregister APP */
    DLT_UNREGISTER_APP();

    dlt_free();

    return 0;
}

/******************/
/* The test cases */
/******************/

int testall(int count,int repeat,int delay,int size)
{
	char buffer[size];
    int num,rnum;

    for(num=0;num< size;num++)
    {
        buffer[num] = num;
    }
    
	/* Test All: Test all start */
    //printf("Test1: Test all\n");
    //DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test1: Test all"));

	for(rnum=0;rnum<repeat;rnum++)
	{
		for(num=1;num<=count;num++)
		{
			DLT_LOG(context_info,DLT_LOG_INFO,DLT_INT(num),DLT_RAW(buffer, size));
			usleep(delay);
		}
	}
	
	/* wait 5 seconds after test */
    //sleep(5);
	//DLT_LOG(context_info,DLT_LOG_INFO,DLT_STRING("Test1: finished"));
	
	return 0;
}



