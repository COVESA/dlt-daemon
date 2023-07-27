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
 * \file dlt-kpi-common.h
 */

#ifndef SRC_KPI_DLT_KPI_COMMON_H_
#define SRC_KPI_DLT_KPI_COMMON_H_

#include <dlt_types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

DltReturnValue dlt_kpi_read_file(char *filename, char *buffer, uint maxLength);
DltReturnValue dlt_kpi_read_file_compact(char *filename, char **target);
int dlt_kpi_get_cpu_count();

#endif /* SRC_KPI_DLT_KPI_COMMON_H_ */
