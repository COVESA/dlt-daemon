/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, V2 - Volvo Group
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
 * \author Shivam Goel <shivam.goel@volvo.com>
 *
 * \copyright Copyright © 2011-2015 V2 - Volvo Group. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-preregister-context-v2.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-preregister-context-v2.c                             **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Shivam Goel <shivam.goel@volvo.com>                           **
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
**  sg          Shivam Goel                V2 - Volvo Group                   **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision:  $
 * $LastChangedDate: 2025-11-12 15:12:06 +0200 (We, 12. Nov 2025) $
 * $LastChangedBy$
 * Initials    Date         Comment
 * sg          12.11.2025   initial
 */

#include <unistd.h> /* for fork() */

#include "dlt.h"
#include "dlt_user_macros.h"

/**
 * @brief sample code for using pre-registered contexts
 */
int main()
{
    DltContext mainContext;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000 * 1000;

    DLT_REGISTER_CONTEXT_V2(mainContext, "CTXP", "main context");

    DLT_LOG_V2(mainContext, DLT_LOG_WARN, DLT_STRING("First message before app registered"));
    nanosleep(&ts, NULL);

    DLT_LOG_V2(mainContext, DLT_LOG_WARN, DLT_STRING("Second message before app registered"));
    nanosleep(&ts, NULL);

    DLT_REGISTER_APP_V2("PRNT", "Sample pre-register application");

    DLT_LOG_V2(mainContext, DLT_LOG_WARN, DLT_STRING("First message after app registered"));
    nanosleep(&ts, NULL);

    DLT_UNREGISTER_APP_V2();

    return 0;
}
