/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file example4.c
 * For further information see http://www.genivi.org/.
 * @licence end@
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

	DLT_REGISTER_APP("EXA4","Fourth Example");

	DLT_REGISTER_CONTEXT(con_exa1,"CON","First context");

	for(num=0;num<256;num++)
	{
		buffer[num] = num;
	}

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_RAW"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_RAW(buffer,256));

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_HEX8"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_HEX8(buffer,256));

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_HEX16"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_HEX16(buffer,256));

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_HEX32"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_HEX32(buffer,256));

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_HEX64"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_HEX64(buffer,256));

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_BIN8"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_BIN8(buffer,256));

	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_STRING("DLT_BIN16"));
	DLT_LOG(con_exa1,DLT_LOG_INFO,DLT_BIN16(buffer,256));

    usleep(1000);

	DLT_UNREGISTER_CONTEXT(con_exa1);

	DLT_UNREGISTER_APP();
}
