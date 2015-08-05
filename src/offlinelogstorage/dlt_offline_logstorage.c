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
#include "dlt_config_file_parser.h"


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

        dlt_logstorage_device_disconnected(handle);
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
 *
 */
void dlt_logstorage_free(DltLogStorage *handle)
{
    int i=0;

    dlt_logstorage_hash_destroy(&(handle->config_htab));

    for(i=0; i<handle->num_filter_keys; i++)
    {
        free(handle->config_data[i].data.file_name);

        if (handle->config_data[i].data.log != NULL)
                fclose(handle->config_data[i].data.log);

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
 * @return               0 on success, -1 on error
 *
 */
int dlt_logstorage_device_disconnected(DltLogStorage *handle)
{
    if (handle == NULL)
        return -1;

    /* If configuration loading was done, free it */
    if (handle->config_status == DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
    {
        dlt_logstorage_free(handle);
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
        p_node->data.records = NULL;

        if(dlt_logstorage_hash_add(p_node->key, &p_node->data, &(handle->config_htab)) != 0)
        {
            dlt_log(LOG_ERR, "Adding to hash table failed, returning failure\n");

            dlt_logstorage_free(handle);

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
    int is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
    char *appid = NULL;
    char *ctxid = NULL;
    DltLogStorageConfigData tmp_data;
    DltConfigFile *config_file = NULL;
    int num_filter_keys = DLT_OFFLINE_LOGSTORAGE_MAX_KEY_NUM;
    int num_filters = 0;
    char filter_name[DLT_CONFIG_FILE_ENTRY_MAX_LEN + 1] = {'\0'};
    char filter_value[DLT_CONFIG_FILE_ENTRY_MAX_LEN + 1] = {'\0'};
    char *filter_key[DLT_OFFLINE_LOGSTORAGE_MAX_KEY_NUM] =
                            {
                            "LogAppName",
                            "ContextName",
                            "LogLevel",
                            "File",
                            "FileSize",
                            "NOFiles"
                            };

    config_file = dlt_config_file_init(config_file_name);
    if (config_file == NULL)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error : File parser init failed\n");
        return -1;
    }

    if (dlt_config_file_get_num_sections(config_file, &num_filters) != 0)
    {
        dlt_config_file_release(config_file);
        dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error : Get number of sections failed\n");
        return -1;
    }

   /* Read and store filters */
    int i = 0;
    int j = 0;
    int ret = -1;
    memset(&tmp_data, 0, sizeof(DltLogStorageConfigData));
    for (i = 0; i < num_filters; i++)
    {
        if (tmp_data.file_name != NULL)
            free(tmp_data.file_name);

        if (appid != NULL)
        {
            free(appid);
            appid = NULL;
        }

        if (ctxid != NULL)
        {
            free(ctxid);
            ctxid = NULL;
        }

        /* Get filter name */
        ret = dlt_config_file_get_section_name(config_file, i, filter_name);
        if (ret !=0)
        {
            dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error : Reading filter name failed\n");
            break;
        }

        /* Validate filter name */
        ret = dlt_logstorage_validate_filter_name(filter_name);
        if (ret !=0)
            continue;
        else
            is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_PRESENT;

        for (j = 0; j < num_filter_keys; j++)
        {
            /* Get filter value for filter keys */
            ret = dlt_config_file_get_value(config_file, filter_name, filter_key[j], filter_value);
            if (ret != 0)
            {
                is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
                dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error : Reading filter value failed\n");
                break;
            }

            /* Validate filter value */
            ret = dlt_logstorage_validate_filter_value(filter_key[j], filter_value, &appid, &ctxid, &tmp_data);
            if ((ret != -1) &&  DLT_OFFLINE_LOGSTORAGE_IS_FILTER_PRESENT(is_filter_set))
            {
                is_filter_set |= ret;
            }
            else
            {
                is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
                break;
            }
        }
        /* If all items of the filter is populated store them */
        if (DLT_OFFLINE_LOGSTORAGE_FILTER_INITIALIZED(is_filter_set))
        {
            ret = dlt_logstorage_prepare_table(handle, appid, ctxid, &tmp_data);
            if (ret != 0)
            {
                dlt_log(LOG_ERR, "dlt_logstorage_store_filters Error :  Storing filter values failed\n");
                break;
            }
            is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
        }
    }

    if (appid != NULL)
        free(appid);
    if (ctxid != NULL)
        free(ctxid);
    if (tmp_data.file_name != NULL)
        free(tmp_data.file_name);

    dlt_config_file_release(config_file);

    return ret;
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
 * @param num       Number of found configurations
 * @return          list of filters received from hashmap or NULL
 */
DltLogStorageConfigData **dlt_logstorage_filter(DltLogStorage *handle, char *appid, char *ctxid, int log_level, int *num)
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
        }
    }

    return config;
}

/**
 * dlt_logstorage_log_file_name
 *
 * Create log file name in the form configured by the user
 *      <filename><delimeter><index><delimeter><timestamp>.dlt
 *
 *      filename:       given in configuration file
 *      delimeter:      Punctuation characters (configured in dlt.conf)
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
void dlt_logstorage_log_file_name(char *log_file_name, DltLogStorageUserConfig file_config, char *name, int idx)
{
    char file_index[10] = {0};

    // create log file name
    memset(log_file_name, 0, PATH_MAX * sizeof(char));
    strcat(log_file_name, name);
    strcat(log_file_name, &file_config.logfile_delimiter);

    sprintf(file_index,"%d",idx);
    if(file_config.logfile_maxcounter != UINT_MAX)
    {
        /* Setup 0's to be appended in file index until max index len*/
        unsigned int digit_idx = 0;
        unsigned int i = 0;
        sprintf(file_index,"%d",idx);
        digit_idx = strlen(file_index);
        for(i=0; i<(file_config.logfile_counteridxlen - digit_idx); i++) {
            strcat(log_file_name, "0");
        }
    }
    strcat(log_file_name, file_index);

    /* Add time stamp if user has configured */
    if(file_config.logfile_timestamp)
    {
        char stamp[DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + 1] = {0};
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        sprintf(stamp, "%c%04d%02d%02d-%02d%02d%02d", file_config.logfile_delimiter, 1900 + tm_info->tm_year, 1 + tm_info->tm_mon,
            tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
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

    if (*head == NULL || (*head)->next == NULL)
        return;

    while (!done)
    {
        DltLogStorageFileList **pv = head;            // "source" of the pointer to the current node in the list struct
        DltLogStorageFileList *nd = *head;            // local iterator pointer
        DltLogStorageFileList *nx = (*head)->next;  // local next pointer

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

    if (*head == NULL || (*head)->next == NULL)
        return;

    for (n = *head; n != NULL; n = n->next )
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
unsigned int dlt_logstorage_get_idx_of_log_file(DltLogStorageUserConfig file_config, char *file)
{
    unsigned int idx = -1;
    char *endptr;
    char *filename;
    unsigned int filename_len = 0 ;
    unsigned int fileindex_len = 0;

    /* Calculate actual file name length */
    filename=strchr(file,file_config.logfile_delimiter);
    filename_len = strlen(file)- strlen(filename);

    /* index is retrived from file name */
    if(file_config.logfile_timestamp)
    {
        fileindex_len = strlen(file)-(DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN
                        + DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + filename_len + 1);
        idx = (int) strtol(&file[strlen(file)-
            (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN+fileindex_len
                +DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN)],
                &endptr, 10);
    }
    else
    {
        fileindex_len = strlen(file)-(DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN  + filename_len + 1);
        idx = (int) strtol(&file[strlen(file)-
                        (DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN
                        +fileindex_len)], &endptr, 10);
    }

    if(endptr == file || idx == 0)
        dlt_log(LOG_ERR, "Unable to calculate index from log file name. Reset index to 001.\n");

    return idx;
}

/**
 * dlt_logstorage_storage_dir_info
 *
 * Read file names of storage directory.
 * Update the file list, arrange it in order of latest and oldest
 *
 * @param file_config   User configurations for log file
 * @param path      Path to storage directory
 * @param  config    DltLogStorageConfigData
 * @return         0 on success, -1 on error
 */
int dlt_logstorage_storage_dir_info(DltLogStorageUserConfig file_config, char *path, DltLogStorageConfigData *config)
{
    int i = 0;
    int cnt = 0;
    int ret = 0;
    struct dirent **files = {0};
    unsigned int current_idx = 0;

    if (path == NULL || config->file_name == NULL)
        return -1;

    cnt = scandir(path, &files, 0, alphasort);
    if (cnt < 0)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_storage_dir_info: Unable to scan storage directory.\n");
        return -1;
    }

    for (i = 0; i < cnt; i++)
    {
        int len = 0;
        len = strlen(config->file_name);
        if ((strncmp(files[i]->d_name, config->file_name, len) == 0) && (files[i]->d_name[len] == file_config.logfile_delimiter))
        {
            DltLogStorageFileList **tmp = NULL;
            current_idx = dlt_logstorage_get_idx_of_log_file(file_config, files[i]->d_name);

            if(config->records == NULL)
            {
                config->records = malloc(sizeof(DltLogStorageFileList));
                if (config->records == NULL)
                {
                    ret = -1;
                    dlt_log(LOG_ERR, "Memory allocation failure while preparing file list \n");
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
                    dlt_log(LOG_ERR, "Memory allocation failure while preparing file list \n");
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
 * Open a log file. Check storage directory for already created files and open the oldest if
 * there is enough space to store at least msg_size.
 * Otherwise create a new file, but take configured max number of files into account and remove
 * the oldest file if needed.
 *
 * @param  config    DltLogStorageConfigData
 * @param  file_config   User configurations for log file
 * @param  dev_path      Storage device path
 * @param  msg_size  Size of incoming message
 * @return 0 on succes, -1 on error
 */
int dlt_logstorage_open_log_file(DltLogStorageConfigData *config, DltLogStorageUserConfig file_config,
                                char *dev_path, int msg_size)
{
    int ret = 0;
    char absolute_file_path[PATH_MAX + 1] = {0};
    char storage_path[DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN] = {0};
    unsigned int num_log_files = 0;
    struct stat s;
    DltLogStorageFileList **tmp = NULL;
    DltLogStorageFileList **newest = NULL;
    char file_name[PATH_MAX + 1] = {0};

    if (config == NULL)
        return -1;

    sprintf(storage_path, "%s/", dev_path);

    /* check if there are already files stored */
    if (config->records == NULL)
    {
        if (dlt_logstorage_storage_dir_info(file_config, storage_path, config) != 0)
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
        dlt_logstorage_log_file_name(file_name, file_config, config->file_name, 1);

        /* concatenate path and file and open absolute path */
        strcat(absolute_file_path, storage_path);
        strcat(absolute_file_path, file_name);
        config->log = fopen(absolute_file_path, "a+");

        /* Add file to file list */
        *tmp = malloc(sizeof(DltLogStorageFileList));
        if (*tmp == NULL)
        {
            dlt_log(LOG_ERR, "Memory allocation failure while adding file name to file list \n");
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
            idx = dlt_logstorage_get_idx_of_log_file(file_config, (*newest)->name);
            idx += 1;

            /* wrap around if max index is reached or an error occured
 *             while calculating index from file name */
            if (idx > file_config.logfile_maxcounter || idx == 0)
                idx = 1;

            dlt_logstorage_log_file_name(file_name, file_config, config->file_name, idx);

            /* concatenate path and file and open absolute path */
            memset(absolute_file_path, 0, sizeof(absolute_file_path)/sizeof(char));
            strcat(absolute_file_path, storage_path);
            strcat(absolute_file_path, file_name);
            config->log = fopen(absolute_file_path, "a+");

            /* Add file to file list */
            *tmp = malloc(sizeof(DltLogStorageFileList));
            if (*tmp == NULL)
            {
                dlt_log(LOG_ERR, "Memory allocation failure while adding file name to file list \n");
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
                memset(absolute_file_path, 0, sizeof(absolute_file_path)/sizeof(char));
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
        dlt_log(LOG_ERR, "dlt_logstorage_create_log_file: Unable to open log file.\n");
        return -1;
    }

    return ret;
}

/**
 * dlt_logstorage_prepare_log_file
 *
 * Prepare the log file for a certain filer. If log file not open or log files max size reached,
 * open a new file.
 *
 * @param config        DltLogStorageConfigData
 * @param file_config   User configurations for log file
 * @param dev_path      Storage device path
 * @param log_msg_size  Size of log message
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_prepare_log_file(DltLogStorageConfigData *config,
                                    DltLogStorageUserConfig file_config,
                                    char *dev_path, int log_msg_size)
{
    int ret = 0;
    struct stat s;

    if (config == NULL)
    {
        return -1;
    }

    if (config->log == NULL) /* open a new log file */
    {
        ret = dlt_logstorage_open_log_file(config, file_config, dev_path, log_msg_size);
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
                ret = dlt_logstorage_open_log_file(config, file_config, dev_path, log_msg_size);
            }
            else /*everything is prepared */
            {
                ret = 0;
            }
        }
        else
        {
            dlt_log(LOG_ERR, "dlt_logstorage_prepare_log_file: Unable to determine file stats.\n");
            ret = -1;
        }
    }
    return ret;
}

/**
 * dlt_logstorage_write
 *
 * Write a message to one or more configured log files, based on filter configuration.
 *
 * @param handle    DltLogStorage handle
 * @param file_config   User configurations for log file
 * @param appid     Application id of sender
 * @param ctxid     Context id of sender
 * @param log_level log_level of message to store
 * @param data1     Data buffer of message header
 * @param size1     Size of message header buffer
 * @param data2     Data buffer of message body
 * @param size2     Size of message body
 * @return          0 on success or write errors < max write errors, -1 on error
 */
int dlt_logstorage_write(DltLogStorage *handle, DltLogStorageUserConfig file_config,
                        char *appid, char *ctxid, int log_level,
                        unsigned char *data1, int size1, unsigned char *data2,
                        int size2, unsigned char *data3, int size3)
{
    DltLogStorageConfigData **config = NULL;
    int i = 0;
    int ret = 0;
    int num = 0;
    int err = 0;

    if (handle == NULL || handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED || handle->config_status != DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
    {
        return 0;
    }

    /* check if log message need to be stored in a certain device based on filter config */
    config = dlt_logstorage_filter(handle, appid, ctxid, log_level, &num);

    if (config != NULL)
    {
        /* store log message in every found filter */
        while (i < num)
        {
            if(config[i] != NULL)
            {
                /* prepare logfile (create and/or open)*/
                ret = dlt_logstorage_prepare_log_file(config[i], file_config,
                                            handle->device_mount_point,
                                            size1 + size2 + size3);
                if (ret == 0) /* log data (write) */
                {
                    fwrite(data1, 1, size1, config[i]->log);
                    fwrite(data2, 1, size2, config[i]->log);
                    fwrite(data3, 1, size3, config[i]->log);
                    ret = ferror(config[i]->log);

                    if (ret != 0)
                    {
                        handle->write_errors += 1;
                        if (handle->write_errors >= DLT_OFFLINE_LOGSTORAGE_MAX_WRITE_ERRORS)
                        {
                            err = -1;
                        }

                        dlt_log(LOG_ERR, "dlt_logstorage_write: Unable to write log to file.\n");
                    }
                    else
                    {
                        /* flush to be sure log is stored on device */
                        ret = fflush(config[i]->log);
                        if (ret != 0)
                        {
                            dlt_log(LOG_ERR, "dlt_logstorage_write: Unable to flush log to file.\n");
                        }
                    }
                }
                else
                {
                    dlt_log(LOG_ERR, "dlt_logstorage_write: Unable to prepare log file.\n");
                }
            }
            i++;
        }

        free(config);
    }

    return err;
}
