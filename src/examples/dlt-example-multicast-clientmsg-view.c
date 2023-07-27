/*
 * Copyright (c) 2019 LG Electronics Inc.
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author
 * Guruprasad KN <guruprasad.kn@lge.com>
 * Sachin Sudhakar Shetty <sachin.shetty@lge.com>
 * Sunil Kovila Sampath <sunil.s@lge.com>
 *
 * \copyright Copyright (c) 2019 LG Electronics Inc.
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-example-multicast-clientmsg-view.c
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */
#include <errno.h>
#include <string.h>
#include <glob.h>
#include <syslog.h>
#include <linux/limits.h> /* for PATH_MAX */
#include <inttypes.h>

#include "dlt_client.h"
#include "dlt_client_cfg.h"

#define DLT_RECEIVE_TEXTBUFSIZE 10024

#define HELLO_PORT 3491
#define HELLO_GROUP "225.0.0.37"

struct clientinfostruct
{
    int fd;
    struct sockaddr_in addr;
    socklen_t addlen;
    DltReceiver receiver;
};

int dlt_receiver_receive_socket_udp(struct clientinfostruct *clientinfo, DltReceiver *receiver)
{
    if ((receiver == NULL) || (clientinfo == NULL)) {
        printf("NULL receiver or clientinfo in dlt_receiver_receive_socket_udp\n");
        return -1;
    }

    if (receiver->buffer == NULL) {
        printf("NULL receiver->buffer in dlt_receiver_receive_socket_udp\n");
        return -1;
    }

    receiver->buf = (char *)receiver->buffer;
    receiver->lastBytesRcvd = receiver->bytesRcvd;

    /* wait for data from socket */
    unsigned int addrlen = sizeof(clientinfo->addr);

    if ((receiver->bytesRcvd = recvfrom(clientinfo->fd,
                                        receiver->buf + receiver->lastBytesRcvd,
                                        receiver->buffersize - receiver->lastBytesRcvd,
                                        0,
                                        (struct sockaddr *)&(clientinfo->addr), &addrlen))
        <= 0) {
        printf("Error\n");
        perror("recvfrom");
        receiver->bytesRcvd = 0;
        return receiver->bytesRcvd;
    } /* if */

    receiver->totalBytesRcvd += receiver->bytesRcvd;
    receiver->bytesRcvd += receiver->lastBytesRcvd;

    return receiver->bytesRcvd;
}

int dlt_receive_message_callback_udp(DltMessage *message)
{
    static char text[DLT_RECEIVE_TEXTBUFSIZE];

    if ((message == NULL)) {
        printf("NULL message in dlt_receive_message_callback_udp\n");
        return -1;
    }

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    else
        dlt_set_storageheader(message->storageheader, "ECU1");

    dlt_message_header(message, text, DLT_RECEIVE_TEXTBUFSIZE, 0);

    printf("%s ", text);

    dlt_message_payload(message, text, DLT_RECEIVE_TEXTBUFSIZE, DLT_OUTPUT_ASCII, 0);

    printf("[%s]\n", text);

    return 0;
}


int main()
{
    struct clientinfostruct clientinfo;
    struct ip_mreq mreq;

    u_int yes = 1;

    /* create what looks like an ordinary UDP socket */
    if ((clientinfo.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(clientinfo.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        exit(1);
    }

    /* set up destination address */
    memset(&clientinfo.addr, 0, sizeof(clientinfo.addr));
    clientinfo.addr.sin_family = AF_INET;
    clientinfo.addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
    clientinfo.addr.sin_port = htons(HELLO_PORT);

    /* bind to receive address */
    if (bind(clientinfo.fd, (struct sockaddr *)&clientinfo.addr, sizeof(clientinfo.addr)) < 0) {
        perror("bind");
        exit(1);
    }

    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr = inet_addr(HELLO_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(clientinfo.fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    DltMessage msg;

    if (dlt_message_init(&msg, 0) == DLT_RETURN_ERROR)
        return DLT_RETURN_ERROR;

    if (dlt_receiver_init(&(clientinfo.receiver),
                          clientinfo.fd,
                          DLT_RECEIVE_UDP_SOCKET,
                          DLT_RECEIVE_BUFSIZE) != DLT_RETURN_OK)
        return DLT_RETURN_ERROR;

    printf("Waiting for message on ip %s port : %d\n", HELLO_GROUP, HELLO_PORT);

    while (1) {
        /* wait for data from socket */
        dlt_receiver_receive_socket_udp(&clientinfo, &(clientinfo.receiver));

        while (dlt_message_read(&msg, (unsigned char *)(clientinfo.receiver.buf),
                                clientinfo.receiver.bytesRcvd, 0, 0) == DLT_MESSAGE_ERROR_OK) {
            dlt_receive_message_callback_udp(&msg);

            if (dlt_receiver_remove(&(clientinfo.receiver),
                                    msg.headersize + msg.datasize - sizeof(DltStorageHeader))
                == DLT_RETURN_ERROR) {
                /* Return value ignored */
                dlt_message_free(&msg, 0);
                return DLT_RETURN_ERROR;
            }
        }

        if (dlt_receiver_move_to_begin(&(clientinfo.receiver)) == DLT_RETURN_ERROR) {
            /* Return value ignored */
            dlt_message_free(&msg, 0);
            return DLT_RETURN_ERROR;
        }
    }
}

