/*
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
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file example4.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: example4.c                                                    **
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

DLT_DECLARE_CONTEXT(con_exa1);

int main()
{
    unsigned char buffer[256];
    int num;
    struct timespec ts;

    DLT_REGISTER_APP("EXA4", "Fourth Example");

    DLT_REGISTER_CONTEXT(con_exa1, "CON", "First context");

    for (num = 0; num < 256; num++)
        buffer[num] = num;

    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_RAW"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_RAW(buffer, 256));

    uint8_t uint8data = 0x2a;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_UINT8"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_UINT8(uint8data));

    uint8_t hex8data = 0x1a;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_HEX8"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_HEX8(hex8data));

    uint16_t hex16data = 0x1ad3;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_HEX16"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_HEX16(hex16data));

    uint32_t hex32data = 0x1abcd3e4;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_HEX32"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_HEX32(hex32data));

    uint64_t hex64data = 0x17b4ddcf34eabb2a;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_HEX64"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_HEX64(hex64data));

    uint8_t bin8data = 0xe2;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_BIN8"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_BIN8(bin8data));
    bin8data = 0x01;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_BIN8"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_BIN8(bin8data));

    uint16_t bin16data = 0x1234;
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_STRING("DLT_BIN16"));
    DLT_LOG(con_exa1, DLT_LOG_INFO, DLT_BIN16(bin16data));

    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;
    nanosleep(&ts, NULL);

    DLT_UNREGISTER_CONTEXT(con_exa1);

    DLT_UNREGISTER_APP();
}
