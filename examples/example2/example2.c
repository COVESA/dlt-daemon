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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file example2.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: example2.c                                                    **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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

#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */

#include <dlt.h>

#include "dlt_id.h"

DLT_DECLARE_CONTEXT(con_exa2);

int main()
{
    int num;
    struct timespec ts;

    DLT_REGISTER_APP("EXA2", "Third Example");
    DLT_REGISTER_CONTEXT(con_exa2, "CON", "First context");

    DLT_NONVERBOSE_MODE();

    for (num = 0; num < 10; num++) {
        DLT_LOG_ID(con_exa2, DLT_LOG_INFO, DLT_EXA2_CON_EXA2_ID1, DLT_INT32(12345678), DLT_STRING("Hello world 1!"));
        DLT_LOG_ID(con_exa2, DLT_LOG_ERROR, DLT_EXA2_CON_EXA2_ID2, DLT_INT32(87654321), DLT_STRING("Hello world 2!"));
        DLT_LOG_ID(con_exa2, DLT_LOG_WARN, DLT_EXA2_CON_EXA2_ID3, DLT_INT32(11223344), DLT_STRING("Hello world 3!"));
        ts.tv_sec = 0;
        ts.tv_nsec = 1000000;
        nanosleep(&ts, NULL);
    }

    DLT_UNREGISTER_CONTEXT(con_exa2);

    DLT_UNREGISTER_APP();
}
