/**
 * Copyright (C) 2018  Advanced Driver Information Technology.
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
 * \author Aditya Paluri <venkataaditya.paluri@in.bosch.com> ADIT 2018
 *
 * \file: dlt_offline_logstorage_behavior_internal.h
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_logstorage_behavior_internal.h                    **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Aditya Paluri venkataaditya.paluri@in.bosch.com               **
**                                                                            **
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
**  ap           Aditya Paluri             ADIT                               **
*******************************************************************************/

#ifndef DLT_OFFLINELOGSTORAGE_BEHAVIOR_INTERNAL_H_
#define DLT_OFFLINELOGSTORAGE_BEHAVIOR_INTERNAL_H_

void dlt_logstorage_log_file_name(char *log_file_name,
                                  DltLogStorageUserConfig *file_config,
                                  DltLogStorageFilterConfig *filter_config,
                                  int idx);

unsigned int dlt_logstorage_sort_file_name(DltLogStorageFileList **head);

void dlt_logstorage_rearrange_file_name(DltLogStorageFileList **head);

unsigned int
dlt_logstorage_get_idx_of_log_file(DltLogStorageUserConfig *file_config,
                                   DltLogStorageFilterConfig *config,
                                   char *file);

int dlt_logstorage_storage_dir_info(DltLogStorageUserConfig *file_config,
                                    char *path,
                                    DltLogStorageFilterConfig *config);

int dlt_logstorage_open_log_file(DltLogStorageFilterConfig *config,
                                 DltLogStorageUserConfig *file_config,
                                 char *dev_path,
                                 int msg_size,
                                 bool is_update_required);

DLT_STATIC int dlt_logstorage_sync_to_file(DltLogStorageFilterConfig *config,
                                           DltLogStorageUserConfig *file_config,
                                           char *dev_path,
                                           DltLogStorageCacheFooter *footer,
                                           unsigned int start_offset,
                                           unsigned int end_offset);

DLT_STATIC int dlt_logstorage_find_dlt_header(void *ptr,
                                              unsigned int offset,
                                              unsigned int cnt);

DLT_STATIC int dlt_logstorage_find_last_dlt_header(void *ptr,
                                                   unsigned int offset,
                                                   unsigned int cnt);

#endif /* DLT_OFFLINELOGSTORAGE_BEHAVIOR_INTERNAL_H_ */
