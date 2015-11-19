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
 * \file dlt-procfs.c
 */

#include "dlt-procfs.h"
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

DLT_DECLARE_CONTEXT(procfs_ctx);

DltProcfsConfig config;

static volatile sig_atomic_t stop_loop = 0;
static DltProcfsProcessList *list, *new_process_list, *stopped_process_list, *update_process_list;
static struct timespec _tmp_time;
static pthread_mutex_t process_list_mutex;

void dlt_procfs_stop_loops(int sig);
void dlt_procfs_init_sigterm_handler();
DltReturnValue dlt_procfs_init_process_lists();
DltReturnValue dlt_procfs_free_process_lists();
void *dlt_procfs_start_process_thread();
DltReturnValue dlt_procfs_process_loop();
DltReturnValue dlt_procfs_update_process_list(DltProcfsProcessList *list, unsigned long int time_dif_ms);
void *dlt_procfs_start_irq_thread();
DltReturnValue dlt_procfs_irq_loop();
void *dlt_procfs_start_check_thread();
DltReturnValue dlt_procfs_check_loop();
DltReturnValue dlt_procfs_log_check_commandlines();

unsigned long int timespec_to_millis(struct timespec *time)
{
    return (time->tv_sec) * 1000 + (time->tv_nsec / 1000000);
}

unsigned long int get_millis()
{
    clock_gettime(CLOCK_REALTIME, &_tmp_time);
    return timespec_to_millis(&_tmp_time);
}

int main(int argc, char **argv)
{
    printf("Launching dlt-procfs...\n");

    if(dlt_procfs_init(argc, argv, &config) < DLT_RETURN_OK)
    {
        fprintf(stderr, "Initialization error!\n");
        return -1;
    }

    dlt_procfs_init_sigterm_handler();

    if(dlt_procfs_init_process_lists() < DLT_RETURN_OK)
    {
        fprintf(stderr, "Error occurred initializing process lists\n");
        return -1;
    }

    if(pthread_mutex_init(&process_list_mutex, NULL) < 0)
    {
        fprintf(stderr, "Error occurred initializing mutex\n");
        return -1;
    }

    DLT_REGISTER_APP("PROC", "/proc/-filesystem logger application");
    DLT_REGISTER_CONTEXT_LL_TS(procfs_ctx, "PROC", "/proc/-filesystem logger context", config.log_level, 0);

    pthread_t process_thread;
    pthread_t irq_thread;
    pthread_t check_thread;

    if(pthread_create(&process_thread, NULL, &dlt_procfs_start_process_thread, NULL) != 0)
    {
        fprintf(stderr, "Could not create thread\n");
        return -1;
    }
    if(pthread_create(&irq_thread, NULL, &dlt_procfs_start_irq_thread, NULL) != 0)
    {
        fprintf(stderr, "Could not create thread\n");
        return -1;
    }
    if(pthread_create(&check_thread, NULL, &dlt_procfs_start_check_thread, NULL) != 0)
    {
        fprintf(stderr, "Could not create thread\n");
        return -1;
    }

    pthread_join(process_thread, NULL);
    pthread_join(irq_thread, NULL);
    pthread_join(check_thread, NULL);

    DLT_UNREGISTER_CONTEXT(procfs_ctx);
    DLT_UNREGISTER_APP();

    pthread_mutex_destroy(&process_list_mutex);

    dlt_procfs_free_process_lists();

    printf("Done.\n");
}

void dlt_procfs_init_sigterm_handler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = dlt_procfs_stop_loops;

    sigaction(SIGTERM, &action, NULL);
}

void dlt_procfs_stop_loops(int sig)
{
    if(sig > -1)
        fprintf(stderr, "dlt-procfs is now terminating due to signal %d...\n", sig);
    else
        fprintf(stderr, "dlt-procfs is now terminating due to an error...\n");

    stop_loop = 1;
}

DltReturnValue dlt_procfs_init_process_lists()
{
    if((list                 = dlt_procfs_create_process_list()) == NULL) return DLT_RETURN_ERROR;
    if((new_process_list     = dlt_procfs_create_process_list()) == NULL) return DLT_RETURN_ERROR;
    if((stopped_process_list = dlt_procfs_create_process_list()) == NULL) return DLT_RETURN_ERROR;
    if((update_process_list  = dlt_procfs_create_process_list()) == NULL) return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_free_process_lists()
{
    DltReturnValue ret = DLT_RETURN_OK;

    if(dlt_procfs_free_process_list(list) < DLT_RETURN_OK)
        ret = DLT_RETURN_ERROR;

    if(dlt_procfs_free_process_list(new_process_list) < DLT_RETURN_OK)
        ret = DLT_RETURN_ERROR;

    if(dlt_procfs_free_process_list(stopped_process_list) < DLT_RETURN_OK)
        ret = DLT_RETURN_ERROR;

    if(dlt_procfs_free_process_list(update_process_list) < DLT_RETURN_OK)
        ret = DLT_RETURN_ERROR;

    return ret;
}

void *dlt_procfs_start_process_thread()
{
    if(dlt_procfs_process_loop() < DLT_RETURN_OK)
        dlt_procfs_stop_loops(-1);

    return NULL;
}

DltReturnValue dlt_procfs_process_loop()
{
    static unsigned long int old_millis, sleep_millis, dif_millis;

    old_millis = get_millis();

    while(!stop_loop)
    {
        /*DltReturnValue ret = */ dlt_procfs_update_process_list(list, config.process_log_interval);
        //if(ret < DLT_RETURN_OK)
        //    return ret;

        dif_millis = get_millis() - old_millis;

        if(dif_millis >= (unsigned long)(config.process_log_interval))
            sleep_millis = 0;
        else
            sleep_millis = config.process_log_interval - dif_millis;

        usleep(sleep_millis * 1000);

        old_millis = get_millis();
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_log_list(DltProcfsProcessList *list, DltReturnValue(*process_callback)(DltProcfsProcess*, char*, int), char *title, int delete_elements)
{
    if(list == NULL || process_callback == NULL || title == NULL)
    {
        fprintf(stderr, "dlt_procfs_log_list(): Nullpointer parameter\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_procfs_reset_cursor(list);
    if(list->cursor == NULL)
        return DLT_RETURN_OK; // list empty; nothing to do

    DltReturnValue ret;
    DltContextData data;

    char buffer[BUFFER_SIZE];
    buffer[0] = '\0';

    if((ret = dlt_user_log_write_start(&procfs_ctx, &data, config.log_level)) < DLT_RETURN_OK)
    {
        fprintf(stderr, "dlt_procfs_log_list(): dlt_user_log_write_start() returned error.\n");
        return ret;
    }

    if((ret = dlt_user_log_write_string(&data, title)) < DLT_RETURN_OK)
    {
        fprintf(stderr, "dlt_procfs_log_list(): dlt_user_log_write_string() returned error.\n");
        return ret;
    }

    do
    {
        if((ret = (*process_callback)(list->cursor, buffer, sizeof(buffer) - 1)) < DLT_RETURN_OK)
            return ret;

        if((ret = dlt_user_log_write_string(&data, buffer)) < DLT_RETURN_OK)
        {
            /* Log buffer full => Write log and start new one*/
            if((ret = dlt_user_log_write_finish(&data)) < DLT_RETURN_OK)
            {
                fprintf(stderr, "dlt_procfs_log_list(): dlt_user_log_write_finish() returned error.\n");
                return ret;
            }

            if((ret = dlt_user_log_write_start(&procfs_ctx, &data, config.log_level)) < DLT_RETURN_OK)
            {
                fprintf(stderr, "dlt_procfs_log_list(): dlt_user_log_write_start() returned error.\n");
                return ret;
            }

            if((ret = dlt_user_log_write_string(&data, title)) < DLT_RETURN_OK)
            {
                fprintf(stderr, "dlt_procfs_log_list(): dlt_user_log_write_string() returned error.\n");
                return ret;
            }
        }
        else if(delete_elements)
        {
            if((ret = dlt_procfs_remove_process_at_cursor(list)) < DLT_RETURN_OK)
                return ret;
        }
        else
        {
            list->cursor = list->cursor->next;
        }
    }
    while(list->cursor != NULL);

    if((ret = dlt_user_log_write_finish(&data)) < DLT_RETURN_OK)
    {
        fprintf(stderr, "dlt_procfs_log_list(): dlt_user_log_write_finish() returned error.\n");
        return ret;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_update_process_list(DltProcfsProcessList *list, unsigned long int time_dif_ms)
{
    static char *strchk;
    static DltReturnValue tmp_ret;
    static struct dirent *current_dir;
    static pid_t current_dir_pid;

    if(list == NULL)
    {
        fprintf(stderr, "dlt_procfs_update_process_list(): Nullpointer parameter");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    DIR *proc_dir = opendir("/proc");
    if(proc_dir == NULL)
    {
        dlt_log(LOG_ERR, "Could not open /proc/ !\n");
        return DLT_RETURN_ERROR;
    }

    current_dir = readdir(proc_dir);
    dlt_procfs_reset_cursor(list);

    int debug_process_count = 0;

    if(pthread_mutex_lock(&process_list_mutex) < 0)
    {
        fprintf(stderr, "Can't lock mutex\n");
        return DLT_RETURN_ERROR;
    }

    while(1)
    {
        if(current_dir == NULL)
        {
            /* no more active processes.. delete all remaining processes in the list */
            if(list->cursor != NULL)
                while(list->cursor != NULL)
                {
                    if((tmp_ret = dlt_procfs_add_process_after_cursor(stopped_process_list, dlt_procfs_clone_process(list->cursor))) < DLT_RETURN_OK)
                        return tmp_ret;

                    dlt_procfs_remove_process_at_cursor(list);
                }

            break;
        }

        current_dir_pid = strtol(current_dir->d_name, &strchk, 10);
        if(*strchk != '\0' || current_dir_pid <= 0)
        {
            /* no valid PID */
            current_dir = readdir(proc_dir); // next process in proc-fs
            continue;
        }

        /* compare the /proc/-filesystem with our process-list */
        if(list->cursor == NULL || current_dir_pid < list->cursor->pid) // New Process
        {
            DltProcfsProcess *new_process = dlt_procfs_create_process(current_dir_pid);
            if(new_process == NULL)
            {
                fprintf(stderr, "Error: Could not create process (out of memory?)\n");
                return DLT_RETURN_ERROR;
            }

            if((tmp_ret = dlt_procfs_add_process_before_cursor(list, new_process)) < DLT_RETURN_OK)
                return tmp_ret;

            if((tmp_ret = dlt_procfs_add_process_before_cursor(new_process_list, dlt_procfs_clone_process(new_process))) < DLT_RETURN_OK)
                return tmp_ret;

            current_dir = readdir(proc_dir); // next process in proc-fs
            debug_process_count++;
        }
        else if(current_dir_pid > list->cursor->pid) // Process ended
        {
            if((tmp_ret = dlt_procfs_add_process_after_cursor(stopped_process_list, dlt_procfs_clone_process(list->cursor))) < DLT_RETURN_OK)
                return tmp_ret;

            if((tmp_ret = dlt_procfs_remove_process_at_cursor(list)) < DLT_RETURN_OK)
                return tmp_ret;
        }
        else if(current_dir_pid == list->cursor->pid) // Staying process
        {
            /* update data */
            if((tmp_ret = dlt_procfs_update_process(list->cursor, time_dif_ms)) < DLT_RETURN_OK)
                    return tmp_ret;

            if(list->cursor->cpu_time > 0) // only log active processes
                if((tmp_ret = dlt_procfs_add_process_after_cursor(update_process_list, dlt_procfs_clone_process(list->cursor))) < DLT_RETURN_OK)
                {
                    fprintf(stderr, "dlt_procfs_update_process_list: Can't add process to list updateProcessList\n");
                    return tmp_ret;
                }

            if((tmp_ret = dlt_procfs_increment_cursor(list)) < DLT_RETURN_OK) // next process in list
                return tmp_ret;

            current_dir = readdir(proc_dir); // next process in proc-fs
            debug_process_count++;
        }
    }

    if(pthread_mutex_unlock(&process_list_mutex) < 0)
    {
        fprintf(stderr, "Can't unlock mutex\n");
        return DLT_RETURN_ERROR;
    }

    /* Log new processes */
    if((tmp_ret = dlt_procfs_log_list(new_process_list, &dlt_procfs_get_msg_process_new, "NEW", 1)) < DLT_RETURN_OK)
        return tmp_ret;

    /* Log stopped processes */
    if((tmp_ret = dlt_procfs_log_list(stopped_process_list, &dlt_procfs_get_msg_process_stop, "STP", 1)) < DLT_RETURN_OK)
        return tmp_ret;

    /* Log active processes */
    if((tmp_ret = dlt_procfs_log_list(update_process_list, &dlt_procfs_get_msg_process_update, "ACT", 1)) < DLT_RETURN_OK)
        return tmp_ret;

    if(closedir(proc_dir) < 0)
        fprintf(stderr, "Could not close /proc/ directory\n");

    return DLT_RETURN_OK;
}

void *dlt_procfs_start_irq_thread()
{
    if(dlt_procfs_irq_loop() < DLT_RETURN_OK)
        dlt_procfs_stop_loops(-1);

    return NULL;
}

DltReturnValue dlt_procfs_irq_loop()
{
    static unsigned long int old_millis, sleep_millis, dif_millis;

    old_millis = get_millis();

    while(!stop_loop)
    {
        /*DltReturnValue ret = */ dlt_procfs_log_interrupts(&procfs_ctx, config.log_level);
        //if(ret < DLT_RETURN_OK)
        //    return ret;

        dif_millis = get_millis() - old_millis;

        if(dif_millis >= (unsigned long)(config.irq_log_interval))
            sleep_millis = 0;
        else
            sleep_millis = config.irq_log_interval - dif_millis;

        usleep(sleep_millis * 1000);

        old_millis = get_millis();
    }

    return DLT_RETURN_OK;
}

void *dlt_procfs_start_check_thread()
{
    if(dlt_procfs_check_loop() < DLT_RETURN_OK)
        dlt_procfs_stop_loops(-1);

    return NULL;
}

DltReturnValue dlt_procfs_check_loop()
{
    static unsigned long int old_millis, sleep_millis, dif_millis;

    old_millis = get_millis();

    while(!stop_loop)
    {
        /*DltReturnValue ret = */ dlt_procfs_log_check_commandlines();
        //if(ret < DLT_RETURN_OK)
        //    return ret;

        dif_millis = get_millis() - old_millis;

        if(dif_millis >= (unsigned long)(config.check_log_interval))
            sleep_millis = 0;
        else
            sleep_millis = config.check_log_interval - dif_millis;

        usleep(sleep_millis * 1000);

        old_millis = get_millis();
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_procfs_log_check_commandlines()
{
    if(pthread_mutex_lock(&process_list_mutex) < 0)
    {
        fprintf(stderr, "Can't lock mutex\n");
        return DLT_RETURN_ERROR;
    }

    DltReturnValue ret = dlt_procfs_log_list(list, dlt_procfs_get_msg_process_commandline, "CHK", 0);

    if(pthread_mutex_unlock(&process_list_mutex) < 0)
    {
        fprintf(stderr, "Can't unlock mutex\n");
        return DLT_RETURN_ERROR;
    }

    return ret;
}
