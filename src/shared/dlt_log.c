/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2024, Mercedes Benz Tech Innovation GmbH
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see https://www.covesa.global/.
 */

/*!
 * \author
 * Daniel Weber <daniel.w.weber@mercedes-benz.com>
 *
 * \copyright Copyright © 2024 Mercedes Benz Tech Innovation GmbH. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_log.c
 */

#include "dlt_log.h"
#include "dlt_common.h"
#include "dlt_multiple_files.h"
#include <syslog.h>
#include <errno.h>
#include <libgen.h>   /* dirname */
#include <limits.h>   /* for NAME_MAX */
#include <string.h>   /* for strlen() */
#include <stdlib.h>   /* for calloc(), free() */
#include <stdarg.h>   /* va_list, va_start */

/* internal logging parameters */
static int logging_level = LOG_INFO;
static char logging_filename[NAME_MAX + 1] = "";
DltLoggingMode logging_mode = DLT_LOG_TO_STDERR;
FILE *logging_handle = NULL;

//use ohandle as an indicator that multiple files logging is active
MultipleFilesRingBuffer multiple_files_ring_buffer = {
        .directory={0},
        .filename={0},
        .fileSize=0,
        .maxSize=0,
        .filenameTimestampBased=false,
        .filenameBase={0},
        .filenameExt={0},
        .ohandle=-1};


void dlt_log_set_filename(const char *filename)
{
    /* check nullpointer */
    if (filename == NULL) {
        dlt_log(LOG_WARNING, "Wrong parameter: filename is NULL\n");
        return;
    }

    strncpy(logging_filename, filename, NAME_MAX);
    logging_filename[NAME_MAX] = 0;
}

void dlt_log_set_level(int level)
{
    if ((level < 0) || (level > LOG_DEBUG)) {
        if (logging_level < LOG_WARNING)
            logging_level = LOG_WARNING;

        dlt_vlog(LOG_WARNING, "Wrong parameter for level: %d\n", level);
    }
    else {
        logging_level = level;
    }
}

DltReturnValue dlt_log_init(int mode)
{
    return dlt_log_init_multiple_logfiles_support((DltLoggingMode)mode, false, 0, 0);
}


DltReturnValue dlt_log_init_multiple_logfiles_support(const DltLoggingMode mode, const bool enable_multiple_logfiles,
                                                      const int logging_file_size, const int logging_files_max_size)
{
    if ((mode < DLT_LOG_TO_CONSOLE) || (mode > DLT_LOG_DROPPED)) {
        dlt_vlog(LOG_WARNING, "Wrong parameter for mode: %d\n", mode);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    logging_mode = mode;

    if (logging_mode != DLT_LOG_TO_FILE) {
        return DLT_RETURN_OK;
    }

    DltReturnValue result;
    if (enable_multiple_logfiles) {
        dlt_user_printf("configure dlt logging using file limits\n");
        result = dlt_log_init_multiple_logfiles(logging_file_size, logging_files_max_size);
        if (result != DLT_RETURN_OK) {
            dlt_user_printf("dlt logging for limits fails with error code=%d, use logging without limits as fallback\n", result);
            result = dlt_log_init_single_logfile();
        }
    } else {
        dlt_user_printf("configure dlt logging without file limits\n");
        result = dlt_log_init_single_logfile();
    }

    return result;
}

DltReturnValue dlt_log_init_single_logfile()
{
    /* internal logging to file */
    errno = 0;
    logging_handle = fopen(logging_filename, "a");

    if (logging_handle == NULL) {
        dlt_user_printf("Internal log file %s cannot be opened, error: %s\n", logging_filename, strerror(errno));
        return DLT_RETURN_ERROR;
    }
    return DLT_RETURN_OK;
}

DltReturnValue dlt_log_init_multiple_logfiles(const int logging_file_size, const int logging_files_max_size)
{
    char path_logging_filename[PATH_MAX + 1];
    strncpy(path_logging_filename, logging_filename, PATH_MAX);
    path_logging_filename[PATH_MAX] = 0;

    const char *directory = dirname(path_logging_filename);
    if (directory[0]) {
        char basename_logging_filename[NAME_MAX + 1];
        strncpy(basename_logging_filename, logging_filename, NAME_MAX);
        basename_logging_filename[NAME_MAX] = 0;

        const char *file_name = basename(basename_logging_filename);
        char filename_base[NAME_MAX];
        if (!dlt_extract_base_name_without_ext(file_name, filename_base, sizeof(filename_base))) return DLT_RETURN_ERROR;

        const char *filename_ext = get_filename_ext(file_name);
        if (!filename_ext) return DLT_RETURN_ERROR;

        DltReturnValue result = multiple_files_buffer_init(
                &multiple_files_ring_buffer,
                directory,
                logging_file_size,
                logging_files_max_size,
                false,
                true,
                filename_base,
                filename_ext);

        return result;
    }

    return DLT_RETURN_ERROR;
}

int dlt_user_printf(const char *format, ...)
{
    if (format == NULL) return -1;

    va_list args;
    va_start(args, format);

    int ret = 0;

    switch (logging_mode) {
        case DLT_LOG_TO_CONSOLE:
        case DLT_LOG_TO_SYSLOG:
        case DLT_LOG_TO_FILE:
        case DLT_LOG_DROPPED:
        default:
            ret = vfprintf(stdout, format, args);
            break;
        case DLT_LOG_TO_STDERR:
            ret = vfprintf(stderr, format, args);
            break;
    }

    va_end(args);

    return ret;
}

DltReturnValue dlt_log(int prio, const char *s)
{
    static const char asSeverity[LOG_DEBUG +
                                 2][11] =
            { "EMERGENCY", "ALERT    ", "CRITICAL ", "ERROR    ", "WARNING  ", "NOTICE   ", "INFO     ", "DEBUG    ",
              "         " };
    static const char sFormatString[] = "[%5u.%06u]~DLT~%5d~%s~%s";
    struct timespec sTimeSpec;

    if (s == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (logging_level < prio)
        return DLT_RETURN_OK;

    if ((prio < 0) || (prio > LOG_DEBUG))
        prio = LOG_DEBUG + 1;

    clock_gettime(CLOCK_MONOTONIC, &sTimeSpec);

    switch (logging_mode) {
        case DLT_LOG_TO_CONSOLE:
            /* log to stdout */
            fprintf(stdout, sFormatString,
                    (unsigned int)sTimeSpec.tv_sec,
                    (unsigned int)(sTimeSpec.tv_nsec / 1000),
                    getpid(),
                    asSeverity[prio],
                    s);
            fflush(stdout);
            break;
        case DLT_LOG_TO_STDERR:
            /* log to stderr */
            fprintf(stderr, sFormatString,
                    (unsigned int)sTimeSpec.tv_sec,
                    (unsigned int)(sTimeSpec.tv_nsec / 1000),
                    getpid(),
                    asSeverity[prio],
                    s);
            break;
        case DLT_LOG_TO_SYSLOG:
            /* log to syslog */
#if !defined (__WIN32__) && !defined(_MSC_VER)
            openlog("DLT", LOG_PID, LOG_DAEMON);
            syslog(prio,
                   sFormatString,
                   (unsigned int)sTimeSpec.tv_sec,
                   (unsigned int)(sTimeSpec.tv_nsec / 1000),
                   getpid(),
                   asSeverity[prio],
                   s);
            closelog();
#endif
            break;
        case DLT_LOG_TO_FILE:
            /* log to file */

            if (dlt_is_log_in_multiple_files_active()) {
                dlt_log_multiple_files_write(sFormatString, (unsigned int)sTimeSpec.tv_sec,
                                             (unsigned int)(sTimeSpec.tv_nsec / 1000), getpid(), asSeverity[prio], s);
            }
            else if (logging_handle) {
                fprintf(logging_handle, sFormatString, (unsigned int)sTimeSpec.tv_sec,
                        (unsigned int)(sTimeSpec.tv_nsec / 1000), getpid(), asSeverity[prio], s);
                fflush(logging_handle);
            }

            break;
        case DLT_LOG_DROPPED:
        default:
            break;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_vlog(int prio, const char *format, ...)
{
    char outputString[2048] = { 0 }; /* TODO: what is a reasonable string length here? */

    va_list args;

    if (format == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (logging_level < prio)
        return DLT_RETURN_OK;

    va_start(args, format);
    vsnprintf(outputString, 2047, format, args);
    va_end(args);

    dlt_log(prio, outputString);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_vnlog(int prio, size_t size, const char *format, ...)
{
    char *outputString = NULL;

    va_list args;

    if (format == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if ((logging_level < prio) || (size == 0))
        return DLT_RETURN_OK;

    if ((outputString = (char *)calloc(size + 1, sizeof(char))) == NULL)
        return DLT_RETURN_ERROR;

    va_start(args, format);
    vsnprintf(outputString, size, format, args);
    va_end(args);

    dlt_log(prio, outputString);

    free(outputString);
    outputString = NULL;

    return DLT_RETURN_OK;
}

void dlt_log_multiple_files_write(const char* format, ...)
{
    char output_string[2048] = { 0 };
    va_list args;
    va_start (args, format);
    vsnprintf(output_string, 2047, format, args);
    va_end (args);
    multiple_files_buffer_write(&multiple_files_ring_buffer, (unsigned char*)output_string, (int)strlen(output_string));
}

void dlt_log_free(void)
{
    if (logging_mode == DLT_LOG_TO_FILE) {
        if (dlt_is_log_in_multiple_files_active()) {
            dlt_log_free_multiple_logfiles();
        } else {
            dlt_log_free_single_logfile();
        }
    }
}

void dlt_log_free_single_logfile()
{
    if (logging_handle != NULL) {
        fclose(logging_handle);
    }
}

void dlt_log_free_multiple_logfiles()
{
    if (DLT_RETURN_ERROR == multiple_files_buffer_free(&multiple_files_ring_buffer)) return;

    // reset indicator of multiple files usage
    multiple_files_ring_buffer.ohandle = -1;
}

bool dlt_is_log_in_multiple_files_active()
{
    return multiple_files_ring_buffer.ohandle > -1;
}
