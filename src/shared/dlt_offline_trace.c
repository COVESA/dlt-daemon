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
 * \file dlt_offline_trace.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_trace.c                                           **
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <syslog.h>

#include <dlt_offline_trace.h>
#include "dlt_common.h"

unsigned int dlt_offline_trace_storage_dir_info(char *path, char *file_name, char *newest, char *oldest)
{
    int i = 0;
    unsigned int num = 0;
    int cnt = 0;
    struct dirent **files = { 0 };
    char *tmp_old = NULL;
    char *tmp_new = NULL;

    if ((path == NULL) || (file_name == NULL) || (newest == NULL) || (oldest == NULL)) {
        printf("dlt_offline_trace_storage_dir_info: Invalid parameter(s)");
        return 0;
    }

    cnt = scandir(path, &files, NULL, alphasort);

    if (cnt < 0)
        return 0;

    for (i = 0; i < cnt; i++) {
        int len = 0;
        len = strlen(file_name);

        if ((strncmp(files[i]->d_name, file_name, len) == 0) && (files[i]->d_name[len] == '.')) {
            num++;

            if ((tmp_old == NULL) || (strlen(tmp_old) >= strlen(files[i]->d_name))) {
                if (tmp_old == NULL)
                    tmp_old = files[i]->d_name;
                /* when file name is smaller, it is older */
                else if (strlen(tmp_old) > strlen(files[i]->d_name))
                    tmp_old = files[i]->d_name;
                else /* filename is equal, do a string compare */
                    if (strcmp(tmp_old, files[i]->d_name) > 0)
                        tmp_old = files[i]->d_name;
            }

            if ((tmp_new == NULL) || (strlen(tmp_new) <= strlen(files[i]->d_name))) {
                if (tmp_new == NULL) {
                    tmp_new = files[i]->d_name;
                }
                /* when file name is longer, it is younger */
                else if (strlen(tmp_new) < strlen(files[i]->d_name))
                {
                    tmp_new = files[i]->d_name;
                }
                else if (strcmp(tmp_new, files[i]->d_name) < 0)
                    tmp_new = files[i]->d_name;
            }
        }
    }

    if (num > 0) {
        if (tmp_old != NULL) {
            if (strlen(tmp_old) < DLT_OFFLINETRACE_FILENAME_MAX_SIZE)
                strncpy(oldest, tmp_old, DLT_OFFLINETRACE_FILENAME_MAX_SIZE);
        }

        if (tmp_new != NULL) {
            if (strlen(tmp_old) < DLT_OFFLINETRACE_FILENAME_MAX_SIZE)
                strncpy(newest, tmp_new, DLT_OFFLINETRACE_FILENAME_MAX_SIZE);
        }
    }

    /* free scandir result */
    for (i = 0; i < cnt; i++)
        free(files[i]);

    free(files);

    return num;
}

void dlt_offline_trace_file_name(char *log_file_name, char *name, unsigned int idx)
{
    char file_index[11]; /* UINT_MAX = 4294967295 -> 10 digits */
    sprintf(file_index, "%010u", idx);

    /* create log file name */
    memset(log_file_name, 0, DLT_OFFLINETRACE_FILENAME_MAX_SIZE * sizeof(char));
    strncat(log_file_name, name, sizeof(DLT_OFFLINETRACE_FILENAME_BASE));
    strncat(log_file_name, DLT_OFFLINETRACE_FILENAME_DELI, sizeof(DLT_OFFLINETRACE_FILENAME_DELI));
    strncat(log_file_name, file_index, sizeof(file_index));
    strncat(log_file_name, DLT_OFFLINETRACE_FILENAME_EXT, sizeof(DLT_OFFLINETRACE_FILENAME_EXT));
}

unsigned int dlt_offline_trace_get_idx_of_log_file(char *file)
{
    const char d[2] = ".";
    char *token;
    unsigned int idx = 0;

    if (file[0] == '\0')
        return 0;

    token = strtok(file, d);
    /* we are interested in 2. token because of log file name */
    token = strtok(NULL, d);

    if (token != NULL)
        idx = strtol(token, NULL, 10);
    else
        idx = 0;

    return idx;
}


DltReturnValue dlt_offline_trace_create_new_file(DltOfflineTrace *trace)
{
    time_t t;
    struct tm *tmp;
    char outstr[200];
    char newest[DLT_OFFLINETRACE_FILENAME_MAX_SIZE] = { 0 };
    char oldest[DLT_OFFLINETRACE_FILENAME_MAX_SIZE] = { 0 };
    unsigned int idx = 0;

    /* set filename */
    if (trace->filenameTimestampBased) {
        int ret = 0;
        t = time(NULL);
        tmp = localtime(&t);

        if (NULL == tmp) {
            printf("dlt_offline_trace_create_new_file: pointer to tmp is NULL!");
            return DLT_RETURN_ERROR;
        }

        if (strftime(outstr, sizeof(outstr), "%Y%m%d_%H%M%S", tmp) == 0) {}

        ret = snprintf(trace->filename, NAME_MAX, "%s/dlt_offlinetrace_%s.dlt", trace->directory, outstr);

        if ((ret < 0) || (ret >= NAME_MAX)) {
            printf("dlt_offlinetrace filename cannot be concatenated\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        int ret = 0;
        /* targeting newest file, ignoring number of files in dir returned */
        dlt_offline_trace_storage_dir_info(trace->directory, DLT_OFFLINETRACE_FILENAME_BASE, newest, oldest);
        idx = dlt_offline_trace_get_idx_of_log_file(newest) + 1;

        dlt_offline_trace_file_name(outstr, DLT_OFFLINETRACE_FILENAME_BASE, idx);
        ret = snprintf(trace->filename, NAME_MAX, "%s/%s", trace->directory, outstr);

        if ((ret < 0) || (ret >= NAME_MAX)) {
            printf("filename cannot be concatenated\n");
            return DLT_RETURN_ERROR;
        }
    }

    /* open DLT output file */
    trace->ohandle = open(trace->filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

    if (trace->ohandle == -1) {
        /* trace file cannot be opened */
        printf("Offline trace file %s cannot be created\n", trace->filename);
        return DLT_RETURN_ERROR;
    } /* if */

    return DLT_RETURN_OK; /* OK */
}

unsigned long dlt_offline_trace_get_total_size(DltOfflineTrace *trace)
{
    struct dirent *dp;
    char filename[256];
    unsigned long size = 0;
    struct stat status;

    /* go through all dlt files in directory */
    DIR *dir = opendir(trace->directory);

    while ((dp = readdir(dir)) != NULL)
        if (strstr(dp->d_name, DLT_OFFLINETRACE_FILENAME_BASE)) {
            int res = snprintf(filename, sizeof(filename), "%s/%s", trace->directory, dp->d_name);

            /* if the total length of the string is greater than the buffer, silently forget it. */
            /* snprintf: a return value of size  or more means that the output was truncated */
            /*           if an output error is encountered, a negative value is returned. */
            if (((unsigned int)res < sizeof(filename)) && (res > 0)) {
                if (0 == stat(filename, &status))
                    size += status.st_size;
                else
                    printf("Offline trace file %s cannot be stat-ed", filename);
            }

            /*else */
            /*{ */
            /*    dlt_log(3, "dlt_offline_trace_get_total_size: long filename ignored"); */
            /*} */
        }

    closedir(dir);

    /* return size */
    return size;
}

int dlt_offline_trace_delete_oldest_file(DltOfflineTrace *trace)
{
    struct dirent *dp;
    char filename[PATH_MAX + 1];
    char filename_oldest[PATH_MAX + 1];
    unsigned long size_oldest = 0;
    struct stat status;
    time_t time_oldest = 0;

    filename[0] = 0;
    filename_oldest[0] = 0;

    /* go through all dlt files in directory */
    DIR *dir = opendir(trace->directory);

    while ((dp = readdir(dir)) != NULL)
        if (strstr(dp->d_name, DLT_OFFLINETRACE_FILENAME_TO_COMPARE)) {
            int res = snprintf(filename, sizeof(filename), "%s/%s", trace->directory, dp->d_name);

            /* if the total length of the string is greater than the buffer, silently forget it. */
            /* snprintf: a return value of size  or more means that the output was truncated */
            /*           if an output error is encountered, a negative value is returned. */
            if (((unsigned int)res < sizeof(filename)) && (res > 0)) {
                if (0 == stat(filename, &status)) {
                    if ((time_oldest == 0) || (status.st_mtime < time_oldest)) {
                        time_oldest = status.st_mtime;
                        size_oldest = status.st_size;
                        strncpy(filename_oldest, filename, PATH_MAX);
                        filename_oldest[PATH_MAX] = 0;
                    }
                }
                else {
                    printf("Old offline trace file %s cannot be stat-ed", filename);
                }
            }
        }

    closedir(dir);

    /* delete file */
    if (filename_oldest[0]) {
        if (remove(filename_oldest)) {
            printf("Remove file %s failed!\n", filename_oldest);
            return -1; /* ERROR */
        }
    }
    else {
        printf("No file to be removed!\n");
        return -1;     /* ERROR */
    }

    /* return size of deleted file*/
    return size_oldest;
}

DltReturnValue dlt_offline_trace_check_size(DltOfflineTrace *trace)
{

    struct stat status;

    /* check for existence of offline trace directory */
    if (stat(trace->directory, &status) == -1) {
        dlt_vlog(LOG_ERR, "Offline trace directory: %s doesn't exist \n", trace->directory);
        return DLT_RETURN_ERROR;
    }

    /* check for accesibilty of offline trace directory */
    else if (access(trace->directory, W_OK) != 0)
    {
        dlt_vlog(LOG_ERR, "Offline trace directory: %s doesn't have the write access \n", trace->directory);
        return DLT_RETURN_ERROR;
    }

    /* check size of complete offline trace */
    while ((int)dlt_offline_trace_get_total_size(trace) > (trace->maxSize - trace->fileSize))
        /* remove oldest files as long as new file will not fit in completely into complete offline trace */
        if (dlt_offline_trace_delete_oldest_file(trace) < 0)
            return DLT_RETURN_ERROR;

    return DLT_RETURN_OK; /* OK */
}

DltReturnValue dlt_offline_trace_init(DltOfflineTrace *trace,
                                      const char *directory,
                                      int fileSize,
                                      int maxSize,
                                      int filenameTimestampBased)
{

    /* init parameters */
    strncpy(trace->directory, directory, NAME_MAX);
    trace->directory[NAME_MAX] = 0;
    trace->fileSize = fileSize;
    trace->maxSize = maxSize;
    trace->filenameTimestampBased = filenameTimestampBased;
    /* check complete offlien trace size, remove old logs if needed */
    dlt_offline_trace_check_size(trace);

    return dlt_offline_trace_create_new_file(trace);
}

DltReturnValue dlt_offline_trace_write(DltOfflineTrace *trace,
                                       unsigned char *data1,
                                       int size1,
                                       unsigned char *data2,
                                       int size2,
                                       unsigned char *data3,
                                       int size3)
{

    if (trace->ohandle <= 0)
        return DLT_RETURN_ERROR;

    /* check file size here */
    if ((lseek(trace->ohandle, 0, SEEK_CUR) + size1 + size2 + size3) >= trace->fileSize) {
        /* close old file */
        close(trace->ohandle);
        trace->ohandle = -1;

        /* check complete offline trace size, remove old logs if needed */
        dlt_offline_trace_check_size(trace);

        /* create new file */
        dlt_offline_trace_create_new_file(trace);
    }

    /* write data into log file */
    if (data1 && (trace->ohandle >= 0)) {
        if (write(trace->ohandle, data1, size1) != size1) {
            printf("Offline trace write failed!\n");
            return DLT_RETURN_ERROR;
        }
    }

    if (data2 && (trace->ohandle >= 0)) {
        if (write(trace->ohandle, data2, size2) != size2) {
            printf("Offline trace write failed!\n");
            return DLT_RETURN_ERROR;
        }
    }

    if (data3 && (trace->ohandle >= 0)) {
        if (write(trace->ohandle, data3, size3) != size3) {
            printf("Offline trace write failed!\n");
            return DLT_RETURN_ERROR;
        }
    }

    return DLT_RETURN_OK; /* OK */
}

DltReturnValue dlt_offline_trace_free(DltOfflineTrace *trace)
{

    if (trace->ohandle <= 0)
        return DLT_RETURN_ERROR;

    /* close last used log file */
    close(trace->ohandle);

    return DLT_RETURN_OK; /* OK */
}
