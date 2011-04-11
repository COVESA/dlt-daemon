/*
 * Dlt Adaptor for forwarding Syslog messages to Dlt
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
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
