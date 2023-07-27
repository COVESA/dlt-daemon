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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-shell.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-shell.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
**  lm          Lassi Marttala             BMW                                **
*******************************************************************************/
#include "dlt.h"
#include "dlt-system.h"

#include <string.h>
#include <stdlib.h>

#define DLT_SHELL_COMMAND_MAX_LENGTH 1024

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(shellContext)

int dlt_shell_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    (void)length;

    DLT_LOG(shellContext, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-shell, injection callback"));
    char text[DLT_SHELL_COMMAND_MAX_LENGTH];
    int syserr = 0;

    if (length <= DLT_SHELL_COMMAND_MAX_LENGTH - 1) {
        strncpy(text, data, length);
        text[length] = 0;
    }
    else {
        strncpy(text, data, DLT_SHELL_COMMAND_MAX_LENGTH - 1);
        text[DLT_SHELL_COMMAND_MAX_LENGTH - 1] = 0;
    }

    DLT_LOG(shellContext, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-shell, injection injection id:"),
            DLT_UINT32(service_id));
    DLT_LOG(shellContext, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-shell, injection data:"),
            DLT_STRING(text));

    switch (service_id) {
    case 0x1001:

        if ((syserr = system(text)) != 0)
            DLT_LOG(shellContext, DLT_LOG_ERROR,
                    DLT_STRING("dlt-system-shell, abnormal exit status."),
                    DLT_STRING(text),
                    DLT_INT(syserr));
        else
            DLT_LOG(shellContext, DLT_LOG_INFO,
                    DLT_STRING("Shell command executed:"),
                    DLT_STRING(text));

        break;
    default:
        DLT_LOG(shellContext, DLT_LOG_ERROR,
                DLT_STRING("dlt-system-shell, unknown command received."),
                DLT_UINT32(service_id),
                DLT_STRING(text));
        break;
    }

    return 0;
}

void init_shell()
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-shell, register callback"));
    DLT_REGISTER_CONTEXT(shellContext, "CMD", "Execute Shell commands");
    DLT_REGISTER_INJECTION_CALLBACK(shellContext, 0x1001, dlt_shell_injection_callback);
}
