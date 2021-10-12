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
 * \file: dlt-logd-converter.cpp
 * For further information see http://www.covesa.org/.
 **/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logd-converter.cpp                                        **
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

#include "dlt-logd-converter.hpp"

DLT_DECLARE_CONTEXT(dlt_ctx_self)
DLT_DECLARE_CONTEXT(dlt_ctx_main)
DLT_DECLARE_CONTEXT(dlt_ctx_rdio)
DLT_DECLARE_CONTEXT(dlt_ctx_evnt) /* Binary Buffer */
DLT_DECLARE_CONTEXT(dlt_ctx_syst)
DLT_DECLARE_CONTEXT(dlt_ctx_crsh)
DLT_DECLARE_CONTEXT(dlt_ctx_stat) /* Binary Buffer */
DLT_DECLARE_CONTEXT(dlt_ctx_secu) /* Binary Buffer */
DLT_DECLARE_CONTEXT(dlt_ctx_krnl)
DLT_DECLARE_CONTEXT(dlt_ctx_othe)

/* Global variables */
DLT_STATIC dlt_logd_configuration *logd_conf = nullptr;
volatile sig_atomic_t exit_parser_loop = false;
DLT_STATIC unordered_map<string, DltContext*> map_ctx_json;
bool json_is_available = false;

/**
 * Print manual page for instruction.
 */
DLT_STATIC void usage(char *prog_name)
{
    char version[255];
    dlt_get_version(version, 255);

    cout << "Usage: " << prog_name << " [-h] [-c FILENAME]" << endl;
    cout << "Application to manage Android logs." << endl;
    cout << "Format and forward Android messages from ANDROID to DLT." << endl;
    cout << version << endl;
    cout << "Options:" << endl;
    cout << " -h           Display a short help text." << endl;
    cout << " -c filename  Use an alternative configuration file." << endl;
    cout << "              Default: " << CONFIGURATION_FILE_DIR << endl;
}

/**
 * Initialize configuration to default values.
 */
DLT_STATIC int init_configuration()
{
    logd_conf = new dlt_logd_configuration;
    if (logd_conf == nullptr) {
        cerr << "Fail to allocate, out of memory!" << endl;
        return -1;
    }
    logd_conf->appID = strdup("LOGD");
    logd_conf->ctxID = strdup("LOGF");
    logd_conf->json_file_dir = strdup(JSON_FILE_DIR);
    logd_conf->default_ctxID = strdup("OTHE");
    logd_conf->conf_file_dir = strdup(CONFIGURATION_FILE_DIR);
    return 0;
}

/**
 * Read command line options and set the values in provided structure
 */
DLT_STATIC int read_command_line(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "hc:")) != -1) {
        switch (opt) {
            case 'h':
            {
                usage(argv[0]);
                exit(0);
                return -1;
            }
            case 'c':
            {
                if (logd_conf->conf_file_dir) {
                    delete logd_conf->conf_file_dir;
                    logd_conf->conf_file_dir = nullptr;
                }
                logd_conf->conf_file_dir = new char [strlen(optarg)+1];
                strcpy(logd_conf->conf_file_dir, optarg);
                break;
            }
            default:
            {
                usage(argv[0]);
                return -1;
            }
        }
    }
    return 0;
}

/**
 * Read options from the configuration file
 */
DLT_STATIC int load_configuration_file(const char *file_name)
{
    ifstream file(file_name);
    char *token;
    string pattern;

    if (!file.is_open()) {
        return -1;
    }

    while (!file.eof()) {
        getline(file, pattern);
        if (pattern.size() == 0) {
            continue;
        }
        if (pattern[0] != '#') {
            token = strtok(&pattern[0], " \t=");
            if (strcmp(token, "ApplicationID") == 0) {
                token = strtok(NULL, " \t=");
                if (token != nullptr) {
                    if (logd_conf->appID) {
                        delete logd_conf->appID;
                    }
                    logd_conf->appID = strndup(token, DLT_ID_SIZE);
                }
            }
            else if (strcmp(token, "ContextID") == 0) {
                token = strtok(NULL, " \t=");
                if (token != nullptr) {
                    if (logd_conf->ctxID) {
                        delete logd_conf->ctxID;
                    }
                    logd_conf->ctxID = strndup(token, DLT_ID_SIZE);
                }
            }
            else if (strcmp(token, "AndroidLogdJSONpath") == 0) {
                token = strtok(NULL, " \t=");
                if (token != nullptr) {
                    if (logd_conf->json_file_dir) {
                        delete logd_conf->json_file_dir;
                    }
                    logd_conf->json_file_dir = strndup(token, MAX_LINE);
                }
            }
            else if (strcmp(token, "AndroidLogdContextID") == 0) {
                token = strtok(NULL, " \t=");
                if (token != nullptr) {
                    if (logd_conf->default_ctxID) {
                        delete logd_conf->default_ctxID;
                    }
                    logd_conf->default_ctxID = strndup(token, DLT_ID_SIZE);
                }
            }
        }
    }
    file.close();
    return 0;
}

DLT_STATIC void clean_mem()
{
    if (logd_conf->appID) {
        delete logd_conf->appID;
        logd_conf->appID = nullptr;
    }
    if (logd_conf->ctxID) {
        delete logd_conf->ctxID;
        logd_conf->ctxID = nullptr;
    }
    if (logd_conf->json_file_dir) {
        delete logd_conf->json_file_dir;
        logd_conf->json_file_dir = nullptr;
    }
    if (logd_conf->default_ctxID) {
        delete logd_conf->default_ctxID;
        logd_conf->default_ctxID = nullptr;
    }
    if (logd_conf->conf_file_dir) {
        delete logd_conf->conf_file_dir;
        logd_conf->conf_file_dir = nullptr;
    }
    if (logd_conf) {
        delete logd_conf;
        logd_conf = nullptr;
    }
    if (json_is_available) {
        for (auto &map_malloc: map_ctx_json) {
            delete map_malloc.second;
            map_malloc.second = nullptr;
        }
        map_ctx_json.clear();
    }
}

/**
 * Parses data from a json file into an internal data
 * structure and do registration with the new ctxID.
 */
DLT_STATIC void json_parser()
{
#ifndef DLT_UNIT_TESTS
    Json::Value console = Json::nullValue;
    Json::CharReaderBuilder builder;
    string errs;
    ifstream file(logd_conf->json_file_dir);
    if (parseFromStream(builder, file, &console, &errs)) {
        json_is_available = true;
    }

    if (json_is_available) {
        Json::Value::iterator iter;
        DLT_REGISTER_CONTEXT(dlt_ctx_othe, logd_conf->default_ctxID, "");

        for (iter = console.begin(); iter != console.end(); ++iter) {
            string json_ctxID = iter.key().asString();
            string json_tag = (*iter)["tag"].asString();
            string json_description = (*iter)["description"].asString();
#else
    if (json_is_available) {
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
#endif
            DltContext *ctx = new DltContext();
            auto ret = map_ctx_json.emplace(json_tag, ctx);
            if (!ret.second) {
                DLT_LOG(dlt_ctx_self, DLT_LOG_WARN,
                        DLT_STRING(json_tag.c_str()),
                        DLT_STRING("is duplicated, please check the json file."));
                delete ctx;
                ctx = nullptr;
            }
            else {
                DLT_REGISTER_CONTEXT(*(ret.first->second),
                                    json_ctxID.c_str(),
                                    json_description.c_str());
            }
#ifdef DLT_UNIT_TESTS
        }
    }
#else
        }
    }
    file.close();
#endif
}

/**
 * Doing tag matching in a loop from first
 * elementof json vector to the end of the list.
 */
DLT_STATIC DltContext* find_tag_in_json(const char *tag)
{
    string tag_str(tag);
    auto search = map_ctx_json.find(tag_str);
    if (search == map_ctx_json.end()) {
        DLT_LOG(dlt_ctx_self, DLT_LOG_VERBOSE,
                DLT_STRING(tag),
                DLT_STRING("could not be found. Apply default contextID:"),
                DLT_STRING(logd_conf->default_ctxID));
        return &(dlt_ctx_othe);
    }
    else {
         DLT_LOG(dlt_ctx_self, DLT_LOG_VERBOSE,
                DLT_STRING("Tag found and applied:"),
                DLT_STRING(tag));
        return search->second;
    }
}

DLT_STATIC struct logger *init_logger(struct logger_list *logger_list, log_id_t log_id)
{
    struct logger *logger;
#ifndef DLT_UNIT_TESTS
    logger = android_logger_open(logger_list, log_id);
    if (logger == nullptr) {
        DLT_LOG(dlt_ctx_self, DLT_LOG_WARN,
                DLT_STRING("Could not open logd buffer ID = "), DLT_INT64(log_id));
    }
#else
    logger = t_android_logger_open(logger_list, log_id);
#endif
    return logger;
}

DLT_STATIC struct logger_list *init_logger_list(bool skip_binary_buffers)
{
    struct logger_list *logger_list;
#ifndef DLT_UNIT_TESTS
    logger_list = android_logger_list_alloc(O_RDONLY, 0, 0);
    if (logger_list == nullptr) {
        DLT_LOG(dlt_ctx_self, DLT_LOG_FATAL, DLT_STRING("Could not allocate logger list"));
        return nullptr;
    }
#else
    logger_list = t_android_logger_list_alloc(READ_ONLY, 0, 0);
    if (logger_list == nullptr) {
        return nullptr;
    }
#endif
    /**
     * logd buffer types are defined in:
     * system/core/include/log/android/log.h
     */
    init_logger(logger_list, LOG_ID_MAIN);
    init_logger(logger_list, LOG_ID_RADIO);
    init_logger(logger_list, LOG_ID_SYSTEM);
    init_logger(logger_list, LOG_ID_KERNEL);
    init_logger(logger_list, LOG_ID_CRASH);

    if (!skip_binary_buffers) {
        init_logger(logger_list, LOG_ID_EVENTS);
        init_logger(logger_list, LOG_ID_STATS);
        init_logger(logger_list, LOG_ID_SECURITY);
    }

    return logger_list;
}

DLT_STATIC DltContext *get_log_context_from_log_msg(struct log_msg *log_msg)
{
    if (log_msg) {
        switch (log_msg->id()) {
        case LOG_ID_MAIN:
            return &dlt_ctx_main;
        case LOG_ID_RADIO:
            return &dlt_ctx_rdio;
        case LOG_ID_EVENTS:
            return &dlt_ctx_evnt;
        case LOG_ID_SYSTEM:
            return &dlt_ctx_syst;
        case LOG_ID_CRASH:
            return &dlt_ctx_crsh;
        case LOG_ID_STATS:
            return &dlt_ctx_stat;
        case LOG_ID_SECURITY:
            return &dlt_ctx_secu;
        case LOG_ID_KERNEL:
            return &dlt_ctx_krnl;
        default:
            return &dlt_ctx_self;
        }
    }
    else {
        return &dlt_ctx_self;
    }
}

DLT_STATIC uint32_t get_timestamp_from_log_msg(struct log_msg *log_msg)
{
    if (log_msg) {
        /* in 0.1 ms = 100 us */
        return (uint32_t)log_msg->entry.sec * 10000 + (uint32_t)log_msg->entry.nsec / 100000;
    }
    else {
        DLT_LOG(dlt_ctx_self, DLT_LOG_WARN, DLT_STRING("Could not receive any log message"));
        return (uint32_t)DLT_FAIL_TO_GET_LOG_MSG;
    }
}

DLT_STATIC DltLogLevelType get_log_level_from_log_msg(struct log_msg *log_msg)
{
    if (log_msg) {
        android_LogPriority priority = static_cast<android_LogPriority>(log_msg->msg()[0]);
        switch (priority) {
        case ANDROID_LOG_VERBOSE:
            return DLT_LOG_VERBOSE;
        case ANDROID_LOG_DEBUG:
            return DLT_LOG_DEBUG;
        case ANDROID_LOG_INFO:
            return DLT_LOG_INFO;
        case ANDROID_LOG_WARN:
            return DLT_LOG_WARN;
        case ANDROID_LOG_ERROR:
            return DLT_LOG_ERROR;
        case ANDROID_LOG_FATAL:
            return DLT_LOG_FATAL;
        case ANDROID_LOG_SILENT:
            return DLT_LOG_OFF;
        case ANDROID_LOG_UNKNOWN:
        case ANDROID_LOG_DEFAULT:
        default:
            return DLT_LOG_DEFAULT;
        }
    }
    else {
        DLT_LOG(dlt_ctx_self, DLT_LOG_WARN, DLT_STRING("Could not receive any log message"));
        return DLT_LOG_DEFAULT;
    }
}

void signal_handler(int signal)
{
    (void) signal;
    if (signal == SIGTERM) {
        exit_parser_loop = true;
    }
}

DLT_STATIC int logd_parser_loop(struct logger_list *logger_list)
{
    int ret;
    DltContext *ctx = nullptr;
#ifndef DLT_UNIT_TESTS
    struct log_msg log_msg;
    DLT_LOG(dlt_ctx_self, DLT_LOG_VERBOSE, DLT_STRING("Entering parsing loop"));

    while (!exit_parser_loop) {
        ret = android_logger_list_read(logger_list, &log_msg);
        if (ret == -EAGAIN || ret == -EINTR) {
            if (exit_parser_loop == true) {
                break;
            }
            continue;
        }
        else if (ret == -EINVAL || ret == -ENOMEM || ret == -ENODEV || ret == -EIO) {
            DLT_LOG(dlt_ctx_self, DLT_LOG_FATAL,
                    DLT_STRING("Could not retrieve logs, permanent error="), DLT_INT32(ret));
            return ret;
        }
        else if (ret <= 0) {
            DLT_LOG(dlt_ctx_self, DLT_LOG_ERROR,
                    DLT_STRING("android_logger_list_read unexpected return="), DLT_INT32(ret));
            return ret;
        }
#else
        extern struct dlt_log_container *dlt_log_data;
        extern struct log_msg t_log_msg;
        struct log_msg &log_msg = t_log_msg;
        ret = t_android_logger_list_read(logger_list, &log_msg);
            if (ret == -EAGAIN || ret == -EINTR ||
                ret == -EINVAL || ret == -ENOMEM ||
                ret == -ENODEV || ret == -EIO) {
        return ret;
        }
#endif
        /* Look into system/core/liblog/logprint.c for buffer format.
           "<priority:1><tag:N>\0<message:N>\0" */
        const char *tag = "";
        const char *message = "";
        if (log_msg.entry.len > 1) {
            tag = log_msg.msg() + 1;
        }
        if (log_msg.entry.len > 1 + strlen(tag) + 1) {
            message = tag + strlen(tag) + 1;
        }

        /* Find tag in JSON file, apply new contextID if available */
        if ((log_msg.id() == LOG_ID_MAIN) && (json_is_available)) {
            ctx = find_tag_in_json(tag);
        }
        else {
            ctx = get_log_context_from_log_msg(&log_msg);
        }

        DltLogLevelType log_level = get_log_level_from_log_msg(&log_msg);

        uint32_t ts = get_timestamp_from_log_msg(&log_msg);

#ifndef DLT_UNIT_TESTS
        /* Binary buffers are not supported by DLT_STRING DLT_RAW would need the message length */
        DLT_LOG_TS(*ctx, log_level, ts,
                    DLT_STRING(tag),
                    DLT_INT32(log_msg.entry.pid),
                    DLT_UINT32(log_msg.entry.tid),
                    DLT_STRING(message));
    }

    DLT_LOG(dlt_ctx_self, DLT_LOG_VERBOSE, DLT_STRING("Exited parsing loop"));
#else
    dlt_log_data->ctx = ctx;
    dlt_log_data->log_level = log_level;
    dlt_log_data->ts = ts;
    dlt_log_data->tag = tag;
    dlt_log_data->message = message;
#endif
    return EXIT_SUCCESS;
}

#ifndef DLT_UNIT_TESTS
int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    bool skip_binary_buffers = true;
    if (init_configuration() < 0) {
        cerr << "dlt-logd-converter could not allocate memory." << endl;
        return -1;
    }
    if (read_command_line(argc, argv) < 0) {
        cerr << "Failed to read command line!" << endl;
        return -1;
    }
    if (load_configuration_file(logd_conf->conf_file_dir) < 0) {
        cout << "No configuration file found, use default values!" << endl;
    }

    DLT_REGISTER_APP(logd_conf->appID, "logd -> dlt adapter");
    DLT_REGISTER_CONTEXT(dlt_ctx_self, logd_conf->ctxID, "logd retriever");
    DLT_REGISTER_CONTEXT(dlt_ctx_rdio, "RDIO", "logd type: rdio");
    DLT_REGISTER_CONTEXT(dlt_ctx_syst, "SYST", "logd type: syst");
    DLT_REGISTER_CONTEXT(dlt_ctx_crsh, "CRSH", "logd type: crsh");
    DLT_REGISTER_CONTEXT(dlt_ctx_krnl, "KRNL", "logd type: krnl");
    if (!skip_binary_buffers) {
        DLT_REGISTER_CONTEXT(dlt_ctx_evnt, "EVNT", "logd type: evnt");
        DLT_REGISTER_CONTEXT(dlt_ctx_stat, "STAT", "logd type: stat");
        DLT_REGISTER_CONTEXT(dlt_ctx_secu, "SECU", "logd type: secu");
    }

    /* Parse json data into internal data structure and do registration */
    json_parser();
    if (json_is_available) {
        DLT_LOG(dlt_ctx_self, DLT_LOG_INFO,
                        DLT_STRING("Found JSON file at "),
                        DLT_STRING(logd_conf->json_file_dir),
                        DLT_STRING(". Extension is ON!"));
    }
    else {
        DLT_LOG(dlt_ctx_self, DLT_LOG_INFO,
                        DLT_STRING("No JSON file available at "),
                        DLT_STRING(logd_conf->json_file_dir);
                        DLT_STRING(". Extension is OFF!"));
        DLT_REGISTER_CONTEXT(dlt_ctx_main, "MAIN", "logd type: main");
    }

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTERM, &act, 0);

    struct logger_list *logger_list;
    /* Binary buffers are currently not supported */
    logger_list = init_logger_list(skip_binary_buffers);
    if (logger_list == nullptr) {
        return EXIT_FAILURE;
    }

    int ret;

    /* Main loop */
    ret = logd_parser_loop(logger_list);

    android_logger_list_free(logger_list);

    DLT_UNREGISTER_CONTEXT(dlt_ctx_krnl);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_crsh);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_syst);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_rdio);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_self);
    if (!skip_binary_buffers) {
        DLT_UNREGISTER_CONTEXT(dlt_ctx_evnt);
        DLT_UNREGISTER_CONTEXT(dlt_ctx_stat);
        DLT_UNREGISTER_CONTEXT(dlt_ctx_secu);
    }

    if (json_is_available) {
        DLT_UNREGISTER_CONTEXT(dlt_ctx_othe);
        for (auto &tag_map: map_ctx_json) {
            DLT_UNREGISTER_CONTEXT(*(tag_map.second));
        }
    }
	else {
        DLT_UNREGISTER_CONTEXT(dlt_ctx_main);
    }
    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();
    clean_mem();
    return ret;
}
#endif
