/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015, Advanced Driver Information Technology
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
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
 * \author Christoph Lipka <clipka@jp.adit-jv.com>
 *
 * \copyright Copyright © 2015 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_config_file_parser.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_config_file_parser.h                                      **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
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
**  cl          Christoph Lipka            ADIT                               **
*******************************************************************************/

#ifndef _DLT_CONFIG_FILE_PARSER_H_
#define _DLT_CONFIG_FILE_PARSER_H_


/* definitions */
#define DLT_CONFIG_FILE_PATH_MAX_LEN       100 /* absolute path including filename */
#define DLT_CONFIG_FILE_ENTRY_MAX_LEN      100 /* Entry for section, key and value */
#define DLT_CONFIG_FILE_LINE_MAX_LEN       210
#define DLT_CONFIG_FILE_SECTIONS_MAX       125
#define DLT_CONFIG_FILE_KEYS_MAX            25 /* Maximal keys per section */

typedef struct DltConfigKeyData
{
    char *key;
    char *data;
    struct DltConfigKeyData *next;
} DltConfigKeyData;

/* Config file section structure */
typedef struct
{
    int num_entries;          /* number of entries */
    char *name;               /* name of section */
    char *keys;               /* keys */
    DltConfigKeyData *list;
} DltConfigFileSection;

typedef struct
{
    int num_sections;               /* number of sections */
    DltConfigFileSection *sections; /* sections */
} DltConfigFile;

/**
 * dlt_config_file_init
 *
 * Load the configuration file and stores all data in
 * internal data structures.
 *
 * @param file_name File to be opened
 * @return          Pointer to DltConfigFile object or NULL on error
 */
DltConfigFile *dlt_config_file_init(char *file_name);

/**
 * dlt_config_file_release
 *
 * Release config file and frees all internal data. Has to be called after
 * after all data is read.
 *
 * @param file      DltConfigFile
 */
void dlt_config_file_release(DltConfigFile *file);

/**
 * dlt_config_file_get_section_name
 *
 * Get name of section number.
 *
 * @param[in]  file      DltConfigFile
 * @param[in]  num       Number of section
 * @param[out] name      Section name
 * @return     0 on success, else -1
 */
int dlt_config_file_get_section_name(const DltConfigFile *file,
                                     int num,
                                     char *name);

/**
 * dlt_config_file_get_num_sections
 *
 * Get the number of sections inside configuration file
 *
 * @param[in]  file     DltConfigFile
 * @param[out] num      Number of sections inside configuration file
 * @return     0 on success, else -1
 */
int dlt_config_file_get_num_sections(const DltConfigFile *file, int *num);

/**
 * dlt_config_file_get_value
 *
 * Get value of key in specified section.
 *
 * @param[in]   file      DltConfigFile
 * @param[in]   section   Name of section
 * @param[in]   key       Key
 * @param[out]  value     Value
 * @return      0 on success, else -1
 */
int dlt_config_file_get_value(const DltConfigFile *file,
                              const char *section,
                              const char *key,
                              char *value);

/**
 * dlt_config_file_check_section_name_exists
 *
 * Get name of section number.
 *
 * @param[in]  file      DltConfigFile
 * @param[in]  name      Section name
 * @return     0 on success/exist, else -1
 */
int dlt_config_file_check_section_name_exists(const DltConfigFile *file,
                                             const char *name);
#endif
