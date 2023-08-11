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
 * \file dlt_env_ll.c
 */

#include "dlt_user.h"
#include <string.h>
#include <stdlib.h>

#define DLT_ENV_LL_SET_INCREASE 10


/* a generic entry looks like:
 * ll_item ::= apid:ctid:ll
 * ll_set  ::= ll_item |
 *             ll_set;ll_item
 */

/**
 * @brief extract id out of given string
 *
 * Extract 4-byte string out of given environment string, the pointer of the
 * environment string is moved to the next un-used character and the extracted
 * id is copied into \param id
 *
 * Example:
 * env[] = "abcd:1234:3"
 * char res[4u];
 * char * tmp = &env[0];
 * int ret = extract_id(&tmp, res);
 * assert(ret == 0);
 * assert(*tmp == ':');
 * assert(res[3] == 'd');
 *
 * @param env    Environment variable
 * @param id     Extracted ID
 * @return 0 if successful, -1 else
 */
int dlt_env_extract_id(char **const env, char *id)
{
    int i;

    if (!env || !id) {
        return -1;
    }

    if (!(*env)) {
        return -1;
    }

    memset(id, 0, 4);

    for (i = 0; (i < 4) && (**env != ':') && (**env != 0); ++i) {
        *id++ = *((*env)++);
    }

    /* the next/last character must be ':' */
    if ((0 != **env) && (':' == **env)) {
        return 0;
    }

    return -1;
}


/**
 * @brief convert a given string to lower-case
 *
 * Stops end of string or if ';' is detected
 */
int dlt_env_helper_to_lower(char **const env, char *result, int const res_len)
{
    int count = 0;
    char ch;

    if (!env || !result) {
        return -1;
    }

    if (!(*env)) {
        return -1;
    }

    ch = *(*env);

    while (ch && (count < res_len - 1) && (ch != ';')) {
        if ((ch >= 'A') && (ch <= 'Z')) {
            result[count] = ch + 'a' - 'A';
        } else {
            result[count] = ch;
        }

        ch = *(++(*env));
        ++count;
    }

    result[count] = 0;

    if (!ch || (ch == ';')) { /* full input was parsed */
        return 0;
    } else {
        return -1;
    }
}


int dlt_env_extract_symbolic_ll(char **const env, int8_t *ll)
{
    char result[strlen("verbose") + 1];

    if (!env || !ll) {
        return -1;
    }

    if (!(*env)) {
        return -1;
    }

    if (dlt_env_helper_to_lower(env, &result[0], sizeof(result)) == 0) {
        if (strncmp("default", result, sizeof(result)) == 0) {
            *ll = -1;
        } else if (strncmp("off", result, sizeof(result)) == 0) {
            *ll = 0;
        } else if (strncmp("fatal", result, sizeof(result)) == 0) {
            *ll = 1;
        } else if (strncmp("error", result, sizeof(result)) == 0) {
            *ll = 2;
        } else if (strncmp("warning", result, sizeof(result)) == 0) {
            *ll = 3;
        } else if (strncmp("info", result, sizeof(result)) == 0) {
            *ll = 4;
        } else if (strncmp("debug", result, sizeof(result)) == 0) {
            *ll = 5;
        } else if (strncmp("verbose", result, sizeof(result)) == 0) {
            *ll = 6;
        } else {
            return -1;
        }

        if (**env != 0) {
            (*env)++;
        }

        return 0;
    } else {
        return -1;
    }
}


/**
 * @brief extract log-level out of given string
 *
 * A valid log-level is a numeric value in the range of -1 .. 6, with:
 * -1: default
 *  0: off
 *  1: fatal
 *  2: error
 *  3: warning
 *  4: info
 *  5: debug
 *  6: verbose
 * During parsing, the environment string is moved to the next un-used character and the extracted
 * log-level is written into \param ll
 *
 * Example:
 * env[] = "abcd:1234:6"
 * int ll;
 * char ** tmp = &env[10]; // tmp points to '6'!
 * int ret = extract_ll(&tmp, &ll);
 * assert(ret == 0);
 * assert(*tmp == NULL);
 * assert(ll == 6);
 *
 * @param env    Environment variable
 * @param ll     Extracted log level
 * @return 0 if successful, -1 else
 */
int dlt_env_extract_ll(char **const env, int8_t *ll)
{
    if (!env || !ll) {
        return -1;
    }

    if (!(*env)) {
        return -1;
    }

    /* extract number */
    if (**env == '-') {
        (*env)++;

        if (**env == '1') {
            *ll = -1;
            (*env)++;
        }
    } else {
        if ((**env >= '0') && (**env < '7')) {
            *ll = **env - '0';
            (*env)++;
        } else if (dlt_env_extract_symbolic_ll(env, ll) != 0) {
            return -1;
        }
    }

    /* check end, either next char is NULL or ';' */
    if ((**env == ';') || (**env == 0)) {
        return 0;
    }

    return -1;
}


/**
 * @brief extract one item out of string
 *
 * @return 0 if successful, -1 else
 */
int dlt_env_extract_ll_item(char **const env, dlt_env_ll_item *const item)
{
    int ret = -1;

    if (!env || !item) {
        return -1;
    }

    if (!(*env)) {
        return -1;
    }

    memset(item, 0, sizeof(dlt_env_ll_item));
    ret = dlt_env_extract_id(env, item->appId);

    if (ret == -1) {
        return -1;
    }

    (*env)++;
    ret = dlt_env_extract_id(env, item->ctxId);

    if (ret == -1) {
        return -1;
    }

    (*env)++;
    ret = dlt_env_extract_ll(env, &item->ll);

    if (ret == -1) {
        return -1;
    }

    return 0;
}


/**
 * @brief initialize ll_set
 *
 * Must call release_ll_set before exit to release all memory
 *
 * @return -1 if memory could not be allocated
 * @return 0 on success
 */
int dlt_env_init_ll_set(dlt_env_ll_set *const ll_set)
{
    if (!ll_set) {
        return -1;
    }

    ll_set->array_size = DLT_ENV_LL_SET_INCREASE;
    ll_set->item = (dlt_env_ll_item *)malloc(sizeof(dlt_env_ll_item) * ll_set->array_size);

    if (!ll_set->item) {
        /* should trigger a warning: no memory left */
        ll_set->array_size = 0;
        return -1;
    }

    ll_set->num_elem = 0u;
    return 0;
}


/**
 * @brief release ll_set
 */
void dlt_env_free_ll_set(dlt_env_ll_set *const ll_set)
{
    if (!ll_set) {
        return;
    }

    if (ll_set->item != NULL) {
        free(ll_set->item);
        ll_set->item = NULL;
    }

    ll_set->array_size = 0u;
    ll_set->num_elem = 0u;
}


/**
 * @brief increase size of ll_set by LL_SET_INCREASE elements
 *
 * @return -1 if memory could not be allocated
 * @return 0 on success
 */
int dlt_env_increase_ll_set(dlt_env_ll_set *const ll_set)
{
    dlt_env_ll_item *old_set;
    size_t old_size;

    if (!ll_set) {
        return -1;
    }

    old_set = ll_set->item;
    old_size = ll_set->array_size;

    ll_set->array_size += DLT_ENV_LL_SET_INCREASE;
    ll_set->item = (dlt_env_ll_item *)malloc(sizeof(dlt_env_ll_item) * ll_set->array_size);

    if (!ll_set->item) {
        /* should trigger a warning: no memory left */
        ll_set->array_size -= DLT_ENV_LL_SET_INCREASE;
        return -1;
    } else {
        memcpy(ll_set->item, old_set, sizeof(dlt_env_ll_item) * old_size);
        free(old_set);
        return 0;
    }
}


/**
 * @brief extract all items out of string
 *
 * The given set is initialized within this function (memory is allocated).
 * Make sure, that the caller frees this memory when it is no longer needed!
 *
 * @return 0 if successful, -1 else
 */
int dlt_env_extract_ll_set(char **const env, dlt_env_ll_set *const ll_set)
{
    if (!env || !ll_set) {
        return -1;
    }

    if (!(*env)) {
        return -1;
    }

    if (dlt_env_init_ll_set(ll_set) == -1) {
        return -1;
    }

    do {
        if (ll_set->num_elem == ll_set->array_size) {
            if (dlt_env_increase_ll_set(ll_set) == -1) {
                return -1;
            }
        }

        if (dlt_env_extract_ll_item(env, &ll_set->item[ll_set->num_elem++]) == -1) {
            return -1;
        }

        if (**env == ';') {
            (*env)++;
        }
    } while (**env != 0);

    return 0;
}


/**
 * @brief check if two ids match
 *
 * @return 1 if matching, 0 if not
 */
int dlt_env_ids_match(char const *const a, char const *const b)
{
    if (a[0] != b[0]) {
        return 0;
    }

    if (a[1] != b[1]) {
        return 0;
    }

    if (a[2] != b[2]) {
        return 0;
    }

    if (a[3] != b[3]) {
        return 0;
    }

    return 1;
}


/**
 * @brief check if (and how) apid and ctid match with given item
 *
 * Resulting priorities:
 * - no apid, no ctid only ll given in item: use ll with prio 1
 * - no apid, ctid matches: use ll with prio 2
 * - no ctid, apid matches: use ll with prio 3
 * - apid, ctid matches: use ll with prio 4
 *
 * In case of error, -1 is returned.
 */
int dlt_env_ll_item_get_matching_prio(dlt_env_ll_item const *const item,
                                      char const *const apid,
                                      char const *const ctid)
{
    if ((!item) || (!apid) || (!ctid)) {
        return -1;
    }

    if (item->appId[0] == 0) {
        if (item->ctxId[0] == 0) {
            return 1;
        } else if (dlt_env_ids_match(item->ctxId, ctid)) {
            return 2;
        }
    } else if (dlt_env_ids_match(item->appId, apid)) {
        if (item->ctxId[0] == 0) {
            return 3;
        } else if (dlt_env_ids_match(item->ctxId, ctid)) {
            return 4;
        }
    }

    return 0;
}


/**
 * @brief adjust log-level based on values given through environment
 *
 * Iterate over the set of items, and find the best match (\see ll_item_get_matching_prio)
 * For any item that matches, the one with the highest priority is selected and that
 * log-level is returned.
 *
 * If no item matches or in case of error, the original log-level (\param ll) is returned
 */
int dlt_env_adjust_ll_from_env(dlt_env_ll_set const *const ll_set,
                               char const *const apid,
                               char const *const ctid,
                               int const ll)
{
    if ((!ll_set) || (!apid) || (!ctid)) {
        return ll;
    }

    int res = ll;
    int prio = 0; /* no match so far */
    size_t i;

    for (i = 0; i < ll_set->num_elem; ++i) {
        int p = dlt_env_ll_item_get_matching_prio(&ll_set->item[i], apid, ctid);

        if (p > prio) {
            prio = p;
            res = ll_set->item[i].ll;

            if (p == 4) { /* maximum reached, immediate return */
                return res;
            }
        }
    }

    return res;
}


