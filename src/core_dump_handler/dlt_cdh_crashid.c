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
 * \file dlt_cdh_crashid.c
 */

#include <stdlib.h>
#include <syslog.h>
#include <sys/procfs.h>
#include <sys/user.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <asm/prctl.h>
#include <inttypes.h>

#include "dlt_cdh.h"
#include "dlt_cdh_cpuinfo.h"

#ifdef HAS_CITYHASH_C
#   include "city_c.h"
#endif

/*ARM32 specific */
/*#define REG_FRAME_POINTER   11 */
/*#define REG_INSTR_POINTER   12 */
/*#define REG_STACK_POINTER   13 */
/*#define REG_LINK_REGISTER   14 */
/*#define REG_PROC_COUNTER    15 */

#ifdef HAS_CITYHASH_C
static cdh_status_t crashid_cityhash(proc_info_t *p_proc);
#endif

cdh_status_t get_phdr_num(proc_info_t *p_proc, unsigned int p_address, int *phdr_num)
{
    int i = 0;

    if (phdr_num == NULL)
        return CDH_NOK;

    for (i = 0; i < p_proc->m_Ehdr.e_phnum; i++)
        if ((p_proc->m_pPhdr[i].p_vaddr < p_address)
            && (p_proc->m_pPhdr[i].p_vaddr + p_proc->m_pPhdr[i].p_memsz > p_address)) {
            *phdr_num = i;
            return CDH_OK;
        }

    *phdr_num = -1;

    return CDH_NOK;
}

/* Thanks to libunwind for the following definitions, which helps to */
#define ALIGN(x, a) (((x) + (a) - 1UL) & ~((a) - 1UL))
#define NOTE_SIZE(_hdr) (sizeof (_hdr) + ALIGN((_hdr).n_namesz, 4) + (_hdr).n_descsz)

cdh_status_t get_crashed_registers(proc_info_t *p_proc)
{
    int found = CDH_NOK; /* CDH_OK, when we find the page note associated to PID of crashed process */
    unsigned int offset = 0;

    /* TODO: if no notes were found m_note_page_size was not set to 0 which leads to a crash in this loop because it is then used */
    /* uninitialised here => this is an x86_64 issue */
    while (found != CDH_OK && offset < p_proc->m_note_page_size) {
        /* Crash mentioned in TODO dlt_cdh_coredump.c line 163 */
        ELF_Nhdr *ptr_note = (ELF_Nhdr *)(p_proc->m_Nhdr + offset);

        if (ptr_note->n_type == NT_PRSTATUS) {
            /* The first PRSTATUS note is the one of the crashed thread */
            prstatus_t *prstatus = (prstatus_t *)((char *)ptr_note + sizeof(ELF_Nhdr) + ALIGN(ptr_note->n_namesz, 4));

            p_proc->m_crashed_pid = prstatus->pr_pid;

            get_registers(prstatus, &p_proc->m_registers);
            found = CDH_OK;
        }

        offset += NOTE_SIZE(*ptr_note);
    }

    return found;
}

#ifdef HAS_CITYHASH_C

cdh_status_t crashid_cityhash(proc_info_t *p_proc)
{
#   define CRASHID_BUF_SIZE         MAX_PROC_NAME_LENGTH + sizeof(uint64_t)

    char cityhash_in[CRASHID_BUF_SIZE];
    uint64_t cityhash_result = 0;
    memcpy(cityhash_in, p_proc->name, MAX_PROC_NAME_LENGTH);
    memcpy(cityhash_in + MAX_PROC_NAME_LENGTH, &p_proc->m_crashid_phase1, sizeof(uint64_t));

    cityhash_result = CityHash64(cityhash_in, CRASHID_BUF_SIZE);
    memcpy(p_proc->m_crashid, &cityhash_result, sizeof(uint64_t));

    return CDH_OK;
#   undef CRASHID_BUF_SIZE
}

#endif /* HAS_CITYHASH_C */

cdh_status_t create_crashid(proc_info_t *p_proc)
{
    uint32_t final_lr = 0;
    uint32_t final_pc = 0;
    int pc_phnum = 0;
    int lr_phnum = 0;

    /* translate address from virtual address (process point of view) to offset in the stack memory page */
#define ADDRESS_REBASE(__x, __phdr_num)               (__x - p_proc->m_pPhdr[__phdr_num].p_vaddr)
    /* read value in the stack at position offset: +/- sizeof(), depends on stack growing upward or downward */
#define READ_STACK_VALUE(__offset, __type)  (*(__type *)(stack_page + __offset - sizeof(__type)))

    get_phdr_num(p_proc, p_proc->m_registers.pc, &pc_phnum);
    final_pc = ADDRESS_REBASE(p_proc->m_registers.pc, pc_phnum);

    get_phdr_num(p_proc, p_proc->m_registers.lr, &lr_phnum);

    if (lr_phnum >= 0)
        final_lr = ADDRESS_REBASE(p_proc->m_registers.lr, lr_phnum);

    p_proc->m_crashid_phase1 = p_proc->signal << 24;
    p_proc->m_crashid_phase1 |= (uint64_t)final_lr;
    p_proc->m_crashid_phase1 <<= 32;
    p_proc->m_crashid_phase1 |= (uint64_t)final_pc;

#ifdef HAS_CITYHASH_C
    crashid_cityhash(p_proc);
#else
    memcpy(p_proc->m_crashid, &p_proc->m_crashid_phase1, sizeof(uint64_t));
#endif

    syslog(LOG_INFO,
           "Crash in \"%s\", thread=\"%s\", pid=%d, crashID=%" PRIx64 ", based on signal=%d, PC=0x%x, caller=0x%x",
           p_proc->name,
           p_proc->threadname,
           p_proc->pid,
           *((uint64_t *)p_proc->m_crashid),
           p_proc->signal,
           final_pc, final_lr
           );

    return CDH_OK;
}

int write_crashid_to_filesystem(proc_info_t *p_proc)
{
    FILE *crashid_file = NULL;

    if ((crashid_file = fopen(CRASHID_FILE, "wt")) == NULL) {
        syslog(LOG_ERR, "(pid=%d) cannot write crashid to %s: %s", p_proc->pid, CRASHID_FILE, strerror(errno));
        return CDH_NOK;
    }

    fprintf(crashid_file, "%" PRIx64, *(uint64_t *)p_proc->m_crashid);
    fclose(crashid_file);

    return CDH_OK;
}

cdh_status_t treat_crash_data(proc_info_t *p_proc)
{
    if (get_crashed_registers(p_proc) != CDH_OK) {
        syslog(LOG_ERR, "registers not found in notes");
        return CDH_NOK;
    }

    if (create_crashid(p_proc) != CDH_OK) {
        syslog(LOG_ERR, "crashid not generated");
        return CDH_NOK;
    }

    write_crashid_to_filesystem(p_proc);

    return CDH_OK;
}

