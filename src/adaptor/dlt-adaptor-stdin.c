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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-adaptor-stdin.c
*/

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-adaptor-stdin.c                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
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
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlt_common.h"
#include "dlt_user.h"

#define MAXSTRLEN 1024

#define PS_DLT_APP_DESC      "stdin adaptor application"
#define PS_DLT_CONTEXT_DESC  "stdin adaptor context"

#define PS_DLT_APP "SINA"
#define PS_DLT_CONTEXT "SINC"

DLT_DECLARE_CONTEXT(mycontext);

int main(int argc, char* argv[])
{
    char str[MAXSTRLEN];
    int opt;

    char apid[DLT_ID_SIZE];
    char ctid[DLT_ID_SIZE];
    char version[255];
    int timeout = -1;

    dlt_set_id(apid, PS_DLT_APP);
    dlt_set_id(ctid, PS_DLT_CONTEXT);

    while ((opt = getopt(argc, argv, "a:c:ht:")) != -1)
    {
        switch (opt)
        {
        case 'a':
        {
            dlt_set_id(apid,optarg);
            break;
        }
        case 'c':
        {
            dlt_set_id(ctid,optarg);
            break;
        }
        case 't':
        {
          timeout = atoi(optarg);
          break;
        }
        case 'h':
        {
            dlt_get_version(version,255);

            printf("Usage: dlt-adaptor-stdin [options]\n");
            printf("Adaptor for forwarding input from stdin to DLT daemon.\n");
            printf("%s \n", version);
            printf("Options:\n");
            printf("  -a apid      - Set application id to apid (default: SINA)\n");
            printf("  -c ctid      - Set context id to ctid (default: SINC)\n");
            printf("  -t timeout   - Set timeout when sending messages at exit, in ms (Default: 10000 = 10sec)\n");
            printf("  -h           - This help\n");
            return 0;
            break;
        }
        default: /* '?' */
        {
            fprintf(stderr, "Unknown option '%c'\n", optopt);
            return -1;
        }
        }
    }

    DLT_REGISTER_APP(apid,PS_DLT_APP_DESC);
    DLT_REGISTER_CONTEXT(mycontext, ctid, PS_DLT_CONTEXT_DESC);
    if (timeout > -1)
    {
      dlt_set_resend_timeout_atexit(timeout);
    }

    while (fgets(str, MAXSTRLEN, stdin))
    {
        if (strcmp(str,"")!=0)
        {
            DLT_LOG(mycontext, DLT_LOG_INFO, DLT_STRING(str));
        }
    }

    DLT_UNREGISTER_CONTEXT(mycontext);
    DLT_UNREGISTER_APP();

    return 0;
}

