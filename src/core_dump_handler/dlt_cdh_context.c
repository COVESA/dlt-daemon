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
 * \file dlt_cdh_context.c
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <syslog.h>

#include "dlt_cdh.h"

/* Global buffer for file reading */
char g_buffer[4096];

/* ===================================================================
** Method      : get_exec_name(...)
**
** Description : read executable filename
**
** Parameters  : INPUT p_pid_str            pid of the process
**               OUTPUT p_exec_name         executable name
**               INPUT p_exec_name_maxsize  size of p_exec_name buffer
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t get_exec_name(unsigned int p_pid, char *p_exec_name, int p_exec_name_maxsize)
{
    char l_exe_link[CORE_MAX_FILENAME_LENGTH] = { 0 };
    char *l_name_ptr = NULL;

    memset(l_exe_link, 0, sizeof(l_exe_link));
    snprintf(l_exe_link, sizeof(l_exe_link) - 1, "/proc/%d/exe", p_pid);

    if (readlink(l_exe_link, g_buffer, p_exec_name_maxsize) < 0)
        return CDH_NOK;

    if ((l_name_ptr = strrchr(g_buffer, '/')) == NULL)
        return CDH_NOK;

    memset(p_exec_name, 0, p_exec_name_maxsize);
    strncpy(p_exec_name, l_name_ptr + 1, p_exec_name_maxsize - 1);

    return CDH_OK;
}

/* ===================================================================
** Method      : dump_file_to(...)
**
** Description : dump the content of file p_src_filename to the file descriptor p_fout
**
** Parameters  : INPUT p_src_filename
**               INPUT p_fout
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t dump_file_to(const char *p_src_filename, FILE *p_fout)
{
    FILE *l_fin = NULL;
    int bytes_read = 0;

    if (p_fout == NULL)
        return CDH_NOK;

    fprintf(p_fout, "\n==== Dumping file <%s> ====\n", p_src_filename);

    if ((l_fin = fopen(p_src_filename, "rt")) == NULL) {
        syslog(LOG_ERR, "ERR opening info file '%s' for dumping [%s]",
               p_src_filename,
               strerror(errno));

        fprintf(p_fout, "**error**\n");

        return CDH_NOK;
    }

    while ((bytes_read = fread(g_buffer, 1, sizeof(g_buffer), l_fin)) != 0) {
        int i = 0;

        /* changes all "\0" in the file to a "\n" */
        /* (needed for example for /proc/<pid>/cmdline, to keep all arguments) */
        for (i = 0; i < bytes_read; i++)
            if (g_buffer[i] == '\000')
                g_buffer[i] = '\n';

        fwrite(g_buffer, 1, bytes_read, p_fout);

        if (ferror(p_fout)) {
            syslog(LOG_ERR, "Writing in context file failed [%s]", strerror(errno));
            fclose(p_fout);
            fclose(l_fin);

            return CDH_NOK;

        }
    }

    if (ferror(l_fin)) {
        syslog(LOG_ERR, "reading '%s' failed [%s]", p_src_filename, strerror(errno));
        fclose(l_fin);

        return CDH_NOK;
    }

    fclose(l_fin);

    fprintf(p_fout, "\n");
    return CDH_OK;
}

/************************************************************************************************** / */
/* "ls -l" implementation for /proc/<pid>/fd (at least) */
/* Taken from coreutils sources, lib/filemode.c */
/* */
/* Return a character indicating the type of file described by
 * file mode BITS:
 * '-' regular file
 * 'b' block special file
 * 'c' character special file
 * 'C' high performance ("contiguous data") file
 * 'd' directory
 * 'D' door
 * 'l' symbolic link
 * 'm' multiplexed file (7th edition Unix; obsolete)
 * 'n' network special file (HP-UX)
 * 'p' fifo (named pipe)
 * 'P' port
 * 's' socket
 * 'w' whiteout (4.4BSD)
 * '?' some other file type  */

static char ftypelet(mode_t bits)
{
    /* These are the most common, so test for them first.  */
    if (S_ISREG(bits))
        return '-';

    if (S_ISDIR(bits))
        return 'd';

    /* Other letters standardized by POSIX 1003.1-2004.  */
    if (S_ISBLK(bits))
        return 'b';

    if (S_ISCHR(bits))
        return 'c';

    if (S_ISLNK(bits))
        return 'l';

    if (S_ISFIFO(bits))
        return 'p';

    /* Other file types (though not letters) standardized by POSIX.  */
    if (S_ISSOCK(bits))
        return 's';

    /* Nonstandard file types.
     * if (S_ISCTG (bits))
     * return 'C';
     * if (S_ISDOOR (bits))
     * return 'D';
     * if (S_ISMPB (bits) || S_ISMPC (bits))
     * return 'm';
     * if (S_ISNWK (bits))
     * return 'n';
     * if (S_ISPORT (bits))
     * return 'P';
     * if (S_ISWHT (bits))
     * return 'w';
     */

    return '?';
}

void strmode(mode_t mode, char *str)
{
    if (str == NULL)
        return;

    str[0] = ftypelet(mode);
    str[1] = mode & S_IRUSR ? 'r' : '-';
    str[2] = mode & S_IWUSR ? 'w' : '-';
    str[3] = (mode & S_ISUID
              ? (mode & S_IXUSR ? 's' : 'S')
              :
              (mode & S_IXUSR ? 'x' : '-'));
    str[4] = mode & S_IRGRP ? 'r' : '-';
    str[5] = mode & S_IWGRP ? 'w' : '-';
    str[6] = (mode & S_ISGID
              ? (mode & S_IXGRP ? 's' : 'S')
              :
              (mode & S_IXGRP ? 'x' : '-'));
    str[7] = mode & S_IROTH ? 'r' : '-';
    str[8] = mode & S_IWOTH ? 'w' : '-';
    str[9] = (mode & S_ISVTX
              ? (mode & S_IXOTH ? 't' : 'T')
              :
              (mode & S_IXOTH ? 'x' : '-'));
    str[10] = ' ';
    str[11] = '\0';
}

/* ===================================================================
** Method      : list_dircontent_to(...)
**
** Description : list the filenames in p_dirname directory to the file descriptor p_fout
**
** Parameters  : INPUT p_dirname
**               INPUT p_fout
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t list_dircontent_to(const char *p_dirname, FILE *p_fout)
{
    DIR *l_dd = NULL; /* directory descriptor */
    struct dirent *l_entity = NULL;

    if ((l_dd = opendir(p_dirname)) == NULL) {
        syslog(LOG_ERR, "ERR reading info dir '%s' failed [%s]", p_dirname, strerror(errno));
        return CDH_NOK;
    }

    fprintf(p_fout, "==== Listing directory <%s> ====\n", p_dirname);

    while ((l_entity = readdir(l_dd)) != NULL) {
        char l_fullpath[CORE_MAX_FILENAME_LENGTH] = { 0 };
        char l_linkpath[CORE_MAX_FILENAME_LENGTH] = { 0 };
        char l_modebuf[12] = { 0 };

        struct stat l_stat;
        ssize_t l_size = 0;

        if (!strcmp(l_entity->d_name, ".") || !strcmp(l_entity->d_name, ".."))
            continue;

        snprintf(l_fullpath, sizeof(l_fullpath), "%s/%s", p_dirname, l_entity->d_name);

        if (lstat(l_fullpath, &l_stat) < 0) {
            syslog(LOG_ERR, "ERR lstat on '%s' failed. [%s]", l_fullpath, strerror(errno));
            continue;
        }

        strmode(l_stat.st_mode, l_modebuf);

        fprintf(p_fout, "%s  %ld  %d %d %ld %4s",
                l_modebuf,
                l_stat.st_nlink,
                l_stat.st_uid,
                l_stat.st_gid,
                l_stat.st_size,
                l_entity->d_name);

        switch (l_stat.st_mode & S_IFMT) {
        case S_IFBLK:
            fprintf(p_fout, " [block device]\n");
            break;

        case S_IFCHR:
            fprintf(p_fout, " [character device]\n");
            break;

        case S_IFDIR:
            fprintf(p_fout, " [directory]\n");
            break;

        case S_IFIFO:
            fprintf(p_fout, " [FIFO/pipe]\n");
            break;

        case S_IFLNK:
            l_size = readlink(l_fullpath, l_linkpath, sizeof(l_linkpath));
            l_linkpath[l_size] = 0;
            fprintf(p_fout, " -> %s\n", l_linkpath);
            break;

        case S_IFREG:
            fprintf(p_fout, " [regular file]\n");
            break;

        case S_IFSOCK:
            fprintf(p_fout, " [socket]\n");
            break;

        default:
            fprintf(p_fout, " [unknown?]\n");
            break;
        }
    } /* while ( (l_entity = readdir(l_dd)) != NULL ) */

    fprintf(p_fout, "===========================\n");
    closedir(l_dd);

    return CDH_OK;
}

/************************************************************************************************** / */
/* END of "ls -l" implementation for /proc/<pid>/fd (at least) */
/************************************************************************************************** / */

/* ===================================================================
** Method      : write_proc_context(...)
**
** Description : write the context data of the crashed process
**               (context data coming mainly from /proc)
**
** Parameters  : INPUT p_proc   crashed process info
**
** Returns     : 0 if success, else -1
** ===================================================================*/
cdh_status_t write_proc_context(const proc_info_t *p_proc)
{
    FILE *l_fout = NULL;
    char l_procfile[256] = { 0 };
    char l_outfilename[CORE_MAX_FILENAME_LENGTH] = { 0 };

    if (p_proc == NULL)
        return CDH_NOK;

    snprintf(l_outfilename, sizeof(l_outfilename), CONTEXT_FILE_PATTERN,
             CORE_TMP_DIRECTORY,
             p_proc->timestamp,
             p_proc->name,
             p_proc->pid);

    if ((l_fout = fopen(l_outfilename, "w+t")) == NULL) {
        syslog(LOG_ERR, "ERR Cannot open context file '%s' [%s]", l_outfilename, strerror(errno));
        return CDH_NOK;
    }

#define PROC_FILENAME(x) do { \
        snprintf(l_procfile, sizeof(l_procfile), "/proc/%d/"x, \
                 p_proc->pid); \
} while (0)

    fprintf(l_fout, "ProcName:%s\n", p_proc->name);
    fprintf(l_fout, "ThreadName:%s\n", p_proc->threadname);
    fprintf(l_fout, "PID:%d\n", p_proc->pid);
    fprintf(l_fout, "signal:%d\n", p_proc->signal);

    PROC_FILENAME("cmdline");
    dump_file_to(l_procfile, l_fout);

    PROC_FILENAME("cgroup");
    dump_file_to(l_procfile, l_fout);

    PROC_FILENAME("stack");
    dump_file_to(l_procfile, l_fout);

    dump_file_to("/proc/loadavg", l_fout);
    dump_file_to("/etc/sysrel", l_fout);
    dump_file_to("/proc/version", l_fout);

    PROC_FILENAME("environ");
    dump_file_to(l_procfile, l_fout);

    PROC_FILENAME("status");
    dump_file_to(l_procfile, l_fout);

    PROC_FILENAME("sched");
    dump_file_to(l_procfile, l_fout);

    PROC_FILENAME("maps");
    dump_file_to(l_procfile, l_fout);

    PROC_FILENAME("fd");
    list_dircontent_to(l_procfile, l_fout);

#undef PROC_FILENAME

    fflush(l_fout);

    return CDH_OK;
}

