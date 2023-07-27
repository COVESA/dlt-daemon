/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2018, Codethink Ltd.
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
 * \author Jonathan Sambrook <jonathan.sambrook@codethink.co.uk>
 *
 * \copyright Copyright © 2018 Codethink Ltd. \n
 * Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-sortbytimestamp.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-sortbytimestamp.c                                         **
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
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/uio.h> /* writev() */

#include "dlt_common.h"

#define DLT_VERBUFSIZE  255
#define FIFTY_SEC_IN_MSEC 500000
#define THREE_MIN_IN_SEC  180

typedef struct sTimestampIndex {
    int num;
    uint32_t tmsp;
    uint32_t systmsp;
} TimestampIndex;

int verbosity = 0;

/**
 * Print information, conditional upon requested verbosity level
 */
void verbose(int level, char *msg, ...) PRINTF_FORMAT(2, 3);
void verbose(int level, char *msg, ...) {
    if (level <= verbosity) {
        if (verbosity > 1) { /* timestamp */
            time_t tnow = time((time_t *)0);

            if (tnow != -1) {
                char snow[50];
                ctime_r(&tnow, snow);
                /* suppress newline char */
                snow[strlen(snow) - 1] = 0;
                printf("%s: ", snow);
            }
        }

        int len = (int) strlen(msg);
        va_list args;
        va_start (args, msg);
        vprintf(msg, args);
        va_end(args);

        /* lines without a terminal newline aren't guaranteed to be displayed */
        if (msg[len - 1] != '\n')
            fflush(stdout);
    }
}

/**
 * Comparison function for use with qsort
 * Used for time stamp
 */
int compare_index_timestamps(const void *a, const void *b) {
    int ret = -1;
    if (((TimestampIndex *)a)->tmsp > ((TimestampIndex *)b)->tmsp)
        ret = 1;
    else if (((TimestampIndex *)a)->tmsp == ((TimestampIndex *)b)->tmsp)
        ret = 0;

    return ret;
}

/**
 * Comparison function for use with qsort
 * Used for system time
 */
int compare_index_systime(const void *a, const void *b) {
    int ret = -1;
    if(((TimestampIndex *) a)->systmsp > ((TimestampIndex *) b)->systmsp)
        ret = 1;
    else if(((TimestampIndex *) a)->systmsp == ((TimestampIndex *) b)->systmsp)
        ret = 0;

    return ret;
}

/**
 * Write the messages in the order specified by the given index
 */
void write_messages(int ohandle, DltFile *file,
        TimestampIndex *timestamps, uint32_t message_count) {
    struct iovec iov[2];
    ssize_t bytes_written;
    uint32_t i = 0;
    int last_errno = 0;

    verbose(1, "Writing %d messages\n", message_count);

    for (i = 0; i < message_count; ++i) {
        errno = 0;
        if ((0 == i % 1001) || (i == message_count - 1))
            verbose(2, "Writing message %d\r", i);

        if (dlt_file_message(file, timestamps[i].num, 0) < DLT_RETURN_OK)
            continue;
        iov[0].iov_base = file->msg.headerbuffer;
        iov[0].iov_len = file->msg.headersize;
        iov[1].iov_base = file->msg.databuffer;
        iov[1].iov_len = file->msg.datasize;

        bytes_written = writev(ohandle, iov, 2);
        last_errno = errno;

        if (0 > bytes_written) {
            printf("%s: returned an error [%s]!\n",
                    __func__,
                    strerror(last_errno));
            if (ohandle > 0) {
                close(ohandle);
                ohandle = -1;
            }
            if (timestamps) {
                free(timestamps);
                timestamps = NULL;
            }

            dlt_file_free(file, 0);
            exit (-1);
        }
    }

    verbose (2, "\n");
}

/**
 * Print usage information of tool.
 */
void usage() {
    char version[DLT_VERBUFSIZE];

    dlt_get_version(version, DLT_VERBUFSIZE);

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
int main(int argc, char *argv[]) {
    int vflag = 0;
    int cflag = 0;
    char *fvalue = 0;
    char *bvalue = 0;
    char *evalue = 0;
    char *ivalue = 0;
    char *ovalue = 0;

    TimestampIndex *timestamp_index = 0;
    TimestampIndex *temp_timestamp_index = 0;
    uint32_t message_count = 0;

    uint32_t count = 0;
    uint32_t start = 0;
    uint32_t delta_tmsp = 0;
    uint32_t delta_systime = 0;
    size_t i;

    int c;

    DltFile file;
    DltFilter filter;

    int ohandle = -1;

    int num, begin, end;

    opterr = 0;

    verbose(1, "Configuring\n");

    while ((c = getopt (argc, argv, "vchf:b:e:")) != -1) {
        switch (c) {
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
            if ((optopt == 'f') || (optopt == 'b') || (optopt == 'e'))
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
            usage();
            return -1;    /*for parasoft */
        }
        }
    }

    /* Don't use vflag on quietest levels */
    if (verbosity > 2)
        vflag = 1;

    verbose (1, "Initializing\n");

    /* Initialize structure to use DLT file */
    dlt_file_init(&file, vflag);

    /* first parse filter file if filter parameter is used */
    if (fvalue) {
        if (bvalue || evalue) {
            fprintf(stderr, "ERROR: can't specify a range *and* filtering!\n");
            dlt_file_free(&file, vflag);
            return -1;
        }

        if (dlt_filter_load(&filter, fvalue, vflag) < DLT_RETURN_OK) {
            dlt_file_free(&file, vflag);
            return -1;
        }

        dlt_file_set_filter(&file, &filter, vflag);
    }

    ivalue = argv[optind];

    if (!ivalue) {
        dlt_file_free(&file, vflag);
        fprintf(stderr, "ERROR: Need an input file!\n");
        return -1;
    }

    ovalue = argv[optind + 1];

    if (ovalue) {
        ohandle = open(ovalue, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

        if (ohandle == -1) {
            dlt_file_free(&file, vflag);
            fprintf(stderr, "ERROR: Output file %s cannot be opened!\n", ovalue);
            return -1;
        }
    }
    else {
        dlt_file_free(&file, vflag);
        fprintf(stderr, "ERROR: Need an output file!\n");
        return -1;
    }

    verbose(1, "Loading\n");

    /* load, analyze data file and create index list */
    if (dlt_file_open(&file, ivalue, vflag) >= DLT_RETURN_OK) {
        while (dlt_file_read(&file, vflag) >= DLT_RETURN_OK) {
        }
    }

    if (cflag) {
        if (fvalue)
            printf("Loaded %d messages, %d after filtering.\n", file.counter_total, file.counter);
        else
            printf("Loaded %d messages.\n", file.counter_total);
    }

    if (bvalue)
        begin = atoi(bvalue);
    else
        begin = 0;

    if (evalue)
        end = atoi(evalue);
    else
        end = file.counter - 1;

    if ((begin < 0) || (end < 0) || (begin > end) ||
        (begin >= file.counter) || (end >= file.counter)) {
        fprintf(stderr, "ERROR: Selected message [begin-end]-[%d-%d] is out of range!\n", begin, end);
        dlt_file_free(&file, vflag);
        close(ohandle);
        return -1;
    }

    verbose(2, "Begin: %d End: %d Range: %d\n", begin, end, 1 + end - begin);

    verbose(1, "Allocating memory\n");

    message_count = (uint32_t) (1 + end - begin);

    timestamp_index = (TimestampIndex *) malloc(sizeof(TimestampIndex) * (message_count + 1));

    if (timestamp_index == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for message index!\n");
        dlt_file_free(&file, vflag);
        close(ohandle);
        return -1;
    }

    verbose(1, "Filling %d entries\n", message_count);

    for (num = begin; num <= end; num++) {
        if (dlt_file_message(&file, num, vflag) < DLT_RETURN_OK)
            continue;
        timestamp_index[num - begin].num = num;
        timestamp_index[num - begin].systmsp = file.msg.storageheader->seconds;
        timestamp_index[num - begin].tmsp = file.msg.headerextra.tmsp;
    }

    /* This step is extending the array one more element by copying the first element */
    timestamp_index[num].num = timestamp_index[0].num;
    timestamp_index[num].systmsp = timestamp_index[0].systmsp;
    timestamp_index[num].tmsp = timestamp_index[0].tmsp;

    verbose(1, "Sorting\n");
    qsort((void *) timestamp_index, message_count, sizeof(TimestampIndex), compare_index_systime);

    for (num = begin; num <= end; num++) {
        delta_tmsp = (uint32_t)llabs((int64_t)timestamp_index[num + 1].tmsp - timestamp_index[num].tmsp);
        delta_systime = (uint32_t)llabs((int64_t)timestamp_index[num + 1].systmsp - timestamp_index[num].systmsp);

        /*
         * Here is a try to detect a new cycle of boot in system.
         * Relatively, if there are gaps whose systime is larger than 3 mins and
         * timestamp is larger than 15 secs should be identified as a new boot cycle.
         */
        count++;
        if(delta_tmsp > FIFTY_SEC_IN_MSEC || delta_systime >= THREE_MIN_IN_SEC) {
            verbose(1, "Detected a new cycle of boot\n");
            temp_timestamp_index = (TimestampIndex *) malloc(sizeof(TimestampIndex) * count);

            if (temp_timestamp_index == NULL) {
                fprintf(stderr, "ERROR: Failed to allocate memory for array\n");
                dlt_file_free(&file, vflag);
                close(ohandle);
                return -1;
            }

            for (i = 0; i < count; i++) {
                memcpy((void*) &temp_timestamp_index[i],
                        (void*) &timestamp_index[start + i],
                        sizeof(TimestampIndex));
            }
            qsort((void *) temp_timestamp_index, count, sizeof(TimestampIndex),
                    compare_index_timestamps);

            write_messages(ohandle, &file, temp_timestamp_index, count);
            free(temp_timestamp_index);
            temp_timestamp_index = NULL;
            start = start + count;
            count = 0;
        }
    }

    /*
     * In case there is only cycle of boot in DLT file,
     * sort the DLT file again by timestamp then write
     * all messages out.
     */
    if (count == message_count) {
        qsort((void *) timestamp_index, message_count + 1,
              sizeof(TimestampIndex), compare_index_timestamps);
        write_messages(ohandle, &file, timestamp_index, count);
    }

    close(ohandle);
    verbose(1, "Tidying up.\n");
    free(timestamp_index);
    timestamp_index = NULL;
    dlt_file_free(&file, vflag);
    return 0;
}
