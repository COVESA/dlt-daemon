/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
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
 * \file dlt-adaptor-udp.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-adaptor-udp.c                                             **
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
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "dlt_common.h"
#include "dlt_user.h"

/* Port number, to which the syslogd-ng sends its log messages */
#define RCVPORT               47111

#define MAXSTRLEN             1024

#define PU_DLT_APP_DESC      "udp adaptor application"
#define PU_DLT_CONTEXT_DESC  "udp adaptor context"

#define PU_DLT_APP "UDPA"
#define PU_DLT_CONTEXT "UDPC"

DLT_DECLARE_CONTEXT(mycontext);

int main(int argc, char* argv[])
{
    int sock;
    int bytes_read;
    socklen_t addr_len;
    int opt, port;
    char recv_data[MAXSTRLEN];
    struct sockaddr_in client_addr, server_addr;

    char apid[DLT_ID_SIZE];
    char ctid[DLT_ID_SIZE];
    char version[255];

    dlt_set_id(apid, PU_DLT_APP);
    dlt_set_id(ctid, PU_DLT_CONTEXT);

    port = RCVPORT;

    while ((opt = getopt(argc, argv, "a:c:hp:")) != -1)
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
        case 'h':
        {
            dlt_get_version(version);

            printf("Usage: dlt-adaptor-udp [options]\n");
            printf("Adaptor for forwarding received UDP messages to DLT daemon.\n");
            printf("%s \n", version);
            printf("Options:\n");
            printf("-a apid      - Set application id to apid (default: UDPA)\n");
            printf("-c ctid      - Set context id to ctid (default: UDPC)\n");
            printf("-p           - Set receive port number for UDP messages (default: %d) \n", port);
            printf("-h           - This help\n");
            return 0;
            break;
        }
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        default: /* '?' */
        {
            fprintf(stderr, "Unknown option '%c'\n", optopt);
            exit(3);
            return 3;//for parasoft
        }
        }
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);


    if (bind(sock, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        perror("Bind");
        return -1;
    }

    addr_len = sizeof(struct sockaddr);

    DLT_REGISTER_APP(apid,PU_DLT_APP_DESC);
    DLT_REGISTER_CONTEXT(mycontext,ctid,PU_DLT_CONTEXT_DESC);

    while (1)
    {
        bytes_read = 0;

        bytes_read = recvfrom(sock, recv_data, MAXSTRLEN, 0,
                              (struct sockaddr *)&client_addr, &addr_len);

        if (bytes_read == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                DLT_UNREGISTER_CONTEXT(mycontext);
                DLT_UNREGISTER_APP();
                exit(1);
            }
        }

        recv_data[bytes_read] = '\0';

        if (bytes_read != 0)
        {
            DLT_LOG(mycontext, DLT_LOG_INFO, DLT_STRING(recv_data));
        }
    }

    DLT_UNREGISTER_CONTEXT(mycontext);
    DLT_UNREGISTER_APP();

    return 0;
}
