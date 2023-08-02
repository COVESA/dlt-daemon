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
 * \file dlt-convert.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-convert.c                                                 **
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
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/uio.h> /* writev() */

#include "dlt_common.h"

#define COMMAND_SIZE        1024    /* Size of command */
#define FILENAME_SIZE       1024    /* Size of filename */
#define DLT_EXTENSION       "dlt"
#define DLT_CONVERT_WS      "/tmp/dlt_convert_workspace/"

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[DLT_CONVERT_TEXTBUFSIZE];

    dlt_get_version(version, 255);

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
    printf("  -t            Handling input compressed files (tar.gz)\n");
}

void empty_dir(const char *dir)
{
    struct dirent **files = { 0 };
    struct stat st;
    uint32_t n = 0;
    char tmp_filename[FILENAME_SIZE] = { 0 };
    uint32_t i;

    if (dir == NULL)
        fprintf(stderr, "ERROR: %s: invalid arguments\n", __FUNCTION__);

    if (stat(dir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            n = (uint32_t) scandir(dir, &files, NULL, alphasort);

            /* Do not include /. and /.. */
            if (n < 2)
                fprintf(stderr, "ERROR: Failed to scan %s with error %s\n",
                        dir, strerror(errno));
            else if (n == 2)
                printf("%s is already empty\n", dir);
            else {
                for (i = 2; i < n; i++) {
                    memset(tmp_filename, 0, FILENAME_SIZE);
                    snprintf(tmp_filename, FILENAME_SIZE, "%s%s", dir, files[i]->d_name);

                    if (remove(tmp_filename) != 0)
                        fprintf(stderr, "ERROR: Failed to delete %s with error %s\n",
                                tmp_filename, strerror(errno));
                }
                if (files) {
                    for (i = 0; i < n ; i++)
                        if (files[i]) {
                            free(files[i]);
                            files[i] = NULL;
                        }
                    free(files);
                    files = NULL;
                }
            }
        }
        else
            fprintf(stderr, "ERROR: %s is not a directory\n", dir);
    }
    else
        fprintf(stderr, "ERROR: Failed to stat %s with error %s\n", dir, strerror(errno));
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    int vflag = 0;
    int cflag = 0;
    int aflag = 0;
    int sflag = 0;
    int xflag = 0;
    int mflag = 0;
    int wflag = 0;
    int tflag = 0;
    char *fvalue = 0;
    char *bvalue = 0;
    char *evalue = 0;
    char *ovalue = 0;

    int index;
    int c;

    DltFile file;
    DltFilter filter;

    int ohandle = -1;

    int num, begin, end;

    char text[DLT_CONVERT_TEXTBUFSIZE] = { 0 };

    /* For handling compressed files */
    char tmp_filename[FILENAME_SIZE] = { 0 };
    struct stat st = { 0 };
    struct dirent **files = { 0 };
    int n = 0;
    int i = 0;

    struct iovec iov[2];
    int bytes_written = 0;
    int syserr = 0;

    opterr = 0;

    while ((c = getopt (argc, argv, "vcashxmwtf:b:e:o:")) != -1) {
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
        case 't':
        {
            tflag = 1;
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
            if ((optopt == 'f') || (optopt == 'b') || (optopt == 'e') || (optopt == 'o'))
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
            return -1;    /*for parasoft */
        }
        }
    }

    /* Initialize structure to use DLT file */
    dlt_file_init(&file, vflag);

    /* first parse filter file if filter parameter is used */
    if (fvalue) {
        if (dlt_filter_load(&filter, fvalue, vflag) < DLT_RETURN_OK) {
            dlt_file_free(&file, vflag);
            return -1;
        }

        dlt_file_set_filter(&file, &filter, vflag);
    }

    if (ovalue) {
        ohandle = open(ovalue, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

        if (ohandle == -1) {
            dlt_file_free(&file, vflag);
            fprintf(stderr, "ERROR: Output file %s cannot be opened!\n", ovalue);
            return -1;
        }
    }

    if (tflag) {
        /* Prepare the temp dir to untar compressed files */
        if (stat(DLT_CONVERT_WS, &st) == -1) {
            if (mkdir(DLT_CONVERT_WS, 0700) != 0) {
                fprintf(stderr,"ERROR: Cannot create temp dir %s!\n", DLT_CONVERT_WS);
                if (ovalue)
                    close(ohandle);

                return -1;
            }
        }
        else {
            if (S_ISDIR(st.st_mode))
                empty_dir(DLT_CONVERT_WS);
            else
                fprintf(stderr, "ERROR: %s is not a directory", DLT_CONVERT_WS);
        }

        for (index = optind; index < argc; index++) {
            /* Check extension of input file
             * If it is a compressed file, uncompress it
             */
            if (strcmp(get_filename_ext(argv[index]), DLT_EXTENSION) != 0) {
                syserr = dlt_execute_command(NULL, "tar", "xf", argv[index], "-C", DLT_CONVERT_WS, NULL);
                if (syserr != 0)
                    fprintf(stderr, "ERROR: Failed to uncompress %s to %s with error [%d]\n",
                            argv[index], DLT_CONVERT_WS, WIFEXITED(syserr));
            }
            else {
                syserr = dlt_execute_command(NULL, "cp", argv[index], DLT_CONVERT_WS, NULL);
                if (syserr != 0)
                    fprintf(stderr, "ERROR: Failed to copy %s to %s with error [%d]\n",
                            argv[index], DLT_CONVERT_WS, WIFEXITED(syserr));
            }

        }

        n = scandir(DLT_CONVERT_WS, &files, NULL, alphasort);
        if (n == -1) {
            fprintf(stderr,"ERROR: Cannot scan temp dir %s!\n", DLT_CONVERT_WS);
            if (ovalue)
                close(ohandle);

            return -1;
        }

        /* do not include ./ and ../ in the files */
        argc = optind + (n - 2);
    }

    for (index = optind; index < argc; index++) {
        if (tflag) {
            memset(tmp_filename, 0, FILENAME_SIZE);
            snprintf(tmp_filename, FILENAME_SIZE, "%s%s",
                    DLT_CONVERT_WS, files[index - optind + 2]->d_name);

            argv[index] = tmp_filename;
        }

        /* load, analyze data file and create index list */
        if (dlt_file_open(&file, argv[index], vflag) >= DLT_RETURN_OK) {
            while (dlt_file_read(&file, vflag) >= DLT_RETURN_OK) {
            }
        }

        if (aflag || sflag || xflag || mflag || ovalue) {
            if (bvalue)
                begin = atoi(bvalue);
            else
                begin = 0;

            if (evalue && (wflag == 0))
                end = atoi(evalue);
            else
                end = file.counter - 1;

            if ((begin < 0) || (begin >= file.counter)) {
                fprintf(stderr, "ERROR: Selected first message %d is out of range!\n", begin);
                if (ovalue)
                    close(ohandle);

                dlt_file_free(&file, vflag);
                return -1;
            }

            if ((end < 0) || (end >= file.counter) || (end < begin)) {
                fprintf(stderr, "ERROR: Selected end message %d is out of range!\n", end);
                if (ovalue)
                    close(ohandle);

                dlt_file_free(&file, vflag);
                return -1;
            }

            for (num = begin; num <= end; num++) {
                if (dlt_file_message(&file, num, vflag) < DLT_RETURN_OK)
                    continue;

                if (xflag) {
                    printf("%d ", num);
                    if (dlt_message_print_hex(&(file.msg), text, DLT_CONVERT_TEXTBUFSIZE, vflag) < DLT_RETURN_OK)
                        continue;
                }
                else if (aflag) {
                    printf("%d ", num);

                    if (dlt_message_header(&(file.msg), text, DLT_CONVERT_TEXTBUFSIZE, vflag) < DLT_RETURN_OK)
                        continue;

                    printf("%s ", text);

                    if (dlt_message_payload(&file.msg, text, DLT_CONVERT_TEXTBUFSIZE, DLT_OUTPUT_ASCII, vflag) < DLT_RETURN_OK)
                        continue;

                    printf("[%s]\n", text);
                }
                else if (mflag) {
                    printf("%d ", num);
                    if (dlt_message_print_mixed_plain(&(file.msg), text, DLT_CONVERT_TEXTBUFSIZE, vflag) < DLT_RETURN_OK)
                        continue;
                }
                else if (sflag) {
                    printf("%d ", num);

                    if (dlt_message_header(&(file.msg), text, DLT_CONVERT_TEXTBUFSIZE, vflag) < DLT_RETURN_OK)
                        continue;

                    printf("%s \n", text);
                }

                /* if file output enabled write message */
                if (ovalue) {
                    iov[0].iov_base = file.msg.headerbuffer;
                    iov[0].iov_len = (uint32_t) file.msg.headersize;
                    iov[1].iov_base = file.msg.databuffer;
                    iov[1].iov_len = (uint32_t) file.msg.datasize;

                    bytes_written =(int) writev(ohandle, iov, 2);

                    if (0 > bytes_written) {
                        printf("in main: writev(ohandle, iov, 2); returned an error!");
                        close(ohandle);
                        dlt_file_free(&file, vflag);
                        return -1;
                    }
                }

                /* check for new messages if follow flag set */
                if (wflag && (num == end)) {
                    while (1) {
                        while (dlt_file_read(&file, 0) >= 0){
                        }

                        if (end == (file.counter - 1)) {
                            /* Sleep if no new message was received */
                            struct timespec req;
                            req.tv_sec = 0;
                            req.tv_nsec = 100000000;
                            nanosleep(&req, NULL);
                        }
                        else {
                            /* set new end of log file and continue reading */
                            end = file.counter - 1;
                            break;
                        }
                    }
                }
            }
        }

        if (cflag) {
            printf("Total number of messages: %d\n", file.counter_total);

            if (file.filter)
                printf("Filtered number of messages: %d\n", file.counter);
        }
    }

    if (ovalue)
        close(ohandle);

    if (tflag) {
        empty_dir(DLT_CONVERT_WS);
        if (files) {
            for (i = 0; i < n ; i++)
                if (files[i])
                    free(files[i]);

            free(files);
        }
        rmdir(DLT_CONVERT_WS);
    }
    if (index == optind) {
        /* no file selected, show usage and terminate */
        fprintf(stderr, "ERROR: No file selected\n");
        usage();
        return -1;
    }

    dlt_file_free(&file, vflag);

    return 0;
}
