/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-procfs-process.c
 */

#include "dlt-procfs-process.h"
#include <pthread.h>
#include <unistd.h>

DltReturnValue dlt_procfs_read_process_file_to_str(pid_t pid, char **target_str, char *subdir);
unsigned long int dlt_procfs_read_process_stat_to_ulong(pid_t pid, unsigned int index);
DltReturnValue dlt_procfs_read_process_stat_cmdline(pid_t pid, char **buffer);

DltReturnValue dlt_procfs_process_update_io_wait(DltProcfsProcess *process, unsigned long int time_dif_ms)
{
    if(process == NULL)
    {
        fprintf(stderr, "dlt_procfs_process_update_io_wait(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    unsigned long int total_io_wait = dlt_procfs_read_process_stat_to_ulong(process->pid, 42);

    process->io_wait = (total_io_wait - process->last_io_wait) * 1000 / sysconf(_SC_CLK_TCK); // busy milliseconds since last update
    if(time_dif_ms > 0)
        process->io_wait = process->io_wait * 1000 / time_dif_ms; // busy milliseconds per second

    process->last_io_wait = total_io_wait;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_process_update_cpu_time(DltProcfsProcess *process, unsigned long int time_dif_ms)
{
    if(process == NULL)
    {
        fprintf(stderr, "dlt_procfs_process_update_cpu_time(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    unsigned long int utime = dlt_procfs_read_process_stat_to_ulong(process->pid, 14);
    unsigned long int stime = dlt_procfs_read_process_stat_to_ulong(process->pid, 15);

    unsigned long total_cpu_time = utime + stime;

    process->cpu_time = (total_cpu_time - process->last_cpu_time) * 1000 / sysconf(_SC_CLK_TCK); // busy milliseconds since last update
    if(time_dif_ms > 0)
        process->cpu_time = process->cpu_time * 1000 / time_dif_ms; // busy milliseconds per second

    process->last_cpu_time = total_cpu_time;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_process_update_rss(DltProcfsProcess *process)
{
    if(process == NULL)
    {
        fprintf(stderr, "dlt_procfs_process_update_rss(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    process->rss = dlt_procfs_read_process_stat_to_ulong(process->pid, 24);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_process_update_ctx_switches(DltProcfsProcess *process)
{
    if(process == NULL)
    {
        fprintf(stderr, "dlt_procfs_process_update_ctx_switches(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char *buffer, *tok, *last_tok;
    char *delim = " :\t\n";
    last_tok = NULL;

    DltReturnValue ret;
    if((ret = dlt_procfs_read_process_file_to_str(process->pid, &buffer, "status")) < DLT_RETURN_OK) return ret;

    process->ctx_switches = 0;

    tok = strtok(buffer, delim);
    while(tok != NULL)
    {
        if(last_tok != NULL)
        {
            if(strcmp(last_tok, "voluntary_ctxt_switches") == 0 || strcmp(last_tok, "nonvoluntary_ctxt_switches") == 0)
            {
                char *chk;
                process->ctx_switches += strtol(tok, &chk, 10);

                if(*chk != '\0')
                {
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

DltReturnValue dlt_procfs_process_update_io_bytes(DltProcfsProcess *process)
{
    if(process == NULL)
    {
        fprintf(stderr, "dlt_procfs_process_update_io_bytes: Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char *buffer, *tok, *last_tok;
    char *delim = " :\t\n";
    last_tok = NULL;

    DltReturnValue ret;
    if((ret = dlt_procfs_read_process_file_to_str(process->pid, &buffer, "io")) < DLT_RETURN_OK)
        return ret;

    process->io_bytes = 0;

    tok = strtok(buffer, delim);
    while(tok != NULL)
    {
        if(last_tok != NULL)
        {
            if(strcmp(last_tok, "rchar") == 0 || strcmp(last_tok, "wchar") == 0)
            {
                char *chk;
                process->io_bytes += strtoul(tok, &chk, 10);

                if(*chk != '\0')
                {
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

DltReturnValue dlt_procfs_update_process(DltProcfsProcess *process, unsigned long int time_dif_ms)
{
    dlt_procfs_process_update_io_wait(process, time_dif_ms);
    dlt_procfs_process_update_cpu_time(process, time_dif_ms);
    dlt_procfs_process_update_rss(process);
    dlt_procfs_process_update_ctx_switches(process);
    dlt_procfs_process_update_io_bytes(process);

    return DLT_RETURN_OK;
}

DltProcfsProcess *dlt_procfs_create_process(int pid)
{
    DltProcfsProcess *new_process = malloc(sizeof(DltProcfsProcess));
    memset(new_process, 0, sizeof(DltProcfsProcess));

    new_process->pid = pid;
    new_process->ppid = (pid_t)dlt_procfs_read_process_stat_to_ulong(pid, 4);

    dlt_procfs_read_process_file_to_str(pid, &(new_process->command_line), "cmdline");
    if(new_process->command_line != NULL)
        if(strlen(new_process->command_line) == 0)
        {
            free(new_process->command_line);
            dlt_procfs_read_process_stat_cmdline(pid, &(new_process->command_line));
        }

    dlt_procfs_update_process(new_process, 0);

    return new_process;
}

DltProcfsProcess *dlt_procfs_clone_process(DltProcfsProcess *original)
{
    if(original == NULL)
    {
        fprintf(stderr, "dlt_procfs_clone_process: Nullpointer parameter\n");
        return NULL;
    }

    // DltProcfsProcess *new_process = dlt_procfs_create_process(original->pid);
    DltProcfsProcess *new_process = malloc(sizeof(DltProcfsProcess));
    if(new_process == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    memcpy(new_process, original, sizeof(DltProcfsProcess));

    if(original->command_line != NULL)
    {
        new_process->command_line = malloc(strlen(original->command_line) + 1);
        if(new_process->command_line == NULL)
        {
            fprintf(stderr, "Out of memory\n");
            return NULL;
        }
        strncpy(new_process->command_line, original->command_line, strlen(original->command_line) + 1);
    }
    else
        new_process->command_line = NULL;

    new_process->next = new_process->prev = NULL;

    return new_process;
}

DltReturnValue dlt_procfs_free_process(DltProcfsProcess *process)
{
    if(process == NULL)
    {
        fprintf(stderr, "dlt_procfs_free_process: Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if(process->command_line != NULL)
        free(process->command_line);

    free(process);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_print_process(DltProcfsProcess *process)
{
    if(process == NULL)
    {
        fprintf(stderr, "Error: Nullpointer parameter\n");
        return DLT_RETURN_ERROR;
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

DltReturnValue dlt_procfs_read_process_file_to_str(pid_t pid, char **target_str, char *subdir)
{
    if(target_str == NULL)
    {
        fprintf(stderr, "Error: Nullpointer parameter\n");
        return DLT_RETURN_ERROR;
    }

    *target_str = NULL;

    if(pid <= 0)
    {
        fprintf(stderr, "Error: Invalid PID\n");
        return DLT_RETURN_ERROR;
    }

    if(subdir == NULL)
    {
        fprintf(stderr, "Error: Nullpointer parameter\n");
        return DLT_RETURN_ERROR;
    }

    char filename[BUFFER_SIZE];
    snprintf(filename, BUFFER_SIZE, "/proc/%d/%s", pid, subdir);

    return dlt_procfs_read_file_compact(filename, target_str);
}

unsigned long int dlt_procfs_read_process_stat_to_ulong(pid_t pid, unsigned int index)
{
    if(pid <= 0)
    {
        fprintf(stderr, "dlt_procfs_read_process_stat_to_ulong(): Invalid PID\n");
        return 0;
    }

    char *buffer = NULL;
    DltReturnValue tmp = dlt_procfs_read_process_file_to_str(pid, &buffer, "stat");
    if(tmp < DLT_RETURN_OK)
    {
        if(buffer != NULL)
            free(buffer);

        return tmp;
    }

    char *tok = strtok(buffer, " \t\n");
    unsigned int i = 1, found = 0;

    while(tok != NULL)
    {
        if(i == index)
        {
            found = 1;
            break;
        }
        i++;
        tok = strtok(NULL, " \t\n");
    }

    unsigned long int ret = 0;

    if(found)
    {
        char *check = NULL;
        ret = strtoul(tok, &check, 10);
        if(*check != '\0')
        {
            fprintf(stderr, "dlt_procfs_read_process_stat_to_ulong(): Could not extract token\n");
            ret = 0;
        }
    }
    else
        fprintf(stderr, "dlt_procfs_read_process_stat_to_ulong(): Index not found\n");

    free(buffer);

    return ret;
}

DltReturnValue dlt_procfs_read_process_stat_cmdline(pid_t pid, char **buffer)
{
    if(pid <= 0)
    {
        fprintf(stderr, "dlt_procfs_read_process_stat_cmdline(): Invalid PID\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if(buffer == NULL)
    {
        fprintf(stderr, "dlt_procfs_read_process_stat_cmdline(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    char *tmp_buffer = NULL;
    DltReturnValue tmp = dlt_procfs_read_process_file_to_str(pid, &tmp_buffer, "stat");
    if(tmp < DLT_RETURN_OK)
    {
        if(tmp_buffer != NULL)
            free(tmp_buffer);

        return tmp;
    }

    char *tok = strtok(tmp_buffer, " \t\n");
    unsigned int i = 1;

    while(tok != NULL)
    {
        if(i == 2)
        {
            break;
        }
        i++;
        tok = strtok(NULL, " \t\n");
    }

    if(i == 2)
    {
        (*buffer) = malloc(strlen(tok) + 1);
        strncpy(*buffer, tok, strlen(tok) + 1);
    }
    else
    {
        fprintf(stderr, "dlt_procfs_read_process_stat_cmdline(): cmdline entry not found\n");
        return DLT_RETURN_ERROR;
    }

    free(tmp_buffer);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_get_msg_process_update(DltProcfsProcess *process, char *buffer, int maxlen)
{
    if(process == NULL || buffer == NULL)
    {
        fprintf(stderr, "dlt_procfs_log_process_new(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d;%lu;%ld;%ld;%lu;%lu", process->pid, process->cpu_time, process->rss, process->ctx_switches, process->io_bytes, process->io_wait);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_get_msg_process_new(DltProcfsProcess *process, char *buffer, int maxlen)
{
    if(process == NULL || buffer == NULL)
    {
        fprintf(stderr, "dlt_procfs_log_process_new(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d;%d;%s", process->pid, process->ppid, process->command_line);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_get_msg_process_stop(DltProcfsProcess *process, char *buffer, int maxlen)
{
    if(process == NULL || buffer == NULL)
    {
        fprintf(stderr, "dlt_procfs_log_process_new(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d", process->pid);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_get_msg_process_commandline(DltProcfsProcess *process, char *buffer, int maxlen)
{
    if(process == NULL || buffer == NULL)
    {
        fprintf(stderr, "dlt_procfs_log_process_new(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    snprintf(buffer, maxlen, "%d;%s", process->pid, process->command_line);

    return DLT_RETURN_OK;
}
