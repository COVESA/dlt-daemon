/**
 * Copyright (C) 2019-2022 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * dlt-logd-converter : Retrieve log entries from logd and forward them to DLT.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \Author Luu Quang Minh <Minh.LuuQuang@vn.bosch.com> ADIT 2022
 *
 * \file: gtest_dlt_logd_converter.cpp
 * For further information see http://www.covesa.org/.
 **/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: gtest_dlt_logd_converter.cpp                                  **
**                                                                            **
**  TARGET    : ANDROID                                                       **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Minh.LuuQuang@vn.bosch.com                                    **
**                                                                            **
**  PURPOSE   : Retrieve log entries from logd and forward them to DLT.       **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

#include "gtest/gtest.h"
#include "dlt-logd-converter.hpp"

/* MACRO */
#undef CONFIGURATION_FILE_DIR
#undef JSON_FILE_DIR
#define CONFIGURATION_FILE_DIR "dlt-logd-converter.conf"
#define JSON_FILE_DIR "dlt-logdctxt.json"
#define ABNORMAL_CONFIGURATION_FILE_DIR "abnormal-dlt-logd-converter.conf"

extern dlt_logd_configuration *logd_conf;
extern unordered_map<string, DltContext*> map_ctx_json;
extern bool json_is_available;
extern volatile sig_atomic_t exit_parser_loop;

DLT_IMPORT_CONTEXT(dlt_ctx_self);
DLT_IMPORT_CONTEXT(dlt_ctx_main);
DLT_IMPORT_CONTEXT(dlt_ctx_rdio);
DLT_IMPORT_CONTEXT(dlt_ctx_evnt);
DLT_IMPORT_CONTEXT(dlt_ctx_syst);
DLT_IMPORT_CONTEXT(dlt_ctx_crsh);
DLT_IMPORT_CONTEXT(dlt_ctx_stat);
DLT_IMPORT_CONTEXT(dlt_ctx_secu);
DLT_IMPORT_CONTEXT(dlt_ctx_krnl);
DLT_IMPORT_CONTEXT(dlt_ctx_othe);

struct logger_list *t_logger_list = nullptr;
struct dlt_log_container *dlt_log_data = nullptr;
struct log_msg t_log_msg;

string t_load_json_file()
{
    ifstream file(JSON_FILE_DIR);
    char *token;
    string pattern;
    string json_sequence;

    file.is_open();
    while (!file.eof()) {
        getline(file, pattern);
        if (pattern.size() == 0) {
            continue;
        }
        if (pattern[0] != '#') {
            token = strtok(&pattern[0], " {\":,}");
            while( token != NULL ) {
                if(strcmp(token, "tag") != 0 && strcmp(token, "description") != 0) {
                    json_sequence = json_sequence + strdup(token) + " ";
                }
                else {
                    if(pattern.find("\"\"") != string::npos) {
                        json_sequence = json_sequence + strdup("null") + " ";
                    }
                }
            token = strtok(NULL, " {\":,}");
            }
        }
    }
    file.close();
    return json_sequence;
}

struct logger *t_android_logger_open(struct logger_list *logger_list, log_id_t log_id)
{
    if (logger_list ==nullptr || (log_id >= LOG_ID_MAX)) {
        return nullptr;
    }
    logger_list->log_mask |= 1 << log_id;
    uintptr_t t_logger = log_id | LOGGER_LOGD;
    return reinterpret_cast<struct logger*>(t_logger);
}

struct logger_list *t_android_logger_list_alloc(int mode, unsigned int tail, pid_t pid)
{
    t_logger_list = new logger_list;
    t_logger_list->mode = mode;
    t_logger_list->tail = tail;
    t_logger_list->pid = pid;
    t_logger_list->log_mask = 0;

    return t_logger_list;
}

int t_android_logger_list_read(logger_list *logger_list, struct log_msg *t_log_msg)
{
    if (!t_log_msg->entry.len) {
        t_log_msg->entry.len = LOGGER_ENTRY_MAX_LEN;
    }
    if (!t_log_msg->entry.hdr_size) {
        t_log_msg->entry.hdr_size = 100;
    }
    if (!t_log_msg->entry.sec) {
        t_log_msg->entry.sec = 1/10000;
    }
    if (!t_log_msg->entry.nsec) {
        t_log_msg->entry.nsec = 100000;
    }
    if (!t_log_msg->entry.lid) {
        t_log_msg->entry.lid = LOG_ID_MAIN;
    }
    if (!t_log_msg->buf[t_log_msg->entry.hdr_size]) {
        t_log_msg->buf[t_log_msg->entry.hdr_size] = (unsigned char)ANDROID_LOG_INFO;
    }
    if (logger_list->signal == -EINVAL) {
        return -EINVAL;
    }
    if (logger_list->signal == -EAGAIN) {
        return -EAGAIN;
    }
    if (logger_list->signal == -EINTR) {
        return -EINTR;
    }
    if (logger_list->signal == -ENOMEM) {
        return -ENOMEM;
    }
    if (logger_list->signal == -ENODEV) {
        return -ENODEV;
    }
    if (logger_list->signal == -EIO) {
        return -EIO;
    }
    return logger_list->signal;
}

TEST(t_usage, normal)
{
    char version[255];
    dlt_get_version(version, 255);
    stringstream buffer;
    streambuf *prev_cout_buf = cout.rdbuf(buffer.rdbuf());

    usage(strdup("dlt-logd-converter"));

    EXPECT_NE(buffer.str().find("Usage: dlt-logd-converter [-h] [-c FILENAME]"),
            string::npos);
    EXPECT_NE(buffer.str().find("Application to manage Android logs."),
            string::npos);
    EXPECT_NE(buffer.str().find("Format and forward Android messages from ANDROID to DLT."),
            string::npos);
    EXPECT_NE(buffer.str().find(string(version)),
            string::npos);
    EXPECT_NE(buffer.str().find("Options:"),
            string::npos);
    EXPECT_NE(buffer.str().find(" -h           Display a short help text."),
            string::npos);
    EXPECT_NE(buffer.str().find(" -c filename  Use an alternative configuration file."),
            string::npos);
    EXPECT_NE(buffer.str().find("              Default: "),
            string::npos);
    EXPECT_NE(buffer.str().find("/vendor/etc/dlt-logd-converter.conf"),
            string::npos);
    cout.rdbuf(prev_cout_buf);
}

TEST(t_init_configuration, normal)
{
    int ret = init_configuration();

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_STREQ("LOGD", logd_conf->appID);
    EXPECT_STREQ("LOGF", logd_conf->ctxID);
    EXPECT_STREQ("/vendor/etc/dlt-logdctxt.json", logd_conf->json_file_dir);
    EXPECT_STREQ("OTHE", logd_conf->default_ctxID);
    EXPECT_STREQ("/vendor/etc/dlt-logd-converter.conf", logd_conf->conf_file_dir);
}

TEST(t_read_command_line, normal)
{
    char *arg[2];
    arg[0] = strdup("dlt-logd-converter");
    arg[1] = strdup("h");
    EXPECT_EQ(DLT_RETURN_OK, read_command_line(2, arg));

    char *t_arg[3];
    t_arg[0] = strdup("dlt-logd-converter");
    t_arg[1] = strdup("c");
    t_arg[2] = strdup("custom.conf");
    EXPECT_EQ(DLT_RETURN_OK, read_command_line(3, t_arg));
}

TEST(t_load_configuration_file, normal)
{
    logd_conf->appID = strdup("LOGD");
    logd_conf->ctxID = strdup("LOGF");
    logd_conf->json_file_dir = strdup(JSON_FILE_DIR);
    logd_conf->default_ctxID = strdup("OTHE");
    logd_conf->conf_file_dir = strdup(CONFIGURATION_FILE_DIR);
    int ret = load_configuration_file(CONFIGURATION_FILE_DIR);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_STREQ("LOGD", logd_conf->appID);
    EXPECT_STREQ("LOGF", logd_conf->ctxID);
    EXPECT_STREQ("/vendor/etc/dlt-logdctxt.json", logd_conf->json_file_dir);
    EXPECT_STREQ("OTHE", logd_conf->default_ctxID);
}

TEST(t_load_configuration_file, abnormal)
{
    logd_conf->appID = strdup("LOGD");
    logd_conf->ctxID = strdup("LOGF");
    logd_conf->json_file_dir = strdup(JSON_FILE_DIR);
    logd_conf->default_ctxID = strdup("OTHE");
    logd_conf->conf_file_dir = strdup(CONFIGURATION_FILE_DIR);
    int ret = load_configuration_file(ABNORMAL_CONFIGURATION_FILE_DIR);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_STREQ("DLOG", logd_conf->appID);
    EXPECT_STREQ("DLT", logd_conf->ctxID);
    EXPECT_STREQ("dlt-logdctxt.json", logd_conf->json_file_dir);
    EXPECT_STREQ("OTHE", logd_conf->default_ctxID);
}

TEST(t_load_configuration_file, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_ERROR, load_configuration_file(nullptr));
}

TEST(t_clean_mem, normal)
{
    logd_conf->appID = strdup("LOGD");
    logd_conf->ctxID = strdup("LOGF");
    logd_conf->json_file_dir = strdup(JSON_FILE_DIR);
    logd_conf->default_ctxID = strdup("OTHE");
    logd_conf->conf_file_dir = strdup(CONFIGURATION_FILE_DIR);

    json_is_available = true;
    string json_ctxID;
    string json_tag;
    string json_description;
    string str = t_load_json_file();
    char *token = strtok(&str[0], " ");
    while(token != nullptr) {
        json_ctxID = string(token);
        token = strtok(nullptr, " ");

        json_tag = string(token);
        token = strtok(nullptr, " ");

        json_description = string(token);
        if (json_description == "null") {
            json_description = "";
        }
        token = strtok(nullptr, " ");

        DltContext *ctx = new DltContext();
        auto ret = map_ctx_json.emplace(json_tag, ctx);
        if (!ret.second) {
            delete ctx;
            ctx = nullptr;
        }
    }

    clean_mem();

    EXPECT_TRUE(logd_conf == nullptr);
    EXPECT_TRUE(map_ctx_json.find("QtiVehicleHal") == map_ctx_json.end());
    EXPECT_TRUE(map_ctx_json.find("NetworkSecurityConfig") == map_ctx_json.end());
    EXPECT_TRUE(map_ctx_json.find("ProcessState") == map_ctx_json.end());
    EXPECT_TRUE(map_ctx_json.find("Zygote") == map_ctx_json.end());
}

TEST(t_json_parser, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, system("dlt-daemon -d > /dev/null"));
    EXPECT_EQ(DLT_RETURN_OK, system("sleep 0.2"));
    DLT_REGISTER_APP("LOGD", "logd -> dlt adapter");
    DLT_REGISTER_CONTEXT(dlt_ctx_self, "LOGF", "logd retriever");

    json_is_available = true;
    json_parser();

    EXPECT_TRUE(map_ctx_json.find("QtiVehicleHal") != map_ctx_json.end());
    EXPECT_TRUE(map_ctx_json.find("NetworkSecurityConfig") != map_ctx_json.end());
    EXPECT_TRUE(map_ctx_json.find("ProcessState") != map_ctx_json.end());
    EXPECT_TRUE(map_ctx_json.find("Zygote") != map_ctx_json.end());

    DLT_UNREGISTER_CONTEXT(dlt_ctx_self);
    for (auto &map_malloc: map_ctx_json) {
       DLT_UNREGISTER_CONTEXT(*(map_malloc.second));
       delete map_malloc.second;
       map_malloc.second = nullptr;
    }
    map_ctx_json.clear();

    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
    EXPECT_LT(DLT_RETURN_OK, system("kill -9 $(pgrep -f \"dlt-daemon -d\") > /dev/null"));
}

TEST(t_find_tag_in_json, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, system("dlt-daemon -d > /dev/null"));
    EXPECT_EQ(DLT_RETURN_OK, system("sleep 0.2"));

    string json_ctxID;
    string json_tag;
    string json_description;
    string str = t_load_json_file();
    char *token = strtok(&str[0], " ");
    while(token != nullptr) {
        json_ctxID = string(token);
        token = strtok(nullptr, " ");

        json_tag = string(token);
        token = strtok(nullptr, " ");

        json_description = string(token);
        if (json_description == "null") {
            json_description = "";
        }
        token = strtok(nullptr, " ");

        DltContext *ctx = new DltContext();
        auto ret = map_ctx_json.emplace(json_tag, ctx);
        if (!ret.second) {
            delete ctx;
            ctx = nullptr;
        }
    }

    EXPECT_EQ(find_tag_in_json("QtiVehicleHal"),
            (map_ctx_json.find("QtiVehicleHal")->second));
    EXPECT_EQ(find_tag_in_json("NetworkSecurityConfig"),
            (map_ctx_json.find("NetworkSecurityConfig")->second));
    EXPECT_EQ(find_tag_in_json("ProcessState"),
            (map_ctx_json.find("ProcessState")->second));
    EXPECT_EQ(find_tag_in_json("Zygote"),
            (map_ctx_json.find("Zygote")->second));
    EXPECT_EQ(find_tag_in_json("Other tags"), &(dlt_ctx_othe));

    for (auto &map_malloc: map_ctx_json) {
       delete map_malloc.second;
       map_malloc.second = nullptr;
    }
    map_ctx_json.clear();

    EXPECT_LT(DLT_RETURN_OK, system("kill -9 $(pgrep -f \"dlt-daemon -d\") > /dev/null"));
}

TEST(t_find_tag_in_json, nullpointer)
{
    (void)(::testing::GTEST_FLAG(death_test_style) = "threadsafe");
    ASSERT_ANY_THROW(find_tag_in_json(nullptr));
}

TEST(t_init_logger, normal)
{
    t_logger_list = new logger_list;
    t_logger_list->mode = READ_ONLY;
    t_logger_list->tail = 0;
    t_logger_list->pid = 0;

    struct logger *logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_MAIN | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_MAIN));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_RADIO | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_RADIO));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_EVENTS | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_EVENTS));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_SYSTEM | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_SYSTEM));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_CRASH | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_CRASH));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_STATS | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_STATS));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_SECURITY | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_SECURITY));
    logger_ptr = reinterpret_cast<struct logger*>(LOG_ID_KERNEL | LOGGER_LOGD);
    EXPECT_EQ(logger_ptr, init_logger(t_logger_list, LOG_ID_KERNEL));
    delete t_logger_list;
    t_logger_list = nullptr;
}

TEST(t_init_logger, nullpointer)
{
    EXPECT_EQ(nullptr, t_logger_list);
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_MAIN));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_RADIO));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_EVENTS));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_SYSTEM));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_CRASH));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_STATS));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_SECURITY));
    EXPECT_EQ(nullptr, init_logger(t_logger_list, LOG_ID_KERNEL));
}

TEST(t_init_logger_list, normal)
{
    EXPECT_EQ(0x009B, (init_logger_list(true)->log_mask) & 0x00FF);
    EXPECT_EQ(0x00FF, (init_logger_list(false)->log_mask) & 0x00FF);
    delete t_logger_list;
    t_logger_list = nullptr;
}

TEST(t_get_log_context_from_log_msg, normal)
{
    struct log_msg *t_log_msg = nullptr;
    t_log_msg = new log_msg;
    t_log_msg->entry.lid = LOG_ID_MAIN;
    EXPECT_EQ(&dlt_ctx_main, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_RADIO;
    EXPECT_EQ(&dlt_ctx_rdio, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_EVENTS;
    EXPECT_EQ(&dlt_ctx_evnt, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_SYSTEM;
    EXPECT_EQ(&dlt_ctx_syst, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_CRASH;
    EXPECT_EQ(&dlt_ctx_crsh, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_STATS;
    EXPECT_EQ(&dlt_ctx_stat, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_SECURITY;
    EXPECT_EQ(&dlt_ctx_secu, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = LOG_ID_KERNEL;
    EXPECT_EQ(&dlt_ctx_krnl, get_log_context_from_log_msg(t_log_msg));
    t_log_msg->entry.lid = 1024;
    EXPECT_EQ(&dlt_ctx_self, get_log_context_from_log_msg(t_log_msg));
    delete t_log_msg;
    t_log_msg = nullptr;
}

TEST(t_get_log_context_from_log_msg, nullpointer)
{
    struct log_msg *t_log_msg = nullptr;
    EXPECT_EQ(&dlt_ctx_self, get_log_context_from_log_msg(t_log_msg));
}

TEST(t_get_timestamp_from_log_msg, normal)
{
    struct log_msg *t_log_msg = nullptr;
    t_log_msg = new log_msg;
    t_log_msg->entry.sec = 0;
    t_log_msg->entry.nsec = 0;
    EXPECT_EQ(0, get_timestamp_from_log_msg(t_log_msg));
    t_log_msg->entry.sec = 1/10000;
    t_log_msg->entry.nsec = 100000;
    EXPECT_EQ(1, get_timestamp_from_log_msg(t_log_msg));
    t_log_msg->entry.sec = 100;
    t_log_msg->entry.nsec = 1000000;
    EXPECT_EQ(1000010, get_timestamp_from_log_msg(t_log_msg));
}

TEST(t_get_timestamp_from_log_msg, nullpointer)
{
    struct log_msg *t_log_msg = nullptr;
    EXPECT_EQ(DLT_FAIL_TO_GET_LOG_MSG, get_timestamp_from_log_msg(t_log_msg));
}

TEST(t_get_log_level_from_log_msg, normal)
{
    struct log_msg *t_log_msg = nullptr;
    t_log_msg = new log_msg;
    t_log_msg->entry.hdr_size = 0;
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_VERBOSE;
    EXPECT_EQ(DLT_LOG_VERBOSE, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_DEBUG;
    EXPECT_EQ(DLT_LOG_DEBUG, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_INFO;
    EXPECT_EQ(DLT_LOG_INFO, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_WARN;
    EXPECT_EQ(DLT_LOG_WARN, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_ERROR;
    EXPECT_EQ(DLT_LOG_ERROR, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_FATAL;
    EXPECT_EQ(DLT_LOG_FATAL, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_SILENT;
    EXPECT_EQ(DLT_LOG_OFF, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_UNKNOWN;
    EXPECT_EQ(DLT_LOG_DEFAULT, get_log_level_from_log_msg(t_log_msg));
    t_log_msg->buf[0] = (unsigned char)ANDROID_LOG_DEFAULT;
    EXPECT_EQ(DLT_LOG_DEFAULT, get_log_level_from_log_msg(t_log_msg));
    delete t_log_msg;
    t_log_msg = nullptr;
}

TEST(t_get_log_level_from_log_msg, nullpointer)
{
    struct log_msg *t_log_msg = nullptr;
    EXPECT_EQ(DLT_LOG_DEFAULT, get_log_level_from_log_msg(t_log_msg));
}

TEST(t_signal_handler, normal)
{
    exit_parser_loop = false;
    signal_handler(SIGTERM);
    EXPECT_TRUE(exit_parser_loop == true);
    exit_parser_loop = false;
    signal_handler(SIGSEGV);
    EXPECT_TRUE(exit_parser_loop == false);
}

TEST(t_logd_parser_loop, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, system("dlt-daemon -d > /dev/null"));
    EXPECT_EQ(DLT_RETURN_OK, system("sleep 0.2"));

    struct logger_list *t_list = nullptr;
    t_list = new logger_list;
    t_list->mode = READ_ONLY;
    t_list->tail = 0;
    t_list->pid = 0;

    t_list->signal = -EINVAL;
    EXPECT_TRUE(logd_parser_loop(t_list) == -EINVAL);
    t_list->signal = -EAGAIN;
    EXPECT_TRUE(logd_parser_loop(t_list) == -EAGAIN);
    t_list->signal = -EINTR;
    EXPECT_TRUE(logd_parser_loop(t_list) == -EINTR);
    t_list->signal = -ENOMEM;
    EXPECT_TRUE(logd_parser_loop(t_list) == -ENOMEM);
    t_list->signal = -ENODEV;
    EXPECT_TRUE(logd_parser_loop(t_list) == -ENODEV);
    t_list->signal = -EIO;
    EXPECT_TRUE(logd_parser_loop(t_list) == -EIO);

    delete t_list;
    t_list = nullptr;

    t_logger_list = new logger_list;
    t_logger_list->mode = READ_ONLY;
    t_logger_list->tail = 0;
    t_logger_list->pid = 0;
    t_logger_list->signal = 0;

    json_is_available = true;
    string json_ctxID;
    string json_tag;
    string json_description;
    string str = t_load_json_file();
    char *token = strtok(&str[0], " ");
    while(token != nullptr) {
        json_ctxID = string(token);
        token = strtok(nullptr, " ");

        json_tag = string(token);
        token = strtok(nullptr, " ");

        json_description = string(token);
        if (json_description == "null") {
            json_description = "";
        }
        token = strtok(nullptr, " ");

        DltContext *ctx = new DltContext();
        auto ret = map_ctx_json.emplace(json_tag, ctx);
        if (!ret.second) {
            delete ctx;
            ctx = nullptr;
        }
    }

    dlt_log_data = new dlt_log_container;
    json_is_available = true;

    /* TEST with available tag in JSON file */
    unsigned char tag[sizeof("QtiVehicleHal")] = "QtiVehicleHal";
    unsigned char message[sizeof("dlt-logd-converter")] = "dlt-logd-converter";
    for (uint idx = 0; idx < sizeof(tag); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + 1] = tag[idx];
    }
    for (uint idx = 0; idx < sizeof(message); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + sizeof(tag) + 1] = message[idx];
    }

    auto search = map_ctx_json.find("QtiVehicleHal");
    int ret = logd_parser_loop(t_logger_list);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(search->second, dlt_log_data->ctx);
    EXPECT_EQ(DLT_LOG_INFO, dlt_log_data->log_level);
    EXPECT_EQ(1, dlt_log_data->ts);
    EXPECT_STREQ("QtiVehicleHal", dlt_log_data->tag);
    EXPECT_STREQ("dlt-logd-converter", dlt_log_data->message);

    /* TEST with other tags not listed JSON file */
    unsigned char othe_tag[sizeof("OtherTags")] = "OtherTags";
    unsigned char othe_message[sizeof("No tag found")] = "No tag found";
    for (uint idx = 0; idx < sizeof(othe_tag); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + 1] = othe_tag[idx];
    }
    for (uint idx = 0; idx < sizeof(othe_message); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + sizeof(othe_tag) + 1] = othe_message[idx];
    }

    search = map_ctx_json.find("OtherTags");
    ret = logd_parser_loop(t_logger_list);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(&(dlt_ctx_othe), dlt_log_data->ctx);
    EXPECT_EQ(DLT_LOG_INFO, dlt_log_data->log_level);
    EXPECT_EQ(1, dlt_log_data->ts);
    EXPECT_STREQ("OtherTags", dlt_log_data->tag);
    EXPECT_STREQ("No tag found", dlt_log_data->message);

    /* TEST with another buffer */
    t_log_msg.entry.lid = LOG_ID_RADIO;
    unsigned char radio_tag[sizeof("RadioTag")] = "RadioTag";
    unsigned char radio_message[sizeof("It is from radio buffer")] = "It is from radio buffer";
    for (uint idx = 0; idx < sizeof(radio_tag); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + 1] = radio_tag[idx];
    }
    for (uint idx = 0; idx < sizeof(radio_message); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + sizeof(radio_tag) + 1] = radio_message[idx];
    }

    ret = logd_parser_loop(t_logger_list);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(&(dlt_ctx_rdio), dlt_log_data->ctx);
    EXPECT_EQ(DLT_LOG_INFO, dlt_log_data->log_level);
    EXPECT_EQ(1, dlt_log_data->ts);
    EXPECT_STREQ("RadioTag", dlt_log_data->tag);
    EXPECT_STREQ("It is from radio buffer", dlt_log_data->message);

    /* TEST with no JSON file */
    t_log_msg.entry.lid = LOG_ID_KERNEL;
    json_is_available = false;
    unsigned char no_json_tag[sizeof("Kernel")] = "Kernel";
    unsigned char no_json_message[sizeof("It is from kernel")] = "It is from kernel";
    for (uint idx = 0; idx < sizeof(no_json_tag); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + 1] = no_json_tag[idx];
    }
    for (uint idx = 0; idx < sizeof(no_json_message); idx++) {
        t_log_msg.buf[idx + t_log_msg.entry.hdr_size + sizeof(no_json_tag) + 1] = no_json_message[idx];
    }

    ret = logd_parser_loop(t_logger_list);

    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(&(dlt_ctx_krnl), dlt_log_data->ctx);
    EXPECT_EQ(DLT_LOG_INFO, dlt_log_data->log_level);
    EXPECT_EQ(1, dlt_log_data->ts);
    EXPECT_STREQ("Kernel", dlt_log_data->tag);
    EXPECT_STREQ("It is from kernel", dlt_log_data->message);

    delete dlt_log_data;
    dlt_log_data = nullptr;
    delete t_logger_list;
    t_logger_list = nullptr;

    EXPECT_LT(DLT_RETURN_OK, system("kill -9 $(pgrep -f \"dlt-daemon -d\") > /dev/null"));
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
