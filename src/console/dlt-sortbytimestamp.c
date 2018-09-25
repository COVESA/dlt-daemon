/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2018, Codethink Ltd.
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
 * \author Jonathan Sambrook <jonathan.sambrook@codethink.co.uk>
 *
 * \copyright Copyright © 2018 Codethink Ltd. \n
 * Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-sortbytimestamp.cpp
*/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-sortbytimestamp.cpp                                       **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Jonathan Sambrook jonathasambrook@codethink.co.uk             **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
**  js          Jonathan Sambrook          Codethink                          **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/uio.h> /* writev() */

#include "dlt_common.h"

#define DLT_VERBUFSIZE  255

typedef struct sTimestampIndex {
    int num;
    uint32_t tmsp;
} TimestampIndex;

int verbosity = 0;

/**
 * Print information, conditional upon requested verbosity level
 */
void verbose(int level, char * msg, ...)
{
    if (level <= verbosity)
    {
        if (verbosity > 1) /* timestamp */
        {
            time_t tnow = time((time_t*)0);
            if (tnow != -1) {
                char * snow = ctime(&tnow);
                /* suppress newline char */ 
                snow[strlen(snow) - 1] = 0;
                printf("%s: ", snow);
            }
        }

        int len = strlen(msg);
        va_list args;
        va_start (args, msg);
        vprintf(msg, args);

        /* lines without a terminal newline aren't guaranteed to be displayed */
        if (msg[len-1] != '\n')
        {
            fflush(stdout);
        }
    }
}

/**
 * Comparison function for use with qsort
 */
int compare_index_timestamps(const void* a, const void *b)
{
    if (((TimestampIndex*)a)->tmsp > ((TimestampIndex*)b)->tmsp)
    {
        return 1;
    }
    else if (((TimestampIndex*)a)->tmsp == ((TimestampIndex*)b)->tmsp)
    {
        return 0;
    }
    return -1;
}

/**
 * Write the messages in the order specified by the given index
 */
void write_messages(int ohandle, DltFile *file, TimestampIndex *timestamps, int message_count)
{
    struct iovec iov[2];
    int bytes_written;

    verbose(1, "Writing %d messages\n", message_count);

    for (int i = 0; i < message_count; ++i)
    {
        if (0 == i % 1001 || i == message_count - 1)
        {
            verbose(2, "Writing message %d\r", i);
        }
        dlt_file_message(file,timestamps[i].num,0);
        iov[0].iov_base = file->msg.headerbuffer;
        iov[0].iov_len = file->msg.headersize;
        iov[1].iov_base = file->msg.databuffer;
        iov[1].iov_len = file->msg.datasize;

        bytes_written = writev(ohandle, iov, 2);
        if (0 > bytes_written){
                printf("in main: writev(ohandle, iov, 2); returned an error!" );
                dlt_file_free(file,0);
                exit (-1);
        }
    }

    verbose (2, "\n");
}

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[DLT_VERBUFSIZE];

    dlt_get_version(version,DLT_VERBUFSIZE);

    printf("Usage: dlt-sortbytimestamp [options] [commands] file_in file_out\n");
    printf("Read DLT file, sort by timestamp and store the messages again.\n");
    printf("Use filters to filter DLT messages.\n");
    printf("Use range to cut DLT file. Indices are zero based.\n");
    printf("%s \n", version);
    printf("Commands:\n");
    printf("  -h            Usage\n");
    printf("Options:\n");
    printf("  -v            Verbosity. Multiple uses will effect an increase in loquacity\n");
    printf("  -c            Count number of messages\n");
    printf("  -f filename   Enable filtering of messages\n");
    printf("  -b number     First message in range to be handled (default: first message)\n");
    printf("  -e number     Last message in range to be handled (default: last message)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    int vflag = 0;
    int cflag = 0;
    char *fvalue = 0;
    char *bvalue = 0;
    char *evalue = 0;
    char *ivalue = 0;
    char *ovalue = 0;

    TimestampIndex *timestamp_index = 0;
    int32_t message_count = 0;

    int c;

    DltFile file;
    DltFilter filter;

    int ohandle=-1;

    int num, begin, end;

    opterr = 0;

    verbose(1, "Configuring\n");

    while ((c = getopt (argc, argv, "vchf:b:e:")) != -1)
        switch (c)
        {
        case 'v':
            {
                verbosity += 1;
                break;
            }
        case 'c':
			{
            	cflag = 1;
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
        case '?':
            {
                if (optopt == 'f' || optopt == 'b' || optopt == 'e')
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

    /* Don't use vflag on quietest levels */
    if (verbosity > 2)
    {
        vflag = 1;
    }

    verbose (1, "Initializing\n");

    /* initialise structure to use DLT file */
    dlt_file_init(&file,vflag);

    /* first parse filter file if filter parameter is used */
    if (fvalue)
    {
        if (bvalue || evalue)
        {
            fprintf(stderr,"ERROR: can't specify a range *and* filtering!\n");
            dlt_file_free(&file,vflag);
            return -1;
        }

        if (dlt_filter_load(&filter,fvalue,vflag) < DLT_RETURN_OK)
        {
            dlt_file_free(&file,vflag);
            return -1;
        }

        dlt_file_set_filter(&file,&filter,vflag);
    }

    ivalue = argv[optind];

    if (!ivalue)
    {
        dlt_file_free(&file,vflag);
        fprintf(stderr,"ERROR: Need an input file!\n");
        return -1;
    }

    ovalue = argv[optind + 1];

    if (ovalue)
    {
        ohandle = open(ovalue,O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */
        if (ohandle == -1)
        {
            dlt_file_free(&file,vflag);
            fprintf(stderr,"ERROR: Output file %s cannot be opened!\n",ovalue);
            return -1;
        }
    }
    else
    {
        dlt_file_free(&file,vflag);
        fprintf(stderr,"ERROR: Need an output file!\n");
        return -1;
    }

    verbose(1, "Loading\n");

    /* load, analyse data file and create index list */
    if (dlt_file_open(&file,ivalue,vflag) >= DLT_RETURN_OK)
    {
        while (dlt_file_read(&file,vflag) >= DLT_RETURN_OK)
        {
        }
    }

    if (cflag) {
        if (fvalue)
        {
            printf("Loaded %d messages, %d after filtering.\n", file.counter_total, file.counter);
        }
        else
        {
            printf("Loaded %d messages.\n", file.counter_total);
        }
    }

    if (bvalue)
    {
        begin = atoi(bvalue);
    }
    else
    {
        begin = 0;
    }

    if (evalue)
    {
        end = atoi(evalue);
    }
    else
    {
        end = file.counter-1;
    }

    if (begin<0 || begin>=file.counter || begin>end)
    {
        fprintf(stderr,"ERROR: Selected first message %d is out of range!\n",begin);
        return -1;
    }
    if (end<0 || end<begin || end>=file.counter)
    {
        fprintf(stderr,"ERROR: Selected end message %d is out of range!\n",end);
        return -1;
    }

    verbose(2, "Begin: %d End: %d Range: %d\n", begin, end, 1 + end - begin);

    verbose(1, "Allocating memory\n");

    message_count = 1 + end - begin;

    timestamp_index = (TimestampIndex*)malloc(sizeof(TimestampIndex) * message_count);

    if (timestamp_index == 0)
    {
        fprintf(stderr,"ERROR: Failed to allocate memory for message index!\n");
        dlt_file_free(&file,vflag);
        return -1;
    }

    verbose(1, "Filling %d entries\n", message_count);
    for (num = begin; num <= end; num++)
    {
        dlt_file_message(&file,num,vflag);
        timestamp_index[num - begin].num = num;
        timestamp_index[num - begin].tmsp = file.msg.headerextra.tmsp;
    }

    verbose(1, "Sorting\n");
    qsort((void*)timestamp_index, message_count, sizeof(TimestampIndex), compare_index_timestamps);

    write_messages(ohandle, &file, timestamp_index, message_count);
    close(ohandle);

    verbose(1, "Tidying up.\n");
    free(timestamp_index);
    dlt_file_free(&file,vflag);
    return 0;
}
