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

#define STACK_GUARD_SIZE    PTHREAD_STACK_4K /* 4 KiB guard page to prevent Stack Overflow */
#define STACK_USABLE_SIZE   (PTHREAD_STACK_4K * 64u) /* 256 KiB thread stack */
#define STACK_TOTAL_SIZE    (STACK_GUARD_SIZE + STACK_USABLE_SIZE)
static std::atomic_bool g_stop_notified{false};
static std::atomic_bool g_adapter_ctx_registered{false};

using std::chrono_literals::operator""ms;
using std::chrono_literals::operator""s;

/* Teach dlt about json_decoder_error_t */
template<>
inline int32_t logToDlt(DltContextData &log, const json_decoder_error_t &value)
{
    return logToDlt(log, static_cast<int>(value));
}
extern DltContext dltQnxSystem;

static DltContext dltQnxSlogger2Context;

static std::set<std::string> dltWarnedMissingMappings;

extern DltQnxSystemThreads g_threads;

static std::unordered_map<std::string, std::unique_ptr<DltContext>> g_slog2file;

static void *stackaddr = NULL;
static const size_t STACK_ALLOC_SIZE = PTHREAD_STACK_4K * 4;

/**
 * \brief Free the mmap'd thread stack memory.
 *
 * Unmaps the stack region allocated for the slogger2 thread
 * and resets the pointer to nullptr.
 */
static void free_stackaddr()
{
    if (stackaddr) {
        if (munmap(stackaddr, STACK_TOTAL_SIZE) != 0) {
            fprintf(stderr, "munmap failed: %s\n", strerror(errno));
        }
        stackaddr = nullptr;
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

/**
 * \brief Load the slog2-to-DLT context mapping from a JSON file.
 *
 * Parses the JSON file and populates g_slog2file with DltContext
 * entries keyed by slog2 file name. Each entry maps a slog2 source
 * to a registered DLT context ID.
 *
 * \param json_filename  Path to the JSON mapping file.
 */
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

    /* go to first element in dlt-slog2ctxt.json e.g. "ADIO" */
    auto ret = json_decoder_push_object(dec.get(), nullptr, false);
    while (ret == JSON_DECODER_OK) {
        ctxtID = json_decoder_name(dec.get());

        /* go into the element e.g. { name: "", description: "" } */
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

/**
 * Map the slog2 logfile name to a dlt context
 * e.g. i2c_service.2948409 -> Context with id "I2CS"
 */
static DltContext *dlt_context_from_slog2file(const char *file_name) {
    auto d = strchr(file_name, '.');

    if (d == nullptr)
        return &dltQnxSlogger2Context;

    auto name = std::string(file_name).substr(0, d - file_name);

    auto search = g_slog2file.find(name);
    if (search == g_slog2file.end()) {
        // Only warn once about missing mapping.
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

/**
 *  Function which is invoked by slog2_parse_all()
 *  See slog2_parse_all api docs on qnx.com for details
 */
static int slogger2_callback(slog2_packet_info_t *info, void *payload, void *param)
{
    /*
     * Returning -1 terminates slog2_parse_all()
     * and causes the slogger thread to exit.
     */
    if (dlt_slog2_is_disabled()) {
        /* notify main once so it can join + clear tid + clean mapping */
        if (!g_stop_notified.exchange(true)) {
            int ret = pthread_kill(dlt_get_main_thread(), SIGUSR1);
            if (ret != 0) {
                DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                        DLT_STRING("Failed to send SIGUSR1 to main thread."),
                        DLT_INT(ret));
            }
        }
        return -1;
    }
    auto *conf = static_cast<DltQnxSystemConfiguration*>(param);
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

    DltContextData log_local; /* Used in DLT_* macros, do not rename */
    DltContext *ctxt = dlt_context_from_slog2file(info->file_name);

    if( wait_for_buffer_space(0.8, std::chrono::milliseconds(DLT_QNX_SLOG_ADAPTER_WAIT_BUFFER_TIMEOUT_MS)))
    {
        return 0; // discard message
    }

    int ret;
    ret = dlt_user_log_write_start(ctxt, &log_local, loglevel);

    /* OK means loglevel under threshold */
    if (ret == DLT_RETURN_OK) {
        return 0;
    }

    if (ret != DLT_RETURN_TRUE) {
        fprintf(stderr, "%s: could not log to DLT status=%d\n", __func__, ret);
        return -1;
    }

    if (conf->qnxslogger2.useOriginalTimestamp == 1) {
        /* convert from ns to .1 ms */
        log_local.user_timestamp = (uint32_t) (info->timestamp / 100000);
        log_local.use_timestamp = DLT_USER_TIMESTAMP;
    } else {
        DLT_UINT64(info->timestamp);
    }

    DLT_UINT16(info->sequence_number);
    DLT_STRING((char *)info->file_name);
    DLT_STRING((char *)info->buffer_name);
    DLT_UINT16(info->thread_id);
    DLT_UINT8(info->severity);
    DLT_STRING((char *)payload);

    dlt_user_log_write_finish(&log_local);

    return 0;
}

static void *slogger2_thread(void *v_conf)
{
    auto *conf = static_cast<DltQnxSystemConfiguration*>(v_conf);
    if (conf == nullptr) {
        DLT_LOG_CXX(dltQnxSystem, DLT_LOG_DEBUG, __func__, ": Invalid config data.");
        return nullptr;
    }
    slog2_packet_info_t packet_info = SLOG2_PACKET_INFO_INIT;
    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-qnx-slogger2-adapter, in thread."));
    /**
     * Thread will block inside this function to get new log because
     * flag = SLOG2_PARSE_FLAGS_DYNAMIC
     */
    if (slog2_parse_all(SLOG2_PARSE_FLAGS_DYNAMIC, nullptr, nullptr,
                        &packet_info, slogger2_callback, static_cast<void*>(conf)) == -1) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN,
                    "slog2_parse_all() stops working.\n");
    }
    DLT_LOG_CXX(dltQnxSystem, DLT_LOG_DEBUG, __func__, ": Exited main loop.");
    return nullptr;
}

void start_qnx_slogger2(DltQnxSystemConfiguration *conf)
{
    if (conf == nullptr) {
        fprintf(stderr, "Error in setup local database. No thread created.\n");
        return;
    }
    if (stackaddr != nullptr) {
        DLT_LOG_CXX(dltQnxSystem, DLT_LOG_ERROR,
                    __func__, ": Previous slogger2 thread stack still allocated. No thread created.");
        return;
    }
    int ret;
    pthread_attr_t thread_attr;
    g_stop_notified.store(false);
    /* Get a big enough stack and align it on 4K boundary. */
    stackaddr = mmap(nullptr, STACK_TOTAL_SIZE,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);
    if (stackaddr == MAP_FAILED) {
        stackaddr = nullptr;
        fprintf(stderr, "mmap stack failed: %s\n", strerror(errno));
        return;
    }
    /* Guard page: lowest page becomes inaccessible (PROT_NONE). */
    if (mprotect(stackaddr, STACK_GUARD_SIZE, PROT_NONE) != 0) {
        fprintf(stderr, "mprotect guard page failed: %s\n", strerror(errno));
        free_stackaddr();
        return;
    }
    void *stack_base = static_cast<char *>(stackaddr) + STACK_GUARD_SIZE;
    ret = pthread_attr_init(&thread_attr);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_init failed: %s\n", strerror(ret));
        free_stackaddr();
        return;
    }
    ret = pthread_attr_setstack(&thread_attr, stack_base, STACK_USABLE_SIZE);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_setstack failed: %s\n", strerror(ret));
        free_stackaddr();
        pthread_attr_destroy(&thread_attr);
        return;
    }
    DLT_REGISTER_CONTEXT(dltQnxSlogger2Context, conf->qnxslogger2.contextId, "SLOGGER2 Adapter");
    g_adapter_ctx_registered.store(true, std::memory_order_release);
    /* Lazy-load mapping only when runtime enabled */
    if (dlt_slog2_is_disabled()) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
                    "Runtime disabled: skip mapping load.");
    } else if (!g_slog2file.empty()) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
                    "Mapping already loaded; skip reload.");
    } else {
        dlt_context_map_read(CONFIGURATION_FILES_DIR "/dlt-slog2ctxt.json");
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
                    "Loaded slog2 context mapping with ", g_slog2file.size(), " entries.");
    }
    pthread_t tid;
    ret = pthread_create(&tid, &thread_attr, slogger2_thread, conf);
    if (ret != 0) {
        pthread_attr_destroy(&thread_attr);
        clean_qnx_slogger2();
        fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
        return;
    }
    dlt_set_slog2_thread(tid);
    ret = pthread_attr_destroy(&thread_attr);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_destroy failed: %s\n", strerror(ret));
    }
}

/**
 * \brief Full cleanup of the slogger2 adapter.
 *
 * Releases all internal resources (mapping table, stack memory)
 * and unregisters the adapter DLT context. Logs a warning if
 * the slogger2 thread is still active.
 */
void clean_qnx_slogger2()
{
    /* RAII Deleter handles dlt_unregister_context() for all map entries automatically */
    g_slog2file.clear();
    dltWarnedMissingMappings.clear();
    free_stackaddr();
}

