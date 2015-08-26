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
 * \file dlt-convert.cpp
*/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-convert.cpp                                               **
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
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/uio.h> /* writev() */

#include "dlt_common.h"

#define DLT_CONVERT_TEXTBUFSIZE  10024   /* Size of buffer for text output */

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[DLT_CONVERT_TEXTBUFSIZE];

    dlt_get_version(version,255);

    printf("Usage: dlt-convert [options] [commands] file1 [file2]\n");
    printf("Read DLT files, print DLT messages as ASCII and store the messages again.\n");
    printf("Use filters to filter DLT messages.\n");
    printf("Use Ranges and Output file to cut DLT files.\n");
    printf("Use two files and Output file to join DLT files.\n");
    printf("%s \n", version);
    printf("Commands:\n");
    printf("  -h            Usage\n");
    printf("  -a            Print DLT file; payload as ASCII\n");
    printf("  -x            Print DLT file; payload as hex\n");
    printf("  -m            Print DLT file; payload as hex and ASCII\n");
    printf("  -s            Print DLT file; only headers\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -c            Count number of messages\n");
    printf("  -f filename   Enable filtering of messages\n");
    printf("  -b number     First messages to be handled\n");
    printf("  -e number     Last message to be handled\n");
    printf("  -w            Follow dlt file while file is increasing\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    int vflag = 0;
    int cflag = 0;
    int aflag = 0;
    int sflag = 0;
    int xflag = 0;
    int mflag = 0;
    int wflag = 0;
    char *fvalue = 0;
    char *bvalue = 0;
    char *evalue = 0;
    char *ovalue = 0;

    int index;
    int c;

	DltFile file;
	DltFilter filter;

	int ohandle=-1;

	int num, begin, end;

	char text[DLT_CONVERT_TEXTBUFSIZE];

	struct iovec iov[2];
    int bytes_written;

    opterr = 0;

    while ((c = getopt (argc, argv, "vcashxmwf:b:e:o:")) != -1)
        switch (c)
        {
        case 'v':
			{
            	vflag = 1;
            	break;
			}
        case 'c':
			{
            	cflag = 1;
            	break;
			}
        case 'a':
			{
            	aflag = 1;
            	break;
			}
        case 's':
			{
            	sflag = 1;
            	break;
			}
        case 'x':
			{
            	xflag = 1;
            	break;
			}
        case 'm':
			{
            	mflag = 1;
            	break;
			}
        case 'w':
			{
            	wflag = 1;
            	break;
			}
        case 'h':
			{
            	usage();
            	return -1;
			}
        case 'f':
			{
            	fvalue = optarg;
            	break;
			}
        case 'b':
			{
            	bvalue = optarg;
            	break;
			}
        case 'e':
			{
            	evalue = optarg;
            	break;
			}
        case 'o':
			{
            	ovalue = optarg;
            	break;
			}
        case '?':
			{
		        if (optopt == 'f' || optopt == 'b' || optopt == 'e' || optopt == 'o')
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
            	abort();
                return -1;//for parasoft
			}
        }

    /* initialise structure to use DLT file */
    dlt_file_init(&file,vflag);

    /* first parse filter file if filter parameter is used */
    if (fvalue)
    {
        if (dlt_filter_load(&filter,fvalue,vflag)<0)
        {
            dlt_file_free(&file,vflag);
            return -1;
        }

        dlt_file_set_filter(&file,&filter,vflag);
    }

    if (ovalue)
    {
        ohandle = open(ovalue,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */
        if (ohandle == -1)
        {
            dlt_file_free(&file,vflag);
            fprintf(stderr,"ERROR: Output file %s cannot be opened!\n",ovalue);
            return -1;
        }

    }

    for (index = optind; index < argc; index++)
    {
        /* load, analyse data file and create index list */
        if (dlt_file_open(&file,argv[index],vflag)>=0)
        {
            while (dlt_file_read(&file,vflag)>=0)
            {
            }
        }

        if (aflag || sflag || xflag || mflag || ovalue)
        {
            if (bvalue)
			{
                begin = atoi(bvalue);
            }
			else
			{
                begin = 0;
			}

            if (evalue && (wflag==0))
			{
                end = atoi(evalue);
            }
			else
			{
                end = file.counter-1;
			}

            if (begin<0 || begin>=file.counter)
            {
                fprintf(stderr,"ERROR: Selected first message %d is out of range!\n",begin);
                return -1;
            }
            if (end<0 || end>=file.counter || end<begin)
            {
                fprintf(stderr,"ERROR: Selected end message %d is out of range!\n",end);
                return -1;
            }
            for (num = begin; num <= end ;num++)
            {
                dlt_file_message(&file,num,vflag);

                if (xflag)
                {
                    printf("%d ",num);
                    dlt_message_print_hex(&(file.msg),text,DLT_CONVERT_TEXTBUFSIZE,vflag);
                }
                else if (aflag)
                {
                    printf("%d ",num);

                    dlt_message_header(&(file.msg),text,DLT_CONVERT_TEXTBUFSIZE,vflag);

                    printf("%s ",text);

                    dlt_message_payload(&file.msg,text,DLT_CONVERT_TEXTBUFSIZE,DLT_OUTPUT_ASCII,vflag);

                    printf("[%s]\n",text);
                }
                else if (mflag)
                {
                    printf("%d ",num);
                    dlt_message_print_mixed_plain(&(file.msg),text,DLT_CONVERT_TEXTBUFSIZE,vflag);
                }
                else if (sflag)
                {
                    printf("%d ",num);

                    dlt_message_header(&(file.msg),text,DLT_CONVERT_TEXTBUFSIZE,vflag);

                    printf("%s \n",text);
                }

                /* if file output enabled write message */
                if (ovalue)
                {
                    iov[0].iov_base = file.msg.headerbuffer;
                    iov[0].iov_len = file.msg.headersize;
                    iov[1].iov_base = file.msg.databuffer;
                    iov[1].iov_len = file.msg.datasize;

                    bytes_written = writev(ohandle, iov, 2);
                    if (0 > bytes_written){
                            printf("in main: writev(ohandle, iov, 2); returned an error!" );
                            dlt_file_free(&file,vflag);
                            return -1;
                    }
                }

                /* check for new messages if follow flag set */
                if (wflag && num==end)
                {
                    while (1)
                    {
                        while (dlt_file_read(&file,0)>=0)
                        {
                        }
                        if (end == (file.counter-1))
						{
                            /* Sleep if no new message was received */
                            sleep(1);
                        }
						else
                        {
                            /* set new end of log file and continue reading */
                            end = file.counter-1;
                            break;
                        }
                    }
                }
            }
        }
        if (cflag)
        {
            printf("Total number of messages: %d\n",file.counter_total);
            if (file.filter)
			{
                printf("Filtered number of messages: %d\n",file.counter);
			}
        }
    }
    if (ovalue)
    {
        close(ohandle);
    }
    if (index == optind)
    {
        /* no file selected, show usage and terminate */
        fprintf(stderr,"ERROR: No file selected\n");
        usage();
        return -1;
    }

    dlt_file_free(&file,vflag);

    return 0;
}
