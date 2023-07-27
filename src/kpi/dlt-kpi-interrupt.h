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
 * \file dlt-kpi-interrupt.h
 */

#ifndef SRC_KPI_DLT_KPI_INTERRUPT_H_
#define SRC_KPI_DLT_KPI_INTERRUPT_H_

#include "dlt.h"
#include "dlt-kpi-common.h"

DltReturnValue dlt_kpi_log_interrupts(DltContext *ctx, DltLogLevelType log_level);

#endif /* SRC_KPI_DLT_KPI_INTERRUPT_H_ */
