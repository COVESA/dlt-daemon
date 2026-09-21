/**
 * Copyright (C) 2018-2020 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT QNX system functionality source file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Nguyen Dinh Thi <Thi.NguyenDinh@vn.bosch.com> ADIT 2018
 * \author Felix Herrmann <fherrmann@de.adit-jv.com> ADIT 2020
 *
 * \file: dlt-qnx-slogger2-adapter.cpp
 * For further information see http://www.covesa.org/.
 */
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <pthread.h>
#include <sys/slog2.h>
#include <sys/json.h>
#include <slog2_parse.h>
#include <thread>
#include <atomic>
#include <set>
#include <sys/mman.h>

#include "dlt-qnx-system.h"
#include "dlt_cpp_extension.hpp"

using std::chrono_literals::operator""ms;
using std::chrono_literals::operator""s;

/* Teach dlt about json_decoder_error_t */
template<>
inline int32_t logToDlt(DltContextData &log, const json_decoder_error_t &value)
{
    return logToDlt(log, static_cast<int>(value));
}

std::atomic<bool> g_inj_disable_slog2_cb(false);
std::atomic<bool> g_slog2_thread_alive(false);

extern DltContext dltQnxSystem;

static DltContext dltQnxSlogger2Context;
static std::set<std::string> dltWarnedMissingMappings;

extern DltQnxSystemThreads g_threads;

static std::unordered_map<std::string, std::unique_ptr<DltContext>> g_slog2file;

static void *stackaddr = NULL;
static const size_t STACK_ALLOC_SIZE = PTHREAD_STACK_4K * 4;

void free_stackaddr()
{
    if (stackaddr != MAP_FAILED && stackaddr != NULL) {
        munmap(stackaddr, STACK_ALLOC_SIZE);
        stackaddr = NULL;
    }
}

/* Custom deleter for json_decoder_t to guarantee cleanup */
struct JsonDecoderDeleter {
    void operator()(json_decoder_t *dec) const {
        if (dec) {
            json_decoder_destroy(dec);
        }
    }
};

static void dlt_context_map_read(const char *json_filename)
{
    DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_VERBOSE,
            "Loading Slog2Ctxt Map from json file: ", json_filename);

    std::unique_ptr<json_decoder_t, JsonDecoderDeleter> dec(json_decoder_create());
    if (!dec) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_ERROR,
                "Failed to allocate JSON decoder.");
        return;
    }

    if (json_decoder_parse_file(dec.get(), json_filename) != JSON_DECODER_OK) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_ERROR,
                "Could not load Slog2Ctxt Map from json file: ", json_filename);
        return;
    }

    const char *ctxtID = nullptr;
    const char *name = nullptr;
    const char *description = nullptr;

    auto ret = json_decoder_push_object(dec.get(), nullptr, false);
    while (ret == JSON_DECODER_OK) {
        ctxtID = json_decoder_name(dec.get());

        ret = json_decoder_push_object(dec.get(), nullptr, false);
        if (ret != JSON_DECODER_OK) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN, __func__,
                    ": json parser error while descending into context dict. ret=", ret);
            json_decoder_pop(dec.get());
            ret = json_decoder_pop(dec.get());
            continue;
        }

        ret = json_decoder_get_string(dec.get(), "name", &name, false);
        if (ret != JSON_DECODER_OK) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN, __func__,
                    ": json parser error while retrieving 'name' element of ", ctxtID, ". ret=", ret);
            json_decoder_pop(dec.get());
            json_decoder_pop(dec.get());
            continue;
        }

        ret = json_decoder_get_string(dec.get(), "description", &description, false);
        if (ret != JSON_DECODER_OK) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN, __func__,
                    ": json parser error while retrieving 'description' element of ", ctxtID, ". ret=", ret);
            json_decoder_pop(dec.get());
            json_decoder_pop(dec.get());
            continue;
        }

        auto search = g_slog2file.find(name);
        if (search == g_slog2file.end()) {
            auto ctxt = std::make_unique<DltContext>();
            dlt_register_context(ctxt.get(), ctxtID, description);
            g_slog2file.emplace(name, std::move(ctxt));
        } else {
            dlt_register_context(search->second.get(), ctxtID, description);
        }

        ret = json_decoder_pop(dec.get());
    }
    DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
            "Added ", g_slog2file.size(), " elements into the mapping table.");
}

static DltContext *dlt_context_from_slog2file(const char *file_name) {
    if (file_name == nullptr) {
        return &dltQnxSlogger2Context;
    }

    auto d = strchr(file_name, '.');
    if (d == nullptr)
        return &dltQnxSlogger2Context;

    auto name = std::string(file_name).substr(0, d - file_name);

    auto search = g_slog2file.find(name);
    if (search == g_slog2file.end()) {
        auto it = dltWarnedMissingMappings.find(name);
        if (it == dltWarnedMissingMappings.end()) {
            dltWarnedMissingMappings.insert(name);
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_INFO,
                        "slog2 filename not found in mapping: ", name.c_str());
        }
        return &dltQnxSlogger2Context;
    } else {
        return search->second.get();
    }
}

template <class time, class period>
static bool wait_for_buffer_space(const double max_usage_threshold,
                                  const std::chrono::duration<time, period> max_wait_time) {
    int total_size = 0;
    int used_size = 0;
    double used_percent = 100.0;
    bool timeout = false;
    static bool warning_sent = false;

    const auto end_time = std::chrono::steady_clock::now() + max_wait_time;

    do {
        /* Check thread termination before blocking again */
        if (!g_slog2_thread_alive) {
            return true;
        }

        dlt_user_check_buffer(&total_size, &used_size);

        if (total_size <= 0) {
            used_percent = 100.0;
        } else {
            used_percent = static_cast<double>(used_size) / total_size;
        }

        if (used_percent < max_usage_threshold) {
            warning_sent = false;
            break;
        }

        dlt_user_log_resend_buffer();

        std::this_thread::sleep_for(10ms);
        timeout = std::chrono::steady_clock::now() >= end_time;
    } while (!timeout);

    if (timeout && !warning_sent) {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("failed to get enough buffer space"));
        warning_sent = true;
    }
    return timeout;
}

static int slogger2_callback(slog2_packet_info_t *info, void *payload, void *param)
{
    DltQnxSystemConfiguration* conf = static_cast<DltQnxSystemConfiguration*>(param);

    if (!g_slog2_thread_alive) {
        return -1;
    }

    if (g_inj_disable_slog2_cb) {
        do {
            DLT_LOG(dltQnxSystem, DLT_LOG_INFO,
                    DLT_STRING("Disabling slog2 callback because of injection request."));
            sleep(1);
            if (!g_slog2_thread_alive) {
                return -1;
            }
        } while (g_inj_disable_slog2_cb);
        DLT_LOG(dltQnxSystem, DLT_LOG_INFO,
                DLT_STRING("Enabling slog2 callback because of injection request."));
    }

    if (!info) {
        return 0;
    }

    DltLogLevelType loglevel;
    switch (info->severity)
    {
        case SLOG2_SHUTDOWN:
        case SLOG2_CRITICAL:
            loglevel = DLT_LOG_FATAL;
            break;
        case SLOG2_ERROR:
            loglevel = DLT_LOG_ERROR;
            break;
        case SLOG2_WARNING:
            loglevel = DLT_LOG_WARN;
            break;
        case SLOG2_NOTICE:
        case SLOG2_INFO:
            loglevel = DLT_LOG_INFO;
            break;
        case SLOG2_DEBUG1:
            loglevel = DLT_LOG_DEBUG;
            break;
        case SLOG2_DEBUG2:
            loglevel = DLT_LOG_VERBOSE;
            break;
        default:
            loglevel = DLT_LOG_INFO;
            break;
    }

    DltContextData log_local;
    DltContext *ctxt = dlt_context_from_slog2file(info->file_name);

    if (wait_for_buffer_space(0.8, std::chrono::milliseconds(DLT_QNX_SLOG_ADAPTER_WAIT_BUFFER_TIMEOUT_MS))) {
        return 0; /* Discard message gracefully during saturation or exit */
    }

    int ret = dlt_user_log_write_start(ctxt, &log_local, loglevel);

    if (ret == DLT_RETURN_OK) {
        return 0;
    }

    if (ret != DLT_RETURN_TRUE) {
        return -1;
    }

    if (conf && conf->qnxslogger2.useOriginalTimestamp == 1) {
        log_local.user_timestamp = static_cast<uint32_t>(info->timestamp / 100000);
        log_local.use_timestamp = DLT_USER_TIMESTAMP;
    } else {
        DLT_UINT64(info->timestamp);
    }

    DLT_UINT16(info->sequence_number);
    DLT_STRING(info->file_name ? (char *)info->file_name : "");
    DLT_STRING(info->buffer_name ? (char *)info->buffer_name : "");
    DLT_UINT16(info->thread_id);
    DLT_UINT8(info->severity);
    DLT_STRING(payload ? (char *)payload : "");

    dlt_user_log_write_finish(&log_local);

    return 0;
}

static void *slogger2_thread(void *v_conf)
{
    DltQnxSystemConfiguration *conf = static_cast<DltQnxSystemConfiguration *>(v_conf);

    if (conf == NULL) {
        DLT_LOG_CXX(dltQnxSystem, DLT_LOG_DEBUG, __func__, ": Invalid config data.");
        DLT_UNREGISTER_CONTEXT(dltQnxSlogger2Context);
        pthread_kill(g_threads.main_thread, SIGTERM);
        pthread_exit(NULL);
        return NULL;
    }

    slog2_packet_info_t packet_info = SLOG2_PACKET_INFO_INIT;

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-qnx-slogger2-adapter, inside thread worker."));

    /* Blocks inside parsing loop until stopped or exception occurs */
    if (slog2_parse_all(SLOG2_PARSE_FLAGS_DYNAMIC, NULL, NULL,
                        &packet_info, slogger2_callback, (void*) conf) == -1) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN,
                    "slog2_parse_all() stopped processing.\n");
    }

    DLT_LOG_CXX(dltQnxSystem, DLT_LOG_DEBUG, __func__, ": Exited ring buffer loop.");

    DLT_UNREGISTER_CONTEXT(dltQnxSlogger2Context);
    pthread_exit(NULL);
    return NULL;
}

void start_qnx_slogger2(DltQnxSystemConfiguration *conf)
{
    if (conf == NULL) {
        printf("Error in setup local database. No thread created.\n");
        return;
    }

    int ret;
    pthread_attr_t thread_attr;

    ret = pthread_attr_init(&thread_attr);
    if (ret != 0) {
        printf("pthread_attr_init returned: %d. Error: %d\n", ret, errno);
        return;
    }

    stackaddr = mmap(NULL, STACK_ALLOC_SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);

    if (stackaddr == MAP_FAILED) {
        printf("Unable to map stack memory. Error: %d\n", errno);
        stackaddr = NULL;
        pthread_attr_destroy(&thread_attr);
        return;
    }

    ret = pthread_attr_setstack(&thread_attr, stackaddr, STACK_ALLOC_SIZE);
    if (ret != 0) {
        free_stackaddr();
        pthread_attr_destroy(&thread_attr);
        printf("pthread_attr_setstack returned: %d. Error: %d\n", ret, errno);
        return;
    }

    DLT_REGISTER_CONTEXT(dltQnxSlogger2Context, conf->qnxslogger2.contextId,
                         "SLOGGER2 Adapter");

    dlt_context_map_read(CONFIGURATION_FILES_DIR "/dlt-slog2ctxt.json");

    DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
            "dlt-qnx-slogger2-adapter, start syslog");

    g_slog2_thread_alive = true;
    ret = pthread_create(&g_threads.slog2_thread, &thread_attr, slogger2_thread, conf);
    if (ret != 0) {
        g_slog2_thread_alive = false;
        pthread_attr_destroy(&thread_attr);
        clean_qnx_slogger2();
        fprintf(stderr, "Failed to create thread: %d %s\n", ret, strerror(ret));
        return;
    }

    ret = pthread_attr_destroy(&thread_attr);
    if (ret != 0) {
        printf("Error in pthread_attr_destroy. Returned: %d, Error: %d\n", ret, errno);
        return;
    }
}

void clean_qnx_slogger2()
{
    for (auto& x : g_slog2file) {
        if (x.second != nullptr) {
            dlt_unregister_context(x.second.get());
        }
    }
    g_slog2file.clear();
    dltWarnedMissingMappings.clear();
    free_stackaddr();
}
