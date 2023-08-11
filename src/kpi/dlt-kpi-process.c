/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-kpi-process.c
 */

#include "dlt-kpi-process.h"

#include <pthread.h>
#include <unistd.h>

DltReturnValue dlt_kpi_read_process_file_to_str(pid_t pid, char **target_str, char *subdir);
unsigned long int dlt_kpi_read_process_stat_to_ulong(pid_t pid, unsigned int index);
DltReturnValue dlt_kpi_read_process_stat_cmdline(pid_t pid, char **buffer);

DltReturnValue dlt_kpi_process_update_io_wait(DltKpiProcess *process, unsigned long int time_dif_ms)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    unsigned long int total_io_wait = dlt_kpi_read_process_stat_to_ulong(process->pid, 42);

    int cpu_count = dlt_kpi_get_cpu_count();

    process->io_wait = (total_io_wait - process->last_io_wait) * 1000 / sysconf(_SC_CLK_TCK); /* busy milliseconds since last update */

    if ((time_dif_ms > 0) && (cpu_count > 0))
        process->io_wait = process->io_wait * 1000 / time_dif_ms / cpu_count; /* busy milliseconds per second per CPU */

    process->last_io_wait = total_io_wait;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_process_update_cpu_time(DltKpiProcess *process, unsigned long int time_dif_ms)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    unsigned long int utime = dlt_kpi_read_process_stat_to_ulong(process->pid, 14);
    unsigned long int stime = dlt_kpi_read_process_stat_to_ulong(process->pid, 15);

    unsigned long total_cpu_time = utime + stime;

    if ((process->last_cpu_time > 0) && (process->last_cpu_time <= total_cpu_time)) {
        int cpu_count = dlt_kpi_get_cpu_count();

        process->cpu_time = (total_cpu_time - process->last_cpu_time) * 1000 / sysconf(_SC_CLK_TCK); /* busy milliseconds since last update */

        if ((time_dif_ms > 0) && (cpu_count > 0))
            process->cpu_time = process->cpu_time * 1000 / time_dif_ms / cpu_count; /* busy milliseconds per second per CPU */

    }
    else {
        process->cpu_time = 0;
    }

    process->last_cpu_time = total_cpu_time;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_process_update_rss(DltKpiProcess *process)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    process->rss = dlt_kpi_read_process_stat_to_ulong(process->pid, 24);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_process_update_ctx_switches(DltKpiProcess *process)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char *buffer, *tok, *last_tok;
    char *delim = " :\t\n";
    last_tok = NULL;

    DltReturnValue ret;

    if ((ret = dlt_kpi_read_process_file_to_str(process->pid, &buffer, "status")) < DLT_RETURN_OK) return ret;

    process->ctx_switches = 0;

    tok = strtok(buffer, delim);

    while (tok != NULL) {
        if (last_tok != NULL) {
            if ((strcmp(last_tok,
                        "voluntary_ctxt_switches") == 0) || (strcmp(last_tok, "nonvoluntary_ctxt_switches") == 0)) {
                char *chk;
                process->ctx_switches += strtol(tok, &chk, 10);

                if (*chk != '\0') {
                    fprintf(stderr, "Could not parse ctx_switches info from /proc/%d/status", process->pid);
                    free(buffer);
                    return DLT_RETURN_ERROR;
                }
            }
        }

        last_tok = tok;
        tok = strtok(NULL, delim);
    }

    free(buffer);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_process_update_io_bytes(DltKpiProcess *process)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char *buffer, *tok, *last_tok;
    char *delim = " :\t\n";
    last_tok = NULL;

    DltReturnValue ret;

    if ((ret = dlt_kpi_read_process_file_to_str(process->pid, &buffer, "io")) < DLT_RETURN_OK)
        return ret;

    process->io_bytes = 0;

    tok = strtok(buffer, delim);

    while (tok != NULL) {
        if (last_tok != NULL) {
            if ((strcmp(last_tok, "rchar") == 0) || (strcmp(last_tok, "wchar") == 0)) {
                char *chk;
                process->io_bytes += strtoul(tok, &chk, 10);

                if (*chk != '\0') {
                    fprintf(stderr, "Could not parse io_bytes info from /proc/%d/io", process->pid);
                    free(buffer);
                    return DLT_RETURN_ERROR;
                }
            }
        }

        last_tok = tok;
        tok = strtok(NULL, delim);
    }

    free(buffer);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_update_process(DltKpiProcess *process, unsigned long int time_dif_ms)
{

    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_kpi_process_update_io_wait(process, time_dif_ms);
    dlt_kpi_process_update_cpu_time(process, time_dif_ms);
    dlt_kpi_process_update_rss(process);
    dlt_kpi_process_update_ctx_switches(process);
    dlt_kpi_process_update_io_bytes(process);

    return DLT_RETURN_OK;
}

DltKpiProcess *dlt_kpi_create_process(int pid)
{
    DltKpiProcess *new_process = malloc(sizeof(DltKpiProcess));

    if (new_process == NULL) {
        fprintf(stderr, "%s: Out of Memory \n", __func__);
        return NULL;
    }

    memset(new_process, 0, sizeof(DltKpiProcess));

    new_process->pid = pid;
    new_process->ppid = (pid_t)dlt_kpi_read_process_stat_to_ulong(pid, 4);

    dlt_kpi_read_process_file_to_str(pid, &(new_process->command_line), "cmdline");

    if (new_process->command_line != NULL)
        if (strlen(new_process->command_line) == 0) {
            free(new_process->command_line);
            dlt_kpi_read_process_stat_cmdline(pid, &(new_process->command_line));
        }

    dlt_kpi_update_process(new_process, 0);

    return new_process;
}

DltKpiProcess *dlt_kpi_clone_process(DltKpiProcess *original)
{
    if (original == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return NULL;
    }

    /* DltKpiProcess *new_process = dlt_kpi_create_process(original->pid); */
    DltKpiProcess *new_process = malloc(sizeof(DltKpiProcess));

    if (new_process == NULL) {
        fprintf(stderr, "%s: Out of Memory\n", __func__);
        return NULL;
    }

    memcpy(new_process, original, sizeof(DltKpiProcess));

    if (original->command_line != NULL) {
        new_process->command_line = malloc(strlen(original->command_line) + 1);

        if (new_process->command_line == NULL) {
            fprintf(stderr, "%s: Out of Memory\n", __func__);
            free(new_process);
            return NULL;
        }

        strncpy(new_process->command_line, original->command_line, strlen(original->command_line) + 1);
    }
    else {
        new_process->command_line = NULL;
    }

    new_process->next = new_process->prev = NULL;

    return new_process;
}

DltReturnValue dlt_kpi_free_process(DltKpiProcess *process)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (process->command_line != NULL)
        free(process->command_line);

    free(process);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_print_process(DltKpiProcess *process)
{
    if (process == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    printf("[PID %d]\n", process->pid);
    printf("  > PPID    : %d\n", process->ppid);
    printf("  > CMDLINE : %s\n", process->command_line);
    printf("  > CPUTIME : %lu (busy ms/s)\n", process->cpu_time);
    printf("  > RSS     : %ld\n", process->rss);
    printf("  > CTXSWTC : %ld\n", process->ctx_switches);
    printf("  > IOBYTES : %lu\n", process->io_bytes);
    printf("  > IOWAIT  : %ld (%ld)\n", process->io_wait, process->last_io_wait);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_read_process_file_to_str(pid_t pid, char **target_str, char *subdir)
{
    if (target_str == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_ERROR;
    }

    *target_str = NULL;

    if (pid <= 0) {
        fprintf(stderr, "%s: Invalid Parameter (PID)\n", __func__);
        return DLT_RETURN_ERROR;
    }

    if (subdir == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_ERROR;
    }

    char filename[BUFFER_SIZE];
    snprintf(filename, BUFFER_SIZE, "/proc/%d/%s", pid, subdir);

    return dlt_kpi_read_file_compact(filename, target_str);
}

unsigned long int dlt_kpi_read_process_stat_to_ulong(pid_t pid, unsigned int index)
{
    if (pid <= 0) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return 0;
    }

    char *buffer = NULL;

    if (dlt_kpi_read_process_file_to_str(pid, &buffer, "stat") < DLT_RETURN_OK) {
        /* fprintf(stderr, "dlt_kpi_read_process_stat_to_ulong(): Error while reading process stat file. Pid: %d. Requested index: %u\n", pid, index); // can happen if process closed shortly before */

        if (buffer != NULL)
            free(buffer);

        return 0;
    }

    char *tok = strtok(buffer, " \t\n");
    unsigned int i = 1, found = 0;

    while (tok != NULL) {
        if (i == index) {
            found = 1;
            break;
        }

        i++;
        tok = strtok(NULL, " \t\n");
    }

    unsigned long int ret = 0;

    if (found) {
        char *check = NULL;
        ret = strtoul(tok, &check, 10);

        if (*check != '\0') {
            fprintf(stderr, "dlt_kpi_read_process_stat_to_ulong(): Could not extract token\n");
            ret = 0;
        }
    }
    else {
        fprintf(stderr, "dlt_kpi_read_process_stat_to_ulong(): Index not found\n");
    }

    free(buffer);

    return ret;
}

DltReturnValue dlt_kpi_read_process_stat_cmdline(pid_t pid, char **buffer)
{
    if (pid <= 0) {
        fprintf(stderr, "%s: Invalid Parameter (PID)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (buffer == NULL) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char *tmp_buffer = NULL;
    DltReturnValue tmp = dlt_kpi_read_process_file_to_str(pid, &tmp_buffer, "stat");

    if (tmp < DLT_RETURN_OK) {
        if (tmp_buffer != NULL)
            free(tmp_buffer);

        return tmp;
    }

    char *tok = strtok(tmp_buffer, " \t\n");
    unsigned int i = 1;

    while (tok != NULL) {
        if (i == 2)
            break;

        i++;
        tok = strtok(NULL, " \t\n");
    }

    if (i == 2) {
        (*buffer) = malloc(strlen(tok) + 1);
        strncpy(*buffer, tok, strlen(tok) + 1);
    }
    else {
        fprintf(stderr, "dlt_kpi_read_process_stat_cmdline(): cmdline entry not found\n");
        return DLT_RETURN_ERROR;
    }

    free(tmp_buffer);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_get_msg_process_update(DltKpiProcess *process, char *buffer, int maxlen)
{
    if ((process == NULL) || (buffer == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer,
             maxlen,
             "%d;%lu;%ld;%ld;%lu;%lu",
             process->pid,
             process->cpu_time,
             process->rss,
             process->ctx_switches,
             process->io_bytes,
             process->io_wait);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_get_msg_process_new(DltKpiProcess *process, char *buffer, int maxlen)
{
    if ((process == NULL) || (buffer == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d;%d;%s", process->pid, process->ppid, process->command_line);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_get_msg_process_stop(DltKpiProcess *process, char *buffer, int maxlen)
{
    if ((process == NULL) || (buffer == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d", process->pid);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_kpi_get_msg_process_commandline(DltKpiProcess *process, char *buffer, int maxlen)
{
    if ((process == NULL) || (buffer == NULL)) {
        fprintf(stderr, "%s: Invalid Parameter (NULL)\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d;%s", process->pid, process->command_line);

    return DLT_RETURN_OK;
}
