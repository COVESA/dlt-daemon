/**
 * @licence app begin@
 * Copyright (C) 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT offline log storage functionality source file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Syed Hameed <shameed@jp.adit-jv.com> ADIT 2015
 *
 * \file: dlt_offline_logstorage_behavior.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <syslog.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "dlt_offline_logstorage.h"
#include "dlt_offline_logstorage_behavior.h"
#include "dlt_offline_logstorage_behavior_internal.h"

/**
 * dlt_logstorage_log_file_name
 *
 * Create log file name in the form configured by the user
 *      <filename><delimiter><index><delimiter><timestamp>.dlt
 *
 *      filename:       given in configuration file
 *      delimiter:      Punctuation characters (configured in dlt.conf)
 *      timestamp:      yyyy-mm-dd-hh-mm-ss (enabled/disabled in dlt.conf)
 *      index:          Index len depends on wrap around value in dlt.conf
 *                      ex: wrap around = 99, index will 01..99
 *
 * @param log_file_name     contains complete logfile name
 * @param file_config       User configurations for log file
 * @param name              file name given in configuration file
 * @param idx               continous index of log files
 * @ return                 None
 */
void dlt_logstorage_log_file_name(char *log_file_name,
                                  DltLogStorageUserConfig *file_config,
                                  char *name,
                                  int idx)
{
    if ((log_file_name == NULL) || (file_config == NULL))
    {
        return;
    }

    char file_index[10] = {'\0'};

    // create log file name
    memset(log_file_name, 0, DLT_MOUNT_PATH_MAX * sizeof(char));
    strcat(log_file_name, name);
    strncat(log_file_name, &file_config->logfile_delimiter, 1);

    snprintf(file_index, 10, "%d", idx);

    if (file_config->logfile_maxcounter != UINT_MAX)
    {
        /* Setup 0's to be appended in file index until max index len*/
        unsigned int digit_idx = 0;
        unsigned int i = 0;
        snprintf(file_index, 10, "%d", idx);
        digit_idx = strlen(file_index);

        for (i = 0 ; i < (file_config->logfile_counteridxlen - digit_idx) ; i++)
        {
            strcat(log_file_name, "0");
        }
    }

    strcat(log_file_name, file_index);

    /* Add time stamp if user has configured */
    if (file_config->logfile_timestamp)
    {
        char stamp[DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + 1] = {0};
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        sprintf(stamp,
                "%c%04d%02d%02d-%02d%02d%02d",
                file_config->logfile_delimiter,
                1900 + tm_info->tm_year,
                1 + tm_info->tm_mon,
                tm_info->tm_mday,
                tm_info->tm_hour,
                tm_info->tm_min,
                tm_info->tm_sec);
        strcat(log_file_name, stamp);
    }

    strcat(log_file_name, ".dlt");
}

/**
 * dlt_logstorage_sort_file_name
 *
 * Sort the filenames with index based ascending order (bubble sort)
 *
 * @param head              Log filename list
 * @ return                 None
 */
void dlt_logstorage_sort_file_name(DltLogStorageFileList **head)
{
    int done = 0;

    if ((head == NULL) || (*head == NULL) || ((*head)->next == NULL))
    {
        return;
    }

    while (!done)
    {
        /* "source" of the pointer to the current node in the list struct */
        DltLogStorageFileList **pv = head;
        DltLogStorageFileList *nd = *head; /* local iterator pointer */
        DltLogStorageFileList *nx = (*head)->next; /* local next pointer */

        done = 1;

        while (nx)
        {
            if (nd->idx > nx->idx)
            {
                nd->next = nx->next;
                nx->next = nd;
                *pv = nx;

                done = 0;
            }

            pv = &nd->next;
            nd = nx;
            nx = nx->next;
        }
    }
}

/**
 * dlt_logstorage_rearrange_file_name
 *
 * Rearrange the filenames in the order of latest and oldest
 *
 * @param head              Log filename list
 * @ return                 None
 */
void dlt_logstorage_rearrange_file_name(DltLogStorageFileList **head)
{
    DltLogStorageFileList *n_prev = NULL;
    DltLogStorageFileList *tail = NULL;
    DltLogStorageFileList *wrap_pre = NULL;
    DltLogStorageFileList *wrap_post = NULL;
    DltLogStorageFileList *n = NULL;

    if ((head == NULL) || (*head == NULL) || ((*head)->next == NULL))
    {
        return;
    }

    for (n = *head ; n != NULL ; n = n->next)
    {
        if (n && n_prev)
        {
            if ((n->idx - n_prev->idx) != 1)
            {
                wrap_post = n;
                wrap_pre = n_prev;
            }
        }

        n_prev = n;
    }

    tail = n_prev;

    if (wrap_post && wrap_pre)
    {
        wrap_pre->next = NULL;
        tail->next = *head;
        *head = wrap_post;
    }
}

/**
 * dlt_logstorage_get_idx_of_log_file
 *
 * Extract index of log file name passed as input argument
 *
 * @param file          file name to extract the index from
 * @param file_config   User configurations for log file
 * @return index on success, -1 if no index is found
 */
unsigned int dlt_logstorage_get_idx_of_log_file(DltLogStorageUserConfig *file_config,
                                                char *file)
{
    unsigned int idx = -1;
    char *endptr;
    char *filename;
    unsigned int filename_len = 0;
    unsigned int fileindex_len = 0;

    if ((file_config == NULL) || (file == NULL))
    {
        return -1;
    }

    /* Calculate actual file name length */
    filename = strchr(file, file_config->logfile_delimiter);

    if (filename == NULL)
    {
        dlt_vlog(LOG_ERR, "Cannot extract filename from %s\n", file);
        return -1;
    }

    filename_len = strlen(file) - strlen(filename);

    /* index is retrived from file name */
    if (file_config->logfile_timestamp)
    {
        fileindex_len = strlen(file) -
            (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN +
             DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN +
             filename_len + 1);

        idx = (int) strtol(&file[strlen(file) -
                                 (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN +
                                  fileindex_len +
                                  DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN)],
                           &endptr,
                           10);
    }
    else
    {
        fileindex_len = strlen(file) -
            (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN +
             filename_len + 1);

        idx = (int) strtol(&file[strlen(file) -
                                 (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN
                                  + fileindex_len)], &endptr, 10);
    }

    if ((endptr == file) || (idx == 0))
    {
        dlt_log(LOG_ERR,
                "Unable to calculate index from log file name. Reset to 001.\n");
    }

    return idx;
}

/**
 * dlt_logstorage_storage_dir_info
 *
 * Read file names of storage directory.
 * Update the file list, arrange it in order of latest and oldest
 *
 * @param file_config   User configurations for log file
 * @param path          Path to storage directory
 * @param  config       DltLogStorageFilterConfig
 * @return              0 on success, -1 on error
 */
int dlt_logstorage_storage_dir_info(DltLogStorageUserConfig *file_config,
                                    char *path,
                                    DltLogStorageFilterConfig *config)
{
    int i = 0;
    int cnt = 0;
    int ret = 0;
    struct dirent **files = {0};
    unsigned int current_idx = 0;

    if ((config == NULL) ||
        (file_config == NULL) ||
        (path == NULL) ||
        (config->file_name == NULL))
    {
        return -1;
    }

    cnt = scandir(path, &files, 0, alphasort);

    if (cnt < 0)
    {
        dlt_log(LOG_ERR,
                "dlt_logstorage_storage_dir_info: Failed to scan directory\n");
        return -1;
    }

    for (i = 0 ; i < cnt ; i++)
    {
        int len = 0;
        len = strlen(config->file_name);

        if ((strncmp(files[i]->d_name,
                     config->file_name,
                     len) == 0) &&
            (files[i]->d_name[len] == file_config->logfile_delimiter))
        {
            DltLogStorageFileList **tmp = NULL;
            current_idx = dlt_logstorage_get_idx_of_log_file(file_config,
                                                             files[i]->d_name);

            if (config->records == NULL)
            {
                config->records = malloc(sizeof(DltLogStorageFileList));

                if (config->records == NULL)
                {
                    ret = -1;
                    dlt_log(LOG_ERR,
                            "Memory allocation failed\n");
                    break;
                }

                tmp = &config->records;
            }
            else
            {
                tmp = &config->records;

                while (*(tmp) != NULL)
                {
                    tmp = &(*tmp)->next;
                }

                *tmp = malloc(sizeof(DltLogStorageFileList));

                if (*tmp == NULL)
                {
                    ret = -1;
                    dlt_log(LOG_ERR,
                            "Memory allocation failed\n");
                    break;
                }
            }

            (*tmp)->name = strdup(files[i]->d_name);
            (*tmp)->idx = current_idx;
            (*tmp)->next = NULL;
        }
    }

    if (ret == 0)
    {
        dlt_logstorage_sort_file_name(&config->records);
        dlt_logstorage_rearrange_file_name(&config->records);
    }

    /* free scandir result */
    for (i = 0 ; i < cnt ; i++)
    {
        free(files[i]);
    }

    free(files);

    return ret;
}

/**
 * dlt_logstorage_open_log_file
 *
 * Open a log file. Check storage directory for already created files and open
 * the oldest if there is enough space to store at least msg_size.
 * Otherwise create a new file, but take configured max number of files into
 * account and remove the oldest file if needed.
 *
 * @param  config    DltLogStorageFilterConfig
 * @param  file_config   User configurations for log file
 * @param  dev_path      Storage device path
 * @param  msg_size  Size of incoming message
 * @return 0 on succes, -1 on error
 */
int dlt_logstorage_open_log_file(DltLogStorageFilterConfig *config,
                                 DltLogStorageUserConfig *file_config,
                                 char *dev_path,
                                 int msg_size)
{
    int ret = 0;
    char absolute_file_path[DLT_MOUNT_PATH_MAX + DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN + 1] = {'\0'};
    char storage_path[DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN + 1] = {'\0'};
    unsigned int num_log_files = 0;
    struct stat s;
    DltLogStorageFileList **tmp = NULL;
    DltLogStorageFileList **newest = NULL;
    char file_name[DLT_MOUNT_PATH_MAX + 1] = {'\0'};

    if (config == NULL)
    {
        return -1;
    }

    if (strlen(dev_path) > DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN)
    {
        dlt_vlog(LOG_ERR, "device path '%s' is too long to store\n", dev_path);
        return -1;
    }

    snprintf(storage_path, DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN, "%s/", dev_path);

    /* check if there are already files stored */
    if (config->records == NULL)
    {
        if (dlt_logstorage_storage_dir_info(file_config, storage_path, config)
            != 0)
        {
            return -1;
        }
    }

    /* obtain locations of newest, current file names, file count */
    tmp = &config->records;

    while (*(tmp) != NULL)
    {
        num_log_files += 1;

        if ((*tmp)->next == NULL)
        {
            newest = tmp;
        }

        tmp = &(*tmp)->next;
    }

    /* need new file*/
    if (num_log_files == 0)
    {
        dlt_logstorage_log_file_name(file_name,
                                     file_config,
                                     config->file_name,
                                     1);

        /* concatenate path and file and open absolute path */
        strcat(absolute_file_path, storage_path);
        strcat(absolute_file_path, file_name);
        config->log = fopen(absolute_file_path, "a+");

        /* Add file to file list */
        *tmp = malloc(sizeof(DltLogStorageFileList));

        if (*tmp == NULL)
        {
            dlt_log(LOG_ERR,
                    "Memory allocation for file name failed\n");
            return -1;
        }

        (*tmp)->name = strdup(file_name);
        (*tmp)->idx = 1;
        (*tmp)->next = NULL;
    }
    else /* newest file available*/
    {
        strcat(absolute_file_path, storage_path);
        strcat(absolute_file_path, (*newest)->name);

        ret = stat(absolute_file_path, &s);

        /* if size is enough, open it */
        if ((ret == 0) && (s.st_size + msg_size < (int)config->file_size))
        {
            config->log = fopen(absolute_file_path, "a+");
        }
        else /* no space in file or file stats cannot be read */
        {
            unsigned int idx = 0;

            /* get index of newest log file */
            idx = dlt_logstorage_get_idx_of_log_file(file_config,
                                                     (*newest)->name);
            idx += 1;

            /* wrap around if max index is reached or an error occurred
             * while calculating index from file name */
            if ((idx > file_config->logfile_maxcounter) || (idx == 0))
            {
                idx = 1;
            }

            dlt_logstorage_log_file_name(file_name,
                                         file_config,
                                         config->file_name,
                                         idx);

            /* concatenate path and file and open absolute path */
            memset(absolute_file_path,
                   0,
                   sizeof(absolute_file_path) / sizeof(char));
            strcat(absolute_file_path, storage_path);
            strcat(absolute_file_path, file_name);
            config->log = fopen(absolute_file_path, "a+");

            /* Add file to file list */
            *tmp = malloc(sizeof(DltLogStorageFileList));

            if (*tmp == NULL)
            {
                dlt_log(LOG_ERR,
                        "Memory allocation for file name failed\n");
                return -1;
            }

            (*tmp)->name = strdup(file_name);
            (*tmp)->idx = idx;
            (*tmp)->next = NULL;

            num_log_files += 1;

            /* check if number of log files exceeds configured max value */
            if (num_log_files > config->num_files)
            {
                /* delete oldest */
                DltLogStorageFileList **head = &config->records;
                DltLogStorageFileList *n = *head;
                memset(absolute_file_path,
                       0,
                       sizeof(absolute_file_path) / sizeof(char));
                strcat(absolute_file_path, storage_path);
                strcat(absolute_file_path, (*head)->name);
                remove(absolute_file_path);
                free((*head)->name);
                *head = n->next;
                n->next = NULL;
                free(n);
            }
        }
    }

    if (config->log == NULL)
    {
        dlt_log(LOG_ERR,
                "dlt_logstorage_create_log_file: Unable to open log file.\n");
        return -1;
    }

    return ret;
}

/**
 * dlt_logstorage_find_dlt_header
 *
 * search for dlt header in cache
 *
 * @param ptr         cache starting position
 * @param offset      offset
 * @param cnt         count
 * @return index on success, -1 on error
 */
DLT_STATIC int dlt_logstorage_find_dlt_header(void *ptr,
                                              unsigned int offset,
                                              unsigned int cnt)
{
    int index = 0;
    char substring[] = {'D', 'L', 'T', 0x01};
    while(cnt > 0)
    {
        if (*((char *)(ptr + offset + index)) == 'D')
        {
            if (strncmp(ptr + offset + index , substring, 4) == 0)
            {
                return index;
            }
        }
        cnt--;
        index++;
    }
    return -1;
}

/**
 * dlt_logstorage_check_write_ret
 *
 * check the return value of fwrite
 *
 * @param config      DltLogStorageFilterConfig
 * @param ret         return value of fwrite call
 */
DLT_STATIC void dlt_logstorage_check_write_ret(DltLogStorageFilterConfig *config,
                                               int ret)
{
    if (config == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: cannot retrieve config information\n", __func__);
    }
    if (ret <= 0)
    {
        if (ferror(config->log) != 0)
        {
            dlt_vlog(LOG_ERR, "%s: failed to write cache into log file\n", __func__);
        }
    }
    else
    {
        /* force sync */
        if (fflush(config->log) != 0)
        {
            dlt_vlog(LOG_ERR, "%s: failed to flush log file\n", __func__);
        }
        if (fsync(fileno(config->log)) != 0)
        {
            dlt_vlog(LOG_ERR, "%s: failed to sync log file\n", __func__);
        }
    }
}

/**
 * dlt_logstorage_sync_create_new_file
 *
 * Write the log message and open new log file
 *
 * @param config            DltLogStorageFilterConfig
 * @param file_config       DltLogStorageUserConfig
 * @param dev_path          Storage device mount point
 * @param remain_file_size  log file remaining size
 * @return 0 on success, -1 on error
 */
DLT_STATIC DltReturnValue dlt_logstorage_sync_create_new_file(
        DltLogStorageFilterConfig *config,
        DltLogStorageUserConfig *file_config,
        char *dev_path,
        unsigned int remain_file_size)
{
    int index = 0;
    int ret;
    unsigned int cache_offset = 0;
    unsigned int count = 0;
    DltLogStorageCacheFooter *footer = NULL;

    if (config == NULL || file_config == NULL || dev_path == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: cannot retrieve config information\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }
    footer = (DltLogStorageCacheFooter *)(config->cache +
                                          config->file_size);
    if (footer == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Cannot retrieve cache information\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* sync capable data to file */
    if (footer->offset >= footer->last_sync_offset)
    {
        count = config->file_size - footer->offset;
        if (count > remain_file_size)
        {
            count = remain_file_size;
        }
        index = dlt_logstorage_find_dlt_header(config->cache,footer->offset,count);
        cache_offset = footer->offset;
    }
    else
    {
        count = config->file_size - footer->last_sync_offset;
        if (count > remain_file_size)
        {
            count = remain_file_size;
        }
        index = dlt_logstorage_find_dlt_header(config->cache,
                                               footer->last_sync_offset,
                                               count);
        cache_offset = footer->last_sync_offset;

    }
    if (index >= 0)
    {
        ret = fwrite(config->cache + cache_offset + index, count - index, 1, config->log);
        dlt_logstorage_check_write_ret(config, ret);
        config->current_write_file_offset += count - index;
    }
    if (footer->last_sync_offset == 0)
    {
        footer->last_sync_offset = footer->offset + count;
    }
    else
    {
        footer->last_sync_offset += count;
    }
    config->total_write_count -= count;

    /* sync data to current file in case of file is not full */
    if (config->current_write_file_offset < config->file_size)
    {
        count = config->file_size - config->current_write_file_offset;

        if (footer->last_sync_offset < config->file_size)
        {
            ret = fwrite(config->cache + footer->last_sync_offset, count, 1, config->log);
            dlt_logstorage_check_write_ret(config, ret);
            footer->last_sync_offset += count;
        }
        /* sync remaining amount of file from start of cache */
        else
        {
            config->sync_from_start = 1;
            if (count > footer->offset)
            {
                count = footer->offset;
            }
            ret = fwrite(config->cache, count, 1, config->log);
            dlt_logstorage_check_write_ret(config, ret);
            footer->last_sync_offset = count;
        }
        config->total_write_count -= count;
    }

    config->current_write_file_offset = 0;
    fclose(config->log);
    config->log = NULL;
    /* get always a new file */
    if (dlt_logstorage_prepare_on_msg(config,
                                      file_config,
                                      dev_path,
                                      config->file_size) != 0)
    {
        dlt_vlog(LOG_ERR,
                 "%s: failed to prepare log file for file_size\n", __func__);
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

/**
 * dlt_logstorage_sync_to_file
 *
 * Write the log message to log file
 *
 * @param config        DltLogStorageFilterConfig
 * @param file_config   DltLogStorageUserConfig
 * @param dev_path      Storage device mount point path
 * @return 0 on success, -1 on error
 */
DLT_STATIC DltReturnValue dlt_logstorage_sync_to_file(
        DltLogStorageFilterConfig *config,
        DltLogStorageUserConfig *file_config,
        char *dev_path)
{
    int ret = 0;
    unsigned int remain_file_size = 0;
    unsigned int count = 0;
    DltLogStorageCacheFooter *footer = NULL;

    if (config == NULL || file_config == NULL || dev_path == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: cannot retrieve config information\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }
    footer = (DltLogStorageCacheFooter *)(config->cache + config->file_size);

    if (footer == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Cannot retrieve cache information\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    };
    count = footer->offset - footer->last_sync_offset;
    remain_file_size = config->file_size - config->current_write_file_offset;

    /* sync data to file if required sync data exceeds remaining file size */
    if (count > remain_file_size)
    {
        ret = fwrite(config->cache + footer->last_sync_offset, remain_file_size, 1, config->log);
        dlt_logstorage_check_write_ret(config, ret);
        config->current_write_file_offset += remain_file_size;
        footer->last_sync_offset += remain_file_size;

        count = footer->offset - footer->last_sync_offset;
        config->current_write_file_offset = 0;
        fclose(config->log);
        config->log = NULL;
        /* get always a new file */
        if (dlt_logstorage_prepare_on_msg(config,
                                          file_config,
                                          dev_path,
                                          config->file_size) != 0)
        {
            dlt_vlog(LOG_ERR,
                     "%s: failed to prepare log file for file_size\n", __func__);
            return DLT_RETURN_ERROR;
        }
    }
    ret = fwrite(config->cache + footer->last_sync_offset, count, 1, config->log);
    dlt_logstorage_check_write_ret(config, ret);
    config->current_write_file_offset += count;
    footer->last_sync_offset = footer->offset;
    config->cur_cache_sync = 1;

    return DLT_RETURN_OK;
}

/**
 * dlt_logstorage_sync_capable_data_to_file
 *
 * Write the log message to log file
 *
 * @param config        DltLogStorageFilterConfig
 * @param index_status  check for index is required or not
 * @return 0 on success, -1 on error
 */
DLT_STATIC DltReturnValue dlt_logstorage_sync_capable_data_to_file(
        DltLogStorageFilterConfig *config,
        int index_status)
{
    int ret = 0;
    int index = 0;
    unsigned int count = 0;
    DltLogStorageCacheFooter *footer = NULL;

    if (config == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: cannot retrieve config information\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    footer = (DltLogStorageCacheFooter *)(config->cache + config->file_size);
    if (footer == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Cannot retrieve cache information\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    };
    count = config->file_size - footer->last_sync_offset;
    if (index_status == 1)
    {
        index = dlt_logstorage_find_dlt_header(config->cache,
                                               footer->last_sync_offset,
                                               count);
    }
    if (count > 0 && index >=0)
    {
        ret = fwrite(config->cache + footer->last_sync_offset + index,
                     count - index,
                     1,
                     config->log);
        dlt_logstorage_check_write_ret(config, ret);
        config->current_write_file_offset += count - index;
    }
    config->total_write_count -= count;
    footer->last_sync_offset = 0;

    return DLT_RETURN_OK;
}

/**
 * dlt_logstorage_prepare_on_msg
 *
 * Prepare the log file for a certain filer. If log file not open or log
 * files max size reached, open a new file.
 *
 * @param config        DltLogStorageFilterConfig
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param log_msg_size  Size of log message
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_prepare_on_msg(DltLogStorageFilterConfig *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int log_msg_size)
{
    int ret = 0;
    struct stat s;

    if ((config == NULL) || (file_config == NULL) || (dev_path == NULL))
    {
        return -1;
    }

    if (config->log == NULL) /* open a new log file */
    {
        ret = dlt_logstorage_open_log_file(config,
                                           file_config,
                                           dev_path,
                                           log_msg_size);
    }
    else /* already open, check size and create a new file if needed */
    {
        ret = fstat(fileno(config->log), &s);

        if (ret == 0)
        {
            /* check if adding new data do not exceed max file size */
            if (s.st_size + log_msg_size >= (int)config->file_size)
            {
                fclose(config->log);
                config->log = NULL;
                ret = dlt_logstorage_open_log_file(config,
                                                   file_config,
                                                   dev_path,
                                                   log_msg_size);
            }
            else /*everything is prepared */
            {
                ret = 0;
            }
        }
        else
        {
            dlt_log(LOG_ERR,
                    "dlt_logstorage_prepare_log_file: stat() failed.\n");
            ret = -1;
        }
    }

    return ret;
}

/**
 * dlt_logstorage_write_on_msg
 *
 * Write the log message.
 *
 * @param config        DltLogStorageFilterConfig
 * @param data1         header
 * @param size1         header size
 * @param data2         storage header
 * @param size2         storage header size
 * @param data3         payload
 * @param size3         payload size
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_write_on_msg(DltLogStorageFilterConfig *config,
                                unsigned char *data1,
                                int size1,
                                unsigned char *data2,
                                int size2,
                                unsigned char *data3,
                                int size3)
{
    int ret;

    if ((config == NULL) || (data1 == NULL) || (data2 == NULL) || (data3 == NULL))
    {
        return -1;
    }

    ret = fwrite(data1, 1, size1, config->log);

    if (ret != size1)
    {
        dlt_log(LOG_WARNING, "Wrote less data than specified\n");
    }

    ret = fwrite(data2, 1, size2, config->log);

    if (ret != size2)
    {
        dlt_log(LOG_WARNING, "Wrote less data than specified\n");
    }

    ret = fwrite(data3, 1, size3, config->log);

    if (ret != size3)
    {
        dlt_log(LOG_WARNING, "Wrote less data than specified\n");
    }

    return ferror(config->log);
}

/**
 * dlt_logstorage_sync_on_msg
 *
 * sync data to disk.
 *
 * @param config        DltLogStorageFilterConfig
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param status        Strategy flag
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_sync_on_msg(DltLogStorageFilterConfig *config,
                               DltLogStorageUserConfig *file_config,
                               char *dev_path,
                               int status)
{
    int ret;

    file_config = file_config;  /* satisfy compiler */
    dev_path = dev_path;

    if (config == NULL)
    {
        return -1;
    }

    if (status == DLT_LOGSTORAGE_SYNC_ON_MSG) /* sync on every message */
    {
        ret = fflush(config->log);

        if (ret != 0)
        {
            dlt_log(LOG_ERR, "fflush failed\n");
        }
    }

    return 0;
}

/**
 * dlt_logstorage_prepare_msg_cache
 *
 * Prepare the log file for a certain filer. If log file not open or log
 * files max size reached, open a new file.
 * Create a memory area to cache data.
 *
 * @param config        DltLogStorageFilterConfig
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param log_msg_size  Size of log message
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_prepare_msg_cache(DltLogStorageFilterConfig *config,
                                     DltLogStorageUserConfig *file_config,
                                     char *dev_path,
                                     int log_msg_size)
{
    if ((config == NULL) || (file_config == NULL) || (dev_path == NULL))
    {
        return -1;
    }

    /* Combinations allowed: on Daemon_Exit with on Demand,File_Size with Daemon_Exit
     *  File_Size with on Demand, Specific_Size with Daemon_Exit,Specific_Size with on Demand
     * Combination not allowed : File_Size with Specific_Size
     */
    /* check for combinations of specific_size and file_size strategy */
    if ((DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync, DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0) &&
        (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync, DLT_LOGSTORAGE_SYNC_ON_FILE_SIZE)) > 0)
    {
        dlt_log(LOG_WARNING, "wrong combination of sync strategies \n");
        return -1;
    }

    log_msg_size = log_msg_size; /* satisfy compiler */

    /* create file to sync cache into later */
    if (config->log == NULL)
    {

        if ((DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0) &&
                (config->specific_size > config->file_size))
        {
            dlt_log(LOG_ERR,
                    "Cannot prepare log file for ON_DAEMON_SPECIFIC_SIZE sync\n");
            return -1;
        }
        else
        {
            /* get always a new file */
            if (dlt_logstorage_prepare_on_msg(config,
                                              file_config,
                                              dev_path,
                                              config->file_size) != 0)
            {
                dlt_log(LOG_ERR,
                        "Cannot prepare log file for ON_DAEMON_EXIT sync\n");
                return -1;
            }
        }
    }


    if (config->cache == NULL)
    {

        /* check for sync_specific_size strategy */
        if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
               DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0)
        {
            /* check total logstorage cache size */
            if ((g_logstorage_cache_size +
                 config->specific_size +
                 sizeof(DltLogStorageCacheFooter)) >
                g_logstorage_cache_max)
            {
                dlt_log(LOG_ERR, "Max size of Logstorage Cache already used.");
                return -1;
            }

            /* create cache */
            config->cache = calloc(1,
                                   config->specific_size +
                                   sizeof(DltLogStorageCacheFooter));

            /* update current used cache size */
            g_logstorage_cache_size = config->specific_size +
                                      sizeof(DltLogStorageCacheFooter);
        }
        else  /* other cache strategies */
        {

            /* check total logstorage cache size */
            if ((g_logstorage_cache_size +
                 config->file_size +
                 sizeof(DltLogStorageCacheFooter)) >
                g_logstorage_cache_max)
            {
                dlt_log(LOG_ERR, "Max size of Logstorage Cache already used.");
                return -1;
            }

            /* create cache */
            config->cache = calloc(1,
                                   config->file_size +
                                   sizeof(DltLogStorageCacheFooter));

            /* update current used cache size */
            g_logstorage_cache_size = config->file_size +
                                      sizeof(DltLogStorageCacheFooter);
        }

        if (config->cache == NULL)
        {
            dlt_log(LOG_CRIT,
                    "Cannot allocate memory for filter ring buffer\n");
        }
    }

    return 0;
}

/**
 * dlt_logstorage_write_msg_cache
 *
 * Write the log message.
 *
 * @param config        DltLogStorageFilterConfig
 * @param data1         header
 * @param size1         header size
 * @param data2         storage header
 * @param size2         storage header size
 * @param data3         payload
 * @param size3         payload size
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_write_msg_cache(DltLogStorageFilterConfig *config,
                                   unsigned char *data1,
                                   int size1,
                                   unsigned char *data2,
                                   int size2,
                                   unsigned char *data3,
                                   int size3)
{
    DltLogStorageCacheFooter *footer = NULL;
    DltLogStorageUserConfig *uconfig = NULL;
    char *dev_path = NULL;
    int msg_size;
    int remain_cache_size;
    void *curr_write_addr = NULL;
    int ret = 0;

    if ((config == NULL) || (data1 == NULL) || (size1 < 0) || (data2 == NULL) ||
        (size2 < 0) || (data3 == NULL) || (size3 < 0) || (config->cache == NULL))
    {
        return -1;
    }

    if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                                     DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0)
    {
        footer = (DltLogStorageCacheFooter *)(config->cache +
                                              config->specific_size);
        if (footer == NULL)
        {
            dlt_log(LOG_ERR, "Cannot retrieve cache footer. Address is NULL\n");
            return -1;
        }
        msg_size = size1 + size2 + size3;
        remain_cache_size = config->specific_size - footer->offset;
    }
    else
    {
        footer = (DltLogStorageCacheFooter *)(config->cache +
                                              config->file_size);
        if (footer == NULL)
        {
            dlt_log(LOG_ERR, "Cannot retrieve cache footer. Address is NULL\n");
            return -1;
        }
        msg_size = size1 + size2 + size3;
        remain_cache_size = config->file_size - footer->offset;
    }

    if (msg_size < remain_cache_size) /* add at current position */
    {
        curr_write_addr = (void *)(config->cache + footer->offset);
        footer->offset += msg_size;
    }
    else if (msg_size > remain_cache_size)
    {
        if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                                     DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0)
        {
            /*check for message size exceeds cache size for specific_size strategy */
            if ((unsigned int) msg_size > config->specific_size)
            {
                dlt_log(LOG_WARNING, "Message is larger than cache. Discard.\n");
                return -1 ;
            }
        }
        else if ((unsigned int) msg_size > config->file_size)
        {
            dlt_log(LOG_WARNING, "Message is larger than cache. Discard.\n");
            return -1;
        }

         /*sync to file for specific_size or file_size  */
         if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                                                    DLT_LOGSTORAGE_SYNC_ON_FILE_SIZE) > 0)
         {
             ret = config->dlt_logstorage_sync(config,
                                               uconfig,
                                               dev_path,
                                               DLT_LOGSTORAGE_SYNC_ON_FILE_SIZE);
             if (ret != 0)
             {
                 dlt_log(LOG_ERR,"dlt_logstorage_sync: Unable to sync.\n");
                 return -1;
             }
         }
         else if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                                                         DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0)
         {

             ret = config->dlt_logstorage_sync(config,
                                               uconfig,
                                               dev_path,
                                               DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE);
             if (ret != 0)
             {
                 dlt_log(LOG_ERR,"dlt_logstorage_sync: Unable to sync.\n");
                 return -1;
             }
         }
         else if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                                                         DLT_LOGSTORAGE_SYNC_ON_DEMAND) > 0)
         {
             config->pre_cache_sync = config->cur_cache_sync;
             config->cur_cache_sync = 0;
             if (config->pre_cache_sync == 0)
             {
                 footer->last_sync_offset = 0;
             }
         }

        /* start writing from beginning */
        curr_write_addr = config->cache;
        footer->offset = msg_size;
        footer->wrap_around_cnt += 1;
    }
    else /* message just fits into cache */
    {
        curr_write_addr = (void *)(config->cache + footer->offset);
        footer->wrap_around_cnt += 1;
    }

    /* write data to cache */
    memcpy(curr_write_addr, data1, size1);
    curr_write_addr += size1;
    memcpy(curr_write_addr, data2, size2);
    curr_write_addr += size2;
    memcpy(curr_write_addr, data3, size3);

    return 0;
}

/**
 * dlt_logstorage_sync_msg_cache
 *
 * sync data to disk.
 *
 * @param config        DltLogStorageFilterConfig
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param status        Strategy flag
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_sync_msg_cache(DltLogStorageFilterConfig *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int status)
{
    int ret = 0;
    unsigned int remain_file_size = 0;
    unsigned int count = 0;

    DltLogStorageCacheFooter *footer = NULL;

    if (config == NULL)
    {
        return -1;
    }

    /* sync only, if given strategy is set */
    if (DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync, status) > 0)
    {
        if ((config->log == NULL) || (config->cache == NULL))
        {
            dlt_log(LOG_ERR,
                    "Cannot copy cache to file. One of both is NULL\n");
            return -1;
        }

        /* sync cache data to file for specific_size strategies */

        if ((status == DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE)
                || ((DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                        DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0)
                        && (status == DLT_LOGSTORAGE_SYNC_ON_DEMAND))
                || ((DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                        DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE) > 0)
                        && (status == DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT)))
        {
            footer = (DltLogStorageCacheFooter *)(config->cache +
                                                  config->specific_size);
            if (footer == NULL)
            {
                dlt_log(LOG_ERR, "Cannot retrieve cache information\n");
                return -1;
            }

            count = footer->offset - footer->last_sync_offset;
            /* write ring buffer into file */
            ret = fwrite(config->cache + footer->last_sync_offset, count, 1, config->log);
            dlt_logstorage_check_write_ret(config, ret);
            config->current_write_file_offset += footer->offset - footer->last_sync_offset;
            remain_file_size = config->file_size - config->current_write_file_offset;

            if (status == DLT_LOGSTORAGE_SYNC_ON_DEMAND)
            {
                footer->last_sync_offset = footer->offset;
            }
            else
            {
                footer->last_sync_offset = 0;
            }

            if (remain_file_size < config->specific_size)
            {
                config->current_write_file_offset = 0;
                /* clean ring buffer and reset footer information */
                memset(config->cache,
                       0,
                       config->specific_size + sizeof(DltLogStorageCacheFooter));

                /* close the file, a new one will be created when prepare is
                 * called again */

                fclose(config->log);
                config->log = NULL;
             }
        }
        /* sync cache data to file for file size strategies*/

        else if ((status == DLT_LOGSTORAGE_SYNC_ON_FILE_SIZE) ||
                 (status == DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT) ||
                 ((DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(config->sync,
                                 DLT_LOGSTORAGE_SYNC_ON_FILE_SIZE) > 0)
                         && (status == DLT_LOGSTORAGE_SYNC_ON_DEMAND)))
        {
            footer = (DltLogStorageCacheFooter *)(config->cache +
                                                  config->file_size);
            if (footer == NULL)
            {
                dlt_log(LOG_ERR, "Cannot retrieve cache information\n");
                return -1;
            }

            count = footer->offset - footer->last_sync_offset;
            /* write cache to file */
            ret = fwrite(config->cache + footer->last_sync_offset, count, 1, config->log);
            dlt_logstorage_check_write_ret(config, ret);
            config->current_write_file_offset += footer->offset - footer->last_sync_offset;
            footer->last_sync_offset = footer->offset;

            if ((status != DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT)
                           && (status != DLT_LOGSTORAGE_SYNC_ON_DEMAND))
            {
                config->current_write_file_offset = 0;
                /* clean ring buffer and reset footer information */
                memset(config->cache,
                        0,
                        config->file_size + sizeof(DltLogStorageCacheFooter));

                /* close the file, a new one will be created when prepare is
                 * called again */
                fclose(config->log);
                config->log = NULL;
            }
        }
        /* sync cache to file for on_demand strategy  */

        else if (status == DLT_LOGSTORAGE_SYNC_ON_DEMAND)
        {
            config->sync_from_start = 0;
            if ((file_config == NULL) || (dev_path == NULL))
            {
                dlt_log(LOG_ERR, "Cannot retrieve user configuration or mount point\n");
                return -1;
            }
            footer = (DltLogStorageCacheFooter *)(config->cache +
                                                  config->file_size);
            if (footer == NULL)
            {
                dlt_log(LOG_ERR, "Cannot retrieve cache information\n");
                return -1;
            }
            /* check for wrap around is 0 or cache is synced at least once
             * in every wrap around */

            if ((footer->wrap_around_cnt < 1) || (config->cur_cache_sync == 1))
            {
                ret = dlt_logstorage_sync_to_file(config,
                                                  file_config,
                                                  dev_path);
                if (ret != 0)
                {
                    dlt_vlog(LOG_ERR, "%s: failed to sync data to file \n", __func__);
                }
            }
            else
            {
                remain_file_size = config->file_size - config->current_write_file_offset;

                /* check for total bytes of data need to sync */
                if (footer->offset >= footer->last_sync_offset )
                {
                    config->total_write_count = config->file_size;
                }
                else
                {
                    config->total_write_count = config->file_size - footer->last_sync_offset + footer->offset;
                }

                /* sync data to file if required sync data exceeds remaining file size */
                if (config->total_write_count >= remain_file_size)
                {
                    ret = dlt_logstorage_sync_create_new_file(config,
                                                              file_config,
                                                              dev_path,
                                                              remain_file_size);
                    if (ret != 0)
                    {
                        dlt_vlog(LOG_ERR, "%s: failed to sync and open new file\n", __func__);
                    }

                    /* sync remaining end of cache data to new file*/
                    if (config->sync_from_start == 0)
                    {
                        ret = dlt_logstorage_sync_capable_data_to_file(config, 0);
                        if (ret != 0)
                        {
                            dlt_vlog(LOG_ERR, "%s: failed to sync capable data to file\n", __func__);
                        }
                    }
                }
                /* sync data to file if required sync data less than remaining file size */
                else
                {
                    ret = dlt_logstorage_sync_capable_data_to_file(config, 1);
                    if (ret != 0)
                    {
                        dlt_vlog(LOG_ERR, "%s: failed to sync capable data\n", __func__);
                    }
                }
                /* sync data to file from almost the begin of cache
                 * if still data needs to sync */

                if (config->total_write_count > 0)
                {
                    count = footer->offset - footer->last_sync_offset;
                    ret = fwrite(config->cache + footer->last_sync_offset,
                                 count,
                                 1,
                                 config->log);
                    dlt_logstorage_check_write_ret(config, ret);
                    config->current_write_file_offset += count;

                    footer->last_sync_offset += count;
                }
                config->cur_cache_sync = 1;
            }
        }
    }

    return 0;
}
