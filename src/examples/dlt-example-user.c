/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt-example-user.c
 * For further information see http://www.genivi.org/.
 * @licence end@
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
 Initials    Date         Comment
 aw          13.01.2010   initial
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

DLT_DECLARE_CONTEXT(mycontext);

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version);

    printf("Usage: dlt-example-user [options] message\n");
    printf("Generate DLT messages and store them to file or send them to daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -d delay      Milliseconds to wait between sending messages (Default: 500)\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -n count      Number of messages to be generated (Default: 10)\n");
    printf("  -g            Switch to non-verbose mode (Default: verbose mode)\n");
    printf("  -a            Enable local printing of DLT messages (Default: disabled)\n");
    printf("  -m mode       Set log mode 0=off,1=external,2=internal,3=both\n");
#ifdef DLT_TEST_ENABLE
    printf("  -c       		Corrupt user header\n");
    printf("  -s size       Corrupt message size\n");
    printf("  -z size      	Size of message\n");
#endif /* DLT_TEST_ENABLE */
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    int vflag = 0;
    int gflag = 0;
    int aflag = 0;
#ifdef DLT_TEST_ENABLE
    int cflag = 0;    
    char *svalue = 0;
    char *zvalue = 0;
#endif /* DLT_TEST_ENABLE */
    char *dvalue = 0;
    char *fvalue = 0;
    char *nvalue = 0;
    char *mvalue = 0;
    char *message = 0;

    int index;
    int c;

	char *text;
	int num,maxnum;
	int delay;
	
	int state=-1,newstate;

    opterr = 0;
#ifdef DLT_TEST_ENABLE
    while ((c = getopt (argc, argv, "vgacd:f:n:m:z:s:")) != -1)
#else
    while ((c = getopt (argc, argv, "vgad:f:n:m:")) != -1)
#endif /* DLT_TEST_ENABLE */
    {
        switch (c)
        {
        case 'v':
        {
            vflag = 1;
            break;
        }
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
        }
        }
    }

    for (index = optind; index < argc; index++)
    {
        message = argv[index];
    }

    if (message == 0)
    {
        /* no message, show usage and terminate */
        fprintf(stderr,"ERROR: No message selected\n");
        usage();
        return -1;
    }

    if (fvalue)
    {
        /* DLT is intialised automatically, except another output target will be used */
        if (dlt_init_file(fvalue)<0) /* log to file */
        {
            return -1;
        }
    }

    DLT_REGISTER_APP("LOG","Test Application for Logging");
    DLT_REGISTER_CONTEXT(mycontext,"TEST","Test Context for Logging");

    DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0xFFF, dlt_user_injection_callback);

    text = message;

	if(mvalue)
	{
        printf("Set log mode to %d\n",atoi(mvalue));
		dlt_set_log_mode(atoi(mvalue)); 
	}
	

    if (gflag)
    {
        DLT_NONVERBOSE_MODE();
    }

    if (aflag)
    {
        DLT_ENABLE_LOCAL_PRINT();
    }

    if (nvalue)
    {
        maxnum = atoi(nvalue);
    }
    else
    {
		maxnum = 10;
    }

    if (dvalue)
    {
        delay = atoi(dvalue) * 1000;
    }
    else
    {
        delay = 500 * 1000;
    }

    if (gflag)
    {
        /* DLT messages to test Fibex non-verbose description: dlt-example-non-verbose.xml */
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,10);
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,11,DLT_UINT16(1011));
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,12,DLT_UINT32(1012),DLT_UINT32(1013));
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,13,DLT_UINT8(123),DLT_FLOAT32(1.12));
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,14,DLT_STRING("DEAD BEEF"));
    }

#ifdef DLT_TEST_ENABLE
    if (cflag)
    {
		dlt_user_test_corrupt_user_header(1);
    }
    if (svalue)
    {
		dlt_user_test_corrupt_message_size(1,atoi(svalue));
    }
	if (zvalue)
	{
		char* buffer = malloc(atoi(zvalue));
        DLT_LOG(mycontext,DLT_LOG_WARN,DLT_STRING(text),DLT_RAW(buffer,atoi(zvalue)));		
		free(buffer);
	}
#endif /* DLT_TEST_ENABLE */

    for (num=0;num<maxnum;num++)
    {
        printf("Send %d %s\n",num,text);

		newstate = dlt_get_log_state();
		if(state!=newstate)
		{
			state = newstate;
			if(state == -1) {
				printf("Client unknown state!\n");
			}
			else if(state == 0) {
				printf("Client disconnected!\n");
			}
			else if(state == 1) {
				printf("Client connected!\n");
			}
		}
				
        if (gflag)
        {
            /* Non-verbose mode */
            DLT_LOG_ID(mycontext,DLT_LOG_WARN,num,DLT_INT(num),DLT_STRING(text));
        }
        else
        {
            /* Verbose mode */
            DLT_LOG(mycontext,DLT_LOG_WARN,DLT_INT(num),DLT_STRING(text));
        }

        if (delay>0)
        {
            usleep(delay);
        }
    }

    sleep(1);

    DLT_UNREGISTER_CONTEXT(mycontext);

    DLT_UNREGISTER_APP();

    dlt_free();

    return 0;

}

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    printf("Injection %d, Length=%d \n",service_id,length);
    if (length>0)
    {
        dlt_print_mixed_string(text,1024,data,length,0);
        printf("%s \n", text);
    }

    return 0;
}

