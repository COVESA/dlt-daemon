/**
 * @licence app begin@
 * Copyright (C) 2013 - 2015  Advanced Driver Information Technology.
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
 * \author Syed Hameed <shameed@jp.adit-jv.com> ADIT 2013 - 2015
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file: dlt_offline_logstorage.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include "dlt_offline_logstorage.h"
#include "dlt_offline_logstorage_behavior.h"
#include "dlt_config_file_parser.h"

#define DLT_OFFLINE_LOGSTORAGE_FILTER_ERROR 1
#define DLT_OFFLINE_LOGSTORAGE_STORE_FILTER_ERROR 2

#define GENERAL_BASE_NAME "General"
/* Hash map functions */

static int dlt_logstorage_hash_create(int num_entries, struct hsearch_data *htab)
{
    memset(htab, 0, sizeof(*htab));

    if (hcreate_r(num_entries, htab) == 0)
        return -1;

    return 0;
}

static int dlt_logstorage_hash_destroy(struct hsearch_data *htab)
{
    hdestroy_r(htab);

    return 0;
}

static int dlt_logstorage_hash_add(char *key, void *value, struct hsearch_data *htab)
{
    ENTRY e, *ep;

    memset(&e, 0, sizeof(ENTRY));
    e.key = key;
    e.data = value;

    if (hsearch_r(e, ENTER, &ep, htab) == 0)
        return -1;

    return 0;
}

static void *dlt_logstorage_hash_find(char *key, struct hsearch_data *htab)
{
    ENTRY e, *ep;

    e.key = key;

    if (hsearch_r(e, FIND, &ep, htab) != 0)
        return ep->data;
    else
        return NULL;
}

/* Configuration file parsing helper functions */

int dlt_logstorage_count_ids(const char *str)
{

    if(str == NULL)
        return -1;

    // delimiter is: ","
    const char *p = str;
    int i = 0;
    int num = 1;

    while(p[i] != 0)
    {
        if (p[i] == ',')
            num += 1;

        i++;
    }

    return num;
}

/**
 * dlt_logstorage_read_list_of_names
 *
 * Evaluate app and ctx names given in config file and create a list of names acceptable by DLT Daemon
 * When using SET_APPLICATION_NAME and SET_CONTEXT_NAME there is no constraint that these names have max
 * 4 characters. Internally, these names are cutted down to max 4 chars. To have create valid keys,
 * the internal representation of these names has to be considered.
 * Therefore, a given configuration of "AppLogName = App1,Application2,A3" will be stored as
 * "App1,Appl,A3".
 *
 * @param names        to store the list of names
 * @param value        string given in config file
 * @return             0 on success, -1 on error
 */
int dlt_logstorage_read_list_of_names(char **names, char *value)
{
    int i = 0;
    int y = 0;
    int len = 0;
    char *tok;
    int num = 1;

    /* free, alloce'd memory sot store new apid/ctid */
    if (*names != NULL)
    {
        free(*names);
        *names = NULL;
    }

    if (value == NULL)
    {
        return -1;
    }

    len = strlen(value);

    if (len == 0)
    {
        return -1;
    }

    /* count number of delimiters to get actual number off names */
    num = dlt_logstorage_count_ids(value);

    /* need to alloc space for 5 chars, 4 for the name and "," and "\0" */
    *(names) = (char *)calloc(num * 5, sizeof(char));

    tok = strtok(value, ",");

    i = 1;
    while (tok != NULL)
    {
        len = strlen(tok);
        len = DLT_OFFLINE_LOGSTORAGE_MIN(len, 4);

        strncpy((*names + y), tok, len);
        if (num > 1 && i < num)
        {
            strncpy((*names + y + len), ",", 1);
        }
        y += len + 1;

        i++;
        tok = strtok(NULL, ",");
    }

    return 0;
}

/**
 * dlt_logstorage_read_log_level
 *
 * Evaluate log level given in config file and calculate log level as int
 *
 * @param log_level    to store the log level
 * @param value        string given in config file
 * @return             0 on success, -1 on error
 */
int dlt_logstorage_read_log_level(int *log_level, char *value)
{
    if (value == NULL)
    {
        *log_level = 0;
        return -1;
    }

    if (strcmp(value, "DLT_LOG_FATAL") == 0)
    {
        *log_level = 1;
    }
    else if (strcmp(value, "DLT_LOG_ERROR") == 0)
    {
        *log_level = 2;
    }
    else if (strcmp(value, "DLT_LOG_WARN") == 0)
    {
        *log_level = 3;
    }
    else if (strcmp(value, "DLT_LOG_INFO") == 0)
    {
        *log_level = 4;
    }
    else if (strcmp(value, "DLT_LOG_DEBUG") == 0)
    {
        *log_level = 5;
    }
    else if (strcmp(value, "DLT_LOG_VERBOSE") == 0)
    {
        *log_level = 6;
    }
    else
    {
        *log_level = 0;
        dlt_log(LOG_ERR, "Invalid log level \n");
        return -1;
    }
    return 0;
}

/**
 * dlt_logstorage_read_file_name
 *
 * Evaluate if file name given in config file contains ".." , if not set file name
 *
 * @param file_name    string to store the file name
 * @param value        string given in config file
 * @return             0 on success, -1 on error
 */
int dlt_logstorage_read_file_name(char **file_name, char *value)
{
    int len;

    if (value == NULL || strcmp(value, "") == 0)
    {
        return -1;
    }

    if (*file_name != NULL)
    {
        *file_name = NULL;
    }

    len = strlen(value);

    /* do not allow the user to change directory by adding a path like ../../logfile */
    if (strstr(value, "..") == NULL)
    {
        *file_name = calloc((len+1), sizeof(char));
        strncpy(*file_name, value, len);
    }
    else
    {
        dlt_log(LOG_ERR, "Invalid filename, .. is not accepted due to security issues \n");
        return -1;
    }

    return 0;
}

/**
 * dlt_logstorage_read_number
 *
 * Evaluate file size and number of files given in config file and set file size
 * The file number is checked by converting a string to an unsigned integer
 * width 0 > result < UINT_MAX (excludes 0!)
 * Non-digit characters including spaces and out of boundary will lead to an error -1.
 *
 * @param file_name    string to store the file name
 * @param value        string given in config file
 * @return             0 on success, -1 on error
 */
int dlt_logstorage_read_number(unsigned int *number, char *value)
{
    int i = 0;
    int len = 0;
    unsigned long size = 0;

    if (value == NULL)
    {
        return -1;
    }

    *number = 0;
    len = strlen(value);

    /* check if string consists of digits only */
    for (i = 0; i < len; i++)
    {
        if (isdigit(value[i] == 0))
        {
            dlt_log(LOG_ERR, "Invalid, is not a number \n");
            return -1;
        }
    }

    size = strtoul(value, NULL, 10);

    if (size == 0 || size > UINT_MAX)
    {
        dlt_log(LOG_ERR, "Invalid, is not a number \n");
        return -1;
    }

    *number = (unsigned int) size;

    return 0;
}

/**
 * dlt_logstorage_set_sync_strategy
 *
 * Evaluate sync strategy. The sync strategy is an optional filter
 * configuration parameter.
 * If the given value cannot be associated with a sync strategy, the default
 * sync strategy will be assigned.
 *
 * @param file_name    int to store the sync strategy
 * @param value        string given in config file
 * @return             0 on success, -1 on error
 */
int dlt_logstorage_set_sync_strategy(int *strategy, char *value)
{
    if (value == NULL || strategy == NULL)
    {
        return -1;
    }

    if (strcasestr(value, "ON_MSG") != NULL)
    {
        *strategy = DLT_LOGSTORAGE_SYNC_ON_MSG;
        dlt_log(LOG_DEBUG, "ON_MSG found, ignore other if added\n");
    }
    else /* ON_MSG not set, combination of cache based strategies possible */
    {
        if (strcasestr(value, "ON_DAEMON_EXIT") != NULL)
        {
            *strategy |= DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT;
        }

        if (strcasestr(value, "ON_DEMAND") != NULL)
        {
            *strategy |= DLT_LOGSTORAGE_SYNC_ON_DEMAND;
        }

        if (strcasestr(value, "ON_DEVICE_DISCONNECT") != NULL)
        {
            *strategy |= DLT_LOGSTORAGE_SYNC_ON_DEVICE_DISCONNECT;
        }

        if (*strategy == 0)
        {
            dlt_log(LOG_WARNING, "Unknown sync strategies. Set default ON_MSG\n");
            *strategy = DLT_LOGSTORAGE_SYNC_ON_MSG;
        }
    }
    return 0;
}

/**
 * dlt_logstorage_set_ecuid
 *
 * Evaluate if ECU idenfifier given in config file
 *
 * @param ecuid        string to store the ecuid name
 * @param value        string given in config file
 * @return             0 on success, -1 on error
 */
int dlt_logstorage_set_ecuid(char **ecuid, char *value)
{
    int len;

    if (ecuid == NULL || value == NULL || value[0] == '\0')
    {
        return -1;
    }

    if (*ecuid != NULL)
    {
        free(*ecuid);
        *ecuid = NULL;
    }

    len = strlen(value);
    *ecuid = calloc((len+1), sizeof(char));
    strncpy(*ecuid, value, len);

    return 0;
}

/**
 * dlt_logstorage_create_keys
 *
 * Create keys for hash table
 *
 * From each section [filter] in offline logstorage configuration file, we receive
 * application and context id strings.
 * Application and context id can consist of
 * - a 4char long name
 * - a comma separated list of ids
 * - a wildcard: .*
 *
 * Not allowed is the combination of application id and context id set to wildcard. This
 * will be rejected.
 *
 * If lists given for application and/or context id, all possible combinations are
 * returned as keys in a form "[appid][ctxid], e.g. "APP1:CTX1". If wildcards are used,
 * the non-wildcard value becomes the key, e.g. "APP1:" or ":CTX2".
 *
 * @param[in]: appids: string given from filter configuration
 * @param[in]: ctxids: string given from filter configuration
 * @param[out]: keys: keys to fill into hash table
 * @param[out]: num_keys: number of keys
 * @return: 0 on success, error on failure*
 */
int dlt_logstorage_create_keys(char *appids, char* ctxids, char **keys, int *num_keys)
{
    int i,j;
    int curr_key = 0;
    int curr_len = 0;
    int num_appids = 0;
    int num_ctxids = 0;
    char *aids = NULL;
    char *cids = NULL;
    char *save_aids = NULL;
    char *save_cids = NULL;

    if ((*keys) != NULL)
    {
        free((*keys));
        (*keys) = NULL;
    }

    *num_keys = 0;

    if (appids == NULL || ctxids == NULL)
        return -1;

    /* discard appid=.* and ctxid=.* */
    if ( strncmp(appids, ".*", 2) == 0 && strncmp(ctxids, ".*", 2) == 0 )
    {
        dlt_log(LOG_ERR,"Error: Not allowed combination of wildcards\n");
        return -1;
    }

    aids = strdup(appids);

    cids = (char *) calloc(strlen(ctxids)+1, sizeof(char));
    if (cids == NULL)
    {
        free(aids);
        return -1;
    }

    /* calculate number of keys */
    num_appids = dlt_logstorage_count_ids(appids);
    num_ctxids = dlt_logstorage_count_ids(ctxids);
    *(num_keys) = num_appids * num_ctxids;

    /* alloc needed number of keys */
    *(keys) = (char*) calloc(*num_keys * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN, sizeof(char));
    if (*(keys) == NULL)
    {
        free(aids);
        free(cids);
        return -1;
    }

    /* store all combinations of appid:ctxid in keys */
    for (i = 1; i <= num_appids; i++)
    {
        char *tok_aids = NULL;
        char *tok_cids = NULL;

        if (num_appids > 1 && i == 1)
        {
            tok_aids = strtok_r(aids,",", &save_aids);
        }
        else if (num_appids > 1 && i > 0)
        {
            tok_aids = strtok_r(NULL, ",", &save_aids);
        }
        else
        {
            tok_aids = aids;
        }

        for (j = 1; j <= num_ctxids; j++)
        {
            if (num_ctxids > 1 && j == 1)
            {
                save_cids = NULL;
                memcpy(cids, ctxids, strlen(ctxids));
                tok_cids = strtok_r(cids, ",", &save_cids);
            }
            else if (num_ctxids > 1 && j > 0)
            {
                tok_cids = strtok_r(NULL, ",", &save_cids);
            }
            else
            {
                tok_cids = ctxids;
            }

            if (strncmp(tok_aids, ".*", 2) == 0) /* only context id matters */
            {
                char curr_str[10] = { 0 };
                strncpy(curr_str, ":", 1);
                curr_len = strlen(tok_cids);
                strncat(curr_str, tok_cids, curr_len);
                curr_len = strlen(curr_str);
                strncpy((*keys + (curr_key * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN)), curr_str, curr_len);
            }
            else if (strncmp(tok_cids,".*", 2) == 0) /* only application id matters*/
            {
                char curr_str[10] = { 0 };
                curr_len = strlen(tok_aids);
                strncpy(curr_str, tok_aids, curr_len);
                strncat(curr_str, ":", 1);
                curr_len = strlen(curr_str);
                strncpy((*keys + (curr_key * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN)), curr_str, curr_len);
            }
            else /* key is combination of both */
            {
                char curr_str[10] = { 0 };
                curr_len = strlen(tok_aids);
                strncpy(curr_str, tok_aids, curr_len);
                strncat(curr_str, ":", 1);
                curr_len = strlen(tok_cids);
                strncat(curr_str, tok_cids, curr_len);
                curr_len = strlen(curr_str);
                strncpy((*keys + (curr_key * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN)), curr_str, curr_len);
            }
            curr_key += 1;
        }
    }
    free(aids);
    free(cids);

    return 0;
}

/**
 * dlt_logstorage_device_connected
 *
 * Initializes DLT Offline Logstorage with respect to device status
 *
 * @param handle         DLT Logstorage handle
 * @param mount_point    Device mount path
 * @return               0 on success, -1 on error
 */
int dlt_logstorage_device_connected(DltLogStorage *handle, char *mount_point)
{
    if((handle == NULL) || (mount_point == NULL))
    {
        dlt_log(LOG_ERR, "dlt_logstorage_device_connected Error : Handle error \n");
        return -1;
    }

    if(handle->connection_type == DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_device_connected Error : Device already connected,  \n");
        dlt_log(LOG_ERR, "Send disconnect, connect request \n");

        dlt_logstorage_device_disconnected(
                handle,
                DLT_LOGSTORAGE_SYNC_ON_DEVICE_DISCONNECT);
    }

    strncpy(handle->device_mount_point,mount_point,DLT_MOUNT_PATH_MAX);
    handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle->config_status = 0;
    handle->write_errors = 0;
    handle->num_filter_keys = 0;

    handle->config_data = NULL;
    handle->filter_keys = NULL;

    return 0;
}

/**
 * dlt_logstorage_free
 *
 * Free all allocated memory used in log storage handle
 *
 * @param handle         DLT Logstorage handle
 * @param reason         Reason for freeing the device
 *
 */
void dlt_logstorage_free(DltLogStorage *handle, int reason)
{
    int i=0;

    dlt_logstorage_hash_destroy(&(handle->config_htab));

    for(i=0; i<handle->num_filter_keys; i++)
    {
        /* sync data if necessary */
        /* ignore return value */
        handle->config_data[i].data.dlt_logstorage_sync(
                &(handle->config_data[i].data),
                reason);

        free(handle->config_data[i].data.file_name);

        if (handle->config_data[i].data.ecuid != NULL)
        {
            free(handle->config_data[i].data.ecuid);
        }

        if (handle->config_data[i].data.log != NULL)
        {
            fclose(handle->config_data[i].data.log);
        }

        if (handle->config_data[i].data.cache != NULL)
        {
            free(handle->config_data[i].data.cache);
        }

        DltLogStorageFileList *n = handle->config_data[i].data.records;
        while(n)
        {
            DltLogStorageFileList *n1 = n;
            n = n->next;
            free(n1->name);
            free(n1);
        }
    }

    free(handle->config_data);
    handle->config_data = NULL;

    free(handle->filter_keys);
    handle->filter_keys = NULL;
}


/**
 * dlt_logstorage_device_disconnected
 *
 * De-Initializes DLT Offline Logstorage with respect to device status
 *
 * @param handle         DLT Logstorage handle
 * @param reason         Reason for disconnect
 * @return               0 on success, -1 on error
 *
 */
int dlt_logstorage_device_disconnected(DltLogStorage *handle, int reason)
{
    if (handle == NULL)
        return -1;

    /* If configuration loading was done, free it */
    if (handle->config_status == DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
    {
        dlt_logstorage_free(handle, reason);
    }

    /* Reset all device status */
    memset(handle->device_mount_point,'\0', sizeof(char) * DLT_MOUNT_PATH_MAX);
    handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED;
    handle->config_status = 0;
    handle->write_errors = 0;
    handle->num_filter_keys = 0;

    return 0;
}

/**
 * dlt_logstorage_prepare_table
 *
 * Prepares hash table with keys and data
 *
 * @param handle         DLT Logstorage handle
 * @param appid          Application ID value provided in configuration file
 * @param appid          Context ID value provided in configuration file
 * @param tmp_data       Holds all other configuration values
 * @return               0 on success, -1 on error
 *
 */
int dlt_logstorage_prepare_table(DltLogStorage *handle, char *appid, char *ctxid, DltLogStorageConfigData *tmp_data)
{
    int ret = 0;
    int num_keys = 0;
    char *keys = NULL;
    int idx = 0;

    /* Allocate memory for filters */
    if(handle->config_data == NULL)
    {
        handle->config_data = malloc(sizeof(DltLogStorageConfig) * DLT_OFFLINE_LOGSTORAGE_MAXFILTERS);
        memset(handle->config_data, 0, (sizeof(DltLogStorageConfig) * DLT_OFFLINE_LOGSTORAGE_MAXFILTERS));

        handle->filter_keys = malloc(sizeof(char) * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN * DLT_OFFLINE_LOGSTORAGE_MAXFILTERS);
        memset(handle->filter_keys, 0, sizeof(char) * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN * DLT_OFFLINE_LOGSTORAGE_MAXFILTERS);
    }

    ret = dlt_logstorage_create_keys(appid, ctxid, &keys, &num_keys);
    if (ret != 0)
    {
        dlt_log(LOG_ERR, "Not able to create keys for hash table\n");
        return -1;
    }

    /* hash_add */
    for (idx=0; idx<num_keys; idx++)
    {
        DltLogStorageConfig *p_node = NULL;

        if (num_keys > (DLT_OFFLINE_LOGSTORAGE_MAXFILTERS - handle->num_filter_keys))
        {
            dlt_log(LOG_ERR, "MAX filters reached \n");
            break;
        }

        p_node = &(handle->config_data[handle->num_filter_keys]);

        strcpy(p_node->key, keys+(idx * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN));
        memcpy(&p_node->data, tmp_data, sizeof(DltLogStorageConfigData));
        p_node->data.file_name = strdup(tmp_data->file_name);
        if (tmp_data->ecuid != NULL)
        {
            p_node->data.ecuid = strdup(tmp_data->ecuid);
        }
        else
        {
            p_node->data.ecuid = NULL;
        }
        p_node->data.records = NULL;
        p_node->data.log = NULL;
        p_node->data.cache = NULL;

        if(dlt_logstorage_hash_add(p_node->key, &p_node->data, &(handle->config_htab)) != 0)
        {
            dlt_log(LOG_ERR, "Adding to hash table failed, returning failure\n");

            dlt_logstorage_free(handle, DLT_LOGSTORAGE_SYNC_ON_ERROR);

            free(keys);
            return -1;
        }
        /* update filter keys and number of keys */
        strncat(handle->filter_keys + handle->num_filter_keys * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN, keys, strlen(keys));
        handle->num_filter_keys += 1;
    }
    free(keys);
    return 0;
}

/**
 * dlt_logstorage_validate_filter_value
 *
 * This function analyzes the filter values
 * and stores the values into provided input arguments
 * Here appid and ctxid are used to hold application name and context name
 * values provided in configuration file, all other configuration values
 * are stored in tmp_data
 * the caller can utilize the return values of this
 * function to know when a complete filter set is read
 *
 * @param filter_key     Filter key read from configuration file
 * @param filter_value   Filter value read from configuration file
 * @param appid          Application ID value provided in configuration file
 * @param ctxid          Context ID value provided in configuration file
 * @param tmp_data       Holds all other configuration values
 * @return               Configuration value type on success, -1 on error
 *
 */
int dlt_logstorage_validate_filter_value(char *filter_key, char *filter_value,
                                        char **appid, char **ctxid,
                                        DltLogStorageConfigData *tmp_data)
{
    int ret = -1;

    if (strncmp(filter_key, "LogAppName", strlen("LogAppName")) == 0)
    {
        ret = dlt_logstorage_read_list_of_names(appid, filter_value);
        if (ret == 0)
            ret = DLT_OFFLINE_LOGSTORAGE_APP_INIT;
    }
    else if (strncmp(filter_key, "ContextName", strlen("ContextName")) == 0)
    {
        ret = dlt_logstorage_read_list_of_names(ctxid, filter_value);
        if (ret == 0)
            ret = DLT_OFFLINE_LOGSTORAGE_CTX_INIT;
    }
    else if (strncmp(filter_key, "LogLevel", strlen("LogLevel")) == 0)
    {
        ret = dlt_logstorage_read_log_level(&(tmp_data->log_level), filter_value);
        if (ret == 0)
            ret = DLT_OFFLINE_LOGSTORAGE_LOG_LVL_INIT;
    }
    else if (strncmp(filter_key, "FileSize", strlen("FileSize")) == 0)
    {
        ret = dlt_logstorage_read_number(&(tmp_data->file_size), filter_value);
        if (ret == 0)
            ret = DLT_OFFLINE_LOGSTORAGE_SIZE_INIT;
    }
    else if (strncmp(filter_key, "File", strlen("File")) == 0)
    {
        ret = dlt_logstorage_read_file_name(&(tmp_data->file_name), filter_value);
        if (ret == 0)
            ret = DLT_OFFLINE_LOGSTORAGE_NAME_INIT;
    }
    else if (strncmp(filter_key, "NOFiles", strlen("NOFiles")) == 0)
    {
        ret = dlt_logstorage_read_number(&(tmp_data->num_files), filter_value);
        if (ret == 0)
            ret = DLT_OFFLINE_LOGSTORAGE_NUM_INIT;
    }
    else if (strncmp(filter_key, "SyncBehavior", strlen ("SyncBehavior")) == 0)
    {
       ret = dlt_logstorage_set_sync_strategy(&(tmp_data->sync), filter_value);
       if (ret == 0)
       {
           return DLT_OFFLINE_LOGSTORAGE_SYNC_BEHAVIOR;
       }
    }
    else if (strncmp(filter_key, "EcuID", strlen("EcuID")) == 0)
    {
        ret = dlt_logstorage_set_ecuid(&(tmp_data->ecuid), filter_value);
        if (ret == 0)
        {
            return DLT_OFFLINE_LOGSTORAGE_ECUID;
        }
    }
    else
    {
        /* Invalid filter key */
        ret = -1;
        dlt_log(LOG_ERR, "Invalid filter key");
    }

    if (ret == -1)
        dlt_log(LOG_ERR, "Error in configuration file\n");

    return ret;
}

/**
 * dlt_logstorage_validate_filter_name
 *
 * Validates if the provided filter name is as required [FILTER<number>]
 *
 * @param name           Filter name
 * @return               0 on success, -1 on error
 *
 */
int dlt_logstorage_validate_filter_name(char *name)
{
    int len = 0;
    int idx = 0;

    if (name == NULL)
    {
        return -1;
    }

    len = strlen(name);

    /* Check if section header is of format "FILTER" followed by a number */
    if (strncmp(name,
                DLT_OFFLINE_LOGSTORAGE_CONFIG_SECTION,
                strlen(DLT_OFFLINE_LOGSTORAGE_CONFIG_SECTION)) == 0)
    {
        for (idx=6; idx<len-1; idx++)
        {
            if (!isdigit(name[idx]))
            {
                return -1;
            }
        }
        return 0;
    }
    return -1;
}

void dlt_logstorage_filter_set_strategy(DltLogStorageConfigData *config,
                                        int strategy)
{
    if (config == NULL)
    {
        return;
    }

    if (strategy == DLT_LOGSTORAGE_SYNC_ON_MSG) /* file based */
    {
        config->dlt_logstorage_prepare = &dlt_logstorage_prepare_on_msg;
        config->dlt_logstorage_write = &dlt_logstorage_write_on_msg;
        config->dlt_logstorage_sync = &dlt_logstorage_sync_on_msg;
    }
    else /* cache based */
    {
        config->dlt_logstorage_prepare = &dlt_logstorage_prepare_msg_cache;
        config->dlt_logstorage_write = &dlt_logstorage_write_msg_cache;
        config->dlt_logstorage_sync = &dlt_logstorage_sync_msg_cache;
    }
}

/*Return :
DLT_OFFLINE_LOGSTORAGE_FILTER_ERROR - On filter properties or value is not valid
DLT_OFFLINE_LOGSTORAGE_STORE_FILTER_ERROR - On error while storing in hash table
*/

int dlt_daemon_setup_filter_properties(DltLogStorage *handle, DltConfigFile *config_file, char *sec_name)
{
    /* Read and store filters */
    int j = 0;
    int ret = -1;
    int is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
    char *appid = NULL;
    char *ctxid = NULL;
    DltLogStorageConfigData tmp_data;
    int num_filter_keys = DLT_OFFLINE_LOGSTORAGE_MAX_KEY_NUM;
    char value[DLT_CONFIG_FILE_ENTRY_MAX_LEN + 1] = {'\0'};
    char *filter_section_key[DLT_OFFLINE_LOGSTORAGE_MAX_KEY_NUM] =
                            {
                            "LogAppName",
                            "ContextName",
                            "LogLevel",
                            "File",
                            "FileSize",
                            "NOFiles",
                            "SyncBehavior",
                            "EcuID"
                            };

    memset(&tmp_data, 0, sizeof(DltLogStorageConfigData));

    is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_PRESENT;

    for (j = 0; j < num_filter_keys; j++)
    {
        /* Get filter value for filter keys */
        ret = dlt_config_file_get_value(config_file, sec_name, filter_section_key[j], value);

        /* only return an error when the failure occurred on a mandatory
        * value. */
        if (ret != 0 &&
            strncmp(filter_section_key[j], "SyncBehavior", strlen(filter_section_key[j]))
            != 0 &&
            strncmp(filter_section_key[j], "EcuID", strlen(filter_section_key[j]))
            != 0)
        {
            is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
            ret = DLT_OFFLINE_LOGSTORAGE_FILTER_ERROR;
            dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error : Reading filter value failed\n");
            break;
        }

        /* Validate filter value */
        if (ret == 0)
        {
            ret = dlt_logstorage_validate_filter_value(filter_section_key[j], value, &appid, &ctxid, &tmp_data);
            if ((ret != -1) &&  DLT_OFFLINE_LOGSTORAGE_IS_FILTER_PRESENT(is_filter_set))
            {
                is_filter_set |= ret;
            }
            else
            {
                is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
                ret = DLT_OFFLINE_LOGSTORAGE_FILTER_ERROR;
                break;
            }
        }
        else
        {
            if (tmp_data.sync <= DLT_LOGSTORAGE_SYNC_ON_MSG)
            {
                dlt_log(LOG_INFO,
                    "Sync strategy not given. Use default ON_MSG\n");
                /* set default sync strategy */
                tmp_data.sync = DLT_LOGSTORAGE_SYNC_ON_MSG;
            }
        }
    }

    /* If all items of the filter is populated store them */
    if (DLT_OFFLINE_LOGSTORAGE_FILTER_INITIALIZED(is_filter_set))
    {
        /* depending on the specified strategy set function pointers for
         * prepare, write and sync */
        dlt_logstorage_filter_set_strategy(&tmp_data, tmp_data.sync);

        ret = dlt_logstorage_prepare_table(handle, appid, ctxid, &tmp_data);
        if (ret != 0)
        {
            dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error :  Storing filter values failed\n");
            ret = DLT_OFFLINE_LOGSTORAGE_STORE_FILTER_ERROR;
        }
    }

    if (appid != NULL)
        free(appid);
    if (ctxid != NULL)
        free(ctxid);
    if (tmp_data.file_name != NULL)
        free(tmp_data.file_name);

    if (tmp_data.ecuid != NULL)
    {
        free(tmp_data.ecuid);
    }

    return ret;
}

/**
 * dlt_logstorage_store_filters
 *
 * This function reads the filter keys and values
 * and stores them into the hash map
 *
 * @param handle             DLT Logstorage handle
 * @param config_file_name   Configuration file name
 * @return                   0 on success, -1 on error
 *
 */
int dlt_logstorage_store_filters(DltLogStorage *handle, char *config_file_name)
{
    char error_msg[DLT_DAEMON_TEXTBUFSIZE];
    DltConfigFile *config = NULL;
    int sec = 0;
    int num_sec = 0;
    int ret = 0;
    /* we have to make sure that this function returns success if atleast one
     * filter configuration is valid and stored */
    int valid = -1;

    config = dlt_config_file_init(config_file_name);
    if (config == NULL)
    {
        dlt_log(LOG_CRIT, "Failed to open filter configuration file\n");
        return -1;
    }

    dlt_config_file_get_num_sections(config, &num_sec);

    for (sec = 0; sec < num_sec; sec++)
    {
        char sec_name[DLT_CONFIG_FILE_ENTRY_MAX_LEN];

        if (dlt_config_file_get_section_name(config, sec, sec_name) == -1)
        {
            dlt_log(LOG_CRIT, "Failed to read section name\n");
            return -1;
        }

        if (strstr(sec_name, GENERAL_BASE_NAME) != NULL)
        {
                dlt_log(LOG_CRIT, "General configuration not supported \n");
                continue;
        }
        else if (dlt_logstorage_validate_filter_name(sec_name) == 0)
        {
            ret = dlt_daemon_setup_filter_properties(handle, config, sec_name);
            if (ret == DLT_OFFLINE_LOGSTORAGE_STORE_FILTER_ERROR)
            {
                break;
            }
            else if (ret == DLT_OFFLINE_LOGSTORAGE_FILTER_ERROR)
            {
                /* Continue reading next filter section */
                continue;
            }
            else
            {
                /* Filter properties read and stored successfuly */
                valid = 0;
            }
        }
        else /* unknown section */
        {
            snprintf(error_msg,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "Unknown section: %s",
                     sec_name);
            dlt_log(LOG_WARNING, error_msg);
        }
    }

    dlt_config_file_release(config);

    return valid;
}

/**
 * dlt_logstorage_load_config
 *
 * Read dlt_logstorage.conf file and setup filters in hash table
 * Hash table key consists of "APPID:CTXID", e.g "APP1:CTX1". If
 * wildcards used for application id or context id, the hash table
 * key consists of none wildcard value, e.g. appid=.*, cxtid=CTX1
 * results in "CTX1".
 *
 * Combination of two wildcards is not allowed.
 *
 * @param input_file    pointer to configuration file stored on device
 * @return              0 on success, -1 on error
 */
int dlt_logstorage_load_config(DltLogStorage *handle)
{
    char config_file_name[PATH_MAX + 1] = {'\0'};

    /* Check if handle is NULL or already initialized or already configured  */
    if ((handle == NULL) || (handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED))
        return -1;

    /* Check if this device config was already setup */
    if (handle->config_status == DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_load_config Error : Device already configured\n");
        dlt_log(LOG_ERR, "Send disconnect, connect request to re-configure \n");
        return -1;
    }

    if(snprintf(config_file_name, PATH_MAX, "%s/%s",
                                        handle->device_mount_point,
                                        DLT_OFFLINE_LOGSTORAGE_CONFIG_FILE_NAME) < 0)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_load_config Error : Device already configured\n");
        dlt_log(LOG_ERR, "Send disconnect, connect request to re-configure \n");
        return -1;
    }

    if (dlt_logstorage_hash_create(DLT_OFFLINE_LOGSTORAGE_MAXFILTERS, &(handle->config_htab)) != 0)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_load_config Error : Hash creation failed\n");
        return -1;
    }

    if (dlt_logstorage_store_filters(handle, config_file_name) != 0)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_load_config Error : Storing filters failed\n");
        return -1;
    }

    handle->config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;

    return 0;
}

/**
 * dlt_logstorage_get_loglevel_by_key
 *
 * Obtain the log level for the provided key
 * This function can be used to obtain log level when the actual
 * key stored in the Hash map is availble with the caller
 *
 * @param handle    DltLogstorage handle
 * @param key       key to search for in Hash MAP
 * @return          log level on success:, -1 on error
 */
int dlt_logstorage_get_loglevel_by_key(DltLogStorage *handle, char *key)
{
    DltLogStorageConfigData *config;

    /* Check if handle is NULL,already initialized or already configured  */
    if ((handle == NULL) || (handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
                || (handle->config_status != DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE))
    {
        return -1;
    }

    config = (DltLogStorageConfigData *)dlt_logstorage_hash_find(key, &(handle->config_htab));

    return config->log_level;
}

/**
 * dlt_logstorage_get_config
 *
 * Obtain the configuration data of all filters for provided apid and ctid
 * For a given apid and ctid, there can be 3 possiblities of configuration
 * data available in the Hash map, this function will return the address
 * of configuration data for all these 3 combinations
 *
 * @param handle        DltLogStorage handle
 * @param appid         application id
 * @param ctxid         context id
 * @param num_config    (o/p) contains the number of filter configration data obtained
 * @return              on success: address of configuration data, NULL on failure or no configuration data found
 */
DltLogStorageConfigData **dlt_logstorage_get_config(DltLogStorage *handle, char *apid, char *ctid, int *num_config)
{
    DltLogStorageConfigData **config = NULL;
    DltLogStorageConfigData *ptr_config = NULL;
    char key[3][DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN]= {{'\0'},{'\0'},{'\0'}};
    int i=0;
    int apid_len = 0;
    int ctid_len = 0;

    /* Check if handle is NULL,already initialized or already configured  */
    if ((handle == NULL) || (handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
                || (handle->config_status != DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE))
        return NULL;

    if ((apid == NULL) || (ctid == NULL))
        return NULL;

    /* Check if num_config passed is non zero */
    if (*(num_config) !=0)
       *(num_config) = 0;

    /* Prepare possible keys with apid and ctid,
       Possible combinataions are apid: , :ctid and apid:ctid */
    apid_len = strlen(apid);
    if (apid_len > DLT_ID_SIZE)
       apid_len = DLT_ID_SIZE;

    strncpy(key[0], apid, apid_len);
    strncat(key[0], ":",1);

    ctid_len = strlen(ctid);
    if (ctid_len > DLT_ID_SIZE)
       ctid_len = DLT_ID_SIZE;

    strncpy(key[1], ":", 1);
    strncat(key[1], ctid, ctid_len);

    strncpy(key[2], apid, apid_len);
    strncat(key[2], ":", 1);
    strncat(key[2], ctid, ctid_len);

    config = (DltLogStorageConfigData **)calloc(3, sizeof(DltLogStorageConfigData *));

    if (config == NULL)
        return NULL;

    /* Search the list thrice with keys as -apid: , :ctid and apid:ctid */
    for (i=0; i<3; i++)
    {
        ptr_config = (DltLogStorageConfigData *)dlt_logstorage_hash_find(key[i], &(handle->config_htab));
        if (ptr_config != NULL)
        {
            config[*(num_config)] = ptr_config;
            *(num_config) += 1;
        }
    }

    if (*(num_config) == 0)
    {
        free(config);
        return NULL;
    }

    return config;
}

/**
 * dlt_logstorage_filter
 *
 * Check if log message need to be stored in a certain device based on filter config
 * - get all DltLogStorageConfigData from hash table possible by given apid/ctid (apid:, :ctid, apid:ctid
 * - for each found structure, compare message log level with configured one
 *
 * @param appid     application id
 * @param ctxid     context id
 * @param log_level Log level of message
 * @param ecuid     EcuID given in the message
 * @param num       Number of found configurations
 * @return          list of filters received from hashmap or NULL
 */
DltLogStorageConfigData **dlt_logstorage_filter(DltLogStorage *handle,
                                                char *appid,
                                                char *ctxid,
                                                char *ecuid,
                                                int log_level,
                                                int *num)
{
    DltLogStorageConfigData **config = NULL;
    int i = 0;

    /* filter on names: find DltLogStorageConfigData structures */
    config = dlt_logstorage_get_config(handle, appid, ctxid, num);

    if (config == NULL)
    {
        *num = 0;
        return NULL;
    }

    for (i = 0; i < *num; i++)
    {
        /* filter on log level */
        if (log_level > config[i]->log_level)
        {
            config[i] = NULL;
            continue;
        }

        /* filter on ECU id only if EcuID is set */
        if (config[i]->ecuid != NULL)
        {
            if (strncmp(ecuid, config[i]->ecuid, strlen(ecuid)) != 0)
            {
                config[i] = NULL;
            }
        }
    }

    return config;
}

/**
 * dlt_logstorage_write
 *
 * Write a message to one or more configured log files, based on filter
 * configuration.
 *
 * @param handle    DltLogStorage handle
 * @param config    User configurations for log file
 * @param data1     Data buffer of message header
 * @param size1     Size of message header buffer
 * @param data2     Data buffer of message body
 * @param size2     Size of message body
 * @return          0 on success or write errors < max write errors, -1 on error
 */
int dlt_logstorage_write(DltLogStorage *handle,
                         DltLogStorageUserConfig *uconfig,
                         unsigned char *data1,
                         int size1,
                         unsigned char *data2,
                         int size2,
                         unsigned char *data3,
                         int size3)
{
    DltLogStorageConfigData **config = NULL;
    int i = 0;
    int ret = 0;
    int num = 0;
    int err = 0;
    /* data2 contains DltStandardHeader, DltStandardHeaderExtra and
     * DltExtendedHeader. We are interested in ecuid, apid, ctid and loglevel */
    DltExtendedHeader *extendedHeader;
    DltStandardHeaderExtra *extraHeader;
    DltStandardHeader *standardHeader;
    int standardHeaderExtraLen = 0;

    int log_level = -1;

    if (handle == NULL || uconfig == NULL ||
        data1 == NULL || data2 == NULL || data3 == NULL ||
        handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED ||
        handle->config_status != DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
    {
        return 0;
    }
    /* Calculate real length of DltStandardHeaderExtra */
    standardHeader = (DltStandardHeader *)data2;
    standardHeaderExtraLen = sizeof(DltStandardHeaderExtra);
    if (!DLT_IS_HTYP_WEID(standardHeader->htyp))
    {
        standardHeaderExtraLen -= DLT_ID_SIZE;
    }
    if (!DLT_IS_HTYP_WSID(standardHeader->htyp))
    {
        standardHeaderExtraLen -= DLT_SIZE_WSID;
    }
    if (!DLT_IS_HTYP_WTMS(standardHeader->htyp))
    {
        standardHeaderExtraLen -= DLT_SIZE_WTMS;
    }

    extendedHeader = (DltExtendedHeader *)(data2 +
                                           sizeof(DltStandardHeader) +
                                           standardHeaderExtraLen);
    extraHeader = (DltStandardHeaderExtra *)(data2 + sizeof(DltStandardHeader));

    log_level = DLT_GET_MSIN_MTIN(extendedHeader->msin);

    /* check if log message need to be stored in a certain device based on
     * filter configuration */
    config = dlt_logstorage_filter(handle,
                                   extendedHeader->apid,
                                   extendedHeader->ctid,
                                   extraHeader->ecu,
                                   log_level,
                                   &num);

    if (config != NULL)
    {
        /* store log message in every found filter */
        while (i < num)
        {
            if(config[i] != NULL)
            {
                /* prepare log file (create and/or open)*/
                ret = config[i]->dlt_logstorage_prepare(config[i],
                                                        uconfig,
                                                        handle->device_mount_point,
                                                        size1 + size2 + size3);
                if (ret == 0) /* log data (write) */
                {
                    ret = config[i]->dlt_logstorage_write(config[i],
                                                          data1,
                                                          size1,
                                                          data2,
                                                          size2,
                                                          data3,
                                                          size3);

                    if (ret == 0)
                    {
                        /* flush to be sure log is stored on device */
                        ret = config[i]->dlt_logstorage_sync(config[i],
                              DLT_LOGSTORAGE_SYNC_ON_MSG);
                        if (ret != 0)
                        {
                            dlt_log(LOG_ERR,
                                    "dlt_logstorage_write: Unable to sync.\n");
                        }
                    }
                    else
                    {
                        handle->write_errors += 1;
                        if (handle->write_errors >=
                            DLT_OFFLINE_LOGSTORAGE_MAX_WRITE_ERRORS)
                        {
                            err = -1;
                        }

                        dlt_log(LOG_ERR,
                                "dlt_logstorage_write: Unable to write.\n");
                    }
                }
                else
                {
                    dlt_log(LOG_ERR,
                            "dlt_logstorage_write: Unable to prepare.\n");
                }
            }
            i++;
        }

        free(config);
    }

    return err;
}

int dlt_logstorage_sync_caches(DltLogStorage *handle)
{
    int i = 0;

    if (handle == NULL)
    {
        return -1;
    }

    for (i=0; i<handle->num_filter_keys; i++)
    {
        /* sync data if necessary */
        /* ignore return value */
        if (handle->config_data[i].data.dlt_logstorage_sync(
                &(handle->config_data[i].data),
                DLT_LOGSTORAGE_SYNC_ON_DEMAND) != 0)
        {

            dlt_log(LOG_ERR,
                    "dlt_logstorage_sync_caches: Sync failed."
                    " Continue with next cache.\n");
        }
    }

    return 0;
}
