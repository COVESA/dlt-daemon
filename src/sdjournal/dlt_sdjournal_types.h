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

#ifndef DLT_SDJOURNAL_TYPES_H_
#define DLT_SDJOURNAL_TYPES_H_

#include "dlt_common.h"

// defined in systemd/sd-journal.h
typedef struct sd_journal sd_journal;

// "CHMap" = "Config Hash Map", a hash map of verbosity configurations
// indexed by application id.

struct DltSdJournalCHMapEntry_st
{
    struct DltSdJournalCHMapEntry_st* next;
    char appId[DLT_ID_SIZE];
    int maxLevel;
};
typedef struct DltSdJournalCHMapEntry_st DltSdJournalCHMapEntry;

typedef struct
{
    DltSdJournalCHMapEntry** table;
    size_t tableSize;
    size_t entryCount;
} DltSdJournalCHMap;

typedef struct
{
    sd_journal* handle;
    uint8_t logEntryCount;
    DltReceiver receiver;
    int defaultLogLevel;
    DltSdJournalCHMap* chmap;
} DltSdJournal;

#endif
