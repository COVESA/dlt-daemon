/**
 * Copyright (C) 2017 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT offline log storage functionality internal header file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Aditya Paluri <venkataaditya.paluri@in.bosch.com> ADIT 2017
 *
 * \file: dlt_offline_logstorage_internal.h
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_logstorage_internal.h                             **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Aditya Paluri venkataaditya.paluri@in.bosch.com               **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  ap          Aditya Paluri              ADIT                               **
*******************************************************************************/

#ifndef DLT_OFFLINE_LOGSTORAGE_INTERNAL_H
#define DLT_OFFLINE_LOGSTORAGE_INTERNAL_H

DLT_STATIC int dlt_logstorage_list_destroy(DltLogStorageFilterList **list,
                                           DltLogStorageUserConfig *uconfig,
                                           char *dev_path,
                                           int reason);

DLT_STATIC int dlt_logstorage_list_add_config(DltLogStorageFilterConfig *data,
                                              DltLogStorageFilterConfig **listdata);
DLT_STATIC int dlt_logstorage_list_add(char *key,
                                       int num_keys,
                                       DltLogStorageFilterConfig *data,
                                       DltLogStorageFilterList **list);

DLT_STATIC int dlt_logstorage_list_find(char *key,
                                        DltLogStorageFilterList **list,
                                        DltLogStorageFilterConfig **config);

DLT_STATIC int dlt_logstorage_count_ids(const char *str);

DLT_STATIC int dlt_logstorage_read_number(unsigned int *number, char *value);

DLT_STATIC int dlt_logstorage_read_bool(unsigned int *boolean, char *value);

DLT_STATIC int dlt_logstorage_read_list_of_names(char **names, char *value);

DLT_STATIC int dlt_logstorage_check_apids(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_ctids(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_loglevel(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_gzip_compression(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_filename(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_filesize(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_nofiles(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_sync_strategy(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_ecuid(DltLogStorageFilterConfig *config, char *value);

DLT_STATIC int dlt_logstorage_check_param(DltLogStorageFilterConfig *config,
                                          DltLogstorageFilterConfType ctype,
                                          char *value);

DLT_STATIC int dlt_logstorage_store_filters(DltLogStorage *handle,
                                            char *config_file_name);

void dlt_logstorage_free(DltLogStorage *handle, int reason);

DLT_STATIC int dlt_logstorage_create_keys(char *apids,
                                          char *ctids,
                                          char *ecuid,
                                          char **keys,
                                          int *num_keys);

DLT_STATIC int dlt_logstorage_prepare_table(DltLogStorage *handle,
                                            DltLogStorageFilterConfig *data);

DLT_STATIC int dlt_logstorage_validate_filter_name(char *name);

DLT_STATIC void dlt_logstorage_filter_set_strategy(DltLogStorageFilterConfig *config,
                                                   int strategy);

DLT_STATIC int dlt_logstorage_load_config(DltLogStorage *handle);

DLT_STATIC int dlt_logstorage_filter(DltLogStorage *handle,
                                     DltLogStorageFilterConfig **config,
                                     char *apid,
                                     char *ctid,
                                     char *ecuid,
                                     int log_level);

#endif /* DLT_OFFLINE_LOGSTORAGE_INTERNAL_H */
