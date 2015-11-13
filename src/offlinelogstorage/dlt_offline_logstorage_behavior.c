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
    if (log_file_name == NULL || file_config == NULL)
    {
        return;
    }

    char file_index[10] = {'\0'};

    // create log file name
    memset(log_file_name, 0, DLT_MOUNT_PATH_MAX * sizeof(char));
    strcat(log_file_name, name);
    strncat(log_file_name, &file_config->logfile_delimiter, 1);

    snprintf(file_index, 10, "%d",idx);

    if (file_config->logfile_maxcounter != UINT_MAX)
    {
        /* Setup 0's to be appended in file index until max index len*/
        unsigned int digit_idx = 0;
        unsigned int i = 0;
        snprintf(file_index, 10, "%d",idx);
        digit_idx = strlen(file_index);
        for(i=0; i<(file_config->logfile_counteridxlen - digit_idx); i++)
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

    if (head == NULL || *head == NULL || (*head)->next == NULL)
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

    if (head == NULL || *head == NULL || (*head)->next == NULL)
    {
        return;
    }

    for (n = *head; n != NULL; n = n->next)
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
unsigned int dlt_logstorage_get_idx_of_log_file(
        DltLogStorageUserConfig *file_config,
        char *file)
{
    unsigned int idx = -1;
    char *endptr;
    char *filename;
    unsigned int filename_len = 0 ;
    unsigned int fileindex_len = 0;

    if (file_config == NULL || file == NULL)
    {
        return -1;
    }

    /* Calculate actual file name length */
    filename=strchr(file, file_config->logfile_delimiter);
    filename_len = strlen(file) - strlen(filename);

    /* index is retrived from file name */
    if(file_config->logfile_timestamp)
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

        idx = (int) strtol(&file[strlen(file)-
                        (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN
                        +fileindex_len)], &endptr, 10);
    }

    if (endptr == file || idx == 0)
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
 * @param  config       DltLogStorageConfigData
 * @return              0 on success, -1 on error
 */
int dlt_logstorage_storage_dir_info(DltLogStorageUserConfig *file_config,
                                    char *path,
                                    DltLogStorageConfigData *config)
{
    int i = 0;
    int cnt = 0;
    int ret = 0;
    struct dirent **files = {0};
    unsigned int current_idx = 0;

    if (config == NULL ||
        file_config == NULL ||
        path == NULL ||
        config->file_name == NULL)
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

    for (i = 0; i < cnt; i++)
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

            if(config->records == NULL)
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
                while(*(tmp) != NULL)
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
            (*tmp)->idx =  current_idx;
            (*tmp)->next = NULL;
        }
    }

    if (ret == 0)
    {
        dlt_logstorage_sort_file_name(&config->records);
        dlt_logstorage_rearrange_file_name(&config->records);
    }

    /* free scandir result */
    for (i = 0; i < cnt; i++)
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
 * @param  config    DltLogStorageConfigData
 * @param  file_config   User configurations for log file
 * @param  dev_path      Storage device path
 * @param  msg_size  Size of incoming message
 * @return 0 on succes, -1 on error
 */
int dlt_logstorage_open_log_file(DltLogStorageConfigData *config,
                                 DltLogStorageUserConfig *file_config,
                                 char *dev_path,
                                 int msg_size)
{
    int ret = 0;
    char absolute_file_path[DLT_MOUNT_PATH_MAX + 1] = {'\0'};
    char storage_path[DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN] = {'\0'};
    unsigned int num_log_files = 0;
    struct stat s;
    DltLogStorageFileList **tmp = NULL;
    DltLogStorageFileList **newest = NULL;
    char file_name[DLT_MOUNT_PATH_MAX + 1] = {'\0'};

    if (config == NULL)
    {
        return -1;
    }

    sprintf(storage_path, "%s/", dev_path);

    /* check if there are already files stored */
    if (config->records == NULL)
    {
        if (dlt_logstorage_storage_dir_info(file_config, storage_path, config)
            != 0)
            return -1;
    }

    /* obtain locations of newest, current file names, file count */
    tmp = &config->records;
    while(*(tmp) != NULL)
    {
        num_log_files += 1;
        if((*tmp)->next == NULL)
            newest = tmp;

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
        (*tmp)->idx =  1;
        (*tmp)->next = NULL;
    }
    else /* newest file available*/
    {
        strcat(absolute_file_path, storage_path);
        strcat(absolute_file_path, (*newest)->name);

        ret = stat(absolute_file_path, &s);

        /* if size is enough, open it */
        if (ret == 0 && s.st_size + msg_size < (int)config->file_size)
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
            if (idx > file_config->logfile_maxcounter || idx == 0)
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
                   sizeof(absolute_file_path)/sizeof(char));
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
            (*tmp)->idx =  idx;
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
                       sizeof(absolute_file_path)/sizeof(char));
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
 * dlt_logstorage_prepare_on_msg
 *
 * Prepare the log file for a certain filer. If log file not open or log
 * files max size reached, open a new file.
 *
 * @param config        DltLogStorageConfigData
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param log_msg_size  Size of log message
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_prepare_on_msg(DltLogStorageConfigData *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int log_msg_size)
{
    int ret = 0;
    struct stat s;

    if (config == NULL || file_config == NULL || dev_path == NULL)
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
        if (ret == 0) {
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
 * @param config        DltLogStorageConfigData
 * @param data1         header
 * @param size1         header size
 * @param data2         storage header
 * @param size2         storage header size
 * @param data3         payload
 * @param size3         payload size
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_write_on_msg(DltLogStorageConfigData *config,
                         unsigned char *data1,
                         int size1,
                         unsigned char *data2,
                         int size2,
                         unsigned char *data3,
                         int size3)
{
    int ret;

    if (config == NULL || data1 == NULL || data2 == NULL || data3 == NULL)
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
 * @param config        DltLogStorageConfigData
 * @param status        Strategy flag
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_sync_on_msg(DltLogStorageConfigData *config, int status)
{
    int ret;

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
 * dlt_logstorage_prepare_on_daemon_exit
 *
 * Prepare the log file for a certain filer. If log file not open or log
 * files max size reached, open a new file.
 * Create a memory area to cache data.
 *
 * @param config        DltLogStorageConfigData
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param log_msg_size  Size of log message
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_prepare_on_daemon_exit(DltLogStorageConfigData *config,
                                          DltLogStorageUserConfig *file_config,
                                          char *dev_path,
                                          int log_msg_size)
{
    if (config == NULL || file_config == NULL || dev_path == NULL)
    {
        return -1;
    }

    log_msg_size = log_msg_size; /* satisfy compiler */

    /* open log file */
    if (config->cache == NULL)
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

        /* check total logstorage cache size */
        if ((g_logstorage_cache_size +
                config->file_size +
                sizeof(DltLogStorageCacheFooter)) >
            g_logstorage_cache_max)
        {
            dlt_log(LOG_ERR, "Max size of Logstorage Cache already used.");
            return -1;
        }

        config->cache = calloc(1,
                               config->file_size +
                               sizeof(DltLogStorageCacheFooter));

        /* update current used cache size */
        g_logstorage_cache_size = config->file_size +
                                  sizeof(DltLogStorageCacheFooter);

        if (config->cache == NULL)
        {
            dlt_log(LOG_CRIT,
                    "Cannot allocate memory for filter ring buffer\n");
        }
    }

    return 0;
}

/**
 * dlt_logstorage_write_on_daemon_exit
 *
 * Write the log message.
 *
 * @param config        DltLogStorageConfigData
 * @param data1         header
 * @param size1         header size
 * @param data2         storage header
 * @param size2         storage header size
 * @param data3         payload
 * @param size3         payload size
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_write_on_daemon_exit(DltLogStorageConfigData *config,
                                        unsigned char *data1,
                                        int size1,
                                        unsigned char *data2,
                                        int size2,
                                        unsigned char *data3,
                                        int size3)
{
    DltLogStorageCacheFooter *footer = NULL;
    int msg_size;
    int remain_cache_size;
    void *curr_write_addr = NULL;

    if (config == NULL || data1 == NULL || size1 < 0 || data2 == NULL ||
        size2 < 0 || data3 == NULL || size3 < 0 || config->cache == NULL)
    {
        return -1;
    }

    footer = (DltLogStorageCacheFooter *)(config->cache + config->file_size);
    if (footer == NULL)
    {
        dlt_log(LOG_ERR, "Cannot retrieve cache footer. Address is NULL\n");
        return -1;
    }

    msg_size = size1 + size2 + size3;
    remain_cache_size = config->file_size - footer->offset;

    if (msg_size < remain_cache_size) /* add at current position */
    {
        curr_write_addr = (void *)(config->cache + footer->offset);
        footer->offset += msg_size;
    }
    else if (msg_size > remain_cache_size)
    {
        if ((unsigned int) msg_size > config->file_size)
        {
            dlt_log(LOG_WARNING, "Message is larger than cache. Discard.\n");
            return -1;
        }

        /* start writing from beginning */
        curr_write_addr = config->cache;
        footer->offset = msg_size;
        footer->wrap_around_cnt += 1;
    }
    else /* message just fits into cache */
    {
        curr_write_addr = (void *)(config->cache + footer->offset);
        footer->offset = 0;
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
 * dlt_logstorage_sync_on_daemon_exit
 *
 * sync data to disk.
 *
 * @param config        DltLogStorageConfigData
 * @param status        Strategy flag
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_sync_on_daemon_exit(DltLogStorageConfigData *config,
                                       int status)
{
    int ret = 0;
    DltLogStorageCacheFooter *footer = NULL;

    if (config == NULL)
    {
        return -1;
    }

    if (status == config->sync) /* only sync on exit */
    {
        if (config->log == NULL || config->cache == NULL)
        {
            dlt_log(LOG_ERR,
                    "Cannot copy cache to file. One of both is NULL\n");
            return -1;
        }

        /* TODO: write only what is needed */
        /* write ring buffer into file */
        footer = (DltLogStorageCacheFooter *)(config->cache +
                                              config->file_size);

        if (footer == NULL)
        {
            dlt_log(LOG_ERR, "Cannot retrieve cache information\n");
            return -1;
        }

        /* TODO: check if really the whole cache need to be written */
        /* write cache to file */
        ret = fwrite(config->cache, config->file_size, 1, config->log);

        if (ret <= 0)
        {
            if (ferror(config->log) != 0)
            {
                dlt_log(LOG_CRIT, "Failed to write cache into log file\n");
            }
        }
        else
        {
            /* force sync */
            if (fsync(fileno(config->log)) != 0)
            {
                dlt_log(LOG_ERR, "Failed to sync log file\n");
            }
        }
    }

    return ret;
}
