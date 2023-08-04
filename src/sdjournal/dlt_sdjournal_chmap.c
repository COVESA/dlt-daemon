/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2022, Amazon.com, Inc. or its affiliates.
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 */

/*!
 * \author
 * Haris Okanovic <harisokn@amazon.com>
 *
 * \copyright Copyright (C) 2022 Amazon.com, Inc. or its affiliates. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 */

#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)

#include <assert.h>
#include <stdlib.h>
#include <syslog.h>

#include "dlt_common.h"
#include <dlt_sdjournal_chmap.h>

// assumed dlt_sdjournal_chmap_hash() below
static_assert(DLT_ID_SIZE == sizeof(uint32_t));

DltSdJournalCHMap* dlt_sdjournal_chmap_create(const size_t tableSize) {
    if (tableSize < 1) {
        dlt_vlog(LOG_ERR, "%s: invalid param\n", __func__);
        return NULL;
    }

    DltSdJournalCHMap* chmap = malloc(sizeof(DltSdJournalCHMap));
    if (!chmap) {
        dlt_vlog(LOG_ERR, "%s: malloc() failed to create chmap\n", __func__);
        return NULL;
    }

    memset((void*)chmap, 0, sizeof(DltSdJournalCHMap));

    const size_t tableSizeBytes = sizeof(DltSdJournalCHMapEntry*) * tableSize;
    chmap->table = (DltSdJournalCHMapEntry**)malloc(tableSizeBytes);
    if (!chmap->table) {
        dlt_vlog(LOG_ERR, "%s: malloc() failed to create table\n", __func__);
        free(chmap);
        return NULL;
    }

    memset((void*)chmap->table, 0, tableSizeBytes);
    chmap->tableSize = tableSize;

    return chmap;
}

void dlt_sdjournal_chmap_delete(DltSdJournalCHMap* chmap) {
    if (!chmap) {
        return;
    }

    size_t deletedEntryCount = 0;

    for (size_t tableIdx = 0; tableIdx < chmap->tableSize; ++tableIdx) {
        DltSdJournalCHMapEntry* entry = chmap->table[tableIdx];

        // free linked list
        while (entry) {
            DltSdJournalCHMapEntry* nextEntry = entry->next;
            free(entry);
            ++deletedEntryCount;
            entry = nextEntry;
        }
    }

    if (deletedEntryCount != chmap->entryCount) {
        dlt_vlog(LOG_ERR, "%s: corruption! entry count mismatch: deleted %llu entries but expected %llu\n",
            __func__, (long long unsigned int)deletedEntryCount, (long long unsigned int)chmap->entryCount);
    }

    free(chmap->table);
    free(chmap);
}

/* Helper function: Returns index of specified appId in hash table..
 * Assumes chmap is already initialized and appId is exactly DLT_ID_SIZE.
 */
static size_t dlt_sdjournal_chmap_hash(DltSdJournalCHMap* chmap, const char* appId) {
    uint32_t result;
    memcpy(&result, (void*)appId, sizeof(uint32_t));
    result = result % chmap->tableSize;
    return result;
}

/* Helper function:  Returns a double pointer `R` to resulting node - a pointer to the `next` entry of R's parent.
 * `R` will never be NULL for a valid chmap, but `*R` will be NULL if an entry is not found.
 * Assumes chmap is already initialized and appId is exactly DLT_ID_SIZE.
 */
static DltSdJournalCHMapEntry** dlt_sdjournal_chmap_find(DltSdJournalCHMap* chmap, const char* const appId) {
    const size_t tableIdx = dlt_sdjournal_chmap_hash(chmap, appId);

    DltSdJournalCHMapEntry** entryPtr = &(chmap->table[tableIdx]);

    // find entry object in table and linked list
    for(;;) {
        DltSdJournalCHMapEntry* entry = *entryPtr;
        if (!entry) {
            break;
        }

        if (strncmp(entry->appId, appId, DLT_ID_SIZE) == 0) {
            break;
        }

        entryPtr = &(entry->next);
    }

    return entryPtr;
}

int dlt_sdjournal_chmap_get_max_level(DltSdJournalCHMap* chmap, const char* const appId) {
    if (!chmap || !appId) {
        dlt_vlog(LOG_ERR, "%s: invalid param\n", __func__);
        return DLT_LOG_DEFAULT;
    }

    DltSdJournalCHMapEntry** const entryPtr = dlt_sdjournal_chmap_find(chmap, appId);
    if (!entryPtr) {
        dlt_vlog(LOG_ERR, "%s: corruption! unexpected entryPtr\n", __func__);
        return DLT_LOG_DEFAULT;
    }

    DltSdJournalCHMapEntry* const entry = *entryPtr;
    if (!entry) {
        return DLT_LOG_DEFAULT;
    }

    return entry->maxLevel;
}

int dlt_sdjournal_chmap_set_max_level(DltSdJournalCHMap* chmap, const char* const appId, int level) {
    if (!chmap || !appId || *appId == '\0' || level < DLT_LOG_OFF || level >= DLT_LOG_MAX) {
        dlt_vlog(LOG_ERR, "%s: invalid param\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltSdJournalCHMapEntry** entryPtr = dlt_sdjournal_chmap_find(chmap, appId);
    if (!entryPtr) {
        dlt_vlog(LOG_ERR, "%s: corruption! unexpected entryPtr\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltSdJournalCHMapEntry* entry = *entryPtr;
    if (!entry) {
        entry = malloc(sizeof(DltSdJournalCHMapEntry));
        if (!entry) {
            dlt_vlog(LOG_ERR, "%s: malloc() failed to create entry\n", __func__);
            return DLT_RETURN_ERROR;
        }

        memset((void*)entry, 0, sizeof(DltSdJournalCHMapEntry));

        dlt_set_id(entry->appId, appId);
        // maxLevel handled below

        // connect to linked list
        *entryPtr = entry;
        ++chmap->entryCount;
    }

    // update maxLevel
    entry->maxLevel = level;

    return DLT_RETURN_OK;
}

int dlt_sdjournal_chmap_unset_max_level(DltSdJournalCHMap* chmap, const char* const appId) {
    if (!chmap || !appId) {
        dlt_vlog(LOG_ERR, "%s: invalid param\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltSdJournalCHMapEntry** entryPtr = dlt_sdjournal_chmap_find(chmap, appId);
    if (!entryPtr) {
        dlt_vlog(LOG_ERR, "%s: corruption! unexpected entryPtr\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltSdJournalCHMapEntry* entry = *entryPtr;
    if (entry) {
        DltSdJournalCHMapEntry* const child = entry->next;
        free(entry);

        // fixup linked list
        *entryPtr = child;
        --chmap->entryCount;
    }

    return DLT_RETURN_OK;
}

#endif /* defined(DLT_SYSTEMD_JOURNAL_ENABLE) */
