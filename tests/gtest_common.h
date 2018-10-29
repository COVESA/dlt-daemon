/**
 * @licence app begin@
 * Copyright (C) 2016  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT gtest common header file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Onkar Palkar <onkar.palkar@wipro.com>
 *
 * \file: gtest_common.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
 *                                                                            **
 *  SRC-MODULE: gtest_common.h                                                **
 *                                                                            **
 *  TARGET    : linux                                                         **
 *                                                                            **
 *  PROJECT   : DLT                                                           **
 *                                                                            **
 *  AUTHOR    : onkar.palkar@wipro.com                                        **
 *  PURPOSE   :                                                               **
 *                                                                            **
 *  REMARKS   :                                                               **
 *                                                                            **
 *  PLATFORM DEPENDANT [yes/no]: yes                                          **
 *                                                                            **
 *  TO BE CHANGED BY USER [yes/no]: no                                        **
 *                                                                            **
 ******************************************************************************/

/*******************************************************************************
*                      Author Identity                                       **
*******************************************************************************
*                                                                            **
* Initials     Name                       Company                            **
* --------     -------------------------  ---------------------------------- **
*  op          Onkar Palkar               Wipro                              **
*******************************************************************************/
#ifndef GTEST_COMMON_H
#define GTEST_COMMON_H
int dlt_logstorage_split_key(char *key, char *appid, char *ctxid);
int dlt_logstorage_update_all_contexts(DltDaemon *daemon,
                                       char *id,
                                       int curr_log_level,
                                       int cmp_flag,
                                       int verbose);
int dlt_logstorage_update_context(DltDaemon *daemon,
                                  char *apid,
                                  char *ctxid,
                                  char *ecuid,
                                  int curr_log_level,
                                  int verbose);
int dlt_logstorage_update_context_loglevel(DltDaemon *daemon,
                                           char *key,
                                           int curr_log_level,
                                           int verbose);
void dlt_daemon_logstorage_reset_application_loglevel(DltDaemon *daemon,
                                                      int dev_num,
                                                      int max_device,
                                                      int verbose);
typedef struct {
    char *key; /* The configuration key */
    int (*func)(DltLogStorage *handle, char *value); /* conf handler */
    int is_opt; /* If configuration is optional or not */
} DltLogstorageGeneralConf;

typedef enum {
    DLT_LOGSTORAGE_GENERAL_CONF_BLOCKMODE = 0,
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
    DLT_LOGSTORAGE_FILTER_CONF_FILE,
    DLT_LOGSTORAGE_FILTER_CONF_FILESIZE,
    DLT_LOGSTORAGE_FILTER_CONF_NOFILES,
    DLT_LOGSTORAGE_FILTER_CONF_SYNCBEHAVIOR,
    DLT_LOGSTORAGE_FILTER_CONF_ECUID,
    DLT_LOGSTORAGE_FILTER_CONF_COUNT
} DltLogstorageFilterConfType;

    int dlt_logstorage_hash_create(int num_entries,
                                   struct hsearch_data *htab);
    int dlt_logstorage_hash_destroy(struct hsearch_data *htab);
    int dlt_logstorage_hash_add(char *key, void *value,
                                   struct hsearch_data *htab);
    void *dlt_logstorage_hash_find(char *key, struct hsearch_data *htab);
    int dlt_logstorage_count_ids(const char *str);
    int dlt_logstorage_read_number(unsigned int *number, char *value);
    int dlt_logstorage_read_list_of_names(char **names, char *value);
    int dlt_logstorage_check_apids(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_ctids(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_loglevel(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_filename(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_filesize(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_nofiles(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_sync_strategy(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_ecuid(DltLogStorageFilterConfig *config, char *value);
    int dlt_logstorage_check_param(DltLogStorageFilterConfig *config,
                                      DltLogstorageFilterConfType ctype,
                                      char *value);
    int dlt_logstorage_check_blockmode(DltLogStorage *handle,
                                          char *value);
    int dlt_logstorage_check_general_param(DltLogStorage *handle,
                                              DltLogstorageGeneralConfType ctype,
                                              char *value);
    int dlt_daemon_setup_general_properties(DltLogStorage *handle,
                                               DltConfigFile *config_file,
                                               char *sec_name);
    int dlt_logstorage_store_filters(DltLogStorage *handle,
                                        char *config_file_name);
    void dlt_logstorage_free(DltLogStorage *handle, int reason);
    int dlt_logstorage_create_keys(char *appids,
                                      char *ctxids,
                                      char **keys,
                                      int *num_keys);
    int dlt_logstorage_prepare_table(DltLogStorage *handle,
                                        DltLogStorageFilterConfig *data);
    int dlt_logstorage_validate_filter_name(char *name);
    void dlt_logstorage_filter_set_strategy(DltLogStorageFilterConfig *config,
                                               int strategy);
    int dlt_logstorage_load_config(DltLogStorage *handle);
    int dlt_logstorage_filter(DltLogStorage *handle,
                              DltLogStorageFilterConfig **config,
                              char *apid,
                              char *ctid,
                              char *ecuid,
                              int log_level);
void dlt_logstorage_log_file_name(char *log_file_name,
                                  DltLogStorageUserConfig *file_config,
                                  char *name,
                                  int idx);
void dlt_logstorage_sort_file_name(DltLogStorageFileList **head);
void dlt_logstorage_rearrange_file_name(DltLogStorageFileList **head);
unsigned int dlt_logstorage_get_idx_of_log_file(
    DltLogStorageUserConfig *file_config,
    char *file);
int dlt_logstorage_storage_dir_info(DltLogStorageUserConfig *file_config,
                                    char *path,
                                    DltLogStorageFilterConfig *config);
int dlt_logstorage_open_log_file(DltLogStorageFilterConfig *config,
                                 DltLogStorageUserConfig *file_config,
                                 char *dev_path,
                                 int msg_size);
/* gtest_dlt_daemon_gateway */
typedef enum {
    GW_CONF_IP_ADDRESS = 0,
    GW_CONF_PORT,
    GW_CONF_ECUID,
    GW_CONF_CONNECT,
    GW_CONF_TIMEOUT,
    GW_CONF_SEND_CONTROL,
    GW_CONF_SEND_PERIODIC_CONTROL,
    GW_CONF_SEND_SERIAL_HEADER,
    GW_CONF_COUNT
} DltGatewayConfType;
int enable_all(DltServiceIdFlag *flags);
int init_flags(DltServiceIdFlag *flags);
int set_bit(DltServiceIdFlag *flags, int id);
int bit(DltServiceIdFlag *flags, int id);
int dlt_daemon_filter_name(DltMessageFilter *mf,
                                   DltFilterConfiguration *config,
                                   char *value);
int dlt_daemon_filter_level(DltMessageFilter *mf,
                                   DltFilterConfiguration *config,
                                   char *value);
int dlt_daemon_filter_control_mask(DltMessageFilter *mf,
                                          DltFilterConfiguration *config,
                                          char *value);
int dlt_daemon_filter_client_mask(DltMessageFilter *mf,
                                         DltFilterConfiguration *config,
                                         char *value);
int dlt_daemon_filter_injections(DltMessageFilter *mf,
                                        DltFilterConfiguration *config,
                                        char *value);
int dlt_daemon_set_injection_service_ids(int **ids,
                                                int *num,
                                                char *value);
DltInjectionConfig *dlt_daemon_filter_find_injection_by_name(
    DltInjectionConfig *injections,
    char *name);
int dlt_daemon_injection_name(DltMessageFilter *mf,
                                         DltInjectionConfig *config,
                                         char *value);
int dlt_daemon_injection_apid(DltMessageFilter *mf,
                                         DltInjectionConfig *config,
                                         char *value);
int dlt_daemon_injection_ctid(DltMessageFilter *mf,
                                         DltInjectionConfig *config,
                                         char *value);
int dlt_daemon_injection_ecu_id(DltMessageFilter *mf,
                                           DltInjectionConfig *config,
                                           char *value);
int dlt_daemon_injection_service_id(DltMessageFilter *mf,
                                               DltInjectionConfig *config,
                                               char *value);
int dlt_daemon_get_name(DltMessageFilter *mf, char *val);
int dlt_daemon_get_default_level(DltMessageFilter *mf, char *val);
int dlt_daemon_get_backend(DltMessageFilter *mf, char *val);
int dlt_daemon_setup_filter_section(DltMessageFilter *mf,
                                           DltConfigFile *config,
                                           char *sec_name);
int dlt_daemon_setup_filter_properties(DltMessageFilter *mf,
                                              DltConfigFile *config,
                                              char *sec_name);
void dlt_daemon_filter_backend_level_changed(unsigned int level,
                                             void *ptr1,
                                             void *ptr2);
int dlt_gateway_check_ip(DltGatewayConnection *con, char *value);
int dlt_gateway_check_port(DltGatewayConnection *con, char *value);
int dlt_gateway_check_ecu(DltGatewayConnection *con, char *value);
int dlt_gateway_check_connect_trigger(DltGatewayConnection *con,
                                             char *value);
int dlt_gateway_check_timeout(DltGatewayConnection *con, char *value);
int dlt_gateway_check_send_serial(DltGatewayConnection *con, char *value);
int dlt_gateway_allocate_control_messages(DltGatewayConnection *con);
int dlt_gateway_check_control_messages(DltGatewayConnection *con,
                                              char *value);
int dlt_gateway_check_periodic_control_messages(DltGatewayConnection *con,
                                              char *value);
int dlt_gateway_check_param(DltGateway *gateway,
                                   DltGatewayConnection *con,
                                   DltGatewayConfType ctype,
                                   char *value);
int dlt_gateway_configure(DltGateway *gateway, char *config_file, int verbose);
int dlt_gateway_store_connection(DltGateway *gateway,
                                 DltGatewayConnection *tmp,
                                 int verbose);
int dlt_gateway_parse_get_log_info(DltDaemon *daemon,
                                   char *ecu,
                                   DltMessage *msg,
                                   int verbose);
#endif
