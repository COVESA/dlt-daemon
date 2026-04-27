/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, V2 - Volvo Group
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
 * \author Shivam Goel <shivam.goel@volvo.com>
 *
 * \copyright Copyright © 2011-2015 V2 - Volvo Group. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-convert-v2.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-convert-v2.c                                              **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Shivam Goel <shivam.goel@volvo.com>                           **
**                                                                            **
**  PURPOSE   : Read DLT v2 files, print or store DLT v2 messages.           **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision:  $
 * $LastChangedDate: 2026-04-27 00:00:00 +0000 (Sun, 27. Apr 2026) $
 * $LastChangedBy$
 * Initials    Date         Comment
 * sg          27.04.2026   initial
 */

#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>   /* write() */

#include "dlt_common.h"

#define COMMAND_SIZE        1024    /* Size of command buffer */
#define FILENAME_SIZE       1024    /* Size of filename buffer */
#define DLT_EXTENSION       "dlt"
#define DLT_CONVERT_V2_WS   "/tmp/dlt_convert_v2_workspace/"

/* Index grows in blocks of this many entries */
#define DLT_CVT_V2_INDEX_ALLOC 1000

/*
 * DltConvertFileV2 - encapsulates state for reading a DLT v2 file.
 *
 * The library does not yet provide dlt_file_open_v2 / dlt_file_read_v2, so
 * we implement the same pattern here using dlt_message_read_v2 directly.
 */
typedef struct {
    FILE    *handle;
    long    *index;          /* file offset of each accepted message           */
    int32_t  counter;        /* messages accepted by filter                    */
    int32_t  counter_total;  /* total messages seen                            */
    DltMessageV2 msgv2;      /* parsed representation of the current message   */
    uint8_t *raw_buffer;     /* raw bytes of the current message (as on disk)  */
    uint32_t raw_buf_size;   /* allocated size of raw_buffer                   */
    uint32_t raw_msg_size;   /* actual byte count of the current message       */
    DltFilter *filter;       /* optional filter (NULL = accept all)            */
} DltConvertFileV2;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

static DltReturnValue dlt_cvt_file_v2_init(DltConvertFileV2 *file, int verbose)
{
    if (file == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    memset(file, 0, sizeof(DltConvertFileV2));
    return dlt_message_init_v2(&file->msgv2, verbose);
}

static void dlt_cvt_file_v2_set_filter(DltConvertFileV2 *file, DltFilter *filter)
{
    if (file != NULL)
        file->filter = filter;
}

static DltReturnValue dlt_cvt_file_v2_open(DltConvertFileV2 *file,
                                            const char *filename,
                                            int verbose)
{
    if ((file == NULL) || (filename == NULL))
        return DLT_RETURN_WRONG_PARAMETER;

    file->counter       = 0;
    file->counter_total = 0;

    if (file->handle)
        fclose(file->handle);

    file->handle = fopen(filename, "rb");

    if (file->handle == NULL) {
        fprintf(stderr, "File %s cannot be opened!\n", filename);
        return DLT_RETURN_ERROR;
    }

    if (verbose)
        fprintf(stdout, "Opened DLT v2 file: %s\n", filename);

    return DLT_RETURN_OK;
}

/*
 * dlt_cvt_file_v2_read_one - read and parse one v2 message from the current
 * file position.
 *
 * On success the function populates file->msgv2 and file->raw_buffer /
 * file->raw_msg_size with the raw on-disk bytes of the message.
 *
 * Returns DLT_RETURN_OK on success, DLT_RETURN_ERROR on EOF or parse error.
 */
static DltReturnValue dlt_cvt_file_v2_read_one(DltConvertFileV2 *file, int verbose)
{
    uint8_t fixed_buf[STORAGE_HEADER_V2_FIXED_SIZE];
    uint8_t base_buf[BASE_HEADER_V2_FIXED_SIZE];
    DltBaseHeaderV2 *bh;
    uint32_t storageheadersizev2;
    uint16_t len;
    uint32_t total_size;
    uint8_t  ecidlen;

    /* --- Step 1: read the fixed part of the storage header (14 bytes) --- */
    if (fread(fixed_buf, 1, STORAGE_HEADER_V2_FIXED_SIZE, file->handle)
            != STORAGE_HEADER_V2_FIXED_SIZE)
        return DLT_RETURN_ERROR;  /* EOF or short read */

    /* Verify DLT v2 pattern: 'D','L','T',0x02 */
    if (fixed_buf[0] != 'D' || fixed_buf[1] != 'L' ||
        fixed_buf[2] != 'T' || fixed_buf[3] != 0x02) {
        fprintf(stderr, "ERROR: Not a DLT v2 storage header at current position\n");
        return DLT_RETURN_ERROR;
    }

    /* ecidlen is at byte offset 13 within the storage header */
    ecidlen             = fixed_buf[13];
    storageheadersizev2 = (uint32_t)(STORAGE_HEADER_V2_FIXED_SIZE + ecidlen);

    /* Skip the variable-length ECU ID field inside the storage header */
    if ((ecidlen > 0) && (fseek(file->handle, (long)ecidlen, SEEK_CUR) != 0))
        return DLT_RETURN_ERROR;

    /* --- Step 2: read the base header (7 bytes) to obtain message length --- */
    if (fread(base_buf, 1, BASE_HEADER_V2_FIXED_SIZE, file->handle)
            != BASE_HEADER_V2_FIXED_SIZE)
        return DLT_RETURN_ERROR;

    bh          = (DltBaseHeaderV2 *)base_buf;
    len         = DLT_BETOH_16(bh->len);   /* big-endian: size excluding storage hdr */
    total_size  = storageheadersizev2 + (uint32_t)len;

    /* --- Step 3: seek back to the start of the message and read everything --- */
    if (fseek(file->handle,
              -((long)storageheadersizev2 + (long)BASE_HEADER_V2_FIXED_SIZE),
              SEEK_CUR) != 0)
        return DLT_RETURN_ERROR;

    /* Grow the raw buffer if the current message is larger than previous ones */
    if (total_size > file->raw_buf_size) {
        free(file->raw_buffer);
        file->raw_buffer = (uint8_t *)malloc(total_size);

        if (file->raw_buffer == NULL) {
            file->raw_buf_size = 0;
            fprintf(stderr, "ERROR: Cannot allocate %u bytes for message buffer\n",
                    total_size);
            return DLT_RETURN_ERROR;
        }

        file->raw_buf_size = total_size;
    }

    file->raw_msg_size = total_size;

    if (fread(file->raw_buffer, 1, total_size, file->handle) != total_size)
        return DLT_RETURN_ERROR;

    /* --- Step 4: parse the raw bytes into file->msgv2 --- */
    if (dlt_message_read_v2(&file->msgv2, file->raw_buffer, total_size, 0, verbose)
            != DLT_MESSAGE_ERROR_OK)
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

/*
 * dlt_cvt_file_v2_read - read the next v2 message, update the index, and
 * apply the optional filter.
 *
 * Returns:
 *   DLT_RETURN_TRUE  – message read and passes filter  (index updated)
 *   DLT_RETURN_OK    – message read but filtered out   (index NOT updated)
 *   DLT_RETURN_ERROR – EOF or parse error              (stop reading loop)
 */
static DltReturnValue dlt_cvt_file_v2_read(DltConvertFileV2 *file, int verbose)
{
    long  start_pos;
    long *ptr;

    if (file == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (verbose)
        fprintf(stdout, "%s: Message %d\n", __func__, file->counter_total);

    /* Grow the position index in blocks of DLT_CVT_V2_INDEX_ALLOC */
    if (file->counter % DLT_CVT_V2_INDEX_ALLOC == 0) {
        ptr = (long *)malloc(
            (size_t)((file->counter / DLT_CVT_V2_INDEX_ALLOC) + 1)
            * (size_t)DLT_CVT_V2_INDEX_ALLOC
            * sizeof(long));

        if (ptr == NULL)
            return DLT_RETURN_ERROR;

        if (file->index) {
            memcpy(ptr, file->index, (size_t)file->counter * sizeof(long));
            free(file->index);
        }

        file->index = ptr;
    }

    start_pos = ftell(file->handle);
    if (start_pos < 0)
        return DLT_RETURN_ERROR;

    if (dlt_cvt_file_v2_read_one(file, verbose) != DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    file->counter_total++;

    if (file->filter) {
        if (dlt_message_filter_check_v2(&file->msgv2, file->filter, verbose)
                == DLT_RETURN_TRUE) {
            file->index[file->counter] = start_pos;
            file->counter++;
            return DLT_RETURN_TRUE;
        }
        return DLT_RETURN_OK;   /* filtered out – keep scanning */
    }

    /* No filter: accept every message */
    file->index[file->counter] = start_pos;
    file->counter++;
    return DLT_RETURN_TRUE;
}

/*
 * dlt_cvt_file_v2_message - seek to message `num` (0-based, filter-adjusted
 * index) and populate file->msgv2 / raw_buffer.
 */
static DltReturnValue dlt_cvt_file_v2_message(DltConvertFileV2 *file,
                                               int32_t num,
                                               int verbose)
{
    if ((file == NULL) || (num < 0) || (num >= file->counter))
        return DLT_RETURN_WRONG_PARAMETER;

    if (fseek(file->handle, file->index[num], SEEK_SET) != 0)
        return DLT_RETURN_ERROR;

    return dlt_cvt_file_v2_read_one(file, verbose);
}

static DltReturnValue dlt_cvt_file_v2_free(DltConvertFileV2 *file, int verbose)
{
    if (file == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (file->handle) {
        fclose(file->handle);
        file->handle = NULL;
    }

    if (file->index) {
        free(file->index);
        file->index = NULL;
    }

    free(file->raw_buffer);
    file->raw_buffer  = NULL;
    file->raw_buf_size = 0;
    file->raw_msg_size = 0;

    return dlt_message_free_v2(&file->msgv2, verbose);
}

/* -------------------------------------------------------------------------
 * empty_dir – remove all files inside a directory (shared with dlt-convert)
 * ---------------------------------------------------------------------- */
static void empty_dir(const char *dir)
{
    struct dirent **files = { 0 };
    struct stat st;
    char tmp_filename[FILENAME_SIZE] = { 0 };

    if (dir == NULL) {
        fprintf(stderr, "ERROR: %s: invalid arguments\n", __func__);
        return;
    }

    if (stat(dir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            int n = scandir(dir, &files, NULL, alphasort);

            if (n < 0) {
                fprintf(stderr, "ERROR: Failed to scan %s: %s\n", dir, strerror(errno));
                free(files);
                return;
            }

            if (n < 2) {
                fprintf(stderr, "ERROR: Failed to scan %s: %s\n", dir, strerror(errno));
                for (int i = 0; i < n; i++)
                    free(files[i]);
                free(files);
                return;
            }
            else if (n == 2) {
                printf("%s is already empty\n", dir);
            }
            else {
                for (int i = 2; i < n; i++) {
                    memset(tmp_filename, 0, FILENAME_SIZE);
                    /* Validate filename to prevent path traversal */
                    if (strstr(files[i]->d_name, "/") == NULL &&
                        strstr(files[i]->d_name, "..") == NULL) {
                        snprintf(tmp_filename, FILENAME_SIZE, "%s%s",
                                 dir, files[i]->d_name);
                        if (remove(tmp_filename) != 0)
                            fprintf(stderr, "ERROR: Failed to delete %s: %s\n",
                                    tmp_filename, strerror(errno));
                    } else {
                        fprintf(stderr, "WARNING: Skipping suspicious filename: %s\n",
                                files[i]->d_name);
                    }
                }
            }

            for (int i = 0; i < n; i++) {
                free(files[i]);
                files[i] = NULL;
            }
            free(files);
        }
        else {
            fprintf(stderr, "ERROR: %s is not a directory\n", dir);
        }
    }
    else {
        fprintf(stderr, "ERROR: Failed to stat %s: %s\n", dir, strerror(errno));
    }
}

/* -------------------------------------------------------------------------
 * usage
 * ---------------------------------------------------------------------- */
static void usage(void)
{
    char version[DLT_CONVERT_TEXTBUFSIZE];

    dlt_get_version(version, 255);

    printf("Usage: dlt-convert-v2 [options] [commands] file1 [file2]\n");
    printf("Read DLT v2 files, print DLT v2 messages as ASCII and store the messages again.\n");
    printf("Use filters to filter DLT v2 messages.\n");
    printf("Use Ranges and Output file to cut DLT v2 files.\n");
    printf("Use two files and Output file to join DLT v2 files.\n");
    printf("%s \n", version);
    printf("Commands:\n");
    printf("  -h            Usage\n");
    printf("  -a            Print DLT v2 file; payload as ASCII\n");
    printf("  -x            Print DLT v2 file; payload as hex\n");
    printf("  -m            Print DLT v2 file; payload as hex and ASCII\n");
    printf("  -s            Print DLT v2 file; only headers\n");
    printf("  -o filename   Output messages in new DLT v2 file\n");
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -c            Count number of messages\n");
    printf("  -f filename   Enable filtering of messages\n");
    printf("  -b number     First <number> messages to be handled\n");
    printf("  -e number     Last <number> messages to be handled\n");
    printf("  -w            Follow dlt v2 file while file is increasing\n");
    printf("  -t            Handling input compressed files (tar.gz)\n");
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
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
    char *fvalue = NULL;
    char *bvalue = NULL;
    char *evalue = NULL;
    char *ovalue = NULL;

    int index;
    int c;

    DltConvertFileV2 file;
    DltFilter        filter;

    int ohandle = -1;

    int num, begin, end;

    char text[DLT_CONVERT_TEXTBUFSIZE] = { 0 };

    /* For handling compressed files */
    char tmp_filename[FILENAME_SIZE] = { 0 };
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    struct dirent **files = { 0 };
    int n = 0;

    ssize_t bytes_written = 0;
    int syserr = 0;

    opterr = 0;

    while ((c = getopt(argc, argv, "vcashxmwtf:b:e:o:")) != -1) {
        switch (c) {
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
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);

            usage();
            return -1;
        }
        default:
        {
            return -1;    /* for parasoft */
        }
        }
    }

    /* Initialize v2 file context */
    dlt_cvt_file_v2_init(&file, vflag);

    /* Optionally load the v2 filter file */
    if (fvalue) {
        memset(&filter, 0, sizeof(DltFilter));
        dlt_filter_init(&filter, vflag);

        if (dlt_filter_load_v2(&filter, fvalue, vflag) < DLT_RETURN_OK) {
            dlt_cvt_file_v2_free(&file, vflag);
            return -1;
        }

        dlt_cvt_file_v2_set_filter(&file, &filter);
    }

    if (ovalue) {
        ohandle = open(ovalue, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (ohandle == -1) {
            dlt_cvt_file_v2_free(&file, vflag);
            fprintf(stderr, "ERROR: Output file %s cannot be opened!\n", ovalue);
            return -1;
        }
    }

    if (tflag) {
        /* Prepare the temp directory for uncompressed files */
        if (mkdir(DLT_CONVERT_V2_WS, 0700) != 0) {
            if (errno != EEXIST) {
                fprintf(stderr, "ERROR: Cannot create temp dir %s!\n", DLT_CONVERT_V2_WS);

                if (ovalue) {
                    close(ohandle);
                    ohandle = -1;
                }

                dlt_cvt_file_v2_free(&file, vflag);
                return -1;
            }
        }
        else {
            if (S_ISDIR(st.st_mode))
                empty_dir(DLT_CONVERT_V2_WS);
            else
                fprintf(stderr, "ERROR: %s is not a directory\n", DLT_CONVERT_V2_WS);
        }

        for (index = optind; index < argc; index++) {
            const char *file_ext = get_filename_ext(argv[index]);

            if (file_ext && strcmp(file_ext, DLT_EXTENSION) != 0) {
                syserr = dlt_execute_command(NULL, "tar", "xf", argv[index],
                                             "-C", DLT_CONVERT_V2_WS, NULL);
                if (syserr != 0)
                    fprintf(stderr,
                            "ERROR: Failed to uncompress %s to %s [%d]\n",
                            argv[index], DLT_CONVERT_V2_WS, WIFEXITED(syserr));
            }
            else {
                syserr = dlt_execute_command(NULL, "cp", argv[index],
                                             DLT_CONVERT_V2_WS, NULL);
                if (syserr != 0)
                    fprintf(stderr,
                            "ERROR: Failed to copy %s to %s [%d]\n",
                            argv[index], DLT_CONVERT_V2_WS, WIFEXITED(syserr));
            }
        }

        n = scandir(DLT_CONVERT_V2_WS, &files, NULL, alphasort);

        if (n == -1) {
            fprintf(stderr, "ERROR: Cannot scan temp dir %s!\n", DLT_CONVERT_V2_WS);

            if (ovalue) {
                close(ohandle);
                ohandle = -1;
            }

            dlt_cvt_file_v2_free(&file, vflag);
            return -1;
        }

        /* exclude ./ and ../ */
        argc = optind + (n - 2);
    }

    for (index = optind; index < argc; index++) {
        if (tflag) {
            memset(tmp_filename, 0, FILENAME_SIZE);
            snprintf(tmp_filename, FILENAME_SIZE, "%s%s",
                     DLT_CONVERT_V2_WS, files[index - optind + 2]->d_name);
            argv[index] = tmp_filename;
        }

        /* Open file and build the message index */
        if (dlt_cvt_file_v2_open(&file, argv[index], vflag) >= DLT_RETURN_OK) {
            while (dlt_cvt_file_v2_read(&file, vflag) >= DLT_RETURN_OK) { }
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
                fprintf(stderr,
                        "ERROR: Selected first message %d is out of range!\n",
                        begin);

                if (ovalue) {
                    close(ohandle);
                    ohandle = -1;
                }

                dlt_cvt_file_v2_free(&file, vflag);
                return -1;
            }

            if ((end < 0) || (end >= file.counter) || (end < begin)) {
                fprintf(stderr,
                        "ERROR: Selected end message %d is out of range!\n",
                        end);

                if (ovalue) {
                    close(ohandle);
                    ohandle = -1;
                }

                dlt_cvt_file_v2_free(&file, vflag);
                return -1;
            }

            for (num = begin; num <= end; num++) {
                if (dlt_cvt_file_v2_message(&file, num, vflag) < DLT_RETURN_OK)
                    continue;

                if (xflag) {
                    printf("%d ", num);
                    if (dlt_message_print_hex_v2(&file.msgv2, text,
                                                  DLT_CONVERT_TEXTBUFSIZE,
                                                  vflag) < DLT_RETURN_OK)
                        continue;
                }
                else if (aflag) {
                    printf("%d ", num);

                    if (dlt_message_header_v2(&file.msgv2, text,
                                               DLT_CONVERT_TEXTBUFSIZE,
                                               vflag) < DLT_RETURN_OK)
                        continue;

                    printf("%s ", text);

                    if (dlt_message_payload_v2(&file.msgv2, text,
                                                DLT_CONVERT_TEXTBUFSIZE,
                                                DLT_OUTPUT_ASCII,
                                                vflag) < DLT_RETURN_OK)
                        continue;

                    printf("[%s]\n", text);
                }
                else if (mflag) {
                    printf("%d ", num);
                    if (dlt_message_print_mixed_plain_v2(&file.msgv2, text,
                                                          DLT_CONVERT_TEXTBUFSIZE,
                                                          vflag) < DLT_RETURN_OK)
                        continue;
                }
                else if (sflag) {
                    printf("%d ", num);

                    if (dlt_message_header_v2(&file.msgv2, text,
                                               DLT_CONVERT_TEXTBUFSIZE,
                                               vflag) < DLT_RETURN_OK)
                        continue;

                    printf("%s \n", text);
                }

                /* Write message to output file if requested.
                 * raw_buffer contains the complete on-disk bytes of the message
                 * (storage header + base header + extras + extended header +
                 * payload), so it can be written verbatim. */
                if (ovalue) {
                    bytes_written = write(ohandle, file.raw_buffer, file.raw_msg_size);

                    if (bytes_written < 0) {
                        fprintf(stderr,
                                "ERROR: write to output file failed: %s\n",
                                strerror(errno));
                        close(ohandle);
                        ohandle = -1;
                        dlt_cvt_file_v2_free(&file, vflag);
                        return -1;
                    }
                }

                /* Follow mode: when the last indexed message is reached, wait
                 * for new messages to appear in the file. */
                if (wflag && (num == end)) {
                    while (1) {
                        while (dlt_cvt_file_v2_read(&file, 0) >= DLT_RETURN_OK) { }

                        if (end == (file.counter - 1)) {
                            /* No new messages yet – sleep briefly */
                            struct timespec req;
                            req.tv_sec  = 0;
                            req.tv_nsec = 100000000;   /* 100 ms */
                            nanosleep(&req, NULL);
                        }
                        else {
                            /* New messages found – extend the range */
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

    if (ovalue) {
        close(ohandle);
        ohandle = -1;
    }

    if (tflag) {
        empty_dir(DLT_CONVERT_V2_WS);

        if (files) {
            for (int i = 0; i < n; i++)
                free(files[i]);

            free(files);
        }

        rmdir(DLT_CONVERT_V2_WS);
    }

    if (index == optind) {
        /* No file was selected */
        fprintf(stderr, "ERROR: No file selected\n");
        usage();
        dlt_cvt_file_v2_free(&file, vflag);
        return -1;
    }

    dlt_cvt_file_v2_free(&file, vflag);

    if (fvalue)
        dlt_filter_free(&filter, vflag);

    return 0;
}
