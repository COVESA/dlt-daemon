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
 * \file dlt_client.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_client.c                                                  **
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
 * $LastChangedRevision$
 * $LastChangedDate$
 * $LastChangedBy$
 Initials    Date         Comment
 aw          12.07.2010   initial
 */

#include <stdio.h>

#if defined (__WIN32__) || defined (_MSC_VER)
#pragma warning(disable : 4996) /* Switch off C4996 warnings */
#include <winsock2.h> /* for socket(), connect(), send(), and recv() */
#else
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>
#include <sys/stat.h>
#endif

#if defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#include <syslog.h>
#endif

#include <fcntl.h>

#include <stdlib.h> /* for malloc(), free() */
#include <string.h> /* for strlen(), memcmp(), memmove() */
#include <errno.h>

#include "dlt_types.h"
#include "dlt_client.h"
#include "dlt_client_cfg.h"

static int (*message_callback_function) (DltMessage *message, void *data) = NULL;

void dlt_client_register_message_callback(int (*registerd_callback) (DltMessage *message, void *data)){
    message_callback_function = registerd_callback;
}

int dlt_client_init(DltClient *client, int verbose)
{
    if (verbose)
	{
		printf("Init dlt client struct\n");
	}

    if (client==0)
    {
        return -1;
    }

    client->sock=-1;
    client->servIP=0;
    client->serialDevice=0;
    client->baudrate=DLT_CLIENT_INITIAL_BAUDRATE;
    client->serial_mode=0;
    client->receiver.buffer=0;

    return 0;
}

int dlt_client_connect(DltClient *client, int verbose)
{
	struct sockaddr_in servAddr;
	unsigned short servPort = DLT_DAEMON_TCP_PORT;
	struct hostent *host;            /* Structure containing host information */

    if (client==0)
    {
        return -1;
    }

    if (client->serial_mode==0)
    {
        /* open socket */
        if ((client->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            fprintf(stderr,"ERROR: socket() failed!\n");
            return -1;
        }

        if ((host = (struct hostent*) gethostbyname(client->servIP)) == 0)
        {
            fprintf(stderr, "ERROR: gethostbyname() failed\n");
            return -1;
        }

        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family      = AF_INET;
        servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*( struct in_addr*)( host -> h_addr_list[0])));
        servAddr.sin_port        = htons(servPort);

        if (connect(client->sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        {
            fprintf(stderr,"ERROR: connect() failed!\n");
            return -1;
        }

        if (verbose)
		{
			printf("Connected to DLT daemon (%s)\n",client->servIP);
		}
    }
    else
    {
        /* open serial connection */
        client->sock=open(client->serialDevice,O_RDWR);
        if (client->sock<0)
        {
            fprintf(stderr,"ERROR: Failed to open device %s\n", client->serialDevice);
            return -1;
        }

        if (isatty(client->sock))
        {
			#if !defined (__WIN32__)
            if (dlt_setup_serial(client->sock,client->baudrate)<0)
            {
                fprintf(stderr,"ERROR: Failed to configure serial device %s (%s) \n", client->serialDevice, strerror(errno));
                return -1;
            }
			#else
				return -1;
			#endif
        }
        else
        {
            if (verbose)
            {
            	fprintf(stderr,"ERROR: Device is not a serial device, device = %s (%s) \n", client->serialDevice, strerror(errno));
            }
            return -1;
        }

        if (verbose)
        {
        	printf("Connected to %s\n", client->serialDevice);
        }
    }

    if (dlt_receiver_init(&(client->receiver),client->sock,DLT_CLIENT_RCVBUFSIZE)!=0)
    {
        return -1;
    }

    return 0;
}

int dlt_client_cleanup(DltClient *client, int verbose)
{
    if (verbose)
	{
		printf("Cleanup dlt client\n");
	}

    if (client==0)
    {
        return -1;
    }

    if (client->sock!=-1)
    {
        close(client->sock);
    }

    if (dlt_receiver_free(&(client->receiver))==-1)
    {
		return -1;
    }

    return 0;
}

int dlt_client_main_loop(DltClient *client, void *data, int verbose)
{
	DltMessage msg;
    int ret;

	if (client==0)
	{
        return -1;
	}

    if (dlt_message_init(&msg,verbose)==-1)
    {
		return -1;
    }

    while (1)
    {
        if (client->serial_mode==0)
        {
            /* wait for data from socket */
            ret = dlt_receiver_receive_socket(&(client->receiver));
        }
        else
        {
            /* wait for data from serial connection */
            ret = dlt_receiver_receive_fd(&(client->receiver));
        }

        if (ret<=0)
        {
            /* No more data to be received */
            if (dlt_message_free(&msg,verbose)==-1)
            {
				return -1;
            }

            return 1;
        }

        while (dlt_message_read(&msg,(unsigned char*)(client->receiver.buf),client->receiver.bytesRcvd,0,verbose)==0)
        {
            /* Call callback function */
            if (message_callback_function)
            {
                (*message_callback_function)(&msg,data);
            }

            if (msg.found_serialheader)
            {
                if (dlt_receiver_remove(&(client->receiver),msg.headersize+msg.datasize-sizeof(DltStorageHeader)+sizeof(dltSerialHeader))==-1)
                {
                	/* Return value ignored */
                	dlt_message_free(&msg,verbose);
					return -1;
                }
            }
            else
            {
                if (dlt_receiver_remove(&(client->receiver),msg.headersize+msg.datasize-sizeof(DltStorageHeader))==-1)
                {
                	/* Return value ignored */
                	dlt_message_free(&msg,verbose);
					return -1;
                }
            }
        }

        if (dlt_receiver_move_to_begin(&(client->receiver))==-1)
		{
			/* Return value ignored */
			dlt_message_free(&msg,verbose);
			return -1;
		}
    }

    if (dlt_message_free(&msg,verbose)==-1)
    {
		return -1;
    }

    return 0;
}

int dlt_client_send_inject_msg(DltClient *client, char *apid, char *ctid, uint32_t serviceID, uint8_t *buffer, uint32_t size)
{
	DltMessage msg;
        int ret;
        int offset=0;

	int32_t len;

	if ((client==0) || (client->sock<0) || (apid==0) || (ctid==0) || (buffer==0) || (size==0) ||
		(serviceID<DLT_SERVICE_ID_CALLSW_CINJECTION))
	{
		return -1;
	}

	/* initialise new message */
	if (dlt_message_init(&msg,0)==-1)
	{
		return -1;
	}

	/* prepare payload of data */
	msg.datasize = sizeof(uint32_t) + sizeof(uint32_t) + size;
	if (msg.databuffer && (msg.databuffersize < msg.datasize))
	{
		free(msg.databuffer);
		msg.databuffer=0;
	}
	if (msg.databuffer == 0){
		msg.databuffer = (uint8_t *) malloc(msg.datasize);
		msg.databuffersize = msg.datasize;
	}
	if(msg.databuffer == 0)
	{
		dlt_message_free(&msg,0);
		return -1;
	}

	memcpy(msg.databuffer  , &serviceID,sizeof(serviceID));
	offset+=sizeof(uint32_t);
	memcpy(msg.databuffer+offset, &size, sizeof(size));
	offset+=sizeof(uint32_t);
	memcpy(msg.databuffer+offset, buffer, size);

	/* prepare storage header */
    msg.storageheader = (DltStorageHeader*)msg.headerbuffer;

	if (dlt_set_storageheader(msg.storageheader,"")==-1)
	{
		dlt_message_free(&msg,0);
		return -1;
	}

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

	#if (BYTE_ORDER==BIG_ENDIAN)
		msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
	#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
	dlt_set_id(msg.headerextra.ecu,"");
    //msg.headerextra.seid = 0;
    msg.headerextra.tmsp = dlt_uptime();

    /* Copy header extra parameters to headerbuffer */
    if (dlt_message_set_extraparameters(&msg,0)==-1)
	{
		dlt_message_free(&msg,0);
		return -1;
	}

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer +
						 sizeof(DltStorageHeader) +
						 sizeof(DltStandardHeader) +
						 DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) );

    msg.extendedheader->msin = DLT_MSIN_CONTROL_REQUEST;

    msg.extendedheader->noar = 1; /* number of arguments */

    dlt_set_id(msg.extendedheader->apid,(apid[0]=='\0')?DLT_CLIENT_DUMMY_APP_ID:apid);
	dlt_set_id(msg.extendedheader->ctid,(ctid[0]=='\0')?DLT_CLIENT_DUMMY_CON_ID:ctid);

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) +
                     sizeof(DltStandardHeader) +
                     sizeof(DltExtendedHeader) +
                     DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);

	len=msg.headersize - sizeof(DltStorageHeader) + msg.datasize;
	if (len>UINT16_MAX)
	{
		fprintf(stderr,"Critical: Huge injection message discarded!\n");
		dlt_message_free(&msg,0);

		return -1;
	}

    msg.standardheader->len = DLT_HTOBE_16(len);

	/* Send data (without storage header) */
	if (client->serial_mode)
	{
		/* via FileDescriptor */
		ret=write(client->sock, msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
                if (0 > ret){
                        dlt_message_free(&msg,0);
                        return -1;
                }
		ret=write(client->sock, msg.databuffer,msg.datasize);
                if (0 > ret){
                        dlt_message_free(&msg,0);
                        return -1;
                }
	}
	else
	{
		/* via Socket */
		send(client->sock, (const char *)(msg.headerbuffer+sizeof(DltStorageHeader)),msg.headersize-sizeof(DltStorageHeader),0);
		send(client->sock, (const char *)msg.databuffer,msg.datasize,0);
	}

	/* free message */
	if (dlt_message_free(&msg,0)==-1)
	{
		return -1;
	}

	return 0;
}

int dlt_client_setbaudrate(DltClient *client, int baudrate)
{
	if (client==0)
	{
		return -1;
	}

	client->baudrate = dlt_convert_serial_speed(baudrate);

	return 0;
}

