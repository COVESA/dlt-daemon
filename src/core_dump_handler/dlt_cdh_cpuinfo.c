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
 * \author Gianfranco Costamagna <locutusofborg@debian.org>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_cdh_cpuinfo.c
 */

#include "dlt_cdh_cpuinfo.h"

void get_registers(prstatus_t *prstatus, cdh_registers_t *registers)
{
/*    struct user_regs_struct *ptr_reg = (struct user_regs_struct *)prstatus->pr_reg;

    registers->pc = ptr_reg->pc;*/ /* [REG_PROC_COUNTER]; */

}
