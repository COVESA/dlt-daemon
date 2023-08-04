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

#include <algorithm>
#include <string>

#include <gtest/gtest.h>

extern "C"
{
#include "dlt_sdjournal_chmap.h"
}

const size_t kTestTableSize = 3;

static bool PointerIsNotNull(void* const ptr) {
    return ptr != NULL;
}

TEST(t_dlt_sdjournal_chmap, create_delete)
{
    const char kLetters[] = "ABCDEFGHIJ";

    // permute over various table sizes
    for (size_t tableSize = 1; tableSize <= kTestTableSize; ++tableSize) {
        // permute over various numbers of entries
        for (size_t entryCount = 0; entryCount <= (kTestTableSize + 5); ++entryCount) {
            // create table
            DltSdJournalCHMap* chmap = dlt_sdjournal_chmap_create(tableSize);
            ASSERT_TRUE(chmap != NULL);

            // add entries
            for (size_t entryIdx = 0; entryIdx < entryCount; ++entryIdx) {
                ASSERT_TRUE(entryIdx < sizeof(kLetters)); // sanity check
                char entryId[4] = {'I', 'D', '0', kLetters[entryIdx]};
                ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, entryId, DLT_LOG_ERROR), DLT_RETURN_OK);
            }

            // free everything
            dlt_sdjournal_chmap_delete(chmap);
        }
    }
}

TEST(t_dlt_sdjournal_chmap, bad_params)
{
    ASSERT_TRUE(dlt_sdjournal_chmap_create(0) == NULL);

    DltSdJournalCHMap* chmap = dlt_sdjournal_chmap_create(kTestTableSize);
    ASSERT_TRUE(chmap != NULL);

    // get cases with NULL params
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(NULL, NULL), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(NULL, ""), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(NULL, "A"), DLT_LOG_DEFAULT);

    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, NULL), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, ""), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "A"), DLT_LOG_DEFAULT);

    // get cases for non existing entries
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ABCD"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ABC"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "AB"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "A"), DLT_LOG_DEFAULT);

    // set cases with NULL params
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(NULL, NULL, -1), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(NULL, NULL, DLT_LOG_ERROR), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(NULL, "", DLT_LOG_ERROR), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(NULL, "A", -1), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(NULL, "A", DLT_LOG_ERROR), DLT_RETURN_ERROR);

    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, NULL, -1), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, NULL, DLT_LOG_ERROR), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, "", DLT_LOG_ERROR), DLT_RETURN_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, "A", -1), DLT_RETURN_ERROR);

    dlt_sdjournal_chmap_delete(chmap);
}

TEST(t_dlt_sdjournal_chmap, table_structure)
{
    DltSdJournalCHMap* chmap = dlt_sdjournal_chmap_create(kTestTableSize);
    ASSERT_TRUE(chmap != NULL);

    // API initially returns not found for all
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEFAULT);

    // add new entry via API
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, "ID0A", DLT_LOG_ERROR), DLT_RETURN_OK);

    // check table structure
    {
        ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 1);

        DltSdJournalCHMapEntry** entryPtr = std::find_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull);
        ASSERT_NE(entryPtr, chmap->table + kTestTableSize);

        // check entry object
        DltSdJournalCHMapEntry* entry = *entryPtr;
        ASSERT_TRUE(entry != NULL);
        ASSERT_EQ(std::string(entry->appId, strnlen(entry->appId, 4)), "ID0A");
        ASSERT_EQ(entry->maxLevel, DLT_LOG_ERROR);
    }

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEFAULT);

    // add new entry via API
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, "ID0B", DLT_LOG_WARN), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 2);

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_WARN);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEFAULT);

    // add new entry via API
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, "ID0C", DLT_LOG_INFO), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 3);

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_WARN);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_INFO);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEFAULT);

    // add new entry via API
    ASSERT_EQ(dlt_sdjournal_chmap_set_max_level(chmap, "ID0D", DLT_LOG_DEBUG), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 3); // "ID0D" entry has colliding hash with "ID0A"

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_ERROR);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_WARN);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_INFO);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEBUG);

    // remove entry
    ASSERT_EQ(dlt_sdjournal_chmap_unset_max_level(chmap, "ID0A"), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 3); // "ID0D" entry has colliding hash with "ID0A"

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_WARN);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_INFO);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEBUG);

    // remove entry
    ASSERT_EQ(dlt_sdjournal_chmap_unset_max_level(chmap, "ID0B"), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 2);

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_INFO);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEBUG);

    // remove entry
    ASSERT_EQ(dlt_sdjournal_chmap_unset_max_level(chmap, "ID0C"), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 1);

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEBUG);

    // remove entry
    ASSERT_EQ(dlt_sdjournal_chmap_unset_max_level(chmap, "ID0D"), DLT_RETURN_OK);
    ASSERT_EQ(std::count_if(chmap->table, chmap->table + kTestTableSize, PointerIsNotNull), 0);

    // get results via API
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0A"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0B"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0C"), DLT_LOG_DEFAULT);
    ASSERT_EQ(dlt_sdjournal_chmap_get_max_level(chmap, "ID0D"), DLT_LOG_DEFAULT);

    dlt_sdjournal_chmap_delete(chmap);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
