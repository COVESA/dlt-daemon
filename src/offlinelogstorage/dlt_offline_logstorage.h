/**
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
 * For further information see http://www.covesa.org/.
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
#include <stdbool.h>
#include <zlib.h>
#include "dlt_common.h"
#include "dlt-daemon_cfg.h"
#include "dlt_config_file_parser.h"

#define DLT_OFFLINE_LOGSTORAGE_MAXIDS                 100 /* Maximum entries for each apids and ctids */
#define DLT_OFFLINE_LOGSTORAGE_MAX_POSSIBLE_KEYS        7 /* Max number of possible keys when searching for */

#define DLT_OFFLINE_LOGSTORAGE_INIT_DONE                1 /* For device configuration status */
#define DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED         1
#define DLT_OFFLINE_LOGSTORAGE_FREE                     0
#define DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED      0
#define DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE              1

#define DLT_OFFLINE_LOGSTORAGE_SYNC_CACHES              2 /* sync logstorage caches */

#define DLT_OFFLINE_LOGSTORAGE_MAX_KEY_LEN             15 /* Maximum size for key */
#define DLT_OFFLINE_LOGSTORAGE_MAX_FILE_NAME_LEN       50 /* Maximum file name length of the log file */

#define DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN       4
#define DLT_OFFLINE_LOGSTORAGE_GZ_FILE_EXTENSION_LEN    7
#define DLT_OFFLINE_LOGSTORAGE_INDEX_LEN                3
#define DLT_OFFLINE_LOGSTORAGE_MAX_INDEX              999
#define DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN           16
#define DLT_OFFLINE_LOGSTORAGE_INDEX_OFFSET        (DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_INDEX_LEN)
#define DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN    (DLT_OFFLINE_LOGSTORAGE_MAX_FILE_NAME_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_TIMESTAMP_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_INDEX_LEN + \
                                                    DLT_OFFLINE_LOGSTORAGE_FILE_EXTENSION_LEN + 1)

#define DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN 50
#define DLT_OFFLINE_LOGSTORAGE_CONFIG_FILE_NAME    "dlt_logstorage.conf"

/* +3 because of device number and \0 */
#define DLT_OFFLINE_LOGSTORAGE_MAX_PATH_LEN (DLT_OFFLINE_LOGSTORAGE_MAX_LOG_FILE_LEN + \
                                             DLT_OFFLINE_LOGSTORAGE_CONFIG_DIR_PATH_LEN + 3)

#define DLT_OFFLINE_LOGSTORAGE_MAX(A, B)   ((A) > (B) ? (A) : (B))
#define DLT_OFFLINE_LOGSTORAGE_MIN(A, B)   ((A) < (B) ? (A) : (B))

#define DLT_OFFLINE_LOGSTORAGE_MAX_WRITE_ERRORS     5
#define DLT_OFFLINE_LOGSTORAGE_MAX_KEY_NUM          8

#define DLT_OFFLINE_LOGSTORAGE_CONFIG_SECTION "FILTER"
#define DLT_OFFLINE_LOGSTORAGE_GENERAL_CONFIG_SECTION "GENERAL"
#define DLT_OFFLINE_LOGSTORAGE_NONVERBOSE_STORAGE_SECTION "NON-VERBOSE-STORAGE-FILTER"
#define DLT_OFFLINE_LOGSTORAGE_NONVERBOSE_CONTROL_SECTION "NON-VERBOSE-LOGLEVEL-CTRL"

/* Offline Logstorage sync strategies */
#define DLT_LOGSTORAGE_SYNC_ON_ERROR                  -1 /* error case */
#define DLT_LOGSTORAGE_SYNC_UNSET                     0  /* strategy not set */
#define DLT_LOGSTORAGE_SYNC_ON_MSG                    1 /* default, on message sync */
#define DLT_LOGSTORAGE_SYNC_ON_DAEMON_EXIT            (1 << 1) /* sync on daemon exit */
#define DLT_LOGSTORAGE_SYNC_ON_DEMAND                 (1 << 2) /* sync on demand */
#define DLT_LOGSTORAGE_SYNC_ON_DEVICE_DISCONNECT      (1 << 3) /* sync on device disconnect*/
#define DLT_LOGSTORAGE_SYNC_ON_SPECIFIC_SIZE          (1 << 4) /* sync on after specific size */
#define DLT_LOGSTORAGE_SYNC_ON_FILE_SIZE              (1 << 5) /* sync on file size reached */

#define DLT_OFFLINE_LOGSTORAGE_IS_STRATEGY_SET(S, s) ((S)&(s))

/* logstorage max cache */
extern unsigned int g_logstorage_cache_max;
/* current logstorage cache size */
extern unsigned int g_logstorage_cache_size;

typedef struct
{
    unsigned int offset;          /* current write offset */
    unsigned int wrap_around_cnt; /* wrap around counter */
    unsigned int last_sync_offset; /* last sync position */
    unsigned int end_sync_offset; /* end position of previous round */
} DltLogStorageCacheFooter;

typedef struct
{
    /* File name user configurations */
    int logfile_timestamp;              /* Timestamp set/reset */
    char logfile_delimiter;             /* Choice of delimiter */
    unsigned int logfile_maxcounter;    /* Maximum file index counter */
    unsigned int logfile_counteridxlen; /* File index counter length */
} DltLogStorageUserConfig;

typedef struct DltLogStorageFileList
{
    /* List for filenames */
    char *name;                         /* Filename */
    unsigned int idx;                   /* File index */
    struct DltLogStorageFileList *next; /* Pointer to next */
} DltLogStorageFileList;

typedef struct DltNewestFileName DltNewestFileName;

struct DltNewestFileName
{
    char *file_name;    /* The unique name of file in whole a dlt_logstorage.conf */
    char *newest_file;  /* The real newest name of file which is associated with filename.*/
    unsigned int wrap_id;   /* Identifier of wrap around happened for this file_name */
    DltNewestFileName *next; /* Pointer to next */
};

typedef struct DltLogStorageFilterConfig DltLogStorageFilterConfig;

struct DltLogStorageFilterConfig
{
    /* filter section */
    char *apids;                    /* Application IDs configured for filter */
    char *ctids;                    /* Context IDs configured for filter */
    int log_level;                  /* Log level number configured for filter */
    int reset_log_level;            /* reset Log level to be sent on disconnect */
    char *file_name;                /* File name for log storage configured for filter */
    char *working_file_name;        /* Current open log file name */
    unsigned int wrap_id;           /* Identifier of wrap around happened for this filter */
    unsigned int file_size;         /* MAX File size of storage file configured for filter */
    unsigned int num_files;         /* MAX number of storage files configured for filters */
    int sync;                       /* Sync strategy */
    char *ecuid;                    /* ECU identifier */
    unsigned int gzip_compression;  /* Toggle if log files should be gzip compressed */
    /* callback function for filter configurations */
    int (*dlt_logstorage_prepare)(DltLogStorageFilterConfig *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int log_msg_size,
                                  DltNewestFileName *newest_file_info);
    int (*dlt_logstorage_write)(DltLogStorageFilterConfig *config,
                                DltLogStorageUserConfig *file_config,
                                char *dev_path,
                                unsigned char *data1,
                                int size1,
                                unsigned char *data2,
                                int size2,
                                unsigned char *data3,
                                int size3);
    /* status is strategy, e.g. DLT_LOGSTORAGE_SYNC_ON_MSG is used when callback
     * is called on message received */
    int (*dlt_logstorage_sync)(DltLogStorageFilterConfig *config,
                               DltLogStorageUserConfig *uconfig,
                               char *dev_path,
                               int status);
    FILE *log;                      /* current open log file */
    int fd;                         /* The file descriptor for the active log file */
#ifdef DLT_LOGSTORAGE_USE_GZIP
    gzFile gzlog;                   /* current open gz log file */
#endif
    void *cache;                    /* log data cache */
    unsigned int specific_size;     /* cache size used for specific_size sync strategy */
    unsigned int current_write_file_offset;    /* file offset for specific_size sync strategy */
    DltLogStorageFileList *records; /* File name list */
};

typedef struct DltLogStorageFilterList DltLogStorageFilterList;

struct DltLogStorageFilterList
{
    char *key_list;                   /* List of key */
    int num_keys;                     /* Number of keys */
    DltLogStorageFilterConfig *data;  /* Filter data */
    DltLogStorageFilterList *next;    /* Pointer to next */
};

typedef struct
{
    DltLogStorageFilterList *config_list; /* List of all filters */
    DltLogStorageUserConfig uconfig;   /* User configurations for file name*/
    int num_configs;                   /* Number of configs */
    char device_mount_point[DLT_MOUNT_PATH_MAX + 1]; /* Device mount path */
    unsigned int connection_type;      /* Type of connection */
    unsigned int config_status;        /* Status of configuration */
    int write_errors;                  /* number of write errors */
    DltNewestFileName *newest_file_list; /* List of newest file name */
    int maintain_logstorage_loglevel;  /* Permission to maintain the logstorage loglevel*/
} DltLogStorage;

typedef struct {
    char *key; /* The configuration key */
    int (*func)(DltLogStorage *handle, char *value); /* conf handler */
    int is_opt; /* If configuration is optional or not */
} DltLogstorageGeneralConf;

typedef enum {
    DLT_LOGSTORAGE_GENERAL_CONF_MAINTAIN_LOGSTORAGE_LOGLEVEL = 1,
    DLT_LOGSTORAGE_GENERAL_CONF_COUNT
} DltLogstorageGeneralConfType;

typedef struct {
    char *key; /* Configuration key */
    int (*func)(DltLogStorageFilterConfig *config, char *value); /* conf handler */
    int is_opt; /* If configuration is optional or not */
} DltLogstorageFilterConf;

typedef enum {
    DLT_LOGSTORAGE_FILTER_CONF_LOGAPPNAME = 0,
    DLT_LOGSTORAGE_FILTER_CONF_CONTEXTNAME,
    DLT_LOGSTORAGE_FILTER_CONF_LOGLEVEL,
    DLT_LOGSTORAGE_FILTER_CONF_RESET_LOGLEVEL,
    DLT_LOGSTORAGE_FILTER_CONF_FILE,
    DLT_LOGSTORAGE_FILTER_CONF_FILESIZE,
    DLT_LOGSTORAGE_FILTER_CONF_NOFILES,
    DLT_LOGSTORAGE_FILTER_CONF_SYNCBEHAVIOR,
    DLT_LOGSTORAGE_FILTER_CONF_ECUID,
    DLT_LOGSTORAGE_FILTER_CONF_SPECIFIC_SIZE,
    DLT_LOGSTORAGE_FILTER_CONF_GZIP_COMPRESSION,
    DLT_LOGSTORAGE_FILTER_CONF_COUNT
} DltLogstorageFilterConfType;

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
int dlt_logstorage_device_connected(DltLogStorage *handle,
                                    char *mount_point);

/**
 * dlt_logstorage_device_disconnected
 * De-Initializes DLT Offline Logstorage with respect to device status
 *
 * @param handle         DLT Logstorage handle
 * @param reason         Reason for device disconnection
 * @return               0 on success, -1 on error
 */
int dlt_logstorage_device_disconnected(DltLogStorage *handle,
                                       int reason);
/**
 * dlt_logstorage_get_config
 *
 * Obtain the configuration data of all filters for provided apid and ctid
 * For a given apid and ctid, there can be 3 possiblities of configuration
 * data available in the Hash map, this function will return the address
 * of configuration data for all these 3 combinations
 *
 * @param handle    DltLogStorage handle
 * @param config    Pointer to array of filter configurations
 * @param apid      application id
 * @param ctid      context id
 * @param ecuid     ecu id
 * @return          number of found configurations
 */
int dlt_logstorage_get_config(DltLogStorage *handle,
                              DltLogStorageFilterConfig **config,
                              char *apid,
                              char *ctid,
                              char *ecuid);

/**
 * dlt_logstorage_get_loglevel_by_key
 *
 * Obtain the log level for the provided key
 * This function can be used to obtain log level when the actual
 * key stored in the Hash map is available with the caller
 *
 * @param handle    DltLogstorage handle
 * @param key       key to search for in Hash MAP
 * @return          log level on success:, -1 on error
 */
int dlt_logstorage_get_loglevel_by_key(DltLogStorage *handle, char *key);

/**
 * dlt_logstorage_write
 *
 * Write a message to one or more configured log files, based on filter
 * configuration.
 *
 * @param handle    DltLogStorage handle
 * @param uconfig   User configurations for log file
 * @param data1     Data buffer of message header
 * @param size1     Size of message header buffer
 * @param data2     Data buffer of extended message body
 * @param size2     Size of extended message body
 * @param data3     Data buffer of message body
 * @param size3     Size of message body
 * @return          0 on success or write errors < max write errors, -1 on error
 */
int dlt_logstorage_write(DltLogStorage *handle,
                         DltLogStorageUserConfig *uconfig,
                         unsigned char *data1,
                         int size1,
                         unsigned char *data2,
                         int size2,
                         unsigned char *data3,
                         int size3);

/**
 * dlt_logstorage_sync_caches
 *
 * Sync all caches inside the specified logstorage device.
 *
 * @param  handle    DltLogStorage handle
 * @return 0 on success, -1 otherwise
 */
int dlt_logstorage_sync_caches(DltLogStorage *handle);

#endif /* DLT_OFFLINE_LOGSTORAGE_H */
