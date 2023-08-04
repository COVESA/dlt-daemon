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

#ifndef DLT_SDJOURNAL_CONFIG_HASH_MAP_H
#define DLT_SDJOURNAL_CONFIG_HASH_MAP_H

#include "dlt_sdjournal_types.h"

/* Create/delete hash map. */
DltSdJournalCHMap* dlt_sdjournal_chmap_create(const size_t tableSize);
void dlt_sdjournal_chmap_delete(DltSdJournalCHMap* chmap);

/* Returns configured log level of specified app or DLT_LOG_DEFAULT if not found. */
int dlt_sdjournal_chmap_get_max_level(DltSdJournalCHMap* chmap, const char* const appId);

/* Set max log level of specified app. Returns DLT_RETURN_OK on success or an error code. */
int dlt_sdjournal_chmap_set_max_level(DltSdJournalCHMap* chmap, const char* const appId, int level);

/* Remove max log level config of specified app. Returns DLT_RETURN_OK on success or an error code. */
int dlt_sdjournal_chmap_unset_max_level(DltSdJournalCHMap* chmap, const char* const appId);

#endif
