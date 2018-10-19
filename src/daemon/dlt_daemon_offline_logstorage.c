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
* \file: dlt_daemon_offline_logstorage.c
* For further information see http://www.genivi.org/.
* @licence end@
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "dlt_daemon_offline_logstorage.h"


/**
 * dlt_logstorage_split_key
*
 * Split a given key into appid and ctxid.
 * If APID: - appid = APID and ctxid = .*
 * If :CTID - ctxid = CTID and appid = .*
 * Else appid = APID and ctxid = CTID
 *
 * @param key      Given key of filter hash map
 * @param appid    Application id
 * @param ctxid    Context id
 * @return         0 on success, -1 on error
 */
int dlt_logstorage_split_key(char *key, char *appid, char *ctxid)
{
    char *tok = NULL;
    int len = 0;
    int ret = 0;
    char *sep = NULL;

    if (key == NULL)
    {
        return -1;
    }

    len = strlen(key);

    sep = strchr (key, ':');
    if(sep == NULL)
    {
        return -1;
    }

    /* key is context id only */
    if(key[0] == ':')
    {
        if(len > (DLT_ID_SIZE+1))
            return -1;

        strncpy(ctxid,(key+1),(len-1));
        strncpy(appid, ".*",2);
    }
    /* key is application id only */
    else if (key[len-1] == ':')
    {
        if(len > (DLT_ID_SIZE+1))
            return -1;

        strncpy(appid,key,(len-1));
        strncpy(ctxid, ".*",2);
    }
    /* key is appid:ctxid */
    else
    {
        if(len > DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN)
            return -1;

        /* copy appid and ctxid */
        tok = strtok(key, ":");
        if (tok != NULL)
            strncpy(appid,tok,DLT_ID_SIZE);
        else
            ret = -1;

        tok = strtok(NULL, ":");
        if (tok != NULL)
            strncpy(ctxid,tok,DLT_ID_SIZE);
        else
            ret = -1;
    }

    return ret;
}

/**
 * dlt_logstorage_update_all_contexts
 *
 * Update log level of all contexts of the application by updating the daemon internal table
 * The compare flags (cmp_flag) indicates if Id has to be compared with application ID
 * or Context id of the daemon internal table
 * The log levels are reset if current log level provided is -1 (not sent to application in this case)
 * Reset and sent to application if current log level provided is 0
 *
 * @param daemon            DltDaemon structure
 * @param id                application id or context id
 * @param curr_log_level    log level to be set to context
 * @param cmp_flag          compare flag (1 id is apid, 2 id is ctxid)
 * @param verbose           If set to true verbose information is printed out
 * @return                  0 on success, -1 on error
 */
int dlt_logstorage_update_all_contexts(DltDaemon *daemon, char *id, int curr_log_level, int cmp_flag, int verbose)
{
    int i = 0;
    char tmp_id[DLT_ID_SIZE+1];
    int old_log_level = -1;
    DltDaemonRegisteredUsers* user_list = NULL;

    if((daemon == 0) || (id == NULL))
        return -1;

    if((cmp_flag < 0 ) || (cmp_flag > 2 ))
        return -1;

    memset(tmp_id, 0, sizeof(tmp_id));

    user_list = dlt_daemon_find_users_list(daemon, daemon->ecuid, verbose);
    if (user_list == NULL)
    {
        return -1;
    }

    for (i = 0 ; i < user_list->num_contexts ; i++)
    {
        if (cmp_flag == 1)
        {
            dlt_set_id(tmp_id, user_list->contexts[i].apid);
        }
        else
        {
            dlt_set_id(tmp_id, user_list->contexts[i].ctid);
        }

        if(strcmp(id, tmp_id) == 0)
        {
            if(curr_log_level > 0)
            {
                old_log_level = user_list->contexts[i].storage_log_level;

                user_list->contexts[i].storage_log_level =
                    DLT_OFFLINE_LOGSTORAGE_MAX(
                        curr_log_level,
                        user_list->contexts[i].storage_log_level);

                if (user_list->contexts[i].storage_log_level > old_log_level)
                {
                    if (dlt_daemon_user_send_log_level(daemon,
                                                       &user_list->contexts[i],
                                                       verbose) == -1)
                    {
                        dlt_log(LOG_ERR, "Unable to update loglevel\n");
                        return -1;
                    }
                }
            }
            else    /* The request is to reset log levels */
            {
                /* Set storage level to -1, to clear log levels */
                user_list->contexts[i].storage_log_level = -1;

                if(curr_log_level == DLT_DAEMON_LOGSTORAGE_RESET_SEND_LOGLEVEL)
                {
                    if (dlt_daemon_user_send_log_level(daemon,
                                                       &user_list->contexts[i],
                                                       verbose) == -1)
                    {
                        dlt_log(LOG_ERR, "Unable to reset loglevel\n");
                        return -1;
                    }
                }

            }
        }
    }
    return 0;
}

/**
 * dlt_logstorage_update_context
 *
 * Update log level of a context by updating the daemon internal table
 * The log levels are reset if current log level provided is -1 (not sent to application in this case)
 * Reset and sent to application if current log level provided is 0
 *
 * @param daemon            DltDaemon structure
 * @param apid              application id
 * @param ctxid             context id
 * @param curr_log_level    log level to be set to context
 * @param verbose           If set to true verbose information is printed out
 * @return                  0 on success, -1 on error
 */
int dlt_logstorage_update_context(DltDaemon *daemon, char *apid, char *ctxid, int curr_log_level , int verbose)
{
    DltDaemonContext *context = NULL;
    int old_log_level = -1;

    if((daemon == 0) || (apid == NULL) || (ctxid == NULL))
        return -1;

    context = dlt_daemon_context_find(daemon, apid, ctxid, daemon->ecuid, verbose);

    if (context != NULL)
    {
        if(curr_log_level > 0)
        {
            old_log_level = context->storage_log_level;

            context->storage_log_level  = DLT_OFFLINE_LOGSTORAGE_MAX(curr_log_level, context->storage_log_level);
            if(context->storage_log_level > old_log_level)
            {
                if(dlt_daemon_user_send_log_level(daemon, context, verbose) == -1)
                {
                    dlt_log(LOG_ERR, "Unable to update loglevel\n");
                    return -1;
                }
            }
        }
        else
        {
            context->storage_log_level = -1;

            if(curr_log_level == DLT_DAEMON_LOGSTORAGE_RESET_SEND_LOGLEVEL)
            {
                if(dlt_daemon_user_send_log_level(daemon, context, verbose) == -1)
                {
                    dlt_log(LOG_ERR, "Unable to update loglevel\n");
                    return -1;
                }
            }

        }
    }
    return 0;
}

/**
 * dlt_logstorage_update_context_loglevel
 *
 * Update all contexts or particular context depending provided key
 *
 * @param daemon            Pointer to DLT Daemon structure
 * @param key               Filter key stored in Hash Map
 * @param curr_log_level    log level to be set to context
 * @param verbose           If set to true verbose information is printed out
 * @return                  0 on success, -1 on error
 */
int dlt_logstorage_update_context_loglevel(DltDaemon *daemon, char *key, int curr_log_level , int verbose)
{

    int cmp_flag=0;
    char appid[DLT_ID_SIZE+1] = {'\0'};
    char ctxid[DLT_ID_SIZE+1] = {'\0'};

    PRINT_FUNCTION_VERBOSE(verbose);

    if((daemon == 0) || (key == NULL))
        return -1;

    memset(appid, 0, sizeof(appid));
    memset(ctxid, 0, sizeof(ctxid));

    if(dlt_logstorage_split_key(key, appid, ctxid) != 0)
    {
        dlt_log(LOG_ERR, "Error while updating application log levels (splt key)\n");
        return -1;
    }

    if(strcmp(ctxid, ".*") == 0) /* wildcard for context id, find all contexts of given application id */
    {
        cmp_flag = 1;

        if(dlt_logstorage_update_all_contexts(daemon, appid, curr_log_level, cmp_flag, verbose) != 0)
            return -1;
    }
    else if(strcmp(appid, ".*") == 0) /* wildcard for application id, find all contexts with context id */
    {
        cmp_flag = 2;

        if(dlt_logstorage_update_all_contexts(daemon, ctxid, curr_log_level, cmp_flag, verbose) != 0)
            return -1;
    }
    else /* In case of given application id, context id pair, call available context find function */
    {
         if(dlt_logstorage_update_context(daemon, appid, ctxid, curr_log_level, verbose) != 0)
            return -1;
    }
    return 0;
}

/**
 * dlt_daemon_logstorage_reset_application_loglevel
 *
 * Reset storage log level of all running applications
 * 2 steps for resetting
 * 1. Setup storage_loglevel of all contexts configured for the requested device to -1
 * 2. Re-run update log level for all other configured devices
 *
 * @param daemon        Pointer to DLT Daemon structure
 * @param dev_num       Number of attached DLT Logstorage device
 * @param max_device    Maximum storage devices setup by the daemon
 * @param verbose       If set to true verbose information is printed out
 */
void dlt_daemon_logstorage_reset_application_loglevel(DltDaemon *daemon, int dev_num, int max_device, int verbose)
{
    DltLogStorage *handle = NULL;
    int i = 0;
    char key[DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN] = {'\0'};
    int num_device_configured = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if((daemon == 0) || (dev_num < 0))
    {
        dlt_log(LOG_ERR, "Invalid function parameters used for dlt_daemon_logstorage_reset_application_loglevel\n");
        return;
    }

    handle = &(daemon->storage_handle[dev_num]);
    if ((handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
            || (handle->config_status != DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE))
    {
        return;
    }

    /* First, check number of devices configured */
    for(i = 0; i<max_device; i++)
    {
        if(daemon->storage_handle[i].config_status == DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
        {
            num_device_configured += 1;
        }
    }

    /* for all filters (keys) check if application context are already running and log level need to be reset*/
    for(i = 0; i < handle->num_filter_keys; i++)
    {
        memset(key, 0, sizeof(key));

        strncpy(key, (handle->filter_keys + i * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN), DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN);

        if(num_device_configured == 1)
            /* Reset context log level  and send to application */
            dlt_logstorage_update_context_loglevel(daemon, key, DLT_DAEMON_LOGSTORAGE_RESET_SEND_LOGLEVEL, verbose);
        else
            /* Reset context log level  do not send to application as other devices can have same configuration */
            dlt_logstorage_update_context_loglevel(daemon, key, DLT_DAEMON_LOGSTORAGE_RESET_LOGLEVEL, verbose);
    }

    /* Re-run update log level for all other configured devices */
    for(i=0; i<max_device; i++)
    {
        if(i == dev_num)
            continue;

        if (daemon->storage_handle[i].config_status == DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
        {
            dlt_daemon_logstorage_update_application_loglevel(daemon, i, verbose);
        }
    }
    return;
}

/**
 * dlt_daemon_logstorage_update_application_loglevel
 *
 * Update log level of all running applications with new filter configuration available due
 * to newly attached DltLogstorage device. The log level is only updated when the current
 * application log level is less than the log level obtained from the storage configuration file
 *
 * @param daemon        Pointer to DLT Daemon structure
 * @param dev_num       Number of attached DLT Logstorage device
 * @param verbose       If set to true verbose information is printed out
 */
void dlt_daemon_logstorage_update_application_loglevel(DltDaemon *daemon, int dev_num, int verbose)
{
    DltLogStorage *handle = NULL;
    int i = 0;
    char key[DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN] = {'\0'};


    PRINT_FUNCTION_VERBOSE(verbose);

    if((daemon == 0) || (dev_num < 0))
    {
        dlt_log(LOG_ERR, "Invalid function parameters used for dlt_daemon_logstorage_update_application_loglevel\n");
        return;
    }

    handle = &(daemon->storage_handle[dev_num]);
    if ((handle->connection_type != DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
            || (handle->config_status != DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE))
    {
        return;
    }

    /* for all filters (keys) check if application or context already running and log level need to be updated*/
    for(i = 0; i < handle->num_filter_keys; i++)
    {
        int log_level = -1;

        memset(key, 0, sizeof(key));

        strncpy(key, (handle->filter_keys + i * DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN), DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN);

        /* Obtain storage configuration data */
        log_level = dlt_logstorage_get_loglevel_by_key(handle, key);
        if(log_level < 0)
        {
            dlt_log(LOG_ERR, "Failed to get log level by key \n");
           return;
        }

        /* Update context log level with storage configuration log level */
        dlt_logstorage_update_context_loglevel(daemon, key, log_level, verbose);
    }
    return;
}

/**
 * dlt_daemon_logstorage_get_loglevel
 *
 * Obtain log level as a union of all configured storage devices and filters for the
 * provided application id and context id
 *
 * @param daemon        Pointer to DLT Daemon structure
 * @param max_device    Maximum storage devices setup by the daemon
 * @param apid          Application ID
 * @param ctid          Context ID
 * @return              Log level on success, -1 on error
 */
int dlt_daemon_logstorage_get_loglevel(DltDaemon *daemon, int max_device, char *apid, char *ctid)
{
    DltLogStorageConfigData **config = NULL;
    int i = 0;
    int j = 0;
    int8_t storage_loglevel = -1;
    int8_t retrvd_loglevel = -1;
    int num_config = 0;

    if((daemon == 0) || (max_device == 0) || (apid == NULL) || (ctid == NULL))
        return -1;

    for(i = 0; i<max_device; i++)
    {
        if (daemon->storage_handle[i].config_status == DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
        {
            config = dlt_logstorage_get_config(&(daemon->storage_handle[i]), apid, ctid, &num_config);
            if(config != NULL)
            {
                for(j = 0; j<num_config; j++)
                {
                    retrvd_loglevel = config[j]->log_level;
                    storage_loglevel = DLT_OFFLINE_LOGSTORAGE_MAX(retrvd_loglevel, storage_loglevel);
                }
                free(config);
            }
        }
    }
    return storage_loglevel;
}

/**
 * dlt_daemon_logstorage_write
 *
 * Write log message to all attached storage device. If the called dlt_logstorage_write function is not able
 * to write to the device, DltDaemon will disconnect this device.
 *
 * @param daemon        Pointer to Dlt Daemon structure
 * @param user_config   DltDaemon configuration
 * @param data1         message header buffer
 * @param size1         message header buffer size
 * @param data2         message extended header buffer
 * @param size2         message extended header size
 * @param data3         message data buffer
 * @param size3         message data size
 */
void dlt_daemon_logstorage_write(DltDaemon *daemon,
                                 DltDaemonFlags *user_config,
                                 unsigned char *data1,
                                 int size1,
                                 unsigned char *data2,
                                 int size2,
                                 unsigned char *data3,
                                 int size3)
{
    int i = 0;
    DltLogStorageUserConfig file_config;

    if (daemon == NULL || (user_config->offlineLogstorageMaxDevices <= 0)
        || data1 == NULL || data2 == NULL || data3 == NULL)
    {
        dlt_log(LOG_INFO,
                "dlt_daemon_logstorage_write: message type is not log. "
                "Skip storing.\n");
        return;
    }

    /* Copy user configuration */
    file_config.logfile_timestamp = user_config->offlineLogstorageTimestamp;
    file_config.logfile_delimiter = user_config->offlineLogstorageDelimiter;
    file_config.logfile_maxcounter = user_config->offlineLogstorageMaxCounter;
    file_config.logfile_counteridxlen =
            user_config->offlineLogstorageMaxCounterIdx;

    for (i = 0; i < user_config->offlineLogstorageMaxDevices; i++)
    {
        if (daemon->storage_handle[i].config_status ==
            DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE)
        {
            if (dlt_logstorage_write(&(daemon->storage_handle[i]),
                                     &file_config,
                                     data1,
                                     size1,
                                     data2,
                                     size2,
                                     data3,
                                     size3) != 0)
            {
                dlt_log(LOG_ERR,
                        "dlt_daemon_logstorage_write: failed. "
                        "Disable storage device\n");
                /* DLT_OFFLINE_LOGSTORAGE_MAX_WRITE_ERRORS happened,
                 * therefore remove logstorage device */
                dlt_logstorage_device_disconnected(
                        &(daemon->storage_handle[i]),
                        DLT_LOGSTORAGE_SYNC_ON_DEVICE_DISCONNECT);
            }
        }
    }
}

/**
 * dlt_daemon_logstorage_setup_internal_storage
 *
 * Setup user defined path as offline log storage device
 *
 * @param daemon        Pointer to Dlt Daemon structure
 * @param path          User configured internal storage path
 * @param verbose       If set to true verbose information is printed out
 */
int dlt_daemon_logstorage_setup_internal_storage(DltDaemon *daemon, char *path, int verbose)
{
    int ret = 0;

    if((path == NULL) || (daemon == NULL))
    {
       return -1;
    }

    /* connect internal storage device */
    /* Device index always used as 0 as it is setup on DLT daemon startup */
    ret = dlt_logstorage_device_connected(&(daemon->storage_handle[0]), path);
    if(ret != 0)
    {
        dlt_log(LOG_ERR,"dlt_daemon_logstorage_setup_emmc_support : Device connect failed\n");
        return -1;
    }

    /* setup logstorage with config file settings */
    ret = dlt_logstorage_load_config(&(daemon->storage_handle[0]));
    if(ret != 0)
    {
        dlt_log(LOG_ERR,"dlt_daemon_logstorage_setup_emmc_support : Loading configuration file failed\n");
        return -1;
    }

    /* check if log level of running application need an update */
    dlt_daemon_logstorage_update_application_loglevel(daemon, 0, verbose);

    return ret;
}

void dlt_daemon_logstorage_set_logstorage_cache_size(unsigned int size)
{
    /* store given [KB] size in [Bytes] */
    g_logstorage_cache_max = size * 1000;
}

int dlt_daemon_logstorage_cleanup(DltDaemon *daemon,
                                  DltDaemonLocal *daemon_local,
                                  int verbose)
{
    int i = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL || daemon_local == NULL)
    {
        return -1;
    }

    for (i = 0; i < daemon_local->flags.offlineLogstorageMaxDevices; i++)
    {
        /* call disconnect on all currently connected devices */
        if (daemon->storage_handle[i].connection_type ==
            DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
        {
            dlt_logstorage_device_disconnected(
                    &daemon->storage_handle[i],
                    DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT);
        }
    }

    return 0;
}

int dlt_daemon_logstorage_sync_cache(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     char *mnt_point,
                                     int verbose)
{
    int i = 0;
    DltLogStorage *handle = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL || mnt_point == NULL)
    {
        return -1;
    }

    if (strlen(mnt_point) > 0) /* mount point is given */
    {
        handle = dlt_daemon_logstorage_get_device(daemon,
                                                  daemon_local,
                                                  mnt_point,
                                                  verbose);
        if (handle == NULL)
        {
            return -1;
        }
        else
        {
            if (dlt_logstorage_sync_caches(handle) != 0)
            {
                return -1;
            }
        }
    }
    else /* sync caches for all connected logstorage devices */
    {
        for(i=0; i < daemon_local->flags.offlineLogstorageMaxDevices; i++)
        {
            if (daemon->storage_handle[i].connection_type ==
                    DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
            {
                if (dlt_logstorage_sync_caches(&daemon->storage_handle[i]) != 0)
                {
                    return -1;
                }
            }
        }
    }

    return 0;
}

DltLogStorage *dlt_daemon_logstorage_get_device(DltDaemon *daemon,
                                                DltDaemonLocal *daemon_local,
                                                char *mnt_point,
                                                int verbose)
{
    int i = 0;
    int len1 = 0;
    int len2 = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL || daemon_local == NULL || mnt_point == NULL)
    {
        return NULL;
    }

    len1 = strlen(mnt_point);

    for(i = 0; i < daemon_local->flags.offlineLogstorageMaxDevices; i++)
    {
        len2 = strlen(daemon->storage_handle[i].device_mount_point);

        /* Check if the requested device path is already used as log storage
         * device. Check for strlen first, to avoid comparison errors when
         * final '/' is given or not */
        if (strncmp(daemon->storage_handle[i].device_mount_point,
                    mnt_point,
                    len1 > len2 ? len2 : len1) == 0)
        {
            return &daemon->storage_handle[i];
        }
    }

    return NULL;
}
