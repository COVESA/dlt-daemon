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
 * \file: dlt-logd-converter.hpp
 * For further information see http://www.covesa.org/.
 **/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logd-converter.hpp                                        **
**                                                                            **
**  TARGET    : ANDROID                                                       **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Minh.LuuQuang@vn.bosch.com                                    **
**                                                                            **
**  PURPOSE   : Header for DLT logd converter                                 **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

#ifndef DLT_LOGD_CONVERTER_HPP
#define DLT_LOGD_CONVERTER_HPP

#pragma once
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#ifndef DLT_UNIT_TESTS
#   include <log/log_read.h>
#   include <log/logprint.h>
#   include <json/json.h>
#endif

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <getopt.h>
#include <dlt.h>

using namespace std;

/* MACRO */
#define CONFIGURATION_FILE_DIR "/vendor/etc/dlt-logd-converter.conf"
#define JSON_FILE_DIR "/vendor/etc/dlt-logdctxt.json"
#define MAX_LINE 1024
#define DLT_FAIL_TO_GET_LOG_MSG 0

typedef struct
{
    char *appID;
    char *ctxID;
    char *json_file_dir;
    char *default_ctxID;
    char *conf_file_dir;
} dlt_logd_configuration;

#ifdef DLT_UNIT_TESTS
#define LOGGER_LOGD (1U << 31)
#define READ_ONLY 0x0000
#define NS_PER_SEC 1000000000ULL
#define LOGGER_ENTRY_MAX_LEN (5 * 1024)

struct logger_list
{
    int mode;
    unsigned int tail;
    pid_t pid;
    uint32_t log_mask;
    int signal;
    char dummy_data[96];
};

typedef enum {
    LOG_ID_MIN = 0,

    /** The main log buffer. This is the only log buffer available to apps. */
    LOG_ID_MAIN = 0,
    /** The radio log buffer. */
    LOG_ID_RADIO = 1,
    /** The event log buffer. */
    LOG_ID_EVENTS = 2,
    /** The system log buffer. */
    LOG_ID_SYSTEM = 3,
    /** The crash log buffer. */
    LOG_ID_CRASH = 4,
    /** The statistics log buffer. */
    LOG_ID_STATS = 5,
    /** The security log buffer. */
    LOG_ID_SECURITY = 6,
    /** The kernel log buffer. */
    LOG_ID_KERNEL = 7,

    LOG_ID_MAX,

    /** Let the logging function choose the best log target. */
    LOG_ID_DEFAULT = 0x7FFFFFFF
} log_id_t;

typedef enum {
    /** For internal use only.  */
    ANDROID_LOG_UNKNOWN = 0,
    /** The default priority, for internal use only.  */
    ANDROID_LOG_DEFAULT, /* only for SetMinPriority() */
    /** Verbose logging. Should typically be disabled for a release apk. */
    ANDROID_LOG_VERBOSE,
    /** Debug logging. Should typically be disabled for a release apk. */
    ANDROID_LOG_DEBUG,
    /** Informational logging. Should typically be disabled for a release apk. */
    ANDROID_LOG_INFO,
    /** Warning logging. For use with recoverable failures. */
    ANDROID_LOG_WARN,
    /** Error logging. For use with unrecoverable failures. */
    ANDROID_LOG_ERROR,
    /** Fatal logging. For use when aborting. */
    ANDROID_LOG_FATAL,
    /** For internal use only.  */
    ANDROID_LOG_SILENT, /* only for SetMinPriority(); must be last */
} android_LogPriority;

struct logger_entry {
    uint16_t len;      /* length of the payload */
    uint16_t hdr_size; /* sizeof(struct logger_entry) */
    int32_t pid;       /* generating process's pid */
    uint32_t tid;      /* generating process's tid */
    uint32_t sec;      /* seconds since Epoch */
    uint32_t nsec;     /* nanoseconds */
    uint32_t lid;      /* log id of the payload, bottom 4 bits currently */
    uint32_t uid;      /* generating process's uid */
};

struct log_msg {
    union {
        unsigned char buf[LOGGER_ENTRY_MAX_LEN + 1];
        struct logger_entry entry;
    } __attribute__((aligned(4)));
    uint64_t nsec() const {
        return static_cast<uint64_t>(entry.sec) * NS_PER_SEC + entry.nsec;
    }
    log_id_t id() {
        return static_cast<log_id_t>(entry.lid);
    }
    char* msg() {
        unsigned short hdr_size = entry.hdr_size;
        if (hdr_size >= sizeof(struct log_msg) - sizeof(entry)) {
        return nullptr;
        }
        return reinterpret_cast<char*>(buf) + hdr_size;
    }
    unsigned int len() { return entry.hdr_size + entry.len; }
};

struct dlt_log_container {
    DltContext *ctx;
    DltLogLevelType log_level;
    uint32_t ts;
    const char *tag;
    const char *message;
};

/* Android API faked method for unit test */
string t_load_json_file();
struct logger *t_android_logger_open(logger_list *logger_list,log_id_t log_id);
struct logger_list *t_android_logger_list_alloc(int mode, unsigned int tail, pid_t pid);
int t_android_logger_list_read(logger_list *logger_list, log_msg *t_log_msg);
#endif
/**
 * Prints manual page for instruction
 * @param prog_name binary name from stdin
 * @return void
 */
DLT_STATIC void usage(char *prog_name);
/**
 * Initializes configuration to default values
 * @return scenario statement
 */
DLT_STATIC int init_configuration();
/**
 * Reads command line options and set the values in provided structure
 * @return scenario statement
 */
DLT_STATIC int read_command_line(int argc, char *argv[]);
/**
 * Reads options from the configuration file
 * @param file_name configuration file name/path
 * @return scenario statement
 */
DLT_STATIC int load_configuration_file(const char *file_name);
/**
 * Frees the memory allocated for configuration values.
 * @return void
 */
DLT_STATIC void clean_mem();
/**
 * Parses data from a JSON file into an internal data
 * structure and do registration with the new ctxID.
 * @return void
 */
DLT_STATIC void json_parser();
/**
 * Doing tag matching for pairs in JSON unordered map.
 * @param tag tag of the application that needs new context ID
 * @return pointer to DLT context mapped with the corresponding tag
 */
DLT_STATIC DltContext* find_tag_in_json(const char *tag);
/**
 * Doing initialization for logger.
 * @param logger_list pointer to a logger list struct
 * @param log_id identifies a specific log buffer
 * @return pointer to a logger object logging messages for applications
 */
DLT_STATIC struct logger *init_logger(struct logger_list *logger_list, log_id_t log_id);
/**
 * Doing initialization for logger list.
 * @param skip_binary_buffers boolean value to use event buffers
 * @return pointer to a logger list struct
 */
DLT_STATIC struct logger_list *init_logger_list(bool skip_binary_buffers);
/**
 * Getting a context from an android log message.
 * @param log_msg struct contains android log data including log messages
 * @return pointer to DLT context mapped with the corresponding tag
 */
DLT_STATIC DltContext *get_log_context_from_log_msg(struct log_msg *log_msg);
/**
 * Getting timestamp from an android log message.
 * @param log_msg struct contains android log data including log messages
 * @return timestamp of messages from a logger entry
 */
DLT_STATIC uint32_t get_timestamp_from_log_msg(struct log_msg *log_msg);
/**
 * Getting log level from an android log message.
 * @param log_msg struct contains android log data including log messages
 * @return DLT log level
 */
DLT_STATIC DltLogLevelType get_log_level_from_log_msg(struct log_msg *log_msg);
/**
 * Handling received signal.
 * @param signal  received signal to handle
 * @return void
 */
void signal_handler(int signal);
/**
 * Continuously converting android logs to dlt logs.
 * @param logger_list pointer to a logger list struct
 * @return scenario statement
 */
DLT_STATIC int logd_parser_loop(struct logger_list *logger_list);

#endif /* DLT_LOGD_CONVERTER_HPP */
