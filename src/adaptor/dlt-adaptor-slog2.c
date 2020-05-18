/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2020 Visteon Corporation
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
 * \author 
 * Karthik Shanmugam <karthik.shanmugam@visteon.com>
 *
 * \copyright Copyright © 2020 Visteon Corporation. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-adaptor-slog2.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-adaptor-slog2.c                                           **
**                                                                            **
**  TARGET    : QNX                                                           **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Karthik Shanmugam karthik.shanmugam@visteon.com               **
**                                                                            **
**  PURPOSE   : Log messages from slog2 to DLT                                **
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
**  ks          Karthik Shanmugam           Visteon                           **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/slog2.h>
#include <slog2_parse.h>

#include "dlt_common.h"
#include "dlt_user.h"

#define PL_DLT_APP_DESC      "slog2 adaptor application"
#define PL_DLT_CONTEXT_DESC  "slog2 adaptor context"

#define PL_DLT_APP "SL2A"
#define PL_DLT_CONTEXT "SL2C"

slog2_packet_info_t packet_info = SLOG2_PACKET_INFO_INIT;
int verbosity = DLT_LOG_INFO;

DLT_DECLARE_CONTEXT(mycontext)

int slog2_callback (slog2_packet_info_t *info, void *payload, void *param __attribute__((unused)) )
{
    int log_level = DLT_LOG_VERBOSE;
    switch (info->severity)
    {
        case SLOG2_SHUTDOWN: log_level = DLT_LOG_FATAL;   break;
        case SLOG2_CRITICAL: log_level = DLT_LOG_FATAL;   break;
        case SLOG2_ERROR:    log_level = DLT_LOG_ERROR;   break;
        case SLOG2_WARNING:  log_level = DLT_LOG_WARN;    break;
        case SLOG2_NOTICE:   log_level = DLT_LOG_INFO;    break;
        case SLOG2_INFO:     log_level = DLT_LOG_INFO;    break;
        case SLOG2_DEBUG1:   log_level = DLT_LOG_DEBUG;   break;
        case SLOG2_DEBUG2:   log_level = DLT_LOG_VERBOSE; break;
        default: break;
    }

    if(log_level <= verbosity)
    {
        DLT_LOG(mycontext, log_level, DLT_STRING(payload)); 
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    char apid[DLT_ID_SIZE];
    char ctid[DLT_ID_SIZE];
    char version[255];

    dlt_set_id(apid, PL_DLT_APP);
    dlt_set_id(ctid, PL_DLT_CONTEXT);

    while ((opt = getopt(argc, argv, "a:c:hp:v:")) != -1)
        switch (opt) {
        case 'a':
        {
            dlt_set_id(apid, optarg);
            break;
        }
        case 'c':
        {
            dlt_set_id(ctid, optarg);
            break;
        }
        case 'h':
        {
            dlt_get_version(version, 255);

            printf("Usage: dlt-adaptor-slog2 [options]\n");
            printf("Adaptor for forwarding Slog2 messages to DLT daemon.\n");
            printf("%s \n", version);
            printf("Options:\n");
            printf("-a apid      - Set application id to apid (default: UDPA)\n");
            printf("-c ctid      - Set context id to ctid (default: UDPC)\n");
            printf(
                "-v verbosity level - Set verbosity level (Default: INFO, values: FATAL ERROR WARN INFO DEBUG VERBOSE)\n");
            printf("-h           - This help\n");
            return 0;
            break;
        }
        case 'v':
        {
            if (!strcmp(optarg, "FATAL")) {
                verbosity = DLT_LOG_FATAL;
                break;
            }
            else if (!strcmp(optarg, "ERROR"))
            {
                verbosity = DLT_LOG_ERROR;
                break;
            }
            else if (!strcmp(optarg, "WARN"))
            {
                verbosity = DLT_LOG_WARN;
                break;
            }
            else if (!strcmp(optarg, "INFO"))
            {
                verbosity = DLT_LOG_INFO;
                break;
            }
            else if (!strcmp(optarg, "DEBUG"))
            {
                verbosity = DLT_LOG_DEBUG;
                break;
            }
            else if (!strcmp(optarg, "VERBOSE"))
            {
                verbosity = DLT_LOG_VERBOSE;
                break;
            }
            else {
                printf(
                    "Wrong verbosity level, setting to INFO. Accepted values are: FATAL ERROR WARN INFO DEBUG VERBOSE\n");
                verbosity = DLT_LOG_INFO;
                break;
            }

            break;
        }
        default: /* '?' */
        {
            fprintf(stderr, "Unknown option '%c'\n", optopt);
            exit(3);
            return 3;/*for parasoft */
        }
    }

    DLT_REGISTER_APP(apid, PL_DLT_APP_DESC);
    DLT_REGISTER_CONTEXT(mycontext, ctid, PL_DLT_CONTEXT_DESC);

    if( 0 != slog2_parse_all ( SLOG2_PARSE_FLAGS_DYNAMIC, NULL, NULL, &packet_info, slog2_callback, NULL ) )
    {
        fprintf(stderr, "slog2_parse_all failed\n");
    }

    DLT_UNREGISTER_CONTEXT(mycontext);
    DLT_UNREGISTER_APP();

    return 0;
}
