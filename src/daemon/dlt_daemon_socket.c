/* @licence app begin@
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
 * \file dlt_daemon_socket.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_socket.c                                           **
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

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
*******************************************************************************/

#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>

#include <sys/timerfd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/stat.h>

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"

#include "dlt_daemon_socket.h"

/** Global text output buffer, mainly used for creation of error/warning strings */
static char str[DLT_DAEMON_TEXTBUFSIZE];

int dlt_daemon_socket_open(int *sock)
{
    int yes = 1;

    struct sockaddr_in servAddr;
    unsigned int servPort = DLT_DAEMON_TCP_PORT;

    if ((*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        dlt_log(LOG_ERR, "dlt_daemon_socket_open: socket() failed!\n");
        return -1;
    } /* if */

    if ( -1 == setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
    {
        sprintf(str,"dlt_daemon_socket_open: Setsockopt error in dlt_daemon_local_connection_init: %s\n",strerror(errno));
        dlt_log(LOG_ERR, str);
        return -1;
    }
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port        = htons(servPort);

    if (bind(*sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        dlt_log(LOG_ERR, "dlt_daemon_socket_open: bind() failed!\n");
        return -1;
    } /* if */

    if (listen(*sock, 3) < 0)
    {
        dlt_log(LOG_ERR, "dlt_daemon_socket_open: listen() failed!\n");
        return -1;
    } /* if */

    return 0; /* OK */
}

int dlt_daemon_socket_close(int sock)
{
	close(sock);

	return 0;
}

int dlt_daemon_socket_send(int sock,void* data1,int size1,void* data2,int size2,char serialheader)
{
	/* Optional: Send serial header, if requested */
	if (serialheader)
	{
		if ( 0 > send(sock, dltSerialHeader,sizeof(dltSerialHeader),0) )
			return -1;

	}

	/* Send data */
	if(data1 && size1>0)
	{
		if (0 > send(sock, data1,size1,0))
			return -1;
	}

	if(data2 && size2>0)
	{
		if (0 > send(sock, data2,size2,0))
		return -1;
	}

	return 0;
}
