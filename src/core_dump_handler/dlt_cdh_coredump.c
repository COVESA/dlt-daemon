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
 * \file dlt_cdh_coredump.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include "dlt_cdh.h"

cdh_status_t read_elf_headers(proc_info_t *p_proc)
{
    int phnum = 0;

    /* Read ELF header */
    stream_read(&p_proc->streamer, &p_proc->m_Ehdr, sizeof(p_proc->m_Ehdr));

    /* Read until PROG position */
    stream_move_to_offest(&p_proc->streamer, p_proc->m_Ehdr.e_phoff);

    /* Read and store all program headers */
    p_proc->m_pPhdr = (ELF_Phdr *)malloc(sizeof(ELF_Phdr) * p_proc->m_Ehdr.e_phnum);

    if (p_proc->m_pPhdr == NULL) {
        syslog(LOG_ERR, "Cannot allocate Phdr memory (%d headers)", p_proc->m_Ehdr.e_phnum);
        return CDH_NOK;
    }

    for (phnum = 0; phnum < p_proc->m_Ehdr.e_phnum; phnum++)
        /* Read Programm header */
        stream_read(&p_proc->streamer, &p_proc->m_pPhdr[phnum], sizeof(ELF_Phdr));

    return CDH_OK;
}

int getNotePageIndex(proc_info_t *p_proc)
{
    int i = 0;

    /* Search PT_NOTE section */
    for (i = 0; i < p_proc->m_Ehdr.e_phnum; i++) {
        syslog(LOG_INFO, "==Note section prog_note:%d type:0x%X offset:0x%X size:0x%X (%dbytes)",
               i,
               p_proc->m_pPhdr[i].p_type,
               p_proc->m_pPhdr[i].p_offset,
               p_proc->m_pPhdr[i].p_filesz,
               p_proc->m_pPhdr[i].p_filesz);

        if (p_proc->m_pPhdr[i].p_type == PT_NOTE)
            break;
    }

    return i == p_proc->m_Ehdr.e_phnum ? CDH_NOK : i;
}

cdh_status_t read_notes(proc_info_t *p_proc)
{
    int prog_note = getNotePageIndex(p_proc);
/*    p_proc->m_note_page_size = 0; */
    p_proc->m_Nhdr = NULL;

    /* note page not found, abort */
    if (prog_note < 0) {
        syslog(LOG_ERR, "Cannot find note header page index");
        return CDH_NOK;
    }

    /* Move to NOTE header position */
    if (stream_move_to_offest(&p_proc->streamer, p_proc->m_pPhdr[prog_note].p_offset) != CDH_OK) {
        syslog(LOG_ERR, "Cannot move to note header");
        return CDH_NOK;
    }

    if ((p_proc->m_Nhdr = (char *)malloc(p_proc->m_pPhdr[prog_note].p_filesz)) == NULL) {
        syslog(LOG_ERR, "Cannot allocate Nhdr memory (note size %d bytes)", p_proc->m_pPhdr[prog_note].p_filesz);
        return CDH_NOK;
    }

    if (stream_read(&p_proc->streamer, p_proc->m_Nhdr, p_proc->m_pPhdr[prog_note].p_filesz) != CDH_OK) {
        syslog(LOG_ERR, "Cannot read note header");
        return CDH_NOK;
    }

    p_proc->m_note_page_size = p_proc->m_pPhdr[prog_note].p_filesz;

    return CDH_OK;
}

cdh_status_t init_coredump(proc_info_t *p_proc)
{
    if (p_proc == NULL)
        return CDH_NOK;

    if (p_proc->can_create_coredump) {
        char l_dst_filename[CORE_MAX_FILENAME_LENGTH];

        snprintf(l_dst_filename, sizeof(l_dst_filename), CORE_FILE_PATTERN,
                 CORE_TMP_DIRECTORY,
                 p_proc->timestamp,
                 p_proc->name,
                 p_proc->pid);

        stream_init(&p_proc->streamer, 0, l_dst_filename);
    }
    else {
        stream_init(&p_proc->streamer, 0, NULL);
    }

    return CDH_OK;
}

cdh_status_t close_coredump(proc_info_t *p_proc)
{
    stream_close(&p_proc->streamer);

    return CDH_OK;
}

cdh_status_t treat_coredump(proc_info_t *p_proc)
{
    cdh_status_t ret = CDH_OK;

    /* open src and dest files, allocate read buffer */
    if (init_coredump(p_proc) != CDH_OK) {
        syslog(LOG_ERR, "cannot init coredump system");
        ret = CDH_NOK;
        goto finished;
    }

    if (read_elf_headers(p_proc) == CDH_OK) {
        /* TODO: No NOTES here leads to crash elsewhere!!! dlt_cdh_crashid.c: around line 76 */
        if (read_notes(p_proc) != CDH_OK) {
            syslog(LOG_ERR, "cannot read NOTES");
            ret = CDH_NOK;
            goto finished;
        }
    }
    else if (read_elf_headers(p_proc) != CDH_OK) {
        syslog(LOG_ERR, "cannot read ELF header");
        ret = CDH_NOK;
        goto finished;
    }

finished:

    /* In all cases, we try to finish to read/compress the coredump until the end */
    if (stream_finish(&p_proc->streamer) != CDH_OK)
        syslog(LOG_ERR, "cannot finish coredump compression");

    /* In all cases, let's close the files */
    if (close_coredump(p_proc) != CDH_OK)
        syslog(LOG_ERR, "cannot close coredump system");

    return ret;
}

