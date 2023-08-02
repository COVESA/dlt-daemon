/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015  Intel Corporation
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
 * \author Stefan Vacek <stefan.vacek@intel.com> Intel Corporation
 *
 * \copyright Copyright © 2015 Intel Corporation. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_env_ll_unit_test.cpp
 */

#include "gtest/gtest.h"
#include "dlt_user.h"
#include "dlt_common.h" /* needed for dlt_set_id */

/* simply include the whole file to allow testing it */
#include "src/lib/dlt_env_ll.c"

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(DltExtensionTests, extract_id)
{
    /* testing valid input */
    char id[4u];

    char env0[] = "abcd:1234:3";
    char *tmp = &env0[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), 0);
    ASSERT_EQ(tmp - &env0[0], 4); /* moved 4 bytes */
    ASSERT_EQ(id[0], 'a');
    ASSERT_EQ(id[1], 'b');
    ASSERT_EQ(id[2], 'c');
    ASSERT_EQ(id[3], 'd');

    char env1[] = "abc:1234:3";
    tmp = &env1[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), 0);
    ASSERT_EQ(tmp - &env1[0], 3); /* moved 3 bytes */
    ASSERT_EQ(id[0], 'a');
    ASSERT_EQ(id[1], 'b');
    ASSERT_EQ(id[2], 'c');
    ASSERT_EQ(id[3], 0);

    char env2[] = "ab:1234:3";
    tmp = &env2[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), 0);
    ASSERT_EQ(tmp - &env2[0], 2); /* moved 2 bytes */
    ASSERT_EQ(id[0], 'a');
    ASSERT_EQ(id[1], 'b');
    ASSERT_EQ(id[2], 0);
    ASSERT_EQ(id[3], 0);

    char env3[] = "a:1234:3";
    tmp = &env3[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), 0);
    ASSERT_EQ(tmp - &env3[0], 1); /* moved 1 byte */
    ASSERT_EQ(id[0], 'a');
    ASSERT_EQ(id[1], 0);
    ASSERT_EQ(id[2], 0);
    ASSERT_EQ(id[3], 0);

    char env4[] = ":1234:3";
    tmp = &env4[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), 0);
    ASSERT_EQ(tmp - &env4[0], 0); /* moved 1 byte */
    ASSERT_EQ(id[0], 0);
    ASSERT_EQ(id[1], 0);
    ASSERT_EQ(id[2], 0);
    ASSERT_EQ(id[3], 0);

    char env5[] = "abcd:1234:3;";
    tmp = &env5[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), 0);
    ASSERT_EQ(tmp - &env5[0], 4); /* moved 4 bytes */
    ASSERT_EQ(id[0], 'a');
    ASSERT_EQ(id[1], 'b');
    ASSERT_EQ(id[2], 'c');
    ASSERT_EQ(id[3], 'd');

    /* testing invalid input */
    /* - string too long: abcde:
     * - string too short/missing end: abc
     * - NULL string: <null>
     */
    tmp = NULL;
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), -1);

    char env6[] = "abcd:1234:3";
    tmp = &env6[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, NULL), -1);

    char invalid0[] = "";
    tmp = &invalid0[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), -1);

    char invalid1[] = "abcd"; /* missing delimiter */
    tmp = &invalid1[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), -1);

    char invalid2[] = "abcde"; /* id too long */
    tmp = &invalid2[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, id), -1);
}

TEST(DltExtensionTests, extract_ll)
{
    /* testing valid input */
    int8_t ll;

    char env_1[] = "-1";
    char *tmp = &env_1[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env_1[0], 2); /* moved 2 bytes */
    ASSERT_EQ(ll, -1);

    char env0[] = "0;";
    tmp = &env0[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env0[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 0);

    char env1[] = "1;";
    tmp = &env1[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env1[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 1);

    char env2[] = "2;";
    tmp = &env2[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env2[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 2);

    char env3[] = "3;";
    tmp = &env3[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env3[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 3);

    char env4[] = "4;";
    tmp = &env4[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env4[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 4);

    char env5[] = "5;";
    tmp = &env5[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env5[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 5);

    char env6[] = "6;";
    tmp = &env6[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), 0);
    ASSERT_EQ(tmp - &env6[0], 1); /* moved 1 byte */
    ASSERT_EQ(ll, 6);

    /* testing invalid input */
    /* - number outside range, e.g. -2, 103
     * - missing delimiter
     * - NULL string: <null>
     */
    tmp = NULL;
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), -1);

    char env7[] = "abcd:1234:3";
    tmp = &env7[0];
    ASSERT_EQ(dlt_env_extract_id(&tmp, NULL), -1);

    char invalid0[] = "";
    tmp = &invalid0[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), -1);

    char invalid1[] = "-2"; /* outside range */
    tmp = &invalid1[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), -1);

    char invalid2[] = "8"; /* outside range */
    tmp = &invalid2[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), -1);

    char invalid3[] = "1e"; /* missing delimiter */
    tmp = &invalid3[0];
    ASSERT_EQ(dlt_env_extract_ll(&tmp, &ll), -1);
}

TEST(DltExtensionTests, extract_ll_item)
{
    /* testing valid input */
    dlt_env_ll_item item;

    char env0[] = "abcd:1234:3";
    char *tmp = &env0[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), 0);
    ASSERT_EQ(tmp - &env0[0], 11); /* moved 11 bytes */
    ASSERT_EQ(item.appId[0], 'a');
    ASSERT_EQ(item.appId[1], 'b');
    ASSERT_EQ(item.appId[2], 'c');
    ASSERT_EQ(item.appId[3], 'd');
    ASSERT_EQ(item.ctxId[0], '1');
    ASSERT_EQ(item.ctxId[1], '2');
    ASSERT_EQ(item.ctxId[2], '3');
    ASSERT_EQ(item.ctxId[3], '4');
    ASSERT_EQ(item.ll, 3);

    char env1[] = "::-1;";
    tmp = &env1[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), 0);
    ASSERT_EQ(tmp - &env1[0], 4); /* moved 4 bytes */
    ASSERT_EQ(item.appId[0], 0);
    ASSERT_EQ(item.appId[1], 0);
    ASSERT_EQ(item.appId[2], 0);
    ASSERT_EQ(item.appId[3], 0);
    ASSERT_EQ(item.ctxId[0], 0);
    ASSERT_EQ(item.ctxId[1], 0);
    ASSERT_EQ(item.ctxId[2], 0);
    ASSERT_EQ(item.ctxId[3], 0);
    ASSERT_EQ(item.ll, -1);

    /* testing invalid input */
    /* - string too long: abcde:
     * - string too short/missing end: abc
     * - NULL string: <null>
     */
    tmp = NULL;
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), -1);

    char env2[] = "abcd:1234:3";
    tmp = &env2[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, NULL), -1);

    char invalid0[] = "";
    tmp = &invalid0[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), -1);

    char invalid1[] = "abcd:1234:"; /* missing ll */
    tmp = &invalid1[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), -1);

    char invalid2[] = "abcd:1234"; /* missing ll, missing delimiter in ctxId */
    tmp = &invalid2[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), -1);

    char invalid3[] = "abcd:"; /* missing ll, missing delimiter in appId */
    tmp = &invalid3[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), -1);

    char invalid4[] = "abcd"; /* missing ll, missing delimiter in appId */
    tmp = &invalid4[0];
    ASSERT_EQ(dlt_env_extract_ll_item(&tmp, &item), -1);
}

TEST(DltExtensionTests, basic_ll_set_handling)
{
    dlt_env_init_ll_set(NULL); /* must not crash */
    dlt_env_free_ll_set(NULL); /* must not crash */
    dlt_env_increase_ll_set(NULL); /* must not crash */

    dlt_env_ll_set ll_set;
    dlt_env_init_ll_set(&ll_set);
    EXPECT_TRUE(NULL != ll_set.item);
    EXPECT_EQ(DLT_ENV_LL_SET_INCREASE, ll_set.array_size);
    EXPECT_EQ(0, ll_set.num_elem);

    dlt_env_free_ll_set(&ll_set);
    EXPECT_TRUE(NULL == ll_set.item);
    EXPECT_EQ(0, ll_set.array_size);
    EXPECT_EQ(0, ll_set.num_elem);

    dlt_env_init_ll_set(&ll_set);

    for (int i = 0; i < DLT_ENV_LL_SET_INCREASE; ++i)
        ll_set.item[i].ll = i;

    dlt_env_increase_ll_set(&ll_set);
    EXPECT_EQ(2 * DLT_ENV_LL_SET_INCREASE, ll_set.array_size);

    for (int i = 0; i < DLT_ENV_LL_SET_INCREASE; ++i)
        EXPECT_EQ(ll_set.item[i].ll, i);

    dlt_env_free_ll_set(&ll_set);
    EXPECT_TRUE(NULL == ll_set.item);
    EXPECT_EQ(0, ll_set.array_size);
    EXPECT_EQ(0, ll_set.num_elem);
}

TEST(DltExtensionTests, extract_ll_set)
{
    /* testing valid input */
    dlt_env_ll_set ll_set;

    char env0[] = "abcd:1234:3";
    char *tmp = &env0[0];

    ASSERT_EQ(dlt_env_extract_ll_set(&tmp, &ll_set), 0);
    EXPECT_EQ(ll_set.array_size, DLT_ENV_LL_SET_INCREASE);
    EXPECT_EQ(ll_set.num_elem, 1);
    EXPECT_EQ(ll_set.item[0].ll, 3);

    dlt_env_free_ll_set(&ll_set);

    /* force increasing the list */
    char env1[] =
        "abcd:0000:3;abcd:0001:3;abcd:0002:3;abcd:0003:3;abcd:0004:3;abcd:0005:3;abcd:0006:3;abcd:0007:3;abcd:0008:3;abcd:0009:3;abcd:0010:3";
    tmp = &env1[0];
    ASSERT_EQ(dlt_env_extract_ll_set(&tmp, &ll_set), 0);
    EXPECT_EQ(ll_set.array_size, 2 * DLT_ENV_LL_SET_INCREASE);
    EXPECT_EQ(ll_set.num_elem, 11);

    for (size_t i = 0; i < ll_set.num_elem; ++i)
        EXPECT_EQ(ll_set.item[i].ctxId[3], i % 10 + '0');

    dlt_env_free_ll_set(&ll_set);

    char env2[] = "SINA:SINC:FATAL";
    tmp = &env2[0];

    ASSERT_EQ(dlt_env_extract_ll_set(&tmp, &ll_set), 0);
    EXPECT_EQ(ll_set.array_size, DLT_ENV_LL_SET_INCREASE);
    EXPECT_EQ(ll_set.num_elem, 1);
    EXPECT_EQ(ll_set.item[0].ll, 1);

    dlt_env_free_ll_set(&ll_set);
}

TEST(DltExtensionTests, ids_match)
{
    ASSERT_EQ(1, dlt_env_ids_match("abcd", "abcd"));
    ASSERT_EQ(0, dlt_env_ids_match("abcd", "abce"));
    ASSERT_EQ(0, dlt_env_ids_match("abcd", "abee"));
    ASSERT_EQ(0, dlt_env_ids_match("abcd", "aeee"));
    ASSERT_EQ(0, dlt_env_ids_match("abcd", "eeee"));

    ASSERT_TRUE(dlt_env_ids_match("abcd", "abcd"));
    ASSERT_FALSE(dlt_env_ids_match("abcd", "abce"));
}

TEST(DltExtensionTests, get_matching_prio)
{
    char apid[5] = "ABCD";
    char ctid[5] = "1234";

    dlt_env_ll_item test0;
    dlt_set_id(test0.appId, "");
    dlt_set_id(test0.ctxId, "");
    ASSERT_EQ(1, dlt_env_ll_item_get_matching_prio(&test0, apid, ctid));

    dlt_set_id(test0.appId, "");
    dlt_set_id(test0.ctxId, ctid);
    ASSERT_EQ(2, dlt_env_ll_item_get_matching_prio(&test0, apid, ctid));

    dlt_set_id(test0.appId, apid);
    dlt_set_id(test0.ctxId, "");
    ASSERT_EQ(3, dlt_env_ll_item_get_matching_prio(&test0, apid, ctid));

    dlt_set_id(test0.appId, apid);
    dlt_set_id(test0.ctxId, ctid);
    ASSERT_EQ(4, dlt_env_ll_item_get_matching_prio(&test0, apid, ctid));

    dlt_set_id(test0.appId, "EFGH"); /* appId should not match */
    dlt_set_id(test0.ctxId, ctid);
    ASSERT_EQ(0, dlt_env_ll_item_get_matching_prio(&test0, apid, ctid));

    ASSERT_EQ(-1, dlt_env_ll_item_get_matching_prio(NULL, apid, ctid));
    ASSERT_EQ(-1, dlt_env_ll_item_get_matching_prio(&test0, NULL, ctid));
    ASSERT_EQ(-1, dlt_env_ll_item_get_matching_prio(&test0, apid, NULL));
}

TEST(DltExtensionTests, adjust_ll_from_env)
{
    char apid[5] = "ABCD";
    char ctid[5] = "1234";
    int ll = 42; /* unrealistic value to see that the ll was not touched */

    dlt_env_ll_set ll_set;
    dlt_env_init_ll_set(&ll_set);
    EXPECT_EQ(ll, dlt_env_adjust_ll_from_env(NULL, apid, ctid, ll)); /* orig value in case of error */
    EXPECT_EQ(ll, dlt_env_adjust_ll_from_env(&ll_set, NULL, ctid, ll)); /* orig value in case of error */
    EXPECT_EQ(ll, dlt_env_adjust_ll_from_env(&ll_set, apid, NULL, ll)); /* orig value in case of error */

    EXPECT_EQ(ll, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll)); /* an empty set should not match anything */

    dlt_set_id(ll_set.item[0].appId, "DEAD"); /* not matching */
    dlt_set_id(ll_set.item[0].ctxId, "BEEF");
    ll_set.item[0].ll = 0;
    ll_set.num_elem = 1;
    EXPECT_EQ(ll, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll)); /* not matching anything */

    dlt_set_id(ll_set.item[1].appId, ""); /* empty rule, weakest */
    dlt_set_id(ll_set.item[1].ctxId, "");
    ll_set.item[1].ll = 1;
    ll_set.num_elem = 2;
    EXPECT_EQ(1, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll));

    dlt_set_id(ll_set.item[2].appId, ""); /* prio 2 */
    dlt_set_id(ll_set.item[2].ctxId, ctid);
    ll_set.item[2].ll = 2;
    ll_set.num_elem = 3;
    EXPECT_EQ(2, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll));

    dlt_set_id(ll_set.item[3].appId, apid); /* prio 3 */
    dlt_set_id(ll_set.item[3].ctxId, "");
    ll_set.item[3].ll = 3;
    ll_set.num_elem = 4;
    EXPECT_EQ(3, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll));

    dlt_set_id(ll_set.item[4].appId, apid); /* prio 4 */
    dlt_set_id(ll_set.item[4].ctxId, ctid);
    ll_set.item[4].ll = 4;
    ll_set.num_elem = 5;
    EXPECT_EQ(4, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll));

    dlt_set_id(ll_set.item[5].appId, apid); /* does not matter item[4] will always match */
    dlt_set_id(ll_set.item[5].ctxId, "");
    ll_set.item[5].ll = 5;
    ll_set.num_elem = 6;
    EXPECT_EQ(4, dlt_env_adjust_ll_from_env(&ll_set, apid, ctid, ll)); /* remember, item[4] matches */

    dlt_env_free_ll_set(&ll_set);
}

/* int dlt_env_helper_to_lower(char **env, char *result, int res_len) */
TEST(DltExtensionTests, dlt_env_helper_to_lower)
{
    /* default behavior */
    char env0[] = "1238<><<>>>>#$//abcdABCDEDFGHIJKLMNOPQRSTUVWXYZpo;ABcd";
    char res0[] = "1238<><<>>>>#$//abcdabcdedfghijklmnopqrstuvwxyzpo";
    char *tmp0 = &env0[0];

    char result0[sizeof(res0)];
    ASSERT_EQ(0, dlt_env_helper_to_lower(&tmp0, result0, sizeof(result0)));
    ASSERT_EQ(';', *tmp0); /* next char is ';' */
    ASSERT_STREQ(res0, result0); /* stops at ';' and is correctly converted */

    /* default behavior with end of string */
    char env1[] = "1238<><<>>>>#$//abcdABCDEDFGHIJKLMNOPQRSTUVWXYZpo";
    char res1[] = "1238<><<>>>>#$//abcdabcdedfghijklmnopqrstuvwxyzpo";
    char *tmp1 = &env1[0];

    char result1[sizeof(res1)];
    ASSERT_EQ(0, dlt_env_helper_to_lower(&tmp1, result1, sizeof(result1)));
    ASSERT_EQ(0, *tmp1); /* next char is void */
    ASSERT_STREQ(res1, result1); /* stops at end-of-string and is correctly converted */

    /* result string too short */
    char env2[] = "2238<><<>>>>#$//abcdABCDEDFGHIJKLMNOPQRSTUVWXYZpo";
    char res2[] = "2238<><<>>>>#$//abcdabcdedfg";
    char *tmp2 = &env2[0];

    char result2[sizeof(res2)];
    ASSERT_EQ(-1, dlt_env_helper_to_lower(&tmp2, result2, sizeof(result2)));
    ASSERT_EQ('H', *tmp2); /* next char is void */
    ASSERT_STREQ(res2, result2); /* stops at end-of-string and is partially converted */

    /* input string shorter than result */
    char env3[] = "3338<><<>>>>#$//abcdABCDEDFGHIJKLMNOPQRSTUVWXYZpo";
    char res3[] = "3338<><<>>>>#$//abcdabcdedfghijklmnopqrstuvwxyzpo";
    char *tmp3 = &env3[0];

    char result3[sizeof(res3) + 5];
    ASSERT_EQ(0, dlt_env_helper_to_lower(&tmp3, result3, sizeof(result3)));
    ASSERT_EQ(0, *tmp3); /* next char is void */
    ASSERT_STREQ(res3, result3); /* stops at end-of-string and is correctly converted */
}

/* int dlt_env_extract_symbolic_ll(char **env, int8_t * ll) */
TEST(DltExtensionTests, dlt_env_extract_symbolic_ll)
{
    int8_t result;

    /* correct behavior */
    char env0[] = "DEFAULT;off;fatal;error;warning;info;DeBuG;verbose";
    char *tmp0 = &env0[0];

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('o', *tmp0);
    ASSERT_EQ(-1, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('f', *tmp0);
    ASSERT_EQ(0, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('e', *tmp0);
    ASSERT_EQ(1, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('w', *tmp0);
    ASSERT_EQ(2, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('i', *tmp0);
    ASSERT_EQ(3, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('D', *tmp0);
    ASSERT_EQ(4, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ('v', *tmp0);
    ASSERT_EQ(5, result);

    ASSERT_EQ(0, dlt_env_extract_symbolic_ll(&tmp0, &result));
    ASSERT_EQ(0, *tmp0);
    ASSERT_EQ(6, result);

    /* incorrect behavior */
    char env1[] = "DEF";
    char *tmp1 = &env1[0];

    result = 18;
    ASSERT_EQ(-1, dlt_env_extract_symbolic_ll(&tmp1, &result));
    ASSERT_EQ(0, *tmp1);
    ASSERT_EQ(18, result); /* 'result' is not touched */

    /* incorrect behavior */
    char env2[] = "DEFaultingfBa";
    char *tmp2 = &env2[0];

    result = 28;
    ASSERT_EQ(-1, dlt_env_extract_symbolic_ll(&tmp2, &result));
    ASSERT_EQ('i', *tmp2);
    ASSERT_EQ(28, result); /* 'result' is not touched */
}

