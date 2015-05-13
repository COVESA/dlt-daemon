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

typedef enum _line_type_
{
    LINE_FILTER,
    LINE_APP_NAME,
    LINE_CTX_NAME,
    LINE_LOG_LEVEL,
    LINE_FILE_NAME,
    LINE_FILE_SIZE,
    LINE_FILE_NUMBER,
    LINE_ERROR,
    LINE_COMMENT
} config_line_type;

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
 * dlt_logstorage_parse_line
 *
 * Parse each line of dlt_logstorage.conf file
 *
 * @param input_line    line to be parsed
 * @param value         contains value of line after parsing
 * @return              line type
 */
static config_line_type dlt_logstorage_parse_line(const char *input_line, char *value)
{
    char line[DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE + 1];
    int len;
    int idx;
    char tmp_key[DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE];

    strncpy(line, input_line, DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE);
    line[DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE] = '\0';

    len = (int) strlen(line);

    /* Check if section header is of format "[FILTER" followed by digit and end by "]" */
    if ((strncmp(line, "[FILTER", 7) == 0) && line[len-1] == ']')
    {
        /* Check for [FILTER] */
        if (line[7] == ']')
        {
            return LINE_ERROR;
        }

        for (idx=7; idx<len-1; idx++)
        {
            if (!isdigit(line[idx]))
            {
                return LINE_ERROR;
            }
        }
        strncpy(value, line, len);
        return LINE_FILTER;
    }
    else if (sscanf (line, "%[^=] = %[^;#]", tmp_key, value) == 2)
    {
        if (strcmp(tmp_key, "LogAppName") == 0)
            return LINE_APP_NAME;
        else if (strcmp(tmp_key, "ContextName") == 0)
            return LINE_CTX_NAME;
        else if (strcmp(tmp_key, "LogLevel") == 0)
            return LINE_LOG_LEVEL;
        else if (strcmp(tmp_key, "File") == 0)
            return LINE_FILE_NAME;
        else if (strcmp(tmp_key, "FileSize") == 0)
            return LINE_FILE_SIZE;
        else if (strcmp(tmp_key, "NOFiles") == 0)
            return LINE_FILE_NUMBER;
        else
            return LINE_ERROR;
    }
    else
    {
        return LINE_ERROR;
    }
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
 * @param device_num     device number
 * @return               0 on success, -1 on error
 */
int dlt_logstorage_device_connected(DltLogStorage *handle, int device_num)
{
    if(handle == NULL)
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

    handle->device_num = device_num;
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
    handle->device_num = 0;
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
 * dlt_logstorage_read_line
 *
 * This function reads, analyzes the type of configuration line,
 * Extracts configuration line type and value and
 * stores the values into provided input arguments
 * Here appid and ctxid are used to hold application name and context name
 *  values provided in configuration file, all other configuration values
 *  are stored in tmp_data, the caller can utilize the return values of this
 *  function to know when a complete filter set is read
 *
 * @param line           Configuration line
 * @param line_number    Configuration line number used for logging error line
 * @param appid          Application ID value provided in configuration file
 * @param ctxid          Context ID value provided in configuration file
 * @param tmp_data       Holds all other configuration values
 * @return               Configuration value type on success, -1 on error
 *
 */
int dlt_logstorage_read_line(char *line, int line_number, char **appid, char **ctxid, DltLogStorageConfigData *tmp_data)
{
    char value[DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE] = {'\0'};
    int ret = -1;

    switch (dlt_logstorage_parse_line(line, value))
    {
        case LINE_FILTER:
            /* Reset all keys and values */
            if (appid != NULL)
            {
                free(*appid);
                *appid = NULL;
            }

            if (ctxid != NULL)
            {
                free(*ctxid);
                *ctxid = NULL;
            }

            ret = DLT_OFFLINE_LOGSTORAGE_FILTER_PRESENT;
            break;

        case LINE_APP_NAME:
            ret = dlt_logstorage_read_list_of_names(appid, value);
            if (ret == 0)
                ret = DLT_OFFLINE_LOGSTORAGE_APP_INIT;
            break;

        case LINE_CTX_NAME:
            ret = dlt_logstorage_read_list_of_names(ctxid, value);
            if (ret == 0)
                ret = DLT_OFFLINE_LOGSTORAGE_CTX_INIT;
            break;

        case LINE_LOG_LEVEL:
            ret = dlt_logstorage_read_log_level(&(tmp_data->log_level), value);
            if (ret == 0)
                ret = DLT_OFFLINE_LOGSTORAGE_LOG_LVL_INIT;
            break;

        case LINE_FILE_NAME:
            ret = dlt_logstorage_read_file_name(&(tmp_data->file_name), value);
            if (ret == 0)
                ret = DLT_OFFLINE_LOGSTORAGE_NAME_INIT;
            break;

        case LINE_FILE_SIZE:
            ret = dlt_logstorage_read_number(&(tmp_data->file_size), value);
            if (ret == 0)
                ret = DLT_OFFLINE_LOGSTORAGE_SIZE_INIT;
            break;

        case LINE_FILE_NUMBER:
            ret = dlt_logstorage_read_number(&(tmp_data->num_files), value);
            if (ret == 0)
                ret = DLT_OFFLINE_LOGSTORAGE_NUM_INIT;
            break;

        case LINE_ERROR:
            break;

        default:
            ret = -1;
            break;
    }
    if (ret == -1)
    {
        char error_str[DLT_DAEMON_TEXTBUFSIZE];
        snprintf(error_str,DLT_DAEMON_TEXTBUFSIZE,"Error in configuration file line number : %d  line : %s Invalid\n",line_number, line);
        dlt_log(LOG_ERR, error_str);
    }
    return ret;
}

/**
 * dlt_logstorage_trim_line
 *
 * Remove \n and spaces at end of line
 *
 * @param line           File line
 *
 */
void dlt_logstorage_trim_line(char *line)
{
    int len = 0;

    /* Consider \n and spaces at end of line */
    len = (int) strlen(line) -1;
    while ((len >= 0) && ((line[len] == '\n') || (isspace(line[len]))))
    {
       line[len]=0;
       len--;
    }
    len = (int) strlen(line) -1;
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
    FILE *config_file = NULL;
    char config_file_name[PATH_MAX + 1] = {'\0'};
    char line[DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE] = {'\0'};
    int ret = 0;
    int is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
    char *appid = NULL;
    char *ctxid = NULL;
    DltLogStorageConfigData tmp_data;
    int line_number = 0;

    memset(&tmp_data, 0, sizeof(DltLogStorageConfigData));

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


    if(snprintf(config_file_name, PATH_MAX, "%s%d/%s",
                                        DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH,
                                        handle->device_num,
                                        DLT_OFFLINE_LOGSTORAGE_CONFIG_FILE_NAME) < 0)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_load_config Error : Device already configured\n");
        dlt_log(LOG_ERR, "Send disconnect, connect request to re-configure \n");
        return -1;
    }

    if ((config_file = fopen(config_file_name, "r")) == NULL)
    {
        dlt_log(LOG_ERR, "Cannot open dlt_logstorage.conf file.\n");
        return -1;
    }

    if (dlt_logstorage_hash_create(DLT_OFFLINE_LOGSTORAGE_MAXFILTERS, &(handle->config_htab)) != 0)
    {
        fclose(config_file);
        dlt_log(LOG_ERR, "dlt_logstorage_load_config Error : Hash creation failed\n");
        return -1;
    }

    /* Read configuration file line by line */
    while (fgets(line, DLT_OFFLINE_LOGSTORAGE_MAX_LINE_SIZE, config_file) != NULL)
    {
        line_number += 1;

        /* Ignore empty line, comment line */
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\0')
            continue;

        /* Consider \n and spaces at end of line */
        dlt_logstorage_trim_line(line);

        ret = dlt_logstorage_read_line(line, line_number, &appid, &ctxid, &tmp_data);

        /* Check if filter tag was read */
        if ((ret != -1) && (ret == DLT_OFFLINE_LOGSTORAGE_FILTER_PRESENT))
        {
            is_filter_set = ret;
            if (tmp_data.file_name != NULL)
                free(tmp_data.file_name);/* not used anymore */
        }/* Check if complete filter is read */
        else if ((ret != -1) &&  DLT_OFFLINE_LOGSTORAGE_IS_FILTER_PRESENT(is_filter_set))
        {
            is_filter_set |= ret;
        }
        else
        {
            /* Do not log error here, uninitatilze filter */
            is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
        }
        /* If all items of the filter is populated store them */
        if (DLT_OFFLINE_LOGSTORAGE_FILTER_INITIALIZED(is_filter_set))
        {
            ret = dlt_logstorage_prepare_table(handle, appid, ctxid, &tmp_data);
            if (ret != 0)
                break;

            is_filter_set = DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT;
        }
    }

    if (ret == 0)
        handle->config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;

    free(appid);
    free(ctxid);
    free(tmp_data.file_name);
    fclose(config_file);

    return ret;
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
 * Create log file name in form
 *      <filename>_<index>_<timestamp>.dlt
 *
 *      filename:       given in configuration file
 *      timestamp:      yyyy-mm-dd-hh-mm-ss
 *      index:          3 digit number, beginning with 001
 *
 * @param log_file_name     contains complete logfile name
 * @param name              file name given in configuration file
 * @param idx               continous index of log files
 * @ return                 0 on success, -1 on error
 */
void dlt_logstorage_log_file_name(char *log_file_name, char *name, int idx)
{
    char file_index[4] = {0};
    char stamp[DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + 1] = {0};

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    sprintf(stamp, "_%04d%02d%02d-%02d%02d%02d", 1900 + tm_info->tm_year, 1 + tm_info->tm_mon,
        tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

    sprintf(file_index, "%03d", idx);

    // create log file name
    memset(log_file_name, 0, DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN * sizeof(char));
    strcat(log_file_name, name);
    strcat(log_file_name, "_");
    strcat(log_file_name, file_index);
    strcat(log_file_name, stamp);
    strcat(log_file_name, ".dlt");
}

/**
 * dlt_logstorage_storage_dir_info
 *
 * Get information of storage directory. Return newest, oldest log and number of log files
 *
 * @param path      Path to storage directory
 * @param file_name Base name of log files
 * @param oldest    Oldest log file
 * @param newest    Newest log file
 * @return          number of logfiles on success, -1 on error
 */
int dlt_logstorage_storage_dir_info(char *path, char *file_name, char *newest, char *oldest)
{
    int i = 0;
    int num = 0;
    int cnt = 0;
    struct dirent **files = {0};
    char *tmp_old = NULL;
    char *tmp_new = NULL;

    if (path == NULL || file_name == NULL || newest == NULL || oldest == NULL)
    {
        return -1;
    }

    cnt = scandir(path, &files, 0, alphasort);

    if (cnt < 0)
    {
        return -1;
    }

    for (i = 0; i < cnt; i++)
    {
        int len = 0;
        len = strlen(file_name);
        if ((strncmp(files[i]->d_name, file_name, len) == 0) && (files[i]->d_name[len] == '_'))
        {
            num++;
            if (tmp_old == NULL || strcmp(tmp_old, files[i]->d_name) > 0)
            {
                tmp_old = files[i]->d_name;
            }

            if (tmp_new == NULL || strcmp(tmp_new, files[i]->d_name) < 0)
            {
                tmp_new = files[i]->d_name;
            }
        }
    }

    if (num > 0)
    {
        if (tmp_old != NULL)
        {
            if (strlen(tmp_old) < DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN)
            {
                strncpy(oldest, tmp_old, DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN);
            }
            else
            {
                dlt_log(LOG_ERR, "Found log file name too long. New file will be created.");
            }
        }
        if (tmp_new != NULL)
        {
            if (strlen(tmp_old) < DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN)
            {
                strncpy(newest, tmp_new, DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN);
            }
            else
            {
                dlt_log(LOG_ERR, "Found log file name too long. New file will be created.");
            }
        }
    }

    /* free scandir result */
    for (i = 0; i < cnt; i++)
    {
        free(files[i]);
    }
    free(files);

    return num;
}


/**
 * dlt_logstorage_get_idx_of_log_file
 *
 * Generate an index of the log file to be used.
 *
 * @param file  file name to extract the index from
 * @return index on success, -1 if no index is found
 */
int dlt_logstorage_get_idx_of_log_file(char *file)
{
    int idx = -1;
    char *endptr;

    /* index is retrived from file name */
    idx = (int) strtol(&file[strlen(file)-DLT_OFFLINE_LOGSTORAGE_INDEX_OFFSET], &endptr, 10) + 1;

    if(endptr == file || idx <= 0)
    {
        dlt_log(LOG_ERR, "Unable to calculate index from log file name. Reset index to 001.\n");
        idx = -1;
    }

    return idx;
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
 * @param  dev_num   Number of storage device needed for storage dir path
 * @param  msg_size  Size of incoming message
 * @return 0 on succes, -1 on error
 */
int dlt_logstorage_open_log_file(DltLogStorageConfigData *config, int dev_num, int msg_size)
{
    int ret = 0;
    int idx = 0;
    char absolute_file_path[DLT_OFFLINE_LOGSTORAGE_MAX_PATH_LEN] = {0};
    char storage_path[DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN] = {0};
    char newest[DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN] = {0};
    char oldest[DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN] = {0};
    unsigned int num_log_files = 0;
    struct stat s;

    if (config == NULL)
    {
        return -1;
    }

    /* check if there are already files */
    sprintf(storage_path, "%s%d/", DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH, dev_num);

    ret = dlt_logstorage_storage_dir_info(storage_path, config->file_name,
            newest, oldest);

    if (ret == -1)
    {
        dlt_log(LOG_ERR, "dlt_logstorage_create_log_file: Unable to scan storage directory.\n");
        return -1;
    }

    /* positive number in case of no error */
    num_log_files = (unsigned int) ret;

    if (strlen(newest) == 0) /* need new file*/
    {
        idx += 1;

        dlt_logstorage_log_file_name(newest, config->file_name, idx);

        /* concatenate path and file and open absolute path */
        strcat(absolute_file_path, storage_path);
        strcat(absolute_file_path, newest);
        config->log = fopen(absolute_file_path, "a+");

    }
    else /* newest file available*/
    {
        strcat(absolute_file_path, storage_path);
        strcat(absolute_file_path, newest);

        ret = stat(absolute_file_path, &s);

        /* if size is enough, open it */
        if (ret == 0 && s.st_size + msg_size < config->file_size)
        {
            config->log = fopen(absolute_file_path, "a+");
        }
        else /* no space in file or file stats cannot be read */
        {
            /* get index of newest log file */
            idx = dlt_logstorage_get_idx_of_log_file(newest);

            /* wrap around if max index is reached or an error occured while calculating index from file name */
            if (idx >= DLT_OFFLINE_LOGSTORAGE_MAX_INDEX || idx < 0)
            {
                idx = 1;
            }

            dlt_logstorage_log_file_name(newest, config->file_name, idx);

            /* concatenate path and file and open absolute path */
            memset(absolute_file_path, 0, sizeof(absolute_file_path)/sizeof(char));
            strcat(absolute_file_path, storage_path);
            strcat(absolute_file_path, newest);
            config->log = fopen(absolute_file_path, "a+");

            num_log_files += 1;
            /* check if number of log files exceeds configured max value */
            if (num_log_files > config->num_files)
            {
                /* delete oldest */
                memset(absolute_file_path, 0, sizeof(absolute_file_path)/sizeof(char));
                strcat(absolute_file_path, storage_path);
                strcat(absolute_file_path, oldest);
                remove(absolute_file_path);
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
 * @param dev_num       Numer of storage device
 * @param log_msg_size  Size of log message
 * @return 0 on success, -1 on error
 */
int dlt_logstorage_prepare_log_file(DltLogStorageConfigData *config, int dev_num, int log_msg_size)
{
    int ret = 0;
    struct stat s;

    if (config == NULL)
    {
        return -1;
    }

    if (config->log == NULL) /* open a new log file */
    {
        ret = dlt_logstorage_open_log_file(config, dev_num, log_msg_size);
    }
    else /* already open, check size and create a new file if needed */
    {
        ret = fstat(fileno(config->log), &s);
        if (ret == 0) {
            /* check if adding new data do not exceed max file size */
            if (s.st_size + log_msg_size >= config->file_size)
            {
                fclose(config->log);
                config->log = NULL;
                ret = dlt_logstorage_open_log_file(config, dev_num, log_msg_size);
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
 * @param appid     Application id of sender
 * @param ctxid     Context id of sender
 * @param log_level log_level of message to store
 * @param data1     Data buffer of message header
 * @param size1     Size of message header buffer
 * @param data2     Data buffer of message body
 * @param size2     Size of message body
 * @return          0 on success or write errors < max write errors, -1 on error
 */
int dlt_logstorage_write(DltLogStorage *handle, char *appid, char *ctxid, int log_level, unsigned char *data1, int size1, unsigned char *data2, int size2, unsigned char *data3, int size3)
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
                ret = dlt_logstorage_prepare_log_file(config[i], handle->device_num, size1 + size2 + size3);
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
