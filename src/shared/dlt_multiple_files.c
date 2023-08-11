/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2022, Daimler TSS GmbH
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
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
 * Oleg Tropmann <oleg.tropmann@daimler.com>
 * Daniel Weber <daniel.w.weber@daimler.com>
 *
 * \copyright Copyright © 2022 Daimler TSS GmbH. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_log.c
 */

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
#include <errno.h>
#include <stdarg.h>

#include "dlt_multiple_files.h"
#include "dlt_common.h"

unsigned int multiple_files_buffer_storage_dir_info(const char *path, const char *file_name,
                                                    char *newest, char *oldest)
{
    int i = 0;
    unsigned int num_log_files = 0;
    struct dirent **files = { 0 };
    char *tmp_old = NULL;
    char *tmp_new = NULL;

    if ((path == NULL) || (file_name == NULL) || (newest == NULL) || (oldest == NULL)) {
        fprintf(stderr, "multiple_files_buffer_storage_dir_info: Invalid parameter(s)");
        return 0;
    }

    const int file_cnt = scandir(path, &files, NULL, alphasort);
    if (file_cnt <= 0) return 0;

    for (i = 0; i < file_cnt; i++) {
        int len = 0;
        len = strlen(file_name);

        if ((strncmp(files[i]->d_name, file_name, len) == 0) &&
            (files[i]->d_name[len] == MULTIPLE_FILES_FILENAME_INDEX_DELIM[0])) {
            num_log_files++;

            if ((tmp_old == NULL) || (strlen(tmp_old) >= strlen(files[i]->d_name))) {
                if (tmp_old == NULL) {
                    tmp_old = files[i]->d_name;
                } else if (strlen(tmp_old) > strlen(files[i]->d_name)) {
                    /* when file name is smaller, it is older */
                    tmp_old = files[i]->d_name;
                } else if (strcmp(tmp_old, files[i]->d_name) > 0) {
                    /* filename length is equal, do a string compare */
                    tmp_old = files[i]->d_name;
                }
            }

            if ((tmp_new == NULL) || (strlen(tmp_new) <= strlen(files[i]->d_name))) {
                if (tmp_new == NULL) {
                    tmp_new = files[i]->d_name;
                } else if (strlen(tmp_new) < strlen(files[i]->d_name)) {
                    /* when file name is longer, it is younger */
                    tmp_new = files[i]->d_name;
                } else if (strcmp(tmp_new, files[i]->d_name) < 0) {
                    tmp_new = files[i]->d_name;
                }
            }
        }
    }

    if (num_log_files > 0) {
        if ((tmp_old != NULL) && (strlen(tmp_old) < NAME_MAX)) {
            strncpy(oldest, tmp_old, NAME_MAX);
            oldest[NAME_MAX] = '\0';
        } else if ((tmp_old != NULL) && (strlen(tmp_old) >=  NAME_MAX)) {
            printf("length mismatch of file %s\n", tmp_old);
        }

        if ((tmp_new != NULL) && (strlen(tmp_new) < NAME_MAX)) {
            strncpy(newest, tmp_new, NAME_MAX);
            oldest[NAME_MAX] = '\0';
        } else if ((tmp_new != NULL) && (strlen(tmp_new) >=  NAME_MAX)) {
            printf("length mismatch of file %s\n", tmp_new);
        }
    }

    /* free scandir result */
    for (i = 0; i < file_cnt; i++) free(files[i]);

    free(files);

    return num_log_files;
}

void multiple_files_buffer_file_name(MultipleFilesRingBuffer *files_buffer, const size_t length, const unsigned int idx)
{
    char file_index[11]; /* UINT_MAX = 4294967295 -> 10 digits */
    snprintf(file_index, sizeof(file_index), "%010u", idx);

    /* create log file name */
    char* file_name = files_buffer->filename;
    memset(file_name, 0, length * sizeof(char));

    const size_t size = length - strlen(file_name) - 1;
    strncat(file_name, files_buffer->filenameBase, size);
    strncat(file_name, MULTIPLE_FILES_FILENAME_INDEX_DELIM, size);
    strncat(file_name, file_index, size);
    strncat(file_name, files_buffer->filenameExt, size);
}

unsigned int multiple_files_buffer_get_idx_of_log_file(char *file)
{
    if ((file == NULL) || (file[0] == '\0')) return 0;

    const char d[2] = MULTIPLE_FILES_FILENAME_INDEX_DELIM;
    char *token;

    token = strtok(file, d);
    /* we are interested in 2. token because of log file name */
    token = strtok(NULL, d);

    return token != NULL ? strtol(token, NULL, 10) : 0;
}

DltReturnValue multiple_files_buffer_create_new_file(MultipleFilesRingBuffer *files_buffer)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return DLT_RETURN_ERROR;
    }

    time_t t;
    struct tm tmp;
    char file_path[PATH_MAX + 1];
    unsigned int idx = 0;
    int ret = 0;

    /* set filename */
    if (files_buffer->filenameTimestampBased) {
        /* timestamp format: "yyyymmdd_hhmmss" */
        char timestamp[16];
        t = time(NULL);
        tzset();
        localtime_r(&t, &tmp);

        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tmp);

        ret = snprintf(files_buffer->filename, sizeof(files_buffer->filename), "%s%s%s%s",
                       files_buffer->filenameBase,
                       MULTIPLE_FILES_FILENAME_TIMESTAMP_DELIM, timestamp,
                       files_buffer->filenameExt);

        if ((ret < 0) || ((size_t)ret >= (int)sizeof(files_buffer->filename))) {
            fprintf(stderr, "filename cannot be concatenated\n");
            return DLT_RETURN_ERROR;
        }

        ret = snprintf(file_path, sizeof(file_path), "%s/%s",
                       files_buffer->directory, files_buffer->filename);

        if ((ret < 0) || ((size_t)ret >= (int)sizeof(file_path))) {
            fprintf(stderr, "file path cannot be concatenated\n");
            return DLT_RETURN_ERROR;
        }
    }
    else {
        char newest[NAME_MAX + 1] = { 0 };
        char oldest[NAME_MAX + 1] = { 0 };
        /* targeting newest file, ignoring number of files in dir returned */
        if (0 == multiple_files_buffer_storage_dir_info(files_buffer->directory,
                                                        files_buffer->filenameBase,
                                                        newest,
                                                        oldest)) {
            printf("No multiple files found\n");
        }

        idx = multiple_files_buffer_get_idx_of_log_file(newest) + 1;

        multiple_files_buffer_file_name(files_buffer, sizeof(files_buffer->filename), idx);
        ret = snprintf(file_path, sizeof(file_path), "%s/%s",
                       files_buffer->directory, files_buffer->filename);

        if ((ret < 0) || (ret >= NAME_MAX)) {
            fprintf(stderr, "filename cannot be concatenated\n");
            return DLT_RETURN_ERROR;
        }
    }

    /* open DLT output file */
    errno = 0;
    files_buffer->ohandle = open(file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR |
                                                                S_IRGRP | S_IROTH); /* mode: wb */

    if (files_buffer->ohandle == -1) {
        /* file cannot be opened */
        fprintf(stderr, "file %s cannot be created, error: %s\n", file_path, strerror(errno));
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

ssize_t multiple_files_buffer_get_total_size(const MultipleFilesRingBuffer *files_buffer)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return -1;
    }

    struct dirent *dp;
    char filename[PATH_MAX + 1];
    ssize_t size = 0;
    struct stat status;

    /* go through all dlt files in directory */
    DIR *dir = opendir(files_buffer->directory);
    if (!dir) {
        fprintf(stderr, "directory %s cannot be opened, error=%s\n", files_buffer->directory, strerror(errno));
        return -1;
    }

    while ((dp = readdir(dir)) != NULL) {
        // consider files matching with a specific base name and a particular extension
        if (strstr(dp->d_name, files_buffer->filenameBase)  && strstr(dp->d_name, files_buffer->filenameExt)) {
            int res = snprintf(filename, sizeof(filename), "%s/%s", files_buffer->directory, dp->d_name);

            /* if the total length of the string is greater than the buffer, silently forget it. */
            /* snprintf: a return value of size  or more means that the output was truncated */
            /*           if an output error is encountered, a negative value is returned. */
            if (((unsigned int)res < sizeof(filename)) && (res > 0)) {
                errno = 0;
                if (0 == stat(filename, &status))
                    size += status.st_size;
                else
                    fprintf(stderr, "file %s cannot be stat-ed, error=%s\n", filename, strerror(errno));
            }
        }
    }

    closedir(dir);

    /* return size */
    return size;
}

int multiple_files_buffer_delete_oldest_file(MultipleFilesRingBuffer *files_buffer)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return -1;  /* ERROR */
    }

    struct dirent *dp;
    char filename[PATH_MAX + 1];
    char filename_oldest[PATH_MAX + 1];
    unsigned long size_oldest = 0;
    struct stat status;
    time_t time_oldest = 0;
    int index_oldest = INT_MAX;

    filename[0] = 0;
    filename_oldest[0] = 0;

    /* go through all dlt files in directory */
    DIR *dir = opendir(files_buffer->directory);

    if(!dir)
        return -1;

    while ((dp = readdir(dir)) != NULL) {
        if (strstr(dp->d_name, files_buffer->filenameBase) && strstr(dp->d_name, files_buffer->filenameExt)) {
            int res = snprintf(filename, sizeof(filename), "%s/%s", files_buffer->directory, dp->d_name);

            /* if the total length of the string is greater than the buffer, silently forget it. */
            /* snprintf: a return value of size  or more means that the output was truncated */
            /*           if an output error is encountered, a negative value is returned. */
            if (((unsigned int) res >= sizeof(filename)) || (res <= 0)) {
                printf("Filename for delete oldest too long. Skip file.\n");
                continue;
            }

            if (files_buffer->filenameTimestampBased) {
                errno = 0;
                if (0 == stat(filename, &status)) {
                    if ((time_oldest == 0) || (status.st_mtime < time_oldest)) {
                        time_oldest = status.st_mtime;
                        size_oldest = status.st_size;
                        strncpy(filename_oldest, filename, PATH_MAX);
                        filename_oldest[PATH_MAX] = 0;
                    }
                } else {
                    printf("Old file %s cannot be stat-ed, error=%s\n", filename, strerror(errno));
                }
            } else {
                //index based
                const int index = multiple_files_buffer_get_idx_of_log_file(filename);
                if (index < index_oldest) {
                    index_oldest = index;
                    snprintf(filename, sizeof(filename), "%s/%s", files_buffer->directory, dp->d_name);
                    strncpy(filename_oldest, filename, PATH_MAX);
                    filename_oldest[PATH_MAX] = 0;
                }
            }
        }
    }

    closedir(dir);

    /* delete file */
    if (filename_oldest[0]) {
        if (remove(filename_oldest)) {
            fprintf(stderr, "Remove file %s failed! error=%s\n", filename_oldest, strerror(errno));
            return -1; /* ERROR */
        }
    } else {
        fprintf(stderr, "No file to be removed!\n");
        return -1; /* ERROR */
    }

    /* return size of deleted file*/
    return size_oldest;
}

DltReturnValue multiple_files_buffer_check_size(MultipleFilesRingBuffer *files_buffer)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return DLT_RETURN_ERROR;
    }

    struct stat status;

    /* check for existence of buffer files directory */
    errno = 0;
    if (stat(files_buffer->directory, &status) == -1) {
        fprintf(stderr, "Buffer files directory: %s doesn't exist, error=%s\n", files_buffer->directory, strerror(errno));
        return DLT_RETURN_ERROR;
    }
    /* check for accessibility of buffer files directory */
    else if (access(files_buffer->directory, W_OK) != 0) {
        fprintf(stderr, "Buffer files directory: %s doesn't have the write access \n", files_buffer->directory);
        return DLT_RETURN_ERROR;
    }

    ssize_t total_size = 0;
    /* check size of complete buffer file */
    while ((total_size = multiple_files_buffer_get_total_size(files_buffer)) > (files_buffer->maxSize - files_buffer->fileSize)) {
        /* remove the oldest files as long as new file will not fit in completely into complete multiple files buffer */
        if (multiple_files_buffer_delete_oldest_file(files_buffer) < 0) return DLT_RETURN_ERROR;
    }

    return total_size == -1 ? DLT_RETURN_ERROR : DLT_RETURN_OK;
}

DltReturnValue multiple_files_buffer_open_file_for_append(MultipleFilesRingBuffer *files_buffer) {
    if (files_buffer == NULL || files_buffer->filenameTimestampBased) return DLT_RETURN_ERROR;

    char newest[NAME_MAX + 1] = {0};
    char oldest[NAME_MAX + 1] = {0};
    /* targeting the newest file, ignoring number of files in dir returned */

    if (0 == multiple_files_buffer_storage_dir_info(files_buffer->directory,
                                                   files_buffer->filenameBase, newest, oldest) ) {
        // no file for appending found. Create a new one
        printf("No multiple files for appending found. Create a new one\n");
        return multiple_files_buffer_create_new_file(files_buffer);
    }

    char file_path[PATH_MAX + 1];
    int ret = snprintf(file_path, sizeof(file_path), "%s/%s",
                         files_buffer->directory, newest);

    if ((ret < 0) || (ret >= NAME_MAX)) {
        fprintf(stderr, "filename cannot be concatenated\n");
        return DLT_RETURN_ERROR;
    }

    /* open DLT output file */
    errno = 0;
    files_buffer->ohandle = open(file_path, O_WRONLY | O_APPEND); /* mode: wb */

    return files_buffer->ohandle == -1 ? DLT_RETURN_ERROR : DLT_RETURN_OK;
}

DltReturnValue multiple_files_buffer_init(MultipleFilesRingBuffer *files_buffer,
                                          const char *directory,
                                          const int file_size,
                                          const int max_size,
                                          const bool filename_timestamp_based,
                                          const bool append,
                                          const char *filename_base,
                                          const char *filename_ext)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return DLT_RETURN_ERROR;
    }

    /* init parameters */
    strncpy(files_buffer->directory, directory, NAME_MAX);
    files_buffer->directory[NAME_MAX] = 0;
    files_buffer->fileSize = file_size;
    files_buffer->maxSize = max_size;
    files_buffer->filenameTimestampBased = filename_timestamp_based;
    strncpy(files_buffer->filenameBase, filename_base, NAME_MAX);
    files_buffer->filenameBase[NAME_MAX] = 0;
    strncpy(files_buffer->filenameExt, filename_ext, NAME_MAX);
    files_buffer->filenameExt[NAME_MAX] = 0;

    if (DLT_RETURN_ERROR == multiple_files_buffer_check_size(files_buffer)) return DLT_RETURN_ERROR;

    return (!files_buffer->filenameTimestampBased && append)
        ? multiple_files_buffer_open_file_for_append(files_buffer)
        : multiple_files_buffer_create_new_file(files_buffer);
}

void multiple_files_buffer_rotate_file(MultipleFilesRingBuffer *files_buffer, const int size)
{
    /* check file size here */
    if ((lseek(files_buffer->ohandle, 0, SEEK_CUR) + size) < files_buffer->fileSize) return;

    /* close old file */
    close(files_buffer->ohandle);
    files_buffer->ohandle = -1;

    /* check complete files size, remove old logs if needed */
    if (DLT_RETURN_ERROR == multiple_files_buffer_check_size(files_buffer)) return;

    /* create new file */
    multiple_files_buffer_create_new_file(files_buffer);
}

DltReturnValue multiple_files_buffer_write_chunk(const MultipleFilesRingBuffer *files_buffer,
                                                 const unsigned char *data,
                                                 const int size)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return DLT_RETURN_ERROR;
    }

    if (data && (files_buffer->ohandle >= 0)) {
        if (write(files_buffer->ohandle, data, size) != size) {
            fprintf(stderr, "file write failed!\n");
            return DLT_RETURN_ERROR;
        }
    }
    return DLT_RETURN_OK;
}

DltReturnValue multiple_files_buffer_write(MultipleFilesRingBuffer *files_buffer,
                                           const unsigned char *data,
                                           const int size)
{
    if (files_buffer->ohandle < 0) return DLT_RETURN_ERROR;

    multiple_files_buffer_rotate_file(files_buffer, size);

    /* write data into log file */
    return multiple_files_buffer_write_chunk(files_buffer, data, size);
}

DltReturnValue multiple_files_buffer_free(const MultipleFilesRingBuffer *files_buffer)
{
    if (files_buffer == NULL) {
        fprintf(stderr, "multiple files buffer not set\n");
        return DLT_RETURN_ERROR;
    }

    if (files_buffer->ohandle < 0) return DLT_RETURN_ERROR;

    /* close last used log file */
    close(files_buffer->ohandle);

    return DLT_RETURN_OK;
}
