/**
 * @licence app begin@
 * Copyright (C) 2013 - 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT offline log storage functionality header file.
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
 * \file: dlt_offline_logstorage.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_logstorage.h                                      **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Syed Hameed shameed@jp.adit-jv.com                            **
**              Christoph Lipka clipka@jp.adit-jv.com                         **
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
**  sh          Syed Hameed                ADIT                               **
**  cl          Christoph Lipka            ADIT                               **
*******************************************************************************/

#ifndef DLT_OFFLINE_LOGSTORAGE_H
#define DLT_OFFLINE_LOGSTORAGE_H

#include <search.h>
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"

#define DLT_OFFLINE_LOGSTORAGE_MAXFILTERS          100  /* Maximum entries in hashmap */

#define DLT_OFFLINE_LOGSTORAGE_INIT_DONE           1  /* For device configuration status */
#define DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED    1
#define DLT_OFFLINE_LOGSTORAGE_FREE                0
#define DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED 0
#define DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE         1

#define DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN         10  /* Maximum size for key */
#define DLT_OFFLINE_LOGSTORAGE_MAX_FILE_NAME_LEN   50  /* Maximum file name length of the log file */

#define DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN   4
#define DLT_OFFLINE_LOGSTORAGE_INDEX_LEN            3
#define DLT_OFFLINE_LOGSTORAGE_MAX_INDEX          999
#define DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN       16
#define DLT_OFFLINE_LOGSTORAGE_INDEX_OFFSET        (DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_INDEX_LEN)
#define DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN    (DLT_OFFLINE_LOGSTORAGE_MAX_FILE_NAME_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_INDEX_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN + 1)

#define DLT_OFFLINE_LOGSTORAGE_FILTER_UNINIT       0
#define DLT_OFFLINE_LOGSTORAGE_FILTER_PRESENT      (1<<7)
#define DLT_OFFLINE_LOGSTORAGE_APP_INIT            (1<<6)
#define DLT_OFFLINE_LOGSTORAGE_CTX_INIT            (1<<5)
#define DLT_OFFLINE_LOGSTORAGE_LOG_LVL_INIT        (1<<4)
#define DLT_OFFLINE_LOGSTORAGE_NAME_INIT           (1<<3)
#define DLT_OFFLINE_LOGSTORAGE_SIZE_INIT           (1<<2)
#define DLT_OFFLINE_LOGSTORAGE_SYNC_BEHAVIOR       (1<<1)
#define DLT_OFFLINE_LOGSTORAGE_NUM_INIT            1
/* Sync behavior is optional */
#define DLT_OFFLINE_LOGSTORAGE_FILTER_INIT         0xFD

#define DLT_OFFLINE_LOGSTORAGE_FILTER_INITIALIZED(A) ((A) >= DLT_OFFLINE_LOGSTORAGE_FILTER_INIT)

#define DLT_OFFLINE_LOGSTORAGE_IS_FILTER_PRESENT(A) ((A) & DLT_OFFLINE_LOGSTORAGE_FILTER_PRESENT)

#define DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN 50
#define DLT_OFFLINE_LOGSTORAGE_CONFIG_FILE_NAME    "dlt_logstorage.conf"

/* +3 because of device number and \0 */
#define DLT_OFFLINE_LOGSTORAGE_MAX_PATH_LEN (DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN + \
                                             DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN + 3)

#define DLT_OFFLINE_LOGSTORAGE_MAX(A, B)   ((A) > (B) ? (A) : (B))
#define DLT_OFFLINE_LOGSTORAGE_MIN(A, B)   ((A) < (B) ? (A) : (B))

#define DLT_OFFLINE_LOGSTORAGE_MAX_WRITE_ERRORS     5
#define DLT_OFFLINE_LOGSTORAGE_MAX_KEY_NUM          7

#define DLT_OFFLINE_LOGSTORAGE_CONFIG_SECTION "FILTER"

/* Offline Logstorage sync strategies */
#define DLT_LOGSTORAGE_SYNC_ON_MSG           0x00 /* default, on message sync */
#define DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT   0x01 /* sync on daemon exit */


/* logstorage max cache */
unsigned int g_logstorage_cache_max;
/* current logstorage cache size */
unsigned int g_logstorage_cache_size;

typedef struct
{
    int offset;                   /* current write offset */
    unsigned int wrap_around_cnt; /* wrap around counter */
}DltLogStorageCacheFooter;

typedef struct
{
    /* File name user configurations */
    int logfile_timestamp;              /* Timestamp set/reset */
    char logfile_delimiter;             /* Choice of delimiter */
    unsigned int logfile_maxcounter;    /* Maximum file index counter */
    unsigned int logfile_counteridxlen; /* File index counter length */
}DltLogStorageUserConfig;

typedef struct DltLogStorageFileList
{
    /* List for filenames */
    char *name;                         /* Filename */
    unsigned int idx;                   /* File index */
    struct DltLogStorageFileList *next;
}DltLogStorageFileList;

typedef struct DltLogStorageConfigData DltLogStorageConfigData;

typedef struct DltLogStorageConfigData
{
    /* filter section */
    int log_level;                  /* Log level number configured for filter */
    char *file_name;                /* File name for log storage configured for filter */
    unsigned int file_size;         /* MAX File size of storage file configured for filter */
    unsigned int num_files;         /* MAX number of storage files configured for filters */
    int sync;                       /* Sync strategy */
    /* callback function for filter configurations */
    int (*dlt_logstorage_prepare)(DltLogStorageConfigData *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int log_msg_size);
    int (*dlt_logstorage_write)(DltLogStorageConfigData *config,
                                unsigned char *data1,
                                int size1,
                                unsigned char *data2,
                                int size2,
                                unsigned char *data3,
                                int size3);
    /* status is strategy, e.g. DLT_LOGSTORAGE_SYNC_ON_MSG is used when callback
     * is called on message received */
    int (*dlt_logstorage_sync)(DltLogStorageConfigData *config, int status);
    FILE *log;                      /* current open log file */
    void *cache;                    /* log data cache */
    DltLogStorageFileList *records; /* File name list */
}DltLogStorageConfigData;


typedef struct
{
    char key[DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN]; /* Keys stored in hash table */
    DltLogStorageConfigData data;               /* Data stored in hash table */
}DltLogStorageConfig;


typedef struct
{
    struct hsearch_data config_htab;    /* Hash table object declaration used by hsearch_r()*/
    DltLogStorageConfig *config_data; /* Configuration data  */
    char *filter_keys;                 /* List of all keys stored in config_htab */
    int num_filter_keys;                /* Number of keys */
    char device_mount_point[DLT_MOUNT_PATH_MAX]; /* Device mount path */
    unsigned int connection_type;      /* Type of connection */
    unsigned int config_status;        /* Status of configuration */
    int write_errors;                  /* number of write errors */
}DltLogStorage;

/**
 * dlt_logstorage_device_connected
 *
 * Initializes DLT Offline Logstorage with respect to device status
 *
 *
 * @param handle         DLT Logstorage handle
 * @param mount_point    Device mount path
 * @return               0 on success, -1 on error
 */
extern int dlt_logstorage_device_connected(DltLogStorage *handle, char  *mount_point);

/**
  * dlt_logstorage_load_config
  *
  * Parse dlt_logstorage.conf file in the device and setup internal info table
  *
  * @param handle    DltLogStorage handle
  * @return          0 on success, -1 on error
  */
extern int dlt_logstorage_load_config(DltLogStorage *handle);
/**
 * dlt_logstorage_device_disconnected
 * De-Initializes DLT Offline Logstorage with respect to device status
 *
 * @param handle         DLT Logstorage handle
 * @return               0 on success, -1 on error
 */
extern int dlt_logstorage_device_disconnected(DltLogStorage *handle);
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
extern DltLogStorageConfigData **dlt_logstorage_get_config(DltLogStorage *handle, char *apid, char *ctid, int *num_config);
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
extern int dlt_logstorage_get_loglevel_by_key(DltLogStorage *handle, char *key);

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
extern int dlt_logstorage_write(DltLogStorage *handle, DltLogStorageUserConfig file_config,
                                char *appid, char *ctxid, int log_level,
                                unsigned char *data1, int size1, unsigned char *data2,
                                int size2, unsigned char *data3, int size3);
#endif /* DLT_OFFLINE_LOGSTORAGE_H */
