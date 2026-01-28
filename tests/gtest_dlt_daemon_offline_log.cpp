/*!
 * file gtest_dlt_daemon_logstorage.cpp
 *
 * Descriptiom : Unit test for dlt_logstorage.c
 *
 * Author      : Onkar Palkar
 *
 * Email       : onkar.palkar@wipro.com
 *
 * History     : 30-Jun-2016
 */
#include <gtest/gtest.h>
#include <regex>
#include <fstream>

int connectServer(void);

extern "C"
{
#include "dlt_offline_logstorage.h"
#include "dlt_offline_logstorage_internal.h"
#include "dlt_offline_logstorage_behavior.h"
#include "dlt_offline_logstorage_behavior_internal.h"
#include "dlt_daemon_offline_logstorage.h"
#include "dlt_daemon_offline_logstorage_internal.h"
#include "dlt_daemon_common_cfg.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
}


#define DLT_OFFLINE_LOGSTORAGE_FILTER_ERROR 1
#define DLT_CONFIG_FILE_SECTIONS 3

unsigned int g_logstorage_cache_max;
/* Begin Method: dlt_logstorage::t_dlt_logstorage_list_add*/
TEST(t_dlt_logstorage_list_add, normal)
{
    DltLogStorageFilterList *list = NULL;
    DltLogStorageFilterConfig *data = NULL;
    DltLogStorageUserConfig file_config;
    char path[] = "/tmp";
    char key = 1;
    int num_keys = 1;

    data = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));

    if (data != NULL) {
        dlt_logstorage_filter_set_strategy(data, DLT_LOGSTORAGE_SYNC_ON_MSG);

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(&key, num_keys, data, &list));
        /* Cast away const only for API compatibility, do not modify the string */
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_destroy(&list, &file_config, path, 0));
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_list_add_config*/
TEST(t_dlt_logstorage_list_add_config, normal)
{
    DltLogStorageFilterConfig *data = NULL;
    DltLogStorageFilterConfig *listdata = NULL;

    data = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));
    listdata = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));

    if ((data != NULL) && (listdata != NULL)) {
        dlt_logstorage_list_add_config(data, &listdata);
        free(data);
        free(listdata);
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_list_destroy*/
TEST(t_dlt_logstorage_list_destroy, normal)
{
    DltLogStorageFilterList *list = NULL;
    DltLogStorageFilterConfig *data = NULL;
    DltLogStorageUserConfig file_config;
    char *path = const_cast<char*>("/tmp");
    char key = 1;
    int num_keys = 1;

    data = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));

    if (data != NULL) {
        dlt_logstorage_filter_set_strategy(data, DLT_LOGSTORAGE_SYNC_ON_MSG);

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(&key, num_keys, data, &list));
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_destroy(&list, &file_config, path, 0));
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_list_find*/
TEST(t_dlt_logstorage_list_find, normal)
{
    DltLogStorageFilterList *list = NULL;
    DltLogStorageFilterConfig *data = NULL;
    int num_configs = 0;
    DltLogStorageUserConfig file_config;
    char *path = const_cast<char*>("/tmp");
    char key[] = ":1234:5678";
    char apid[] = "1234";
    char ctid[] = "5678";
    int num_keys = 1;
    DltLogStorageFilterConfig *config[DLT_CONFIG_FILE_SECTIONS_MAX] = { 0 };

    data = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));

    if (data != NULL) {
        data->apids = strdup(apid);
        data->ctids = strdup(ctid);
        dlt_logstorage_filter_set_strategy(data, DLT_LOGSTORAGE_SYNC_ON_MSG);

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key, num_keys, data, &list));

        num_configs = dlt_logstorage_list_find(key, &list, config);

        EXPECT_EQ(1, num_configs);
        EXPECT_NE((DltLogStorageFilterConfig *)NULL, config[0]);

        if (num_configs > 0) {
            EXPECT_STREQ(apid, config[0]->apids);
            EXPECT_STREQ(ctid, config[0]->ctids);
        }

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_destroy(&list, &file_config, path, 0));
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_free*/
TEST(t_dlt_logstorage_free, normal)
{
    char key = 1;
    DltLogStorage handle;
    DltLogStorageFilterConfig *data = NULL;
    int reason = 0;
    handle.num_configs = 0;
    handle.config_list = NULL;
    int num_keys = 1;

    data = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));

    if (data != NULL) {
        dlt_logstorage_filter_set_strategy(data, DLT_LOGSTORAGE_SYNC_ON_MSG);

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(&key, num_keys, data, &handle.config_list));

        dlt_logstorage_free(&handle, reason);
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_count_ids*/
TEST(t_dlt_logstorage_count_ids, normal)
{
    char const *str = "a,b,c,d";

    EXPECT_EQ(4, dlt_logstorage_count_ids(str));
}

TEST(t_dlt_logstorage_count_ids, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_count_ids(NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_read_number*/
TEST(t_dlt_logstorage_read_number, normal)
{
    char str[] = "100";
    unsigned int number;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_read_number(&number, str));
    EXPECT_EQ(100, number);
}

TEST(t_dlt_logstorage_read_number, null)
{
    unsigned int number;

    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_read_number(&number, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_create_keys*/
TEST(t_dlt_logstorage_create_keys, normal)
{
    DltLogStorageFilterConfig data;
    memset(&data, 0, sizeof(DltLogStorageFilterConfig));
    char *keys = NULL;
    int num_keys = 0;
    char apids[] = "1234";
    char ctids[] = "5678";
    char ecuid[] = "ECU1";
    data.apids = apids;
    data.ctids = ctids;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_create_keys(data.apids, data.ctids, ecuid, &keys, &num_keys));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_prepare_table*/
TEST(t_dlt_logstorage_prepare_table, normal)
{
    DltLogStorage handle;
    DltLogStorageFilterConfig data;
    DltLogStorageUserConfig file_config;
    char *path = const_cast<char*>("/tmp");
    memset(&handle, 0, sizeof(DltLogStorage));
    memset(&data, 0, sizeof(DltLogStorageFilterConfig));
    char apids[] = "1234";
    char ctids[] = "5678";
    data.apids = apids;
    data.ctids = ctids;
    data.records = NULL;
    data.log = NULL;
    data.cache = NULL;

    dlt_logstorage_filter_set_strategy(&data, DLT_LOGSTORAGE_SYNC_ON_MSG);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_table(&handle, &data));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_destroy(&handle.config_list, &file_config, path, 0));
}

TEST(t_dlt_logstorage_prepare_table, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_prepare_table(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_validate_filter_name*/
TEST(t_dlt_logstorage_validate_filter_name, normal)
{
    char name[] = "FILTER100";

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_validate_filter_name(name));
}

TEST(t_dlt_logstorage_validate_filter_name, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_validate_filter_name(NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_filter_set_strategy*/
TEST(t_dlt_logstorage_filter_set_strategy, normal)
{
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    dlt_logstorage_filter_set_strategy(&config, DLT_LOGSTORAGE_SYNC_ON_MSG);

    EXPECT_EQ(&dlt_logstorage_prepare_on_msg, config.dlt_logstorage_prepare);

    dlt_logstorage_filter_set_strategy(&config, 2);

    EXPECT_EQ(&dlt_logstorage_prepare_msg_cache, config.dlt_logstorage_prepare);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_read_list_of_names*/
TEST(t_dlt_logstorage_read_list_of_names, normal)
{
    char *namesPtr = NULL;
    char value[] = "a,b,c,d";

    namesPtr = (char *)calloc (1, sizeof(char));

    if (namesPtr != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_read_list_of_names(&namesPtr, value));

        free(namesPtr);
    }
}

TEST(t_dlt_logstorage_read_list_of_names, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_read_list_of_names(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_apids*/
TEST(t_dlt_logstorage_check_apids, normal)
{
    char value[] = "a,b,c,d";
    DltLogStorageFilterConfig config;
    /* Initialize id pointer as NULL pointer for testing only */
    config.apids = (char *)calloc (1, sizeof(char));

    if (config.apids != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_apids(&config, value));
        EXPECT_EQ(DLT_RETURN_OK, strncmp(config.apids, value, 7));
        free(config.apids);
    }
}

TEST(t_dlt_logstorage_check_apids, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_apids(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_ctids*/
TEST(t_dlt_logstorage_check_ctids, normal)
{
    char value[] = "a,b,c,d";
    DltLogStorageFilterConfig config;
    /* Initialize id pointer as NULL pointer for testing only */
    config.ctids = (char *)calloc (1, sizeof(char));

    if (config.ctids != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_ctids(&config, value));
        EXPECT_EQ(DLT_RETURN_OK, strncmp(config.ctids, value, 7));
        free(config.ctids);
    }
}

TEST(t_dlt_logstorage_check_ctids, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_ctids(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_store_config_excluded_apids*/
TEST(t_dlt_logstorage_store_config_excluded_apids, normal)
{
    char value[] = "a,b,c,d";
    DltLogStorageFilterConfig config;
    /* Initialize id pointer as NULL pointer for testing only */
    config.excluded_apids = (char *)calloc (1, sizeof(char));

    if (config.excluded_apids != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_store_config_excluded_apids(&config, value));
        EXPECT_EQ(DLT_RETURN_OK, strncmp(config.excluded_apids, value, 7));
        free(config.excluded_apids);
    }
}

TEST(t_dlt_logstorage_store_config_excluded_apids, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_store_config_excluded_apids(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_store_config_excluded_ctids*/
TEST(t_dlt_logstorage_store_config_excluded_ctids, normal)
{
    char value[] = "a,b,c,d";
    DltLogStorageFilterConfig config;
    /* Initialize id pointer as NULL pointer for testing only */
    config.excluded_ctids = (char *)calloc (1, sizeof(char));

    if (config.excluded_ctids != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_store_config_excluded_ctids(&config, value));
        EXPECT_EQ(DLT_RETURN_OK, strncmp(config.excluded_ctids, value, 7));
        free(config.excluded_ctids);
    }
}

TEST(t_dlt_logstorage_store_config_excluded_ctids, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_store_config_excluded_ctids(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_excluded_ids*/
TEST(t_dlt_logstorage_check_excluded_ids, normal)
{
    char id[] = "log4";
    char not_excluded_id[] = "log0";
    char delim[] = ",";
    char excluded_ids[] = "log1,log2,log3,log4";

    EXPECT_TRUE(dlt_logstorage_check_excluded_ids(id, delim, excluded_ids));
    EXPECT_FALSE(dlt_logstorage_check_excluded_ids(not_excluded_id, delim, excluded_ids));
}

TEST(t_dlt_logstorage_check_excluded_ids, null)
{
    EXPECT_FALSE(dlt_logstorage_check_excluded_ids(NULL, NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_loglevel*/
TEST(t_dlt_logstorage_check_loglevel, normal)
{
    char value[] = "DLT_LOG_FATAL";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_loglevel(&config, value));
    EXPECT_EQ(1, config.log_level);
}

TEST(t_dlt_logstorage_check_loglevel, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_loglevel(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_filename*/
TEST(t_dlt_logstorage_check_filename, normal)
{
    char value[] = "file_name";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.file_name = (char *)calloc (1, sizeof(char));

    if (config.file_name != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_filename(&config, value));

        free(config.file_name);
    }
}

TEST(t_dlt_logstorage_check_filename, abnormal)
{
    char value[] = "../file_name";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.file_name = (char *)calloc (1, sizeof(char));

    if (config.file_name != NULL) {
        EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_filename(&config, value));

        free(config.file_name);
    }
}

TEST(t_dlt_logstorage_check_filename, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_filename(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_filesize*/
TEST(t_dlt_logstorage_check_filesize, normal)
{
    char value[] = "100";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_filesize(&config, value));
    EXPECT_EQ(100, config.file_size);
}

TEST(t_dlt_logstorage_check_filesize, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_filesize(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_nofiles*/
TEST(t_dlt_logstorage_check_nofiles, normal)
{
    char value[] = "100";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_nofiles(&config, value));
    EXPECT_EQ(100, config.num_files);
}

TEST(t_dlt_logstorage_check_nofiles, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_nofiles(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_sync_strategy*/
TEST(t_dlt_logstorage_check_sync_strategy, normal)
{
    char value[] = "ON_MSG";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.sync = 0;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_sync_strategy(&config, value));
    EXPECT_EQ(DLT_LOGSTORAGE_SYNC_ON_MSG, config.sync);
}

TEST(t_dlt_logstorage_check_sync_strategy, abnormal)
{
    char value[] = "UNKNOWN";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.sync = 0;

    EXPECT_EQ(DLT_RETURN_TRUE, dlt_logstorage_check_sync_strategy(&config, value));
    EXPECT_EQ(DLT_LOGSTORAGE_SYNC_ON_MSG, config.sync);
}

TEST(t_dlt_logstorage_check_sync_strategy, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_sync_strategy(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_ecuid*/
TEST(t_dlt_logstorage_check_ecuid, normal)
{
    char value[] = "213";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.ecuid = (char *)calloc (1, sizeof(char));

    if (config.ecuid != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_ecuid(&config, value));
        EXPECT_EQ('2', *config.ecuid);
        EXPECT_EQ('1', *(config.ecuid + 1));
        EXPECT_EQ('3', *(config.ecuid + 2));

        free(config.ecuid);
    }
}

TEST(t_dlt_logstorage_check_ecuid, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_ecuid(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_check_param*/
TEST(t_dlt_logstorage_check_param, normal)
{
    char value[] = "100";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_check_param(&config, DLT_LOGSTORAGE_FILTER_CONF_FILESIZE, value));
    EXPECT_EQ(100, config.file_size);
}

TEST(t_dlt_logstorage_check_param, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_check_param(NULL, DLT_LOGSTORAGE_FILTER_CONF_FILESIZE, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_store_filters*/
TEST(t_dlt_logstorage_store_filters, normal)
{
    DltLogStorage handle;
    memset(&handle, 0, sizeof(DltLogStorage));
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    char *path = const_cast<char*>("/tmp");
    char config_file_name[] = "/tmp/dlt_logstorage.conf";
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = 0;
    handle.write_errors = 0;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_store_filters(&handle, config_file_name));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_destroy(&handle.config_list, &file_config, path, 0));
}

TEST(t_dlt_logstorage_store_filters, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_store_filters(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_load_config*/
TEST(t_dlt_logstorage_load_config, normal)
{
    DltLogStorage handle;
    DltLogStorageUserConfig file_config;
    char *path = const_cast<char*>("/tmp");
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = 0;
    handle.write_errors = 0;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;
    strncpy(handle.device_mount_point, "/tmp", DLT_MOUNT_PATH_MAX);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_load_config(&handle));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_destroy(&handle.config_list, &file_config, path, 0));
}

TEST(t_dlt_logstorage_load_config, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_load_config(NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_device_connected*/
TEST(t_dlt_logstorage_device_connected, normal)
{
    DltLogStorage handle;
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED;
    handle.config_status = 0;
    handle.write_errors = 0;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;
    handle.config_mode = DLT_LOGSTORAGE_CONFIG_FILE;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_device_connected(&handle, "/tmp"));
}

TEST(t_dlt_logstorage_device_connected, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_device_connected(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_device_disconnected*/
TEST(t_dlt_logstorage_device_disconnected, normal)
{
    DltLogStorage handle;
    int reason = 0;
    handle.config_status = 0;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_device_disconnected(&handle, reason));
}

TEST(t_dlt_logstorage_device_disconnected, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_device_disconnected(NULL, 1));
}

TEST(t_dlt_logstorage_get_loglevel_by_key, normal)
{
    char arr[] = "abc";
    char *key = arr;
    DltLogStorageFilterConfig *config = NULL;
    DltLogStorage handle;
    handle.config_status = 0;
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;
    int num_keys = 1;

    config = (DltLogStorageFilterConfig *)calloc(1, sizeof(DltLogStorageFilterConfig));

    if (config != NULL) {
        config->log_level = DLT_LOG_ERROR;

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key, num_keys, config, &(handle.config_list)));
        EXPECT_GE(DLT_LOG_ERROR, dlt_logstorage_get_loglevel_by_key(&handle, key));

        free(config);
    }
}

TEST(t_dlt_logstorage_get_loglevel_by_key, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_get_loglevel_by_key(NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_get_config*/
TEST(t_dlt_logstorage_get_config, normal)
{
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char file_name[] = "file_name";
    int num_config = 0;
    DltLogStorageFilterConfig value = {};
    value.log_level = 0;
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecuid;
    value.file_name = file_name;
    char key0[] = ":1234:\000\000\000\000";
    char key1[] = "::5678\000\000\000\000";
    char key2[] = ":1234:5678";
    DltLogStorageFilterConfig *config[3] = { 0 };
    DltLogStorage handle;
    memset(&handle, 0, sizeof(DltLogStorage));
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    int num_keys = 1;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num_config = dlt_logstorage_get_config(&handle, config, apid, ctid, ecuid);

    EXPECT_EQ(num_config, 3);
}

TEST(t_dlt_logstorage_get_config, null)
{
    int num = -1;
    num = dlt_logstorage_get_config(NULL, NULL, NULL, NULL, NULL);

    EXPECT_EQ(num, 0);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_filter*/
TEST(t_dlt_logstorage_filter, normal)
{
    char apid[] = "1234";
    char ctid[] = "5678";
    char t_apid[] = "4321";
    char t_ctid[] = "8765";
    char ecuid[] = "12";
    char filename[] = "file_name";
    int num = 1;
    DltLogStorageFilterConfig value = {};
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecuid;
    value.file_name = filename;
    value.log_level = DLT_LOG_VERBOSE;
    char key0[] = ":1234:\000\000\000\000";
    char key1[] = "::5678\000\000\000\000";
    char key2[] = ":1234:5678";
    DltLogStorageFilterConfig *config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    DltLogStorage handle;
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;
    int num_keys = 1;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(config[0] != NULL);
    EXPECT_TRUE(config[1] != NULL);
    EXPECT_TRUE(config[2] != NULL);

    /* Filter on excluded application and context */
    value.excluded_apids = apid;
    value.excluded_ctids = ctid;
    DltLogStorageFilterConfig *neg_filter_config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, neg_filter_config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(neg_filter_config[0] == NULL);
    EXPECT_TRUE(neg_filter_config[1] == NULL);
    EXPECT_TRUE(neg_filter_config[2] == NULL);

    /* Change excluded fields */
    value.excluded_apids = t_apid;
    value.excluded_ctids = t_ctid;
    DltLogStorageFilterConfig *t_neg_filter_config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, t_neg_filter_config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(t_neg_filter_config[0] != NULL);
    EXPECT_TRUE(t_neg_filter_config[1] != NULL);
    EXPECT_TRUE(t_neg_filter_config[2] != NULL);

    /* Only filter on excluded contexts */
    value.excluded_apids = NULL;
    value.excluded_ctids = ctid;
    DltLogStorageFilterConfig *neg_filter_ctid_only_config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, neg_filter_ctid_only_config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(neg_filter_ctid_only_config[0] == NULL);
    EXPECT_TRUE(neg_filter_ctid_only_config[1] == NULL);
    EXPECT_TRUE(neg_filter_ctid_only_config[2] == NULL);

    /* Change excluded fields */
    value.excluded_apids = NULL;
    value.excluded_ctids = t_ctid;
    DltLogStorageFilterConfig *t_neg_filter_ctid_only_config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, t_neg_filter_ctid_only_config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(t_neg_filter_ctid_only_config[0] != NULL);
    EXPECT_TRUE(t_neg_filter_ctid_only_config[1] != NULL);
    EXPECT_TRUE(t_neg_filter_ctid_only_config[2] != NULL);

    /* Only filter on excluded applications */
    value.excluded_apids = apid;
    value.excluded_ctids = NULL;
    DltLogStorageFilterConfig *neg_filter_apid_only_config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, neg_filter_apid_only_config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(neg_filter_apid_only_config[0] == NULL);
    EXPECT_TRUE(neg_filter_apid_only_config[1] == NULL);
    EXPECT_TRUE(neg_filter_apid_only_config[2] == NULL);

    /* Change excluded fields */
    value.excluded_apids = t_apid;
    value.excluded_ctids = NULL;
    DltLogStorageFilterConfig *t_neg_filter_apid_only_config[DLT_CONFIG_FILE_SECTIONS] = { 0 };
    handle.config_list = NULL;
    handle.newest_file_list = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));

    num = dlt_logstorage_filter(&handle, t_neg_filter_apid_only_config, apid, ctid, ecuid, 0);

    EXPECT_EQ(num, 3);
    EXPECT_TRUE(t_neg_filter_apid_only_config[0] != NULL);
    EXPECT_TRUE(t_neg_filter_apid_only_config[1] != NULL);
    EXPECT_TRUE(t_neg_filter_apid_only_config[2] != NULL);
}

TEST(t_dlt_logstorage_filter, null)
{
    DltLogStorageFilterConfig *config[3] = { 0 };
    int num = dlt_logstorage_filter(NULL, config, NULL, NULL, NULL, 0);
    EXPECT_EQ(DLT_RETURN_ERROR, num);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_write*/
TEST(t_dlt_logstorage_write, normal)
{
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char file_name[] = "file_name";
    DltLogStorage handle;
    DltLogStorageUserConfig uconfig = {};
    unsigned char data[] = "123";
    int size = 3;
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;
    DltLogStorageFilterConfig value = {};
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecuid;
    value.file_name = file_name;
    char key0[] = ":1234:\000\000\000\000";
    char key1[] = "::5678\000\000\000\000";
    char key2[] = ":1234:5678";
    int num_keys = 1;
    int disable_nw = 0;

    DltMessage msg;
    DltUserHeader userheader;
    int log_level = 4;
    dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG);
    dlt_message_init(&msg, 0);
    msg.storageheader = (DltStorageHeader *)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader, ecuid);
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_PROTOCOL_VERSION1|DLT_HTYP_UEH;
    msg.standardheader->mcnt = 0;
    dlt_message_set_extraparameters(&msg, 0);
    msg.extendedheader = (DltExtendedHeader *)(msg.headerbuffer +
                         sizeof(DltStorageHeader) +
                         sizeof(DltStandardHeader) +
                         DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = (DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) |
                               (uint8_t)(((log_level << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN) | DLT_MSIN_VERB);
    msg.extendedheader->noar = 1;
    dlt_set_id(msg.extendedheader->apid, apid);
    dlt_set_id(msg.extendedheader->ctid, ctid);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write(&handle, &uconfig,
                                             (unsigned char*)&(userheader), sizeof(DltUserHeader),
                                             msg.headerbuffer + sizeof(DltStorageHeader),
                                             (int)(msg.headersize - sizeof(DltStorageHeader)),
                                             data, size, &disable_nw));
    dlt_message_free(&msg, 0);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_write*/
TEST(t_dlt_logstorage_write_v2, normal)
{
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char file_name[] = "file_name";
    DltLogStorage handle;
    DltLogStorageUserConfig uconfig = {};
    unsigned char data[] = "123";
    int size = 3;
    handle.connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    handle.config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    handle.config_list = NULL;
    handle.newest_file_list = NULL;
    DltLogStorageFilterConfig value = {};
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecuid;
    (void)ecuid;
    (void)apid;
    (void)ctid;
    value.file_name = file_name;
    char key0[] = ":1234:\000\000\000\000";
    char key1[] = "::5678\000\000\000\000";
    char key2[] = ":1234:5678";
    int num_keys = 1;
    int disable_nw = 0;

    DltMessageV2 msg;
    DltUserHeader userheader;
    //int log_level = 4;
    dlt_user_set_userheader_v2(&userheader, DLT_USER_MESSAGE_LOG);
    dlt_message_init_v2(&msg, 0);
    size_t _hdrsz = 512;
    msg.headerbufferv2 = (uint8_t*)calloc(1, _hdrsz);
    msg.headersizev2 = (int)_hdrsz;
    msg.storageheadersizev2 = (int)sizeof(DltStorageHeaderV2);
    msg.baseheadersizev2 = (int)sizeof(DltBaseHeaderV2);
    msg.baseheaderextrasizev2 = 0;
    msg.baseheaderv2 = (DltBaseHeaderV2 *)(msg.headerbufferv2 + msg.storageheadersizev2);
    //msg.storageheaderv2 = (DltStorageHeaderV2 *)msg.headerbufferv2;
    //dlt_set_storageheader_v2(&(msg.storageheaderv2), ecuid_len, ecuid);
    msg.baseheaderv2 = (DltBaseHeaderV2 *)(msg.headerbufferv2 + sizeof(DltStorageHeaderV2));
    msg.baseheaderv2->htyp2 = DLT_HTYP2_PROTOCOL_VERSION2|DLT_HTYP2_EH;
    msg.baseheaderv2->mcnt = 0;
    dlt_message_set_extraparameters_v2(&msg, 0);
    /*msg.extendedheaderv2 = (DltExtendedHeaderV2 *)(msg.headerbufferv2 +
                         sizeof(DltStorageHeaderV2) +
                         sizeof(DltBaseHeaderV2) +
                         DLT_STANDARD_HEADER_EXTRA_SIZE_V2(msg.baseheaderv2.htyp2));
    dlt_set_id_v2(&(msg.extendedheaderv2.apid), apid, apid_len);
    dlt_set_id_v2(&(msg.extendedheaderv2.ctid), ctid, ctid_len);*/

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write(&handle, &uconfig,
                                             (unsigned char*)&(userheader), sizeof(DltUserHeader),
                                             msg.headerbufferv2 + sizeof(DltStorageHeaderV2),
                                             (int)(msg.headersizev2 - (int32_t)sizeof(DltStorageHeaderV2)),
                                             data, size, &disable_nw));
    dlt_message_free_v2(&msg, 0);
}

TEST(t_dlt_logstorage_write, null)
{
    EXPECT_EQ(0, dlt_logstorage_write(NULL, NULL, NULL, 1, NULL, 1, NULL, 1, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_sync_caches*/
TEST(t_dlt_logstorage_sync_caches, normal)
{
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char filename[] = "file_name";
    char key[] = "12:1234:5678";
    DltLogStorage handle;
    handle.num_configs = 1;
    handle.config_list = NULL;
    DltLogStorageFilterConfig configs = {};
    configs.apids = apid;
    configs.ctids = ctid;
    configs.ecuid = ecuid;
    configs.file_name = filename;
    int num_keys = 1;

    dlt_logstorage_filter_set_strategy(&configs, DLT_LOGSTORAGE_SYNC_ON_MSG);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key, num_keys, &configs, &(handle.config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_sync_caches(&handle));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_log_file_name*/
TEST(t_dlt_logstorage_log_file_name, normal)
{
    char log_file_name[DLT_MOUNT_PATH_MAX] = { '\0' };
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    file_config.logfile_delimiter = '/';
    file_config.logfile_maxcounter = 0;
    file_config.logfile_timestamp = 0;
    file_config.logfile_counteridxlen = 10;
    char name[] = "log";

    DltLogStorageFilterConfig filter_config;
    memset(&filter_config, 0, sizeof(filter_config));
    filter_config.file_name = &name[0];

    dlt_logstorage_log_file_name(log_file_name, &file_config, &filter_config, "log", 10, 0);
    EXPECT_EQ(std::string("log/0000000000.dlt"), log_file_name);

    file_config.logfile_counteridxlen = 3;
    file_config.logfile_delimiter = '_';
    dlt_logstorage_log_file_name(log_file_name, &file_config, &filter_config, "log", 9, 98);
    EXPECT_EQ(std::string("log_098.dlt"), log_file_name);
}

TEST(t_dlt_logstorage_log_file_name, tmsp)
{
    char log_file_name[DLT_MOUNT_PATH_MAX] = { '\0' };
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    file_config.logfile_delimiter = '_';
    file_config.logfile_maxcounter = 9;
    file_config.logfile_timestamp = 1;
    file_config.logfile_counteridxlen = 2;
    char name[] = "log";

    DltLogStorageFilterConfig filter_config;
    memset(&filter_config, 0, sizeof(filter_config));
    filter_config.file_name = &name[0];

    dlt_logstorage_log_file_name(log_file_name, &file_config, &filter_config, "log", 8,  4);

    // log_04_20210810-094602.dlt
    std::regex r("log_04_\\d{8}-\\d{6}\\.dlt");
    std::cmatch m;
    EXPECT_TRUE(std::regex_search(log_file_name, m, r));
}

TEST(t_dlt_logstorage_log_file_name, optional_index)
{
    char log_file_name[DLT_MOUNT_PATH_MAX];
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    file_config.logfile_delimiter = '_';
    file_config.logfile_maxcounter = 1;
    file_config.logfile_timestamp = 0;
    file_config.logfile_counteridxlen = 4;
    file_config.logfile_optional_counter = true;
    char name[] = "APID";

    DltLogStorageFilterConfig filter_config;
    memset(&filter_config, 0, sizeof(filter_config));
    filter_config.file_name = &name[0];

    dlt_logstorage_log_file_name(log_file_name, &file_config, &filter_config, "APID", 1, 0);
    EXPECT_EQ(std::string("APID.dlt"), log_file_name);

    dlt_logstorage_log_file_name(log_file_name, &file_config, &filter_config, "APID", 2, 0);
    EXPECT_EQ(std::string("APID_0000.dlt"), log_file_name);
}

TEST(t_dlt_logstorage_log_file_name, null)
{
    char name[] = "log";
    dlt_logstorage_log_file_name(NULL, NULL, NULL, name, 0, 0);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_sort_file_name*/
TEST(t_dlt_logstorage_sort_file_name, normal)
{
    DltLogStorageFileList *node1, *node2, *node3;
    DltLogStorageFileList **head;
    node1 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));
    node2 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));
    node3 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));

    if ((node1 != NULL) && (node2 != NULL) && (node3 != NULL)) {
        node1->next = node2;
        node2->next = node3;
        node3->next = NULL;
        head = &node1;

        node1->idx = 8;
        node2->idx = 4;
        node3->idx = 1;

        EXPECT_EQ(8, (*head)->idx);
        EXPECT_EQ(4, ((*head)->next)->idx);
        EXPECT_EQ(1, ((((*head)->next)->next)->idx));

        EXPECT_EQ(8, dlt_logstorage_sort_file_name(head));

        EXPECT_EQ(1, (*head)->idx);
        EXPECT_EQ(4, ((*head)->next)->idx);
        EXPECT_EQ(8, ((((*head)->next)->next)->idx));
        free((((*head)->next)->next));
        free(((*head)->next));
        free(*head);
        node1 = NULL;
        node2 = NULL;
        node3 = NULL;
    }

    if (node1 != NULL)
        free(node1);

    if (node2 != NULL)
        free(node2);

    if (node3 != NULL)
        free(node3);
}
TEST(t_dlt_logstorage_sort_file_name, null)
{
    dlt_logstorage_sort_file_name(NULL);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_rearrange_file_name*/
TEST(t_dlt_logstorage_rearrange_file_name, normal1)
{
    DltLogStorageFileList *node1, *node2, *node3;
    DltLogStorageFileList **head;
    node1 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));
    node2 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));
    node3 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));

    if ((node1 != NULL) && (node2 != NULL) && (node3 != NULL)) {

        node1->next = node2;
        node2->next = node3;
        node3->next = NULL;

        head = &node1;

        node1->idx = 1;
        node2->idx = 4;
        node3->idx = 8;

        EXPECT_EQ(1, (*head)->idx);
        EXPECT_EQ(4, ((*head)->next)->idx);
        EXPECT_EQ(8, ((((*head)->next)->next)->idx));

        dlt_logstorage_rearrange_file_name(head);

        EXPECT_EQ(4, (*head)->idx);
        EXPECT_EQ(8, ((*head)->next)->idx);
        EXPECT_EQ(1, ((((*head)->next)->next)->idx));
        free((((*head)->next)->next));
        free(((*head)->next));
        free(*head);
        node1 = NULL;
        node2 = NULL;
        node3 = NULL;
    }

    if (node1 != NULL)
        free(node1);

    if (node2 != NULL)
        free(node2);

    if (node3 != NULL)
        free(node3);
}

TEST(t_dlt_logstorage_rearrange_file_name, normal2)
{
    DltLogStorageFileList *node1, *node2, *node3;
    DltLogStorageFileList **head;
    node1 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));
    node2 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));
    node3 = (DltLogStorageFileList *)calloc (1, sizeof(DltLogStorageFileList));

    if ((node1 != NULL) && (node2 != NULL) && (node3 != NULL)) {

        node1->next = node2;
        node2->next = node3;
        node3->next = NULL;

        head = &node1;

        node1->idx = 2;
        node2->idx = 4;
        node3->idx = 8;

        EXPECT_EQ(2, (*head)->idx);
        EXPECT_EQ(4, ((*head)->next)->idx);
        EXPECT_EQ(8, ((((*head)->next)->next)->idx));

        dlt_logstorage_rearrange_file_name(head);

        EXPECT_EQ(2, (*head)->idx);
        EXPECT_EQ(4, ((*head)->next)->idx);
        EXPECT_EQ(8, ((((*head)->next)->next)->idx));
        free((((*head)->next)->next));
        free(((*head)->next));
        free(*head);
        node1 = NULL;
        node2 = NULL;
        node3 = NULL;
    }

    if (node1 != NULL)
        free(node1);

    if (node2 != NULL)
        free(node2);

    if (node3 != NULL)
        free(node3);
}

TEST(t_dlt_logstorage_rearrange_file_name, null)
{
    dlt_logstorage_rearrange_file_name(NULL);
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_get_idx_of_log_file*/
TEST(t_dlt_logstorage_get_idx_of_log_file, normal)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char name[] = "Test";
    char *file = const_cast<char*>("Test_002_20160509_191132.dlt");

    DltLogStorageFilterConfig filter_config;
    memset(&filter_config, 0, sizeof(filter_config));
    filter_config.file_name = &name[0];

    EXPECT_EQ(2, dlt_logstorage_get_idx_of_log_file(&file_config, &filter_config, file));

    char *gz_file = const_cast<char*>("Test_142_20160509_191132.dlt.gz");
    filter_config.gzip_compression = 1;

    EXPECT_EQ(142, dlt_logstorage_get_idx_of_log_file(&file_config, &filter_config, gz_file));
}
TEST(t_dlt_logstorage_get_idx_of_log_file, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_get_idx_of_log_file(NULL, NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_storage_dir_info*/
TEST(t_dlt_logstorage_storage_dir_info, normal)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test_002_20160509_191132.dlt");
    config.records = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_storage_dir_info(&file_config, path, &config));
}
TEST(t_dlt_logstorage_storage_dir_info, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_storage_dir_info(NULL, NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_open_log_file*/
TEST(t_dlt_logstorage_open_log_file, normal)
{
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    file_config.logfile_timestamp = 0;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.working_file_name = NULL;
    config.wrap_id = 0;
    config.file_size = 50;
    char tmp_file[100] = "";

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_open_log_file(&config, &file_config, path, 1, true, false));
    EXPECT_STREQ("Test_01.dlt", config.working_file_name);
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_open_log_file(&config, &file_config, path, 1, true, true));
    EXPECT_STREQ("Test_01.dlt", config.working_file_name);
    sprintf(tmp_file, "%s/%s", path, config.working_file_name);
    remove(tmp_file);
}
TEST(t_dlt_logstorage_open_log_file, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_open_log_file(NULL, NULL, NULL, 0, true, false));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_prepare_on_msg*/
TEST(t_dlt_logstorage_prepare_on_msg, normal1)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.working_file_name = NULL;
    config.wrap_id = 0;

    DltNewestFileName newest_file_name;
    newest_file_name.file_name = const_cast<char*>("Test");
    newest_file_name.newest_file = const_cast<char*>("Test_003_20200728_191132.dlt");
    newest_file_name.wrap_id = 0;
    newest_file_name.next = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_on_msg(&config, &file_config, path, 1, &newest_file_name));
}

TEST(t_dlt_logstorage_prepare_on_msg, normal2)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.working_file_name = NULL;
    config.wrap_id = 0;

    DltNewestFileName newest_file_name;
    newest_file_name.file_name = const_cast<char*>("Test");
    newest_file_name.newest_file = const_cast<char*>("Test_003_20200728_191132.dlt");
    newest_file_name.wrap_id = 1;
    newest_file_name.next = NULL;

    /* Create dummy file */
    char dummy_file[100] = "";
    sprintf(dummy_file, "%s/%s", path, newest_file_name.newest_file);
    int ret = 0;
    FILE *fp = fopen(dummy_file, "w");
    ret = ftruncate(fileno(fp), 1024);
    fclose(fp);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_on_msg(&config, &file_config, path, 1, &newest_file_name));

    if (ret == 0)
    {
        remove(dummy_file);
    }
}

TEST(t_dlt_logstorage_prepare_on_msg, normal3)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    char *working_file_name = const_cast<char*>("Test_002_20160509_191132.dlt");
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.working_file_name = strdup(working_file_name);
    config.wrap_id = 0;

    DltNewestFileName newest_file_name;
    newest_file_name.file_name = const_cast<char*>("Test");
    newest_file_name.newest_file = const_cast<char*>("Test_003_20200728_191132.dlt");
    newest_file_name.wrap_id = 1;
    newest_file_name.next = NULL;

    /* Create dummy file */
    char dummy_file[100] = "";
    sprintf(dummy_file, "%s/%s", path, newest_file_name.newest_file);
    int ret = 0;
    FILE *fp = fopen(dummy_file, "w");
    ret = ftruncate(fileno(fp), 1024);
    fclose(fp);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_on_msg(&config, &file_config, path, 1, &newest_file_name));

    if (ret == 0)
    {
        remove(dummy_file);
    }
}

TEST(t_dlt_logstorage_prepare_on_msg, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_prepare_on_msg(NULL, NULL, NULL, 0, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_write_on_msg*/
TEST(t_dlt_logstorage_write_on_msg, normal)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.working_file_name = NULL;
    config.wrap_id = 0;
    config.gzip_compression = 0;
    unsigned int size = 8;
    unsigned char data1[] = "dlt_data";
    unsigned char data2[] = "dlt_data";
    unsigned char data3[] = "dlt_data";

    DltNewestFileName newest_file_name;
    newest_file_name.file_name = const_cast<char*>("Test");
    newest_file_name.newest_file = const_cast<char*>("Test_003_20200728_191132.dlt");
    newest_file_name.wrap_id = 0;
    newest_file_name.next = NULL;

    char tmp_file[100] = "";

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_on_msg(&config, &file_config, path, 1, &newest_file_name));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write_on_msg(&config, &file_config, path,
              data1, size, data2, size, data3, size));
    sprintf(tmp_file, "%s/%s", path, config.working_file_name);
    remove(tmp_file);
}

#ifdef DLT_LOGSTORAGE_USE_GZIP
TEST(t_dlt_logstorage_write_on_msg, gzip)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 191132;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.working_file_name = NULL;
    config.wrap_id = 0;
    config.gzip_compression = 1;
    unsigned int size = 8;
    unsigned char data1[] = "dlt_data";
    unsigned char data2[] = "dlt_data";
    unsigned char data3[] = "dlt_data";

    DltNewestFileName newest_file_name;
    newest_file_name.file_name = const_cast<char*>("Test");
    newest_file_name.newest_file = (char *)"Test_003_20200728_191132.dlt.gz";
    newest_file_name.wrap_id = 0;
    newest_file_name.next = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_on_msg(&config, &file_config, path, 1, &newest_file_name));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write_on_msg(&config, &file_config, path,
              data1, size, data2, size, data3, size));
}
#endif

TEST(t_dlt_logstorage_write_on_msg, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_write_on_msg(NULL, NULL, NULL, NULL, 0, NULL, 0, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_sync_on_msg*/
TEST(t_dlt_logstorage_sync_on_msg, normal)
{
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.working_file_name = NULL;
    config.wrap_id = 0;
    char *path = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_sync_on_msg(&config, &file_config, path, DLT_LOGSTORAGE_SYNC_ON_MSG));
}

TEST(t_dlt_logstorage_sync_on_msg, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_sync_on_msg(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_prepare_msg_cache*/
TEST(t_dlt_logstorage_prepare_msg_cache, normal)
{
    DltLogStorageUserConfig file_config;
    file_config.logfile_timestamp = 1;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 2;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    DltNewestFileName newest_info;
    memset(&newest_info, 0, sizeof(DltNewestFileName));
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.cache = NULL;
    config.file_size = 0;
    config.sync = DLT_LOGSTORAGE_SYNC_ON_DEMAND;
    config.working_file_name = NULL;
    config.wrap_id = 0;
    g_logstorage_cache_max = 16;
    newest_info.file_name = const_cast<char*>("Test");
    newest_info.newest_file = const_cast<char*>("Test_003_20200728_191132.dlt");
    newest_info.wrap_id = 0;
    newest_info.next = NULL;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_msg_cache(&config, &file_config, path, 1, &newest_info));

    free(config.cache);
}

TEST(t_dlt_logstorage_prepare_msg_cache, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_prepare_msg_cache(NULL, NULL, NULL, 0, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_write_msg_cache*/
TEST(t_dlt_logstorage_write_msg_cache, normal)
{
    unsigned int size = 10;
    unsigned char data1[10] = "dlt_data1";
    unsigned char data2[10] = "dlt_data2";
    unsigned char data3[10] = "dlt_dat3";
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    DltLogStorageUserConfig file_config;
    char *path = const_cast<char*>("/tmp");

    config.cache = calloc(1, 50 + sizeof(DltLogStorageCacheFooter));

    if (config.cache != NULL) {
        config.file_size = 50;
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write_msg_cache(&config, &file_config, path,
                  data1, size, data2, size, data3, size));

        free(config.cache);
        config.cache = NULL;
    }
}

TEST(t_dlt_logstorage_write_msg_cache, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_write_msg_cache(NULL, NULL, NULL, NULL, 0, NULL, 0, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_split_key*/
TEST(t_dlt_logstorage_split_key, normal)
{
    char key[] = "dlt:1020:";
    char apid[] = ":2345:";
    char ctid[] = "::6789";
    char ecuid[] = "ECU1";

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_split_key(key, apid, ctid, ecuid));
}

TEST(t_dlt_logstorage_split_key, null)
{
    char key[] = "dlt:1020:";
    char apid[] = "2345";
    char ctid[] = "6789";
    char ecuid[] = "ECU1";
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_split_key(NULL, NULL, NULL, NULL));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_split_key(NULL, apid, ctid, ecuid));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_split_key(key, NULL, ctid, ecuid));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_split_key(key, apid, NULL, ecuid));
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_split_key(key, apid, ctid, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_update_all_contexts*/
TEST(t_dlt_logstorage_update_all_contexts, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));
    char ecu[] = "ECU1";
    char apid[] = "123";

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_update_all_contexts(&daemon, &daemon_local, apid, 1, 1, ecu, 0));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_update_all_contexts(&daemon, &daemon_local, apid, 0, 1, ecu, 0));
}

TEST(t_dlt_logstorage_update_all_contexts, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_update_all_contexts(NULL, NULL, NULL, 0, 0, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_update_context*/
TEST(t_dlt_logstorage_update_context, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));

    int fd = connectServer();
    EXPECT_NE(-1, fd);

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;

    char apid[] = "123";
    char ctid[] = "456";
    char desc[255] = "TEST dlt_logstorage_update_context";
    char ecu[] = "ECU1";

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));
    app = dlt_daemon_application_add(&daemon, apid, getpid(), desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon, apid, ctid, DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT, 0, app->user_handle, desc, daemon.ecuid, 0);
    EXPECT_NE((DltDaemonContext *)(NULL), daecontext);
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_update_context(&daemon, &daemon_local, apid, ctid, ecu, 1, 0));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_update_context(&daemon, &daemon_local, apid, ctid, ecu, 0, 0));
}

TEST(t_dlt_logstorage_update_context, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_update_context(NULL, NULL, NULL, NULL, NULL, 0, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_update_context_loglevel*/
TEST(t_dlt_logstorage_update_context_loglevel, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    DltDaemonContext *daecontext = NULL;
    DltDaemonApplication *app = NULL;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));

    int fd = connectServer();
    EXPECT_NE(-1, fd);

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;

    char apid[] = "123";
    char ctid[] = "456";
    char key[] = ":123:456";
    char desc[255] = "TEST dlt_logstorage_update_context_loglevel";
    char ecu[] = "ECU1";

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));
    app = dlt_daemon_application_add(&daemon, apid, getpid(), desc, fd, ecu, 0);
    daecontext = dlt_daemon_context_add(&daemon, apid, ctid, DLT_LOG_DEFAULT,
                                        DLT_TRACE_STATUS_DEFAULT, 0, app->user_handle, desc, daemon.ecuid, 0);
    EXPECT_NE((DltDaemonContext *)(NULL), daecontext);
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_update_context_loglevel
                  (&daemon, &daemon_local, key, 1, 0));
}

TEST(t_dlt_logstorage_update_context_loglevel, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_logstorage_update_context_loglevel(NULL, NULL, NULL, 0, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_reset_application_loglevel*/
TEST(t_dlt_daemon_logstorage_reset_application_loglevel, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(daemon_local));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;

    char ecu[] = "ECU1";
    int device_index = 0;

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));
    EXPECT_NO_THROW(dlt_daemon_logstorage_reset_application_loglevel(&daemon, &daemon_local, device_index, 1, 0));
}

TEST(t_dlt_daemon_logstorage_reset_application_loglevel, null)
{
    EXPECT_NO_THROW(dlt_daemon_logstorage_reset_application_loglevel(NULL, NULL, 0, 0, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_get_loglevel*/
TEST(t_dlt_daemon_logstorage_get_loglevel, normal)
{
    char ecu[] = "ECU1";
    char apid[] = "1234";
    char ctid[] = "5678";
    char file_name[] = "file_name";
    char key[] = "ECU1:1234:5678";
    int device_index = 0;
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));
    DltLogStorageFilterConfig value;
    memset(&value, 0, sizeof(DltLogStorageFilterConfig));
    value.log_level = 4;
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecu;
    value.file_name = file_name;
    DltLogStorage storage_handle;

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));

    daemon.storage_handle = &storage_handle;
    daemon.storage_handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    daemon.storage_handle->config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    daemon.storage_handle->config_list = NULL;
    daemon.storage_handle->num_configs = 1;
    int num_keys = 1;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_NO_THROW(dlt_daemon_logstorage_update_application_loglevel(&daemon, &daemon_local, device_index, 0));

    EXPECT_EQ(4, dlt_daemon_logstorage_get_loglevel(&daemon, 1, apid, ctid));
}

TEST(t_dlt_daemon_logstorage_get_loglevel, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_daemon_logstorage_get_loglevel(NULL, 0, NULL, NULL));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_update_application_loglevel*/
TEST(t_dlt_daemon_logstorage_update_application_loglevel, normal)
{
    char ecu[] = "key";
    char apid[] = "1234";
    char ctid[] = "5678";
    char file_name[] = "file_name";
    char key[] = "key:1234:5678";
    int device_index = 0;
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));
    DltLogStorageFilterConfig value;
    memset(&value, 0, sizeof(DltLogStorageFilterConfig));
    value.log_level = 5;
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecu;
    value.file_name = file_name;
    DltLogStorage storage_handle;

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));

    daemon.storage_handle = &storage_handle;
    daemon.storage_handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    daemon.storage_handle->config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    daemon.storage_handle->config_list = NULL;
    int num_keys = 1;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_NO_THROW(dlt_daemon_logstorage_update_application_loglevel(&daemon, &daemon_local, device_index, 0));
}

TEST(t_dlt_daemon_logstorage_update_application_loglevel, null)
{
    EXPECT_NO_THROW(dlt_daemon_logstorage_update_application_loglevel(NULL, NULL, 0, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_write*/
TEST(t_dlt_daemon_logstorage_write, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&gateway, 0, sizeof(DltGateway));
    char ecu[] = "ECU1";
    DltLogStorage storage_handle;

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    daemon.storage_handle = &storage_handle;
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char file_name[] = "file_name";
    DltDaemonFlags uconfig;
    uconfig.offlineLogstorageTimestamp = 1;
    uconfig.offlineLogstorageDelimiter = '/';
    uconfig.offlineLogstorageMaxCounter = 5;
    uconfig.offlineLogstorageMaxCounterIdx = 1;
    uconfig.offlineLogstorageMaxDevices = 1;
    unsigned char data[] = "123";
    int size = 3;
    daemon.storage_handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    daemon.storage_handle->config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    daemon.storage_handle->config_list = NULL;
    DltLogStorageFilterConfig value;
    memset(&value, 0, sizeof(DltLogStorageFilterConfig));
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecuid;
    value.file_name = file_name;
    char key0[] = ":1234:\000\000\000\000";
    char key1[] = "::5678\000\000\000\000";
    char key2[] = ":1234:5678";
    int num_keys = 1;

    DltMessage msg;
    DltUserHeader userheader;
    int log_level = 4;
    dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG);
    dlt_message_init(&msg, 0);
    msg.storageheader = (DltStorageHeader *)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader, ecuid);
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_PROTOCOL_VERSION1|DLT_HTYP_UEH;
    msg.standardheader->mcnt = 0;
    dlt_message_set_extraparameters(&msg, 0);
    msg.extendedheader = (DltExtendedHeader *)(msg.headerbuffer +
                         sizeof(DltStorageHeader) +
                         sizeof(DltStandardHeader) +
                         DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = (uint8_t)((DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) |
                               ((log_level << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN) | DLT_MSIN_VERB);
    msg.extendedheader->noar = 1;
    dlt_set_id(msg.extendedheader->apid, apid);
    dlt_set_id(msg.extendedheader->ctid, ctid);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_logstorage_write(&daemon, &uconfig,
                                             (unsigned char*)&(userheader), sizeof(DltUserHeader),
                                             msg.headerbuffer + sizeof(DltStorageHeader),
                                             (int)(msg.headersize - sizeof(DltStorageHeader)),
                                             data, size));
    dlt_message_free(&msg, 0);
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_write*/
TEST(t_dlt_daemon_logstorage_write_v2, normal)
{
    DltDaemon daemon;
    DltGateway gateway;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&gateway, 0, sizeof(DltGateway));
    char ecu[] = "ECU1";
    int ecu_len = (int)strlen(ecu);
    DltLogStorage storage_handle;

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 DLT_DAEMON_RINGBUFFER_MIN_SIZE,
                                 DLT_DAEMON_RINGBUFFER_MAX_SIZE,
                                 DLT_DAEMON_RINGBUFFER_STEP_SIZE,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));
    dlt_set_id_v2(daemon.ecuid2, ecu, (uint8_t)ecu_len);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &gateway, 0, 0));
    daemon.storage_handle = &storage_handle;
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char file_name[] = "file_name";
    DltDaemonFlags uconfig;
    uconfig.offlineLogstorageTimestamp = 1;
    uconfig.offlineLogstorageDelimiter = '/';
    uconfig.offlineLogstorageMaxCounter = 5;
    uconfig.offlineLogstorageMaxCounterIdx = 1;
    uconfig.offlineLogstorageMaxDevices = 1;
    unsigned char data[] = "123";
    int size = 3;
    daemon.storage_handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED;
    daemon.storage_handle->config_status = DLT_OFFLINE_LOGSTORAGE_CONFIG_DONE;
    daemon.storage_handle->config_list = NULL;
    DltLogStorageFilterConfig value;
    memset(&value, 0, sizeof(DltLogStorageFilterConfig));
    value.apids = apid;
    value.ctids = ctid;
    value.ecuid = ecuid;
    value.file_name = file_name;

    char key0[] = ":1234:\000\000\000\000";
    char key1[] = "::5678\000\000\000\000";
    char key2[] = ":1234:5678";
    int num_keys = 1;

    DltMessageV2 msg;
    DltUserHeader userheader;
    //int log_level = 4;
    dlt_user_set_userheader_v2(&userheader, DLT_USER_MESSAGE_LOG);
    dlt_message_init_v2(&msg, 0);
    size_t _hdrsz = 512;
    msg.headerbufferv2 = (uint8_t*)calloc(1, _hdrsz);
    msg.headersizev2 = (int)_hdrsz;
    msg.storageheadersizev2 = (int)sizeof(DltStorageHeaderV2);
    msg.baseheadersizev2 = (int)sizeof(DltBaseHeaderV2);
    msg.baseheaderextrasizev2 = 0;
    msg.baseheaderv2 = (DltBaseHeaderV2 *)(msg.headerbufferv2 + msg.storageheadersizev2);
    //msg.storageheaderv2 = (DltStorageHeaderV2 *)msg.headerbufferv2;
    // dlt_set_storageheader_v2(&(msg.storageheaderv2), ecuid_len, ecuid);
    msg.baseheaderv2 = (DltBaseHeaderV2 *)(msg.headerbufferv2 + sizeof(DltStorageHeaderV2));
    msg.baseheaderv2->htyp2 = DLT_HTYP2_PROTOCOL_VERSION2|DLT_HTYP2_EH;
    msg.baseheaderv2->mcnt = 0;
    dlt_message_set_extraparameters_v2(&msg, 0);
    /*msg.extendedheaderv2 = (DltExtendedHeaderV2 *)(msg.headerbufferv2 +
                         sizeof(DltStorageHeaderV2) +
                         sizeof(DltBaseHeaderV2) +
                         DLT_STANDARD_HEADER_EXTRA_SIZE(msg.baseheaderv2.htyp2));*/
    //dlt_set_id_v2(&(msg.extendedheaderv2.apid), apid, apid_len);
    //dlt_set_id_v2(&(msg.extendedheaderv2.ctid), ctid, ecuid_len);

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key0, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key1, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key2, num_keys, &value, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_logstorage_write(&daemon, &uconfig,
                                             (unsigned char*)&(userheader), sizeof(DltUserHeader),
                                             msg.headerbufferv2 + sizeof(DltStorageHeaderV2),
                                             (int)(msg.headersizev2 - (int32_t)sizeof(DltStorageHeaderV2)),
                                             data, size));
    dlt_message_free_v2(&msg, 0);
}

TEST(t_dlt_daemon_logstorage_write, null)
{
    EXPECT_EQ(-1, dlt_daemon_logstorage_write(NULL, NULL, NULL, 0, NULL, 0, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_setup_internal_storage*/
TEST(t_dlt_daemon_logstorage_setup_internal_storage, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    memset(&daemon, 0, sizeof(DltDaemon));
    memset(&daemon_local, 0, sizeof(DltDaemonLocal));
    memset(&daemon_local.pGateway, 0, sizeof(DltGateway));

    daemon_local.RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
    daemon_local.RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
    daemon_local.RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;
    char ecu[] = "ECU1";
    char path[] = "/tmp";

    EXPECT_EQ(0, dlt_daemon_init(&daemon,
                                 daemon_local.RingbufferMinSize,
                                 daemon_local.RingbufferMaxSize,
                                 daemon_local.RingbufferStepSize,
                                 DLT_RUNTIME_DEFAULT_DIRECTORY,
                                 DLT_LOG_INFO, DLT_TRACE_STATUS_OFF, 0, 0));

    dlt_set_id(daemon.ecuid, ecu);
    EXPECT_EQ(0, dlt_daemon_init_user_information(&daemon, &daemon_local.pGateway, 0, 0));
    DltLogStorage storage_handle;
    daemon.storage_handle = &storage_handle;
    daemon.storage_handle->config_status = 0;
    daemon.storage_handle->connection_type = DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED;
    daemon.storage_handle->config_list = NULL;
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_logstorage_setup_internal_storage(&daemon, &daemon_local, path, 1));
}

TEST(t_dlt_daemon_logstorage_setup_internal_storage, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_daemon_logstorage_setup_internal_storage(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_logstorage::dlt_daemon_logstorage_set_logstorage_cache_size*/
TEST(t_dlt_daemon_logstorage_set_logstorage_cache_size, normal)
{
    EXPECT_NO_THROW(dlt_daemon_logstorage_set_logstorage_cache_size(1));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_cleanup*/
TEST(t_dlt_daemon_logstorage_cleanup, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    daemon_local.flags.offlineLogstorageMaxDevices = 1;
    DltLogStorage storage_handle;
    memset(&storage_handle, 0, sizeof(DltLogStorage));
    daemon.storage_handle = &storage_handle;
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_logstorage_cleanup(&daemon, &daemon_local, 0));
}

TEST(t_dlt_daemon_logstorage_cleanup, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_daemon_logstorage_cleanup(NULL, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_sync_cache*/
TEST(t_dlt_daemon_logstorage_sync_cache, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    daemon_local.flags.offlineLogstorageMaxDevices = 1;
    DltLogStorage storage_handle;
    daemon.storage_handle = &storage_handle;
    daemon.storage_handle->config_status = 0;
    char path[] = "/tmp";
    char apid[] = "1234";
    char ctid[] = "5678";
    char ecuid[] = "12";
    char file_name[] = "file_name";
    char key[] = "12:1234:5678";
    daemon.storage_handle->num_configs = 1;
    daemon.storage_handle->config_list = NULL;
    strncpy(daemon.storage_handle->device_mount_point, "/tmp", 5);
    DltLogStorageFilterConfig configs = {};
    configs.apids = apid;
    configs.ctids = ctid;
    configs.ecuid = ecuid;
    configs.file_name = file_name;
    dlt_logstorage_filter_set_strategy(&configs, DLT_LOGSTORAGE_SYNC_ON_MSG);
    int num_keys = 1;

    EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_list_add(key, num_keys, &configs, &(daemon.storage_handle->config_list)));
    EXPECT_EQ(DLT_RETURN_OK, dlt_daemon_logstorage_sync_cache(&daemon, &daemon_local, path, 0));
}

TEST(t_dlt_daemon_logstorage_sync_cache, null)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_daemon_logstorage_sync_cache(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_daemon_logstorage_get_device*/
TEST(t_dlt_daemon_logstorage_get_device, normal)
{
    DltDaemon daemon;
    DltDaemonLocal daemon_local;
    daemon_local.flags.offlineLogstorageMaxDevices = 1;
    DltLogStorage storage_handle;
    daemon.storage_handle = &storage_handle;
    daemon.storage_handle->config_status = 0;
    char path[] = "/tmp";
    strncpy(daemon.storage_handle->device_mount_point, "/tmp", 5);

    EXPECT_NE((DltLogStorage *)NULL, dlt_daemon_logstorage_get_device(&daemon, &daemon_local, path, 0));
}

TEST(t_dlt_daemon_logstorage_get_device, null)
{
    EXPECT_EQ((DltLogStorage *)NULL, dlt_daemon_logstorage_get_device(NULL, NULL, NULL, 0));
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_find_dlt_header*/
TEST(t_dlt_logstorage_find_dlt_header, normal)
{
    char data[] = { 'a', 'b', 'D', 'L', 'T', 0x01 };
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.cache = calloc(1, sizeof(data));

    if (config.cache != NULL) {
        memcpy(config.cache, data, sizeof(data));
        /* DLT header starts from index 2 */
        EXPECT_EQ(2, dlt_logstorage_find_dlt_header(config.cache, 0, sizeof(data)));
        free(config.cache);
    }
}

TEST(t_dlt_logstorage_find_dlt_header, null)
{
    char data[] = { 'N', 'o', 'H', 'e', 'a', 'd', 'e', 'r' };
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.cache = calloc(1, sizeof(data));

    if (config.cache != NULL) {
        /* config.cache =(void *) "a,b,D,L,T,0x01"; */
        memcpy(config.cache, data, sizeof(data));
        EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_find_dlt_header(config.cache, 0, sizeof(data)));
        free(config.cache);
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_find_last_dlt_header*/
TEST(t_dlt_logstorage_find_last_dlt_header, normal)
{
    char data[] = {'a','b','D','L','T',0x01};
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.cache = calloc(1, sizeof(data));

    if (config.cache != NULL) {
        memcpy(config.cache, data, sizeof(data));

        /* DLT header starts from index 2 */
        EXPECT_EQ(2, dlt_logstorage_find_last_dlt_header(config.cache, 0, sizeof(data)));
        free(config.cache);
    }
}

TEST(t_dlt_logstorage_find_last_dlt_header, null)
{
    char data[] = {'N','o','H','e','a','d','e','r'};
    DltLogStorageFilterConfig config;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    config.cache = calloc(1, sizeof(data));
    if (config.cache != NULL)
    {
        /* config.cache =(void *) "a,b,D,L,T,0x01"; */
        memcpy(config.cache, data, sizeof(data));
        EXPECT_EQ(-1, dlt_logstorage_find_last_dlt_header(config.cache, 0, sizeof(data)));
        free(config.cache);
    }
}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_sync_to_file*/
TEST(t_dlt_logstorage_sync_to_file, normal)
{
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    file_config.logfile_timestamp = 0;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 6;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");
    DltLogStorageFilterConfig config;
    DltNewestFileName newest_info;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    memset(&newest_info, 0, sizeof(DltNewestFileName));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.cache = NULL;
    config.sync = DLT_LOGSTORAGE_SYNC_ON_DEMAND;
    config.num_files = 6;
    config.file_size = 50;
    g_logstorage_cache_max = 16;
    unsigned int size = 10;
    unsigned char data1[10] = {'a', 'b', 'D', 'L', 'T', 0x01 , 'c', 'd', 'e', 'f'};
    unsigned char data2[10] = "dlt_data1";
    unsigned char data3[10] = "dlt_data2";
    newest_info.wrap_id = 0;
    config.wrap_id = 0;
    DltLogStorageCacheFooter *footer = NULL;

    config.cache = calloc(1, config.file_size + sizeof(DltLogStorageCacheFooter));

    if (config.cache != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_msg_cache(&config, &file_config, path, 1, &newest_info));
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write_msg_cache(&config, &file_config, path,
                  data1, size, data2, size, data3, size));

        footer = (DltLogStorageCacheFooter *)((uint8_t*)config.cache + config.file_size);

        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_sync_to_file(&config, &file_config, path,
                  footer, footer->last_sync_offset, footer->offset));
        std::stringstream file_path;
        file_path << path << "/" << config.working_file_name;
        std::ifstream f(file_path.str());
        if (f.is_open())
        {
            std::string line;
            EXPECT_TRUE(std::getline(f, line));
            int idx = -2;
            // First 2 characters should not appear
            for (auto i=2; i < 10; i++)
                EXPECT_TRUE(line[i+idx] == data1[i]);
            idx += 10;
            for (auto i=0; i < 10; i++)
                EXPECT_TRUE(line[i+idx] == data2[i]);
            idx += 10;
            for (auto i=0; i < 10; i++)
                EXPECT_TRUE(line[i+idx] == data3[i]);
            f.close();
        }
        free(config.cache);
        config.cache = NULL;
        std::string tmp_file = file_path.str();
        remove(tmp_file.c_str());
    }
}

TEST(t_dlt_logstorage_sync_to_file, null)
{
    EXPECT_EQ(-1, dlt_logstorage_sync_to_file(NULL, NULL, NULL, NULL, 0, 1));

}

/* Begin Method: dlt_logstorage::t_dlt_logstorage_sync_msg_cache*/
TEST(t_dlt_logstorage_sync_msg_cache, normal)
{
    DltLogStorageUserConfig file_config;
    memset(&file_config, 0, sizeof(DltLogStorageUserConfig));
    file_config.logfile_timestamp = 0;
    file_config.logfile_delimiter = { '_' };
    file_config.logfile_maxcounter = 8;
    file_config.logfile_counteridxlen = 2;
    char *path = const_cast<char*>("/tmp");

    DltLogStorageFilterConfig config;
    DltNewestFileName newest_info;
    memset(&config, 0, sizeof(DltLogStorageFilterConfig));
    memset(&newest_info, 0, sizeof(DltNewestFileName));
    char apids;
    char ctids;
    config.apids = &apids;
    config.ctids = &ctids;
    config.file_name = const_cast<char*>("Test");
    config.records = NULL;
    config.log = NULL;
    config.cache = NULL;
    config.file_size = 50;
    config.sync = DLT_LOGSTORAGE_SYNC_ON_DEMAND;
    config.num_files = 8;
    g_logstorage_cache_max = 16;

    unsigned int size = 10;
    unsigned char data1[10] = {'a', 'b', 'D', 'L', 'T', 0x01 , 'c', 'd', 'e', 'f'};;
    unsigned char data2[10] = "dlt_dataB";
    unsigned char data3[10] = "dlt_dataC";

    config.cache = calloc(1, config.file_size + sizeof(DltLogStorageCacheFooter));

    if (config.cache != NULL) {
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_prepare_msg_cache(&config, &file_config, path, 1, &newest_info));
        EXPECT_EQ(DLT_RETURN_OK, dlt_logstorage_write_msg_cache(&config, &file_config, path, data1, size, data2, size, data3, size));
        EXPECT_EQ(DLT_RETURN_OK,
                  dlt_logstorage_sync_msg_cache(&config, &file_config, path, DLT_LOGSTORAGE_SYNC_ON_DEMAND));
        std::stringstream file_path;
        file_path << path << "/" << config.working_file_name;
        std::ifstream f(file_path.str());
        if (f.is_open())
        {
            std::string line;
            EXPECT_TRUE(std::getline(f, line));
            int idx = -2;
            // First 2 characters should not appear
            for (auto i=2; i < 10; i++)
                EXPECT_TRUE(line[i+idx] == data1[i]);
            idx += 10;
            for (auto i=0; i < 10; i++)
                EXPECT_TRUE(line[i+idx] == data2[i]);
            idx += 10;
            for (auto i=0; i < 10; i++)
                EXPECT_TRUE(line[i+idx] == data3[i]);
            f.close();
        }
        free(config.cache);
        config.cache = NULL;
        std::string tmp_file = file_path.str();
        remove(tmp_file.c_str());
    }
}

TEST(t_dlt_logstorage_sync_msg_cache, null)
{
    EXPECT_EQ(DLT_RETURN_ERROR, dlt_logstorage_sync_msg_cache(NULL, NULL, NULL, 0));
}

int connectServer(void)
{
#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = 8080;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname("127.0.0.1");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr,
           (char *)server->h_addr,
           server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: %s (%d) occured in connect socket\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
#else /* DLT_DAEMON_USE_FIFO_IPC */
    char filename[1024];
    int sockfd;
    snprintf(filename, 1024, "/tmp/dltpipes/dlt%d", getpid());
    /* Try to delete existing pipe, ignore result of unlink */
    unlink(filename);

    mkfifo(filename, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);
    chmod(filename, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);
    sockfd = open(filename, O_RDWR | O_CLOEXEC);
#endif

    return sockfd;
}

#define GTEST_SOCKS_ACCEPTED 2

int main(int argc, char **argv)
{
#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    pid_t cpid;
    cpid = fork();

    if (cpid == -1) {
        printf("fork fail\n");
        return -1;
    }

    if (!cpid) {
        int i = GTEST_SOCKS_ACCEPTED;
        int j, optval = 1;
        char buffer[256];
        int sockfd, newsockfd[GTEST_SOCKS_ACCEPTED], portno;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd == -1) {
            printf("Error in creating socket\n");
            return -1;
        }

        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        portno = 8080;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("setsockopt");
            close(sockfd);
            exit(1);
        }

        j = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

        if (j == -1) {
            perror("Bind Error\n");
            close(sockfd);
            return -1;
        }

        listen(sockfd, 5);

        while (i) {
            clilen = sizeof(cli_addr);
            newsockfd[i - 1] = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

            if (newsockfd[i - 1] == -1) {
                printf("Error in accept");
                return -1;
            }

            memset(buffer, 0, 256);
            (void)(read(newsockfd[i - 1], buffer, 255) + 1); /* just ignore result */
            i--;
        }

        for (j = 0; j < GTEST_SOCKS_ACCEPTED; j++)
            close(newsockfd[i]);

        close(sockfd);
    }
    else {
#endif
        ::testing::InitGoogleTest(&argc, argv);
        ::testing::FLAGS_gtest_break_on_failure = false;
/*        ::testing::FLAGS_gtest_filter = "t_dlt_event_handler_register_connection*"; */
        return RUN_ALL_TESTS();
#ifdef DLT_DAEMON_USE_UNIX_SOCKET_IPC
    }
#endif

    return 0;
}
