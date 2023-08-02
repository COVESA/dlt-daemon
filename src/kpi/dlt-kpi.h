/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
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
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-kpi.h
 */

#ifndef SRC_KPI_DLT_KPI_H_
#define SRC_KPI_DLT_KPI_H_

#include "dlt.h"
#include <syslog.h>

#include "dlt-kpi-common.h"
#include "dlt-kpi-interrupt.h"
#include "dlt-kpi-process.h"
#include "dlt-kpi-process-list.h"

/* CONSTANT DEFINITIONS */
#define DEFAULT_CONF_FILE (CONFIGURATION_FILES_DIR "/dlt-kpi.conf")

#define COMMAND_LINE_SIZE 1024

#define NANOSEC_PER_MILLISEC 1000000
#define NANOSEC_PER_SEC 1000000000

/* STRUCTURES */
typedef struct
{
    char *configurationFileName;
    int customConfigFile;
} DltKpiOptions;

typedef struct
{
    int process_log_interval, irq_log_interval, check_log_interval;
    DltLogLevelType log_level;
} DltKpiConfig;

/* FUNCTION DECLARATIONS: */
DltReturnValue dlt_kpi_read_command_line(DltKpiOptions *options, int argc, char **argv);
DltReturnValue dlt_kpi_read_configuration_file(DltKpiConfig *config, char *file_name);
void dlt_kpi_free_cli_options(DltKpiOptions *options);
DltReturnValue dlt_kpi_init(int argc, char **argv, DltKpiConfig *config);

#endif /* SRC_KPI_DLT_KPI_H_ */
