/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG"
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
 * \copyright Copyright © 2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-control.cpp
*/


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-control.cpp                                               **
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

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */
#include <string.h>     /* for open() */

#include "dlt_client.h"

#define DLT_RECEIVE_TEXTBUFSIZE 10024  /* Size of buffer for text output */

#define DLT_RECEIVE_ECU_ID "RECV"

/* Function prototypes */
int dlt_receive_message_callback(DltMessage *message, void *data);

typedef struct {
    int vflag;
    int yflag;
    char *evalue;

    char *avalue;
    char *cvalue;
    int svalue;
    char *mvalue;
    char *xvalue;
    int tvalue;
    int lvalue;
    int rvalue;
    int dvalue;
    int fvalue;
    int ivalue;
    int oflag;
    int gflag;

    int bvalue;
    char ecuid[4];
    DltFile file;
    DltFilter filter;
} DltReceiveData;


void hexAsciiToBinary (const char *ptr,uint8_t *binary,int *size)
{

	char ch = *ptr;
	int pos = 0;
	binary[pos] = 0;
	int first = 1;
	int found;

	for(;;)
	{

		if(ch == 0)
		{
			*size = pos;
			return;
		}


		found = 0;
		if (ch >= '0' && ch <= '9')
		{
			binary[pos] = (binary[pos] << 4) + (ch - '0');
			found = 1;
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			binary[pos] = (binary[pos] << 4) + (ch - 'A' + 10);
			found = 1;
		}
		else if (ch >= 'a' && ch <= 'f')
		{
			binary[pos] = (binary[pos] << 4) + (ch - 'a' + 10);
			found = 1;
		}
		if(found)
		{
			if(first)
				first = 0;
			else
			{
				first = 1;
				pos++;
				if(pos>=*size)
					return;
				binary[pos]=0;
			}
		}

		ch = *(++ptr);
	}

}

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version,255);

    printf("Usage: dlt-control [options] hostname/serial_device_name\n");
    printf("Send control message to DLT daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("\n");
    printf("  -a id		    Control message application id\n");
    printf("  -c id    		Control message context id\n");
    printf("  -s id    		Control message injection service id\n");
    printf("  -m message    Control message injection in ASCII\n");
    printf("  -x message    Control message injection in Hex e.g. 'ad 01 24 ef'\n");
    printf("  -t milliseconds Timeout to terminate application (Default:1000)'\n");
    printf("  -l loglevel	  Set the log level (0=off - 5=verbose,255=default)\n");
    printf("  -r tracestatus  Set the trace status (0=off - 1=on,255=default)\n");
    printf("  -d loglevel	  Set the default log level (0=off - 5=verbose)\n");
    printf("  -f tracestatus  Set the default trace status (0=off - 1=on)\n");
    printf("  -i enable  	  Enable timing packets (0=off - 1=on)\n");
    printf("  -o 		  	  Store configuration\n");
    printf("  -g 		  	  Reset to factory default\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    DltClient      dltclient;
    DltReceiveData dltdata;
    int c;
    int index;

    /* Initialize dltdata */
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;

    dltdata.avalue = 0;
    dltdata.cvalue = 0;
    dltdata.svalue = 0;
    dltdata.mvalue = 0;
    dltdata.xvalue = 0;
    dltdata.tvalue = 1000;
    dltdata.lvalue = -1;
    dltdata.rvalue = -1;
    dltdata.dvalue = -1;
    dltdata.fvalue = -1;
    dltdata.ivalue = -1;
    dltdata.oflag = -1;
    dltdata.gflag = -1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vhye:b:a:c:s:m:x:t:l:r:d:f:i:og")) != -1)
        switch (c)
        {
        case 'v':
			{
            	dltdata.vflag = 1;
            	break;
			}
        case 'h':
			{
            	usage();
            	return -1;
			}
        case 'y':
			{
            	dltdata.yflag = 1;
            	break;
			}
        case 'e':
			{
            	dltdata.evalue = optarg;
            	break;
			}
        case 'b':
			{
            	dltdata.bvalue = atoi(optarg);
            	break;
			}

        case 'a':
			{
            	dltdata.avalue = optarg;
            	break;
			}
        case 'c':
			{
            	dltdata.cvalue = optarg;
            	break;
			}
        case 's':
			{
            	dltdata.svalue = atoi(optarg);
            	break;
			}
        case 'm':
			{
            	dltdata.mvalue = optarg;
            	break;
			}
        case 'x':
			{
            	dltdata.xvalue = optarg;
            	break;
			}
        case 't':
			{
            	dltdata.tvalue = atoi(optarg);;
            	break;
			}
        case 'l':
			{
            	dltdata.lvalue = atoi(optarg);;
            	break;
			}
        case 'r':
			{
            	dltdata.rvalue = atoi(optarg);;
            	break;
			}
        case 'd':
			{
            	dltdata.dvalue = atoi(optarg);;
            	break;
			}
        case 'f':
			{
            	dltdata.fvalue = atoi(optarg);;
            	break;
			}
        case 'i':
			{
            	dltdata.ivalue = atoi(optarg);;
            	break;
			}
        case 'o':
			{
            	dltdata.oflag = 1;
            	break;
			}
        case 'g':
			{
            	dltdata.gflag = 1;
            	break;
			}

        case '?':
			{
		        if (optopt == 'o' || optopt == 'f')
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

    /* Initialize DLT Client */
    dlt_client_init(&dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_receive_message_callback);

    /* Setup DLT Client structure */
    dltclient.serial_mode = dltdata.yflag;

    if (dltclient.serial_mode==0)
    {
        for (index = optind; index < argc; index++)
        {
            dltclient.servIP = argv[index];
        }

        if (dltclient.servIP == 0)
        {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr,"ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&dltclient,dltdata.vflag);
            return -1;
        }
    }
    else
    {
        for (index = optind; index < argc; index++)
        {
            dltclient.serialDevice = argv[index];
        }

        if (dltclient.serialDevice == 0)
        {
            /* no serial device name selected, show usage and terminate */
            fprintf(stderr,"ERROR: No serial device name specified\n");
            usage();
            return -1;
        }

		dlt_client_setbaudrate(&dltclient,dltdata.bvalue);
    }

    /* initialise structure to use DLT file */
    dlt_file_init(&(dltdata.file),dltdata.vflag);

    /* first parse filter file if filter parameter is used */
    dlt_filter_init(&(dltdata.filter),dltdata.vflag);

    if (dltdata.evalue)
	{
        dlt_set_id(dltdata.ecuid,dltdata.evalue);
    }
	else
	{
        dlt_set_id(dltdata.ecuid,DLT_RECEIVE_ECU_ID);
	}

    /* Connect to TCP socket or open serial device */
    if (dlt_client_connect(&dltclient, dltdata.vflag)!=-1)
    {
    	/* send injection message */
    	if(dltdata.mvalue && dltdata.avalue && dltdata.cvalue)
    	{
    		/* ASCII */
    		printf("Send injection message:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("ServiceId: %d\n",dltdata.svalue);
    		printf("Message: %s\n",dltdata.mvalue);
    		/* send control message in ascii */
    		if (0 != dlt_client_send_inject_msg(&dltclient,dltdata.avalue,dltdata.cvalue,dltdata.svalue,(uint8_t*)dltdata.mvalue,strlen(dltdata.mvalue))) {
			fprintf (stderr, "ERROR: Could not send inject message\n");
		}
    	}
    	else if(dltdata.xvalue && dltdata.avalue && dltdata.cvalue)
    	{
    		/* Hex */
    		uint8_t buffer[1024];
    		int size = 1024;
    		printf("Send injection message:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("ServiceId: %d\n",dltdata.svalue);
    		printf("Message: %s\n",dltdata.xvalue);
    		hexAsciiToBinary(dltdata.xvalue,buffer,&size);
    		printf("Size: %d\n",size);
    		/* send control message in hex */
    		if (0 != dlt_client_send_inject_msg(&dltclient,dltdata.avalue,dltdata.cvalue,dltdata.svalue,buffer,size)) {
			fprintf (stderr, "ERROR: Could not send inject message\n");
		}
    	}
    	else if(dltdata.lvalue!=-1 && dltdata.avalue && dltdata.cvalue)
    	{
    		/* log level */
    		printf("Set log level:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("Loglevel: %d\n",dltdata.lvalue);
    		/* send control message*/
    		if (0 != dlt_client_send_log_level(&dltclient,dltdata.avalue,dltdata.cvalue,dltdata.lvalue)) {
			fprintf (stderr, "ERROR: Could not send log level\n");
		}
    	}
    	else if(dltdata.rvalue!=-1 && dltdata.avalue && dltdata.cvalue)
    	{
    		/* trace status */
    		printf("Set trace status:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("TraceStatus: %d\n",dltdata.rvalue);
    		/* send control message in*/
    		if (0 != dlt_client_send_trace_status(&dltclient,dltdata.avalue,dltdata.cvalue,dltdata.rvalue)) {
			fprintf (stderr, "ERROR: Could not send trace status\n");
		}
    	}
    	else if(dltdata.dvalue!=-1)
    	{
    		/* default log level */
    		printf("Set default log level:\n");
    		printf("Loglevel: %d\n",dltdata.dvalue);
    		/* send control message in*/
    		if (0 != dlt_client_send_default_log_level(&dltclient,dltdata.dvalue)) {
			fprintf (stderr, "ERROR: Could not send default log level\n");
		}
    	}
    	else if(dltdata.rvalue!=-1)
    	{
    		/* default trace status */
    		printf("Set default trace status:\n");
    		printf("TraceStatus: %d\n",dltdata.rvalue);
    		/* send control message in*/
    		if (0 != dlt_client_send_default_trace_status(&dltclient,dltdata.rvalue)) {
			fprintf (stderr, "ERROR: Could not send default trace status\n");
		}
    	}
    	else if(dltdata.ivalue!=-1)
    	{
    		/* timing pakets */
    		printf("Set timing pakets:\n");
    		printf("Timing packets: %d\n",dltdata.ivalue);
    		/* send control message in*/
    		if (0 != dlt_client_send_timing_pakets(&dltclient,dltdata.ivalue)) {
			fprintf (stderr, "ERROR: Could not send timing packets\n");
		}
    	}
    	else if(dltdata.oflag!=-1)
    	{
    		/* default trace status */
    		printf("Store config\n");
    		/* send control message in*/
    		if (0 != dlt_client_send_store_config(&dltclient)) {
			fprintf (stderr, "ERROR: Could not send store config\n");
		}
    	}
    	else if(dltdata.gflag!=-1)
    	{
    		/* reset to factory default */
    		printf("Reset to factory default\n");
    		/* send control message in*/
    		if (0 != dlt_client_send_reset_to_factory_default(&dltclient)) {
			fprintf (stderr, "ERROR: Could send reset to factory default\n");
		}
    	}

        /* Dlt Client Main Loop */
        //dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag);

    	/* Wait timeout */
    	usleep(dltdata.tvalue*1000);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&dltclient,dltdata.vflag);
    }

    dlt_file_free(&(dltdata.file),dltdata.vflag);

    dlt_filter_free(&(dltdata.filter),dltdata.vflag);

    return 0;
}

int dlt_receive_message_callback(DltMessage *message, void *data)
{

    if ((message==0) || (data==0))
	{
        return -1;
	}


    return 0;
}
