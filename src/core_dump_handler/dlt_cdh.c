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
 * \author Magneti Marelli http://www.magnetimarelli.com
 * \author Lutz Helwing <lutz_helwing@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_cdh.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "dlt_cdh.h"
#include <dirent.h>

/* Unusual characters in a windows filename are replaced */
#define UNUSUAL_CHARS                           ":/\\!*"
#define REPLACEMENT_CHAR                        '_'

#define COREDUMP_FILESYSTEM                     "/var"
#define COREDUMP_FILESYSTEM_MIN_SIZE_MB         40
#define COREDUMP_HANDLER_PRIORITY               -19

void core_locks(const proc_info_t *p_proc, int action);

/* ===================================================================
** Method      : init_proc_info(...)
**
** Description : initialises all members of process info structure to defined values
**
** Parameters  : INPUT p_proc
**               OUTPUT pointer to initialised crashed process info structure
**
** Returns     : nothing
** ===================================================================*/
void init_proc_info(proc_info_t *p_proc)
{
    memset(p_proc->name, 0, sizeof(p_proc->name));
    memset(p_proc->threadname, 0, sizeof(p_proc->threadname));

    p_proc->pid = 0;
    p_proc->timestamp = 0;
    p_proc->signal = 0;

    p_proc->can_create_coredump = 1;
    memset(&p_proc->streamer, 0, sizeof(p_proc->streamer));

    memset(&p_proc->m_Ehdr, 0, sizeof(p_proc->m_Ehdr));
    p_proc->m_pPhdr = NULL;
    p_proc->m_Nhdr = NULL;

    p_proc->m_note_page_size = 0;

    memset(&p_proc->m_registers, 0, sizeof(p_proc->m_registers));

    p_proc->m_crashed_pid = 0;
    p_proc->m_crashid_phase1 = 0;
    memset(p_proc->m_crashid, 0, sizeof(p_proc->m_crashid));
}

/* ===================================================================
** Method      : read_args(...)
**
** Description : reads command line arguments
**
** Parameters  : INPUT argc
**               INPUT argv
**               OUTPUT pointer to crashed process info structure
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t read_args(int argc, char **argv, proc_info_t *proc)
{
    if (argc < 5) {
        syslog(LOG_ERR, "Usage: cdh timestamp pid signal procname");
        return CDH_NOK;
    }

    init_proc_info(proc);

    if (sscanf(argv[1], "%u", &proc->timestamp) != 1) {
        syslog(LOG_ERR, "Unable to read timestamp argument <%s>. Closing", argv[1]);
        return CDH_NOK;
    }

    if (sscanf(argv[2], "%d", &proc->pid) != 1) {
        syslog(LOG_ERR, "Unable to read pid argument <%s>. Closing", argv[2]);
        return CDH_NOK;
    }

    if (sscanf(argv[3], "%d", &proc->signal) != 1) {
        syslog(LOG_ERR, "Unable to read signal argument <%s>. Closing", argv[3]);
        return CDH_NOK;
    }

    /* save the thread name given by the kernel */
    strncpy(proc->threadname, argv[4], sizeof(proc->threadname) - 1);

    /* initialize the binary name with threadname... in case we cannot read it from /proc */
    strncpy(proc->name, argv[4], sizeof(proc->name) - 1);

    return CDH_OK;
}

/* ===================================================================
** Method      : remove_unusual_chars(...)
**
** Description : modify the input string to change UNUSUALS_CHARS to
**              REPLACEMENT_CHAR
** Parameters  : INPUT/OUTPUT  string to be modified
**
** Returns     : nothing
** ===================================================================*/
void remove_unusual_chars(char *p_string)
{
    unsigned int l_char_index = 0;

    for (l_char_index = 0; l_char_index < sizeof(UNUSUAL_CHARS) - 1; l_char_index++) {
        char *l_str_pointer = p_string;

        do {
            l_str_pointer = strchr(l_str_pointer, UNUSUAL_CHARS[l_char_index]);

            if (l_str_pointer != NULL) {
                *l_str_pointer = REPLACEMENT_CHAR;
                l_str_pointer++;
            }
        } while (l_str_pointer != NULL);
    }
}

/* ===================================================================
** Method      : check_disk_space(...)
**
** Description : check if there is sufficient disk space to write a coredump
** Parameters  : INPUT/OUTPUT  string to be modified
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t check_disk_space()
{
    struct statvfs stat;
    unsigned long free_size = 0;

    if (statvfs(COREDUMP_FILESYSTEM, &stat) < 0) {
        syslog(LOG_ERR, "ERR cannot stat disk space on %s: %s", COREDUMP_FILESYSTEM, strerror(errno));
        return CDH_NOK;
    }

    /* free space: size of block * number of free blocks (>>20 => MB) */
    free_size = (stat.f_bsize * stat.f_bavail) >> 20;

    if (free_size < COREDUMP_FILESYSTEM_MIN_SIZE_MB) {
        syslog(LOG_WARNING, "ERR insufficient disk space for coredump: %ld MB.", free_size);
        return CDH_NOK;
    }

    syslog(LOG_INFO, "INFO disk space for coredump: %ld MB.", free_size);
    return CDH_OK;
}

void clean_core_tmp_dir()
{
    DIR *d = NULL;
    struct dirent *dir = NULL;

    if ((d = opendir(CORE_TMP_DIRECTORY)) != NULL) {
        char lockfilepath[CORE_MAX_FILENAME_LENGTH];

        while ((dir = readdir(d)) != NULL) {
            struct stat unused_stat;

            /* check if lock file exists */
            snprintf(lockfilepath, sizeof(lockfilepath), "%s/%s", CORE_LOCK_DIRECTORY, dir->d_name);

            if (stat(lockfilepath, &unused_stat) != 0) {
                /* No lock file found for this coredump => from previous LC => delete */
                char filepath[CORE_MAX_FILENAME_LENGTH] = { 0 };

                snprintf(filepath, sizeof(filepath), "%s/%s", CORE_TMP_DIRECTORY, dir->d_name);

                syslog(LOG_INFO, "Cleaning %s: delete file %s", CORE_TMP_DIRECTORY, filepath);

                unlink(filepath);
            }
        }

        closedir(d);
    }
}

/* ===================================================================
** Method      : check_core_directory(...)
**
** Description : checks the availability of core dumps directory.
**               if not available, there is an installation issue.
**
** Parameters  :
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t check_and_create_directory(const char *p_dirname, int create_silently)
{
    int l_need_create = 0;
    int l_need_delete = 0;

    struct stat l_stat;

    if (lstat(p_dirname, &l_stat) < 0) {
        l_need_create = 1;
    }
    else if (!S_ISDIR(l_stat.st_mode))
    {
        l_need_delete = 1;
        l_need_create = 1;
    }

    if (l_need_delete > 0) {
        syslog(LOG_WARNING, "WARN core directory '%s' is not a directory => removing it", p_dirname);

        if (unlink(p_dirname) == -1) {
            syslog(LOG_ERR, "ERR core directory '%s' cannot be unlinked: %s", p_dirname, strerror(errno));
            return CDH_NOK;
        }
    }

    if (l_need_create > 0) {
        if (create_silently == 0)
            syslog(LOG_WARNING, "WARN core directory '%s' does not exist => creation", p_dirname);

        if (mkdir(p_dirname, 0666) == -1) {
            syslog(LOG_ERR, "ERR core directory '%s' cannot be created: %s", p_dirname, strerror(errno));
            return CDH_NOK;
        }
    }

    return CDH_OK;
}

/* ===================================================================
** Method      : check_core_directory(...)
**
** Description : checks the availability of core dumps directory.
**               if not available, there is an installation issue.
**
** Parameters  :
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t check_core_directory()
{
    if (check_and_create_directory(CORE_DIRECTORY, 0) < 0)
        return CDH_NOK;

    if (check_and_create_directory(CORE_TMP_DIRECTORY, 0) < 0)
        return CDH_NOK;

    if (check_and_create_directory(CORE_LOCK_DIRECTORY, 1) < 0)
        return CDH_NOK;

    clean_core_tmp_dir();

    return CDH_OK;
}

/* ===================================================================
** Method      : move_to_core_directory(...)
**
** Description : move the coredump and context files
**              from temporary dir to final core directory
**
** Parameters  :
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t move_to_core_directory(proc_info_t *p_proc)
{
    char l_src_filename[CORE_MAX_FILENAME_LENGTH] = { 0 };
    char l_dst_filename[CORE_MAX_FILENAME_LENGTH] = { 0 };
    char *patterns[] = { CORE_FILE_PATTERN, CONTEXT_FILE_PATTERN };
    unsigned int pattern_num = 0;

    if (p_proc == NULL)
        return CDH_NOK;

    for (pattern_num = 0; pattern_num < sizeof(patterns) / sizeof(char *); pattern_num++) {
        /* Don't move coredump if it cannot be created */
        if ((p_proc->can_create_coredump == 0) && (pattern_num == 0))
            continue;

        snprintf(l_src_filename, sizeof(l_src_filename), patterns[pattern_num],
                 CORE_TMP_DIRECTORY, p_proc->timestamp, p_proc->name, p_proc->pid);

        snprintf(l_dst_filename, sizeof(l_dst_filename), patterns[pattern_num],
                 CORE_DIRECTORY, p_proc->timestamp, p_proc->name, p_proc->pid);

        syslog(LOG_INFO, "Moving coredump from %s to %s", l_src_filename, l_dst_filename);

        if (rename(l_src_filename, l_dst_filename) < 0)
            syslog(LOG_ERR, "Moving failed: %s", strerror(errno));
    }

    return CDH_OK;
}

/* ===================================================================
** Method      : main(...)
**
** Description :
**
** Parameters  : argc, argv
**
** Returns     :
** ===================================================================*/
int main(int argc, char *argv[])
{
    proc_info_t l_proc_info;
/*    char l_exec_name[CORE_MAX_FILENAME_LENGTH] = {0}; */

    openlog("CoredumpHandler", 0, LOG_DAEMON);

    if (read_args(argc, argv, &l_proc_info) < 0)
        exit(-1);

    if (get_exec_name(l_proc_info.pid, l_proc_info.name, sizeof(l_proc_info.name)) != 0)
        syslog(LOG_ERR, "Failed to get executable name");

    syslog(LOG_NOTICE, "Handling coredump procname:%s pid:%d timest:%d signal:%d",
           l_proc_info.name,
           l_proc_info.pid,
           l_proc_info.timestamp,
           l_proc_info.signal);

    /* Increase priority of the coredump handler */
    if (nice(COREDUMP_HANDLER_PRIORITY) != COREDUMP_HANDLER_PRIORITY)
        syslog(LOG_WARNING, "Failed to change CDH priority");

    if (check_disk_space() < 0)
        /*return CDH_NOK; */
        l_proc_info.can_create_coredump = 0;

    if (check_core_directory() < 0)
        /*return CDH_NOK; */
        l_proc_info.can_create_coredump = 0;

    remove_unusual_chars(l_proc_info.name);

    core_locks(&l_proc_info, 1);

    write_proc_context(&l_proc_info);

    treat_coredump(&l_proc_info);

    move_to_core_directory(&l_proc_info);

    core_locks(&l_proc_info, 0);

    treat_crash_data(&l_proc_info);

    closelog();

    return CDH_OK;
}

void core_locks(const proc_info_t *p_proc, int action)
{
    char l_lockfilepath[CORE_MAX_FILENAME_LENGTH] = { 0 };
    char *patterns[] = { CORE_FILE_PATTERN, CONTEXT_FILE_PATTERN };
    unsigned int pattern_num = 0;
    int fd_lockfile = -1;

    if (p_proc == NULL)
        return;

    for (pattern_num = 0; pattern_num < sizeof(patterns) / sizeof(char *); pattern_num++) {
        snprintf(l_lockfilepath, sizeof(l_lockfilepath), patterns[pattern_num],
                 CORE_LOCK_DIRECTORY, p_proc->timestamp, p_proc->name, p_proc->pid);

        switch (action) {
        case 0:
        {
            unlink(l_lockfilepath);
            break;
        }

        case 1:
        {
            if ((fd_lockfile = open(l_lockfilepath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) >= 0) {
                if (write(fd_lockfile, "1", 1) < 0)
                    syslog(LOG_WARNING, "Failed to write lockfile %d: %s", fd_lockfile, strerror(errno));

                close(fd_lockfile);
            }
            else {
                syslog(LOG_WARNING, "Failed to open lockfile %s: %s", l_lockfilepath, strerror(errno));
            }

            break;
        }

        default:
            break;
        }
    }
}
