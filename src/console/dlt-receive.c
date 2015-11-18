/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
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
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-receive.cpp
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
#include <errno.h>
#include <string.h>
#include <glob.h>
#include <syslog.h>
#include <linux/limits.h> /* for PATH_MAX */

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
    char *ovaluebase; /* ovalue without ".dlt" */
    char *fvalue;
    char *evalue;
    int bvalue;
    int64_t climit;
    char ecuid[4];
    int ohandle;
    int64_t totalbytes; /* bytes written so far into the output file, used to check the file size limit */
    int part_num;    /* number of current output file if limit was exceeded */
    DltFile file;
    DltFilter filter;
} DltReceiveData;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version,255);

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
    printf("  -c limit      Restrict file size to <limit> bytes when output to file\n");
    printf("                When limit is reached, a new file is opened. Use K,M,G as\n");
    printf("                suffix to specify kilo-, mega-, giga-bytes respectively\n");
    printf("  -f filename   Enable filtering of messages\n");
}


int64_t convert_arg_to_byte_size(char * arg)
{
    size_t i;
    int64_t factor;
    int64_t result;
    /* check if valid input */
    for (i = 0; i<strlen(arg)-1; ++i)
    {
        if (!isdigit(arg[i]))
        {
            return -2;
        }
    }

    /* last character */
    factor = 1;
    if ((arg[strlen(arg)-1] == 'K') || (arg[strlen(arg)-1] == 'k'))
    {
        factor = 1024;
    }
    else if ((arg[strlen(arg)-1] == 'M') || (arg[strlen(arg)-1] == 'm'))
    {
        factor = 1024 * 1024;
    }
    else if ((arg[strlen(arg)-1] == 'G') || (arg[strlen(arg)-1] == 'g'))
    {
        factor = 1024 * 1024 * 1024;
    }
    else
    {
        if (!isdigit(arg[strlen(arg)-1]))
        {
            return -2;
        }
    }

    /* range checking */
    int64_t const mult = atoll(arg);
    if (((INT64_MAX)/factor) < mult)
    {
      /* Would overflow! */
      return -2;
    }

    result = factor * mult;

    /* The result be at least the size of one message
     * One message consists of its header + user data:
     */
    DltMessage msg;
    int64_t min_size = sizeof(msg.headerbuffer);
    min_size += 2048 /* DLT_USER_BUF_MAX_SIZE */;

    if (min_size > result)
    {
        char tmp[256];
        snprintf(tmp, 256, "ERROR: Specified limit: %li is smaller than a the size of a single message: %li !\n", result, min_size);
        dlt_log(LOG_ERR, tmp);
        result = -2;
    }

    return result;
}


/*
 * open output file
 */
int dlt_receive_open_output_file(DltReceiveData * dltdata)
{
    /* if (file_already_exists) */
    glob_t outer;
    if (glob(dltdata->ovalue, GLOB_TILDE_CHECK | GLOB_NOSORT, NULL, &outer) == 0)
    {
        if (dltdata->vflag)
        {
          char tmp[256];
          snprintf(tmp, 256, "File %s already exists, need to rename first\n", dltdata->ovalue);
          dlt_log(LOG_INFO, tmp);
        }

        if (dltdata->part_num < 0)
        {
            char pattern[PATH_MAX+1];
            pattern[PATH_MAX] = 0;
            snprintf(pattern, PATH_MAX, "%s.*.dlt", dltdata->ovaluebase);
            glob_t inner;

            /* sort does not help here because we have to traverse the
             * full result in any case. Remember, a sorted list would look like:
             * foo.1.dlt
             * foo.10.dlt
             * foo.1000.dlt
             * foo.11.dlt
             */
            if (glob(pattern, GLOB_TILDE_CHECK | GLOB_NOSORT, NULL, &inner) == 0)
            {
              /* search for the highest number used */
              size_t i;
              for (i= 0; i<inner.gl_pathc; ++i)
              {
                /* convert string that follows the period after the initial portion,
                 * e.g. gt.gl_pathv[i] = foo.1.dlt -> atoi("1.dlt");
                 */
                int cur = atoi(&inner.gl_pathv[i][strlen(dltdata->ovaluebase)+1]);
                if (cur > dltdata->part_num)
                {
                  dltdata->part_num = cur;
                }
              }
            }
            globfree(&inner);

            ++dltdata->part_num;

        }

        char filename[PATH_MAX+1];
        filename[PATH_MAX] = 0;

        snprintf(filename, PATH_MAX, "%s.%i.dlt", dltdata->ovaluebase, dltdata->part_num++);
        if (rename(dltdata->ovalue, filename) != 0)
        {
          char tmp[256];
          snprintf(tmp, 256, "ERROR: rename %s to %s failed with error %s\n", dltdata->ovalue, filename, strerror(errno));
          dlt_log(LOG_ERR, tmp);
        }
        else
        {
          if (dltdata->vflag)
          {
            char tmp[256];
            snprintf(tmp, 256, "Renaming existing file from %s to %s\n", dltdata->ovalue, filename);
            dlt_log(LOG_INFO, tmp);
          }
        }

    } /* if (file_already_exists) */
    globfree(&outer);

    dltdata->ohandle = open(dltdata->ovalue, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return dltdata->ohandle;
}


void dlt_receive_close_output_file(DltReceiveData * dltdata)
{
    if (dltdata->ohandle)
    {
        close(dltdata->ohandle);
        dltdata->ohandle = -1;
    }
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
    dltdata.ovaluebase = 0;
    dltdata.fvalue = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;
    dltdata.climit = -1; /* default: -1 = unlimited */
    dltdata.ohandle=-1;
    dltdata.totalbytes = 0;
    dltdata.part_num = -1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashyxmf:o:e:b:c:")) != -1)
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
            	size_t to_copy = strlen(dltdata.ovalue);
            	if (strcmp(&dltdata.ovalue[to_copy-4], ".dlt") == 0)
            	{
            	  to_copy = to_copy - 4;
            	}

            	dltdata.ovaluebase = strndup(dltdata.ovalue, to_copy);
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

        case 'c':
			{
            	dltdata.climit = convert_arg_to_byte_size(optarg);
            	if (dltdata.climit < -1)
            	{
		            fprintf (stderr, "Invalid argument for option -c.\n");
			        /* unknown or wrong option used, show usage information and terminate */
			        usage();
			        return -1;
            	}
            	break;
			}
        case '?':
			{
		        if (optopt == 'o' || optopt == 'f' || optopt == 'c')
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
    dltclient.mode = dltdata.yflag;

    if (dltclient.mode==DLT_CLIENT_MODE_TCP)
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
        if (dlt_filter_load(&(dltdata.filter),dltdata.fvalue,dltdata.vflag) < DLT_RETURN_OK)
        {
            dlt_file_free(&(dltdata.file),dltdata.vflag);
            return -1;
        }

        dlt_file_set_filter(&(dltdata.file),&(dltdata.filter),dltdata.vflag);
    }

    /* open DLT output file */
    if (dltdata.ovalue)
    {
        if (dltdata.climit > -1)
        {
            char tmp[256];
            snprintf(tmp, 256, "Using file size limit of %li bytes\n", dltdata.climit);
            dlt_log(LOG_INFO, tmp);
            dltdata.ohandle = dlt_receive_open_output_file(&dltdata);
        }
        else /* in case no limit for the output file is given, we simply overwrite any existing file */
        {
            dltdata.ohandle = open(dltdata.ovalue, O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }

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
    if (dlt_client_connect(&dltclient, dltdata.vflag) != DLT_RETURN_ERROR)
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

    free(dltdata.ovaluebase);

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

    if ((dltdata->fvalue==0) ||
                    (dltdata->fvalue && dlt_message_filter_check(message,&(dltdata->filter),dltdata->vflag) == DLT_RETURN_TRUE))
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

            if (dltdata->climit > -1)
            {
            	int bytes_to_write = message->headersize + message->datasize;
            	
            	if ((bytes_to_write + dltdata->totalbytes > dltdata->climit))
            	{
                    dlt_receive_close_output_file(dltdata);
                    
            		if (dlt_receive_open_output_file(dltdata) < 0)
            		{
                        printf("ERROR: dlt_receive_message_callback: Unable to open log when maximum filesize was reached!\n");
                        return -1;
            		}
            		
            		dltdata->totalbytes = 0;
            	}
            }
            
            bytes_written = writev(dltdata->ohandle, iov, 2);

            dltdata->totalbytes += bytes_written;

            if (0 > bytes_written){
                    printf("dlt_receive_message_callback: writev(dltdata->ohandle, iov, 2); returned an error!" );
                    return -1;
            }
        }
    }

    return 0;
}
