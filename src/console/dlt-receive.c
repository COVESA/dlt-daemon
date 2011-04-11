/*
 * Dlt Client console utilities - Diagnostic Log and Trace
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
**  SRC-MODULE: dlt-receive.cpp                                               **
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

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */

#include "dlt_client.h"

#define DLT_RECEIVE_TEXTBUFSIZE 10024  /* Size of buffer for text output */

#define DLT_RECEIVE_ECU_ID "RECV"

/* Function prototypes */
int dlt_receive_message_callback(DltMessage *message, void *data);

typedef struct {
    int aflag;
    int sflag;
    int xflag;
    int mflag;
    int vflag;
    int yflag;
    char *ovalue;
    char *fvalue;
    char *evalue;
    int bvalue;
    char ecuid[4];
    int ohandle;
    DltFile file;
    DltFilter filter;
} DltReceiveData;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version);

    printf("Usage: dlt-receive [options] hostname/serial_device_name\n");
    printf("Receive DLT messages from DLT daemon and print or store the messages.\n");
    printf("Use filters to filter received messages.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -a            Print DLT messages; payload as ASCII\n");
    printf("  -x            Print DLT messages; payload as hex\n");
    printf("  -m            Print DLT messages; payload as hex and ASCII\n");
    printf("  -s            Print DLT messages; only headers\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("  -f filename   Enable filtering of messages\n");
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
    dltdata.aflag = 0;
    dltdata.sflag = 0;
    dltdata.xflag = 0;
    dltdata.mflag = 0;
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.ovalue = 0;
    dltdata.fvalue = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;
    dltdata.ohandle=-1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashyxmf:o:e:b:")) != -1)
        switch (c)
        {
        case 'v':
			{
            	dltdata.vflag = 1;
            	break;
			}
        case 'a':
			{
            	dltdata.aflag = 1;
            	break;
			}
        case 's':
			{
            	dltdata.sflag = 1;
            	break;
			}
        case 'x':
			{
            	dltdata.xflag = 1;
            	break;
			}
        case 'm':
			{
            	dltdata.mflag = 1;
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
        case 'f':
			{
            	dltdata.fvalue = optarg;
            	break;
			}
        case 'o':
			{
            	dltdata.ovalue = optarg;
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

    if (dltdata.fvalue)
    {
        if (dlt_filter_load(&(dltdata.filter),dltdata.fvalue,dltdata.vflag)<0)
        {
            dlt_file_free(&(dltdata.file),dltdata.vflag);
            return -1;
        }

        dlt_file_set_filter(&(dltdata.file),&(dltdata.filter),dltdata.vflag);
    }

    /* open DLT output file */
    if (dltdata.ovalue)
    {
        dltdata.ohandle = open(dltdata.ovalue,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

        if (dltdata.ohandle == -1)
        {
            dlt_file_free(&(dltdata.file),dltdata.vflag);
            fprintf(stderr,"ERROR: Output file %s cannot be opened!\n",dltdata.ovalue);
            return -1;
        }
    }

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

        /* Dlt Client Main Loop */
        dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&dltclient,dltdata.vflag);
    }

    /* dlt-receive cleanup */
    if (dltdata.ovalue)
	{
        close(dltdata.ohandle);
	}

    dlt_file_free(&(dltdata.file),dltdata.vflag);

    dlt_filter_free(&(dltdata.filter),dltdata.vflag);

    return 0;
}

int dlt_receive_message_callback(DltMessage *message, void *data)
{
	DltReceiveData *dltdata;
    static char text[DLT_RECEIVE_TEXTBUFSIZE];

	struct iovec iov[2];
    int bytes_written;

    if ((message==0) || (data==0))
	{
        return -1;
	}

    dltdata = (DltReceiveData*)data;

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
    {
        dlt_set_storageheader(message->storageheader,message->headerextra.ecu);
    }
    else
    {
        dlt_set_storageheader(message->storageheader,dltdata->ecuid);
    }

    if ((dltdata->fvalue==0) || (dltdata->fvalue && dlt_message_filter_check(message,&(dltdata->filter),dltdata->vflag)==1))
    {
        /* if no filter set or filter is matching display message */
        if (dltdata->xflag)
        {
            dlt_message_print_hex(message,text,DLT_RECEIVE_TEXTBUFSIZE,dltdata->vflag);
        }
        else if (dltdata->aflag)
        {

            dlt_message_header(message,text,DLT_RECEIVE_TEXTBUFSIZE,dltdata->vflag);

            printf("%s ",text);

            dlt_message_payload(message,text,DLT_RECEIVE_TEXTBUFSIZE,DLT_OUTPUT_ASCII,dltdata->vflag);

            printf("[%s]\n",text);
        }
        else if (dltdata->mflag)
        {
            dlt_message_print_mixed_plain(message,text,DLT_RECEIVE_TEXTBUFSIZE,dltdata->vflag);
        }
        else if (dltdata->sflag)
        {

            dlt_message_header(message,text,DLT_RECEIVE_TEXTBUFSIZE,dltdata->vflag);

            printf("%s \n",text);
        }

        /* if file output enabled write message */
        if (dltdata->ovalue)
        {
            iov[0].iov_base = message->headerbuffer;
            iov[0].iov_len = message->headersize;
            iov[1].iov_base = message->databuffer;
            iov[1].iov_len = message->datasize;

            bytes_written = writev(dltdata->ohandle, iov, 2);
        }
    }

    return 0;
}
