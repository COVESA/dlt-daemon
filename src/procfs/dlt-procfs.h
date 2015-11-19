/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-procfs.h
 */

#ifndef SRC_PROCFS_DLT_PROCFS_H_
#define SRC_PROCFS_DLT_PROCFS_H_

#include "dlt.h"
#include "dlt-procfs-common.h"
#include "dlt-procfs-process.h"
#include "dlt-procfs-process-list.h"
#include "dlt-procfs-interrupt.h"
#include <syslog.h>

// CONSTANT DEFINITIONS
#define DEFAULT_CONF_FILE ( CONFIGURATION_FILES_DIR "/dlt-procfs.conf")

#define COMMAND_LINE_SIZE 1024

// STRUCTURES
typedef struct
{
    char *configurationFileName;
    int customConfigFile;
} DltProcfsOptions;

typedef struct
{
    int process_log_interval, irq_log_interval, check_log_interval;
    DltLogLevelType log_level;
} DltProcfsConfig;

// FUNCTION DECLARATIONS:
DltReturnValue dlt_procfs_read_command_line(DltProcfsOptions *options, int argc, char **argv);
DltReturnValue dlt_procfs_read_configuration_file(DltProcfsConfig *config, char *file_name);
void dlt_procfs_free_cli_options(DltProcfsOptions *options);
DltReturnValue dlt_procfs_init(int argc, char **argv, DltProcfsConfig *config);

#endif /* SRC_PROCFS_DLT_PROCFS_H_ */
