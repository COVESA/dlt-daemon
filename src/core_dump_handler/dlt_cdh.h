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
 * \file dlt_cdh.h
 */

#ifndef DLT_CDH_H
#define DLT_CDH_H

#include <unistd.h>
#include <stdint.h>
#include <elf.h>
#include <sys/procfs.h>
#include <sys/user.h>

#include "dlt_cdh_streamer.h"

#define CORE_DIRECTORY              "/var/core"
#define CORE_TMP_DIRECTORY          "/var/core_tmp"
#define CORE_LOCK_DIRECTORY         "/tmp/.core_locks"
#define CORE_MAX_FILENAME_LENGTH    255
#define MAX_PROC_NAME_LENGTH        32
#define CRASH_ID_LEN                8
#define CRASHID_FILE                "/tmp/.crashid" /* the file where the white screen app will read the crashid */

#define CORE_FILE_PATTERN           "%s/core.%d.%s.%d.gz"
#define CONTEXT_FILE_PATTERN        "%s/context.%d.%s.%d.txt"

#define ELF_Ehdr    Elf32_Ehdr
#define ELF_Phdr    Elf32_Phdr
#define ELF_Shdr    Elf32_Shdr
#define ELF_Nhdr    Elf32_Nhdr

typedef struct
{
    uint64_t pc;
    uint64_t ip;
    uint64_t lr;

} cdh_registers_t;

typedef struct
{
    char name[MAX_PROC_NAME_LENGTH];
    char threadname[MAX_PROC_NAME_LENGTH];
    pid_t pid;
    uint32_t timestamp;
    int signal;

    int can_create_coredump;
    file_streamer_t streamer;

    /* coredump content, for crash id generation */
    ELF_Ehdr m_Ehdr;
    ELF_Phdr *m_pPhdr;
    char *m_Nhdr; /* buffer with all NOTE pages */

    unsigned int m_note_page_size;

    cdh_registers_t m_registers;

    pid_t m_crashed_pid;
    uint64_t m_crashid_phase1;
    unsigned char m_crashid[CRASH_ID_LEN];

} proc_info_t;

cdh_status_t get_exec_name(unsigned int p_pid_str, char *p_exec_name, int p_exec_name_maxsize);
cdh_status_t write_proc_context(const proc_info_t *);
cdh_status_t treat_coredump(proc_info_t *p_proc);
cdh_status_t treat_crash_data(proc_info_t *p_proc);
cdh_status_t move_to_core_directory(proc_info_t *p_proc);
cdh_status_t check_core_directory();

#endif /* #ifndef DLT_CDH_H */
