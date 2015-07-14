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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_client.c
*/

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

#ifdef linux
#include <sys/timerfd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#ifdef linux
#include <linux/stat.h>
#endif

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"

#include "dlt_daemon_socket.h"
#include "dlt_daemon_serial.h"

#include "dlt_daemon_client.h"
#include "dlt_daemon_connection.h"

#include "dlt_daemon_offline_logstorage.h"
/** Global text output buffer, mainly used for creation of error/warning strings */
static char str[DLT_DAEMON_TEXTBUFSIZE];

/** @brief Sends up to 2 messages to all the clients.
 *
 * Runs through the client list and sends the messages to them. If the message
 * transfer fails and the connection is a socket connection, the socket is closed.
 * Takes and release dlt_daemon_mutex.
 *
 * @param daemon Daemon structure needed for socket closure.
 * @param daemon_local Daemon local structure
 * @param data1 The first message to be sent.
 * @param size1 The size of the first message.
 * @param data2 The second message to be send.
 * @param size2 The second message size.
 * @param verbose Needed for socket closure.
 *
 * @return The amount of data transfered.
 */
static int dlt_daemon_client_send_all_multiple(DltDaemon *daemon,
                                               DltDaemonLocal *daemon_local,
                                               void* data1,
                                               int size1,
                                               void* data2,
                                               int size2,
                                               int verbose)
{
    int j, sent = 0;
    DltConnection* temp = NULL;
    int type_mask =
        (DLT_CON_MASK_CLIENT_MSG_TCP | DLT_CON_MASK_CLIENT_MSG_SERIAL);
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    if ((daemon == NULL) || (daemon_local == NULL))
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Invalid parameters\n",
                 __func__);
        dlt_log(LOG_ERR, local_str);
        return 0;
    }

    temp = daemon_local->pEvent.connections;
    temp = dlt_connection_get_next(temp, type_mask);

    /* FIXME: the lock shall include the for loop as data
     * can be affect between each iteration, but
     * dlt_daemon_close_socket may call us too ...
     */
    for (j = 0; ((j < daemon_local->client_connections) && (temp != NULL)); j++)
    {
        int ret = 0;
        DLT_DAEMON_SEM_LOCK();
        ret = dlt_connection_send_multiple(temp,
                                          data1,
                                          size1,
                                          data2,
                                          size2,
                                          daemon->sendserialheader);
        DLT_DAEMON_SEM_FREE();

        if((ret != DLT_DAEMON_ERROR_OK) &&
           DLT_CONNECTION_CLIENT_MSG_TCP == temp->type)
        {
            dlt_daemon_close_socket(temp->fd,
                                    daemon,
                                    daemon_local,
                                    verbose);
        }

        if (ret != DLT_DAEMON_ERROR_OK)
        {
            snprintf(local_str,
                     DLT_DAEMON_TEXTBUFSIZE,
                     "%s: send dlt message failed\n",
                     __func__);
            dlt_log(LOG_WARNING, local_str);
        }
        else
        {
            /* If sent to at  least one client,
             * then do not store in ring buffer
             */
            sent = 1;
        }

        temp = dlt_connection_get_next(temp->next, type_mask);
    } /* for */

    return sent;
}

/** @brief Send out message to all the clients.
 *
 * @param daemon pointer to dlt daemon structure
 * @param daemon_local pointer to dlt daemon local structure
 * @param verbose if set to true verbose information is printed out.
 *
 * @return 1 if transfer succeed, 0 otherwise.
 */
int dlt_daemon_client_send_all(DltDaemon *daemon,
                               DltDaemonLocal *daemon_local,
                               int verbose)
{
    void *msg1, *msg2;
    int msg1_sz, msg2_sz;
    int ret = 0;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    if ((daemon == NULL) || (daemon_local == NULL))
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Invalid parameters\n",
                 __func__);
        dlt_log(LOG_ERR, local_str);
        return 0;
    }

    /* FIXME: the lock shall include the for loop but
     * dlt_daemon_close_socket may call us too ...
     */
    DLT_DAEMON_SEM_LOCK();
    msg1 = daemon_local->msg.headerbuffer + sizeof(DltStorageHeader);
    msg1_sz = daemon_local->msg.headersize - sizeof(DltStorageHeader);
    msg2 = daemon_local->msg.databuffer;
    msg2_sz = daemon_local->msg.datasize;
    DLT_DAEMON_SEM_FREE();

    ret = dlt_daemon_client_send_all_multiple(daemon,
                                               daemon_local,
                                               msg1,
                                               msg1_sz,
                                               msg2,
                                               msg2_sz,
                                               verbose);

    return ret;
}

int dlt_daemon_client_send(int sock,DltDaemon *daemon,DltDaemonLocal *daemon_local,void* storage_header,int storage_header_size,void* data1,int size1,void* data2,int size2,int verbose)
{
	int sent,ret;

    if (sock!=DLT_DAEMON_SEND_TO_ALL && sock!=DLT_DAEMON_SEND_FORCE)
    {
        /* Send message to specific socket */
        if (isatty(sock))
        {
            DLT_DAEMON_SEM_LOCK();

            if((ret=dlt_daemon_serial_send(sock,data1,size1,data2,size2,daemon->sendserialheader)))
            {
                DLT_DAEMON_SEM_FREE();
                dlt_log(LOG_WARNING,"dlt_daemon_client_send: serial send dlt message failed\n");
                return ret;
           }

            DLT_DAEMON_SEM_FREE();
        }
        else
        {
            DLT_DAEMON_SEM_LOCK();

            if((ret=dlt_daemon_socket_send(sock,data1,size1,data2,size2,daemon->sendserialheader)))
            {
                DLT_DAEMON_SEM_FREE();
                dlt_log(LOG_WARNING,"dlt_daemon_client_send: socket send dlt message failed\n");
                return ret;
            }

            DLT_DAEMON_SEM_FREE();
        }
        return DLT_DAEMON_ERROR_OK;
    }

	/* write message to offline trace */
	// In the SEND_BUFFER state we must skip offline tracing because the offline traces
	// are going without buffering directly to the offline trace. Thus we have to filter out
	// the traces that are coming from the buffer.
	if ((sock!=DLT_DAEMON_SEND_FORCE) && (daemon->state != DLT_DAEMON_STATE_SEND_BUFFER))
	{
		if(((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH))
							&& daemon_local->flags.offlineTraceDirectory[0])
		{
			if(dlt_offline_trace_write(&(daemon_local->offlineTrace),storage_header,storage_header_size,data1,size1,data2,size2))
			{
				static int error_dlt_offline_trace_write_failed = 0;
                if(!error_dlt_offline_trace_write_failed)
                {
                	dlt_log(LOG_ERR,"dlt_daemon_client_send: dlt_offline_trace_write failed!\n");
                	error_dlt_offline_trace_write_failed = 1;
                }
				//return DLT_DAEMON_ERROR_WRITE_FAILED;
			}
		}
        /* write messages to offline logstorage only if there is an extended header set
         * this need to be checked because the function is dlt_daemon_client_send is called by
         * newly introduced dlt_daemon_log_internal */
        if(daemon_local->flags.offlineLogstorageMaxDevices > 0)
        {
            dlt_daemon_logstorage_write(daemon, daemon_local->flags.offlineLogstorageMaxDevices, storage_header, storage_header_size, data1, size1, data2, size2);
        }
	}

	/* send messages to daemon socket */
	if((daemon->mode == DLT_USER_MODE_EXTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH))
	{
		if ((sock==DLT_DAEMON_SEND_FORCE) || (daemon->state == DLT_DAEMON_STATE_SEND_DIRECT))
		{
            sent = dlt_daemon_client_send_all_multiple(daemon,
                                                       daemon_local,
                                                       data1,
                                                       size1,
                                                       data2,
                                                       size2,
                                                       verbose);

			if((sock==DLT_DAEMON_SEND_FORCE) && !sent)
			{
				return DLT_DAEMON_ERROR_SEND_FAILED;
			}
		}
    }

    /* Message was not sent to client, so store it in client ringbuffer */
    if ((sock!=DLT_DAEMON_SEND_FORCE) && (daemon->state == DLT_DAEMON_STATE_BUFFER || daemon->state == DLT_DAEMON_STATE_SEND_BUFFER || daemon->state == DLT_DAEMON_STATE_BUFFER_FULL))
    {
    	if(daemon->state == DLT_DAEMON_STATE_BUFFER_FULL)
    		return DLT_DAEMON_ERROR_BUFFER_FULL;

        DLT_DAEMON_SEM_LOCK();
        /* Store message in history buffer */
        if (dlt_buffer_push3(&(daemon->client_ringbuffer),data1,size1,data2,size2,0, 0) < DLT_RETURN_OK)
		{
        	DLT_DAEMON_SEM_FREE();
			dlt_log(LOG_DEBUG,"dlt_daemon_client_send: Buffer is full! Message discarded.\n");
			dlt_daemon_change_state(daemon,DLT_DAEMON_STATE_BUFFER_FULL);
			return DLT_DAEMON_ERROR_BUFFER_FULL;
		}
        DLT_DAEMON_SEM_FREE();
    }

    return DLT_DAEMON_ERROR_OK;

}

int dlt_daemon_client_send_control_message( int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, char* appid, char* ctid, int verbose)
{
	int ret;
    int32_t len;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0) || (appid==0) || (ctid==0))
    {
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* prepare storage header */
    msg->storageheader = (DltStorageHeader*)msg->headerbuffer;

    if (dlt_set_storageheader(msg->storageheader,daemon->ecuid) == DLT_RETURN_ERROR)
    {
		return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* prepare standard header */
    msg->standardheader = (DltStandardHeader*)(msg->headerbuffer + sizeof(DltStorageHeader));
    msg->standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg->standardheader->htyp = ( msg->standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg->standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg->headerextra.ecu,daemon->ecuid);

    //msg->headerextra.seid = 0;

    msg->headerextra.tmsp = dlt_uptime();

    dlt_message_set_extraparameters(msg, verbose);

    /* prepare extended header */
    msg->extendedheader = (DltExtendedHeader*)(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp));
    msg->extendedheader->msin = DLT_MSIN_CONTROL_RESPONSE;

    msg->extendedheader->noar = 1; /* number of arguments */
    if (strcmp(appid,"")==0)
    {
        dlt_set_id(msg->extendedheader->apid,DLT_DAEMON_CTRL_APID);       /* application id */
    }
    else
    {
        dlt_set_id(msg->extendedheader->apid, appid);
    }
    if (strcmp(ctid,"")==0)
    {
        dlt_set_id(msg->extendedheader->ctid,DLT_DAEMON_CTRL_CTID);       /* context id */
    }
    else
    {
        dlt_set_id(msg->extendedheader->ctid, ctid);
    }

    /* prepare length information */
    msg->headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp);

    len=msg->headersize - sizeof(DltStorageHeader) + msg->datasize;
    if (len>UINT16_MAX)
    {
        dlt_log(LOG_WARNING,"Huge control message discarded!\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    msg->standardheader->len = DLT_HTOBE_16(((uint16_t)len));

	if((ret=dlt_daemon_client_send(sock,daemon,daemon_local,msg->headerbuffer,sizeof(DltStorageHeader),msg->headerbuffer+sizeof(DltStorageHeader),msg->headersize-sizeof(DltStorageHeader),
						msg->databuffer,msg->datasize,verbose)))
	{
		dlt_log(LOG_DEBUG,"dlt_daemon_control_send_control_message: DLT message send to all failed!.\n");
		return ret;
	}

    return DLT_DAEMON_ERROR_OK;
}

int dlt_daemon_client_process_control(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    uint32_t id,id_tmp=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0))
    {
        return -1;
    }

    if (msg->datasize < (int32_t)sizeof(uint32_t))
    {
        return -1;
    }

    id_tmp = *((uint32_t*)(msg->databuffer));
    id=DLT_ENDIAN_GET_32(msg->standardheader->htyp ,id_tmp);

    if ((id > 0) && ((id < DLT_SERVICE_ID_LAST_ENTRY) || ((id == DLT_SERVICE_ID_OFFLINE_LOGSTORAGE))))
    {
        /* Control message handling */
        switch (id)
        {
        case DLT_SERVICE_ID_SET_LOG_LEVEL:
        {
            dlt_daemon_control_set_log_level(sock, daemon, daemon_local, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_TRACE_STATUS:
        {
            dlt_daemon_control_set_trace_status(sock, daemon, daemon_local, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_LOG_INFO:
        {
            dlt_daemon_control_get_log_info(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL:
        {
            dlt_daemon_control_get_default_log_level(sock, daemon, daemon_local, verbose);
            break;
        }
        case DLT_SERVICE_ID_STORE_CONFIG:
        {
            if (dlt_daemon_applications_save(daemon, daemon->runtime_application_cfg, verbose)==0)
            {
				if (dlt_daemon_contexts_save(daemon, daemon->runtime_context_cfg, verbose)==0)
				{
					dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
				}
				else
				{
					/* Delete saved files */
					dlt_daemon_control_reset_to_factory_default(daemon, daemon->runtime_application_cfg, daemon->runtime_context_cfg, verbose);
					dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
				}
            }
            else
            {
            	dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
            }
            break;
        }
        case DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT:
        {
            dlt_daemon_control_reset_to_factory_default(daemon, daemon->runtime_application_cfg, daemon->runtime_context_cfg, verbose);
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_VERBOSE_MODE:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_MESSAGE_FILTERING:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_TIMING_PACKETS:
        {
            dlt_daemon_control_set_timing_packets(sock, daemon, daemon_local, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_LOCAL_TIME:
        {
            /* Send response with valid timestamp (TMSP) field */
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_ECU_ID:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_SESSION_ID:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_TIMESTAMP:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_EXTENDED_HEADER:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL:
        {
            dlt_daemon_control_set_default_log_level(sock, daemon, daemon_local, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS:
        {
            dlt_daemon_control_set_default_trace_status(sock, daemon, daemon_local, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_SOFTWARE_VERSION:
        {
            dlt_daemon_control_get_software_version(sock, daemon, daemon_local,  verbose);
            break;
        }
        case DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW:
        {
            dlt_daemon_control_message_buffer_overflow(sock, daemon, daemon_local, daemon->overflow_counter,"",verbose);
            break;
        }
        case DLT_SERVICE_ID_OFFLINE_LOGSTORAGE:
        {
            dlt_daemon_control_service_logstorage(sock, daemon, daemon_local, msg, verbose);
            break;
        }
        default:
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        }
    }
    else
    {
        /* Injection handling */
        dlt_daemon_control_callsw_cinjection(sock, daemon, daemon_local, msg,  verbose);
    }

    return 0;
}

void dlt_daemon_control_get_software_version(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    uint32_t len;
	DltServiceGetSoftwareVersionResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_SOFTWARE_VERSION, DLT_SERVICE_RESPONSE_ERROR,  verbose);
		return;
    }

    /* prepare payload of data */
    len = strlen(daemon->ECUVersionString);

    msg.datasize = sizeof(DltServiceGetSoftwareVersionResponse) + len;
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_SOFTWARE_VERSION, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    resp = (DltServiceGetSoftwareVersionResponse*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->length = len;
    memcpy(msg.databuffer+sizeof(DltServiceGetSoftwareVersionResponse),daemon->ECUVersionString,len);

    /* send message */
    dlt_daemon_client_send_control_message(sock, daemon,daemon_local, &msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

void dlt_daemon_control_get_default_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
	DltServiceGetDefaultLogLevelResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    msg.datasize = sizeof(DltServiceGetDefaultLogLevelResponse);
    if (msg.databuffer && (msg.databuffersize<msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    resp = (DltServiceGetDefaultLogLevelResponse*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->log_level = daemon->default_log_level;

    /* send message */
    dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

void dlt_daemon_control_get_log_info(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    DltServiceGetLogInfoRequest *req;
    DltMessage resp;
    DltDaemonContext *context=0;
    DltDaemonApplication *application=0;

    int num_applications=0, num_contexts=0;
    uint16_t count_app_ids=0, count_con_ids=0;

#if (DLT_DEBUG_GETLOGINFO==1)
    char buf[255];
#endif

    int32_t i,j,offset=0;
    char *apid=0;
    int8_t ll,ts;
    uint16_t len;
    int8_t value;
    int32_t sizecont=0;
    int offset_base;

    uint32_t sid;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    /* prepare pointer to message request */
    req = (DltServiceGetLogInfoRequest*) (msg->databuffer);

    /* initialise new message */
    if (dlt_message_init(&resp,0) == DLT_RETURN_ERROR)
    {
		dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_LOG_INFO, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    /* check request */
    if ((req->options < 3 ) || (req->options>7))
    {
        dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_LOG_INFO, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    if (req->apid[0]!='\0')
    {
        application = dlt_daemon_application_find(daemon, req->apid, verbose);
        if (application)
        {
            num_applications = 1;
            if (req->ctid[0]!='\0')
            {
                context = dlt_daemon_context_find(daemon, req->apid, req->ctid, verbose);

                num_contexts = ((context)?1:0);
            }
            else
            {
                num_contexts = application->num_contexts;
            }
        }
        else
        {
            num_applications = 0;
            num_contexts = 0;
        }
    }
    else
    {
        /* Request all applications and contexts */
        num_applications = daemon->num_applications;
        num_contexts = daemon->num_contexts;
    }

    /* prepare payload of data */

    /* Calculate maximum size for a response */
    resp.datasize = sizeof(uint32_t) /* SID */ + sizeof(int8_t) /* status*/ + sizeof(ID4) /* DLT_DAEMON_REMO_STRING */;

    sizecont = sizeof(uint32_t) /* context_id */;

    /* Add additional size for response of Mode 4, 6, 7 */
    if ((req->options==4) || (req->options==6) || (req->options==7))
    {
        sizecont += sizeof(int8_t); /* log level */
    }

    /* Add additional size for response of Mode 5, 6, 7 */
    if ((req->options==5) || (req->options==6) || (req->options==7))
    {
        sizecont+= sizeof(int8_t); /* trace status */
    }

    resp.datasize+= (num_applications * (sizeof(uint32_t) /* app_id */  + sizeof(uint16_t) /* count_con_ids */)) +
                    (num_contexts * sizecont);

    resp.datasize+= sizeof(uint16_t) /* count_app_ids */;

    /* Add additional size for response of Mode 7 */
    if (req->options==7)
    {
        if (req->apid[0]!='\0')
        {
            if (req->ctid[0]!='\0')
            {
                /* One application, one context */
                // context = dlt_daemon_context_find(daemon, req->apid, req->ctid, verbose);
                if (context)
                {
                    resp.datasize+=sizeof(uint16_t) /* len_context_description */;
                    if (context->context_description!=0)
                    {
                        resp.datasize+=strlen(context->context_description); /* context_description */
                    }
                }
            }
            else
            {
                /* One application, all contexts */
                if ((daemon->applications) && (application))
                {
                    /* Calculate start offset within contexts[] */
                    offset_base=0;
                    for (i=0; i<(application-(daemon->applications)); i++)
                    {
                        offset_base+=daemon->applications[i].num_contexts;
                    }

                    /* Iterate over all contexts belonging to this application */
                    for (j=0;j<application->num_contexts;j++)
                    {

                        context = &(daemon->contexts[offset_base+j]);
                        if (context)
                        {
                            resp.datasize+=sizeof(uint16_t) /* len_context_description */;
                            if (context->context_description!=0)
                            {
                                resp.datasize+=strlen(context->context_description); /* context_description */
                            }
                        }
                    }
                }
            }

            /* Space for application description */
            if (application)
            {
                resp.datasize+=sizeof(uint16_t) /* len_app_description */;
                if (application->application_description!=0)
                {
                    resp.datasize+=strlen(application->application_description); /* app_description */
                }
            }
        }
        else
        {
            /* All applications, all contexts */
            for (i=0;i<daemon->num_contexts;i++)
            {
                resp.datasize+=sizeof(uint16_t) /* len_context_description */;
                if (daemon->contexts[i].context_description!=0)
                {
                    resp.datasize+=strlen(daemon->contexts[i].context_description); /* context_description */
                }
            }

            for (i=0;i<daemon->num_applications;i++)
            {
                resp.datasize+=sizeof(uint16_t) /* len_app_description */;
                if (daemon->applications[i].application_description!=0)
                {
                    resp.datasize+=strlen(daemon->applications[i].application_description); /* app_description */
                }
            }
        }
    }

    if (verbose)
    {
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Allocate %d bytes for response msg databuffer\n", resp.datasize);
        dlt_log(LOG_DEBUG, str);
    }

    /* Allocate buffer for response message */
    resp.databuffer = (uint8_t *) malloc(resp.datasize);
    resp.databuffersize = resp.datasize;

    if (resp.databuffer==0)
    {
        dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_GET_LOG_INFO, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }
    memset(resp.databuffer,0,resp.datasize);
    /* Preparation finished */

    /* Prepare response */
    sid = DLT_SERVICE_ID_GET_LOG_INFO;
    memcpy(resp.databuffer,&sid,sizeof(uint32_t));
    offset+=sizeof(uint32_t);

    value = (((num_applications!=0)&&(num_contexts!=0))?req->options:8); /* 8 = no matching context found */

    memcpy(resp.databuffer+offset,&value,sizeof(int8_t));
    offset+=sizeof(int8_t);

    count_app_ids = num_applications;

    if (count_app_ids!=0)
    {
        memcpy(resp.databuffer+offset,&count_app_ids,sizeof(uint16_t));
        offset+=sizeof(uint16_t);

#if (DLT_DEBUG_GETLOGINFO==1)
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"#apid: %d \n", count_app_ids);
        dlt_log(LOG_DEBUG, str);
#endif

        for (i=0;i<count_app_ids;i++)
        {
            if (req->apid[0]!='\0')
            {
                apid = req->apid;
            }
            else
            {
                if (daemon->applications)
                {
                    apid = daemon->applications[i].apid;
                }
                else
                {
                    /* This should never occur! */
                    apid=0;
                }
            }

            application = dlt_daemon_application_find(daemon, apid, verbose);

            if (application)
            {
                /* Calculate start offset within contexts[] */
                offset_base=0;
                for (j=0; j<(application-(daemon->applications)); j++)
                {
                    offset_base+=daemon->applications[j].num_contexts;
                }

                dlt_set_id((char*)(resp.databuffer+offset),apid);
                offset+=sizeof(ID4);

#if (DLT_DEBUG_GETLOGINFO==1)
                dlt_print_id(buf, apid);
                snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"apid: %s\n",buf);
                dlt_log(LOG_DEBUG, str);
#endif

                if (req->apid[0]!='\0')
                {
                    count_con_ids = num_contexts;
                }
                else
                {
                    count_con_ids = application->num_contexts;
                }

                memcpy(resp.databuffer+offset,&count_con_ids,sizeof(uint16_t));
                offset+=sizeof(uint16_t);

#if (DLT_DEBUG_GETLOGINFO==1)
                snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"#ctid: %d \n", count_con_ids);
                dlt_log(LOG_DEBUG, str);
#endif

                for (j=0;j<count_con_ids;j++)
                {
#if (DLT_DEBUG_GETLOGINFO==1)
                    snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"j: %d \n",j);
                    dlt_log(LOG_DEBUG, str);
#endif
                    if (!((count_con_ids==1) && (req->apid[0]!='\0') && (req->ctid[0]!='\0')))
                    {
                        context = &(daemon->contexts[offset_base+j]);
                    }
                    /* else: context was already searched and found
                             (one application (found) with one context (found))*/

                    if ((context) &&
                            ((req->ctid[0]=='\0') ||
                             ((req->ctid[0]!='\0') && (memcmp(context->ctid,req->ctid,DLT_ID_SIZE)==0)))
                       )
                    {
                        dlt_set_id((char*)(resp.databuffer+offset),context->ctid);
                        offset+=sizeof(ID4);

#if (DLT_DEBUG_GETLOGINFO==1)
                        dlt_print_id(buf, context->ctid);
                        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"ctid: %s \n",buf);
                        dlt_log(LOG_DEBUG, str);
#endif

                        /* Mode 4, 6, 7 */
                        if ((req->options==4) || (req->options==6) || (req->options==7))
                        {
                            ll=context->log_level;
                            memcpy(resp.databuffer+offset,&ll,sizeof(int8_t));
                            offset+=sizeof(int8_t);
                        }

                        /* Mode 5, 6, 7 */
                        if ((req->options==5) || (req->options==6) || (req->options==7))
                        {
                            ts=context->trace_status;
                            memcpy(resp.databuffer+offset,&ts,sizeof(int8_t));
                            offset+=sizeof(int8_t);
                        }

                        /* Mode 7 */
                        if (req->options==7)
                        {
                            if (context->context_description)
                            {
                                len = strlen(context->context_description);
                                memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                                offset+=sizeof(uint16_t);
                                memcpy(resp.databuffer+offset,context->context_description,strlen(context->context_description));
                                offset+=strlen(context->context_description);
                            }
                            else
                            {
                                len = 0;
                                memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                                offset+=sizeof(uint16_t);
                            }
                        }

#if (DLT_DEBUG_GETLOGINFO==1)
                        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"ll=%d ts=%d \n",(int32_t)ll,(int32_t)ts);
                        dlt_log(LOG_DEBUG, str);
#endif
                    }

#if (DLT_DEBUG_GETLOGINFO==1)
                    dlt_log(LOG_DEBUG,"\n");
#endif
                }

                /* Mode 7 */
                if (req->options==7)
                {
                    if (application->application_description)
                    {
                        len = strlen(application->application_description);
                        memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                        offset+=sizeof(uint16_t);
                        memcpy(resp.databuffer+offset,application->application_description,strlen(application->application_description));
                        offset+=strlen(application->application_description);
                    }
                    else
                    {
                        len = 0;
                        memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                        offset+=sizeof(uint16_t);
                    }
                }
            } /* if (application) */
        } /* for (i=0;i<count_app_ids;i++) */
    } /* if (count_app_ids!=0) */

    dlt_set_id((char*)(resp.databuffer+offset),DLT_DAEMON_REMO_STRING);

    /* send message */
    dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&resp,"","",  verbose);

    /* free message */
    dlt_message_free(&resp,0);
}

int dlt_daemon_control_message_buffer_overflow(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, unsigned int overflow_counter,char* apid, int verbose)
{
	int ret;
    DltMessage msg;
	DltServiceMessageBufferOverflowResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	dlt_daemon_control_service_response(sock, daemon,daemon_local, DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    	return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceMessageBufferOverflowResponse);
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    resp = (DltServiceMessageBufferOverflowResponse*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->overflow = DLT_MESSAGE_BUFFER_OVERFLOW;
   	resp->overflow_counter = overflow_counter;

    /* send message */
    if((ret=dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,apid,"",  verbose)))
    {
        dlt_message_free(&msg,0);
    	return ret;
    }

    /* free message */
    dlt_message_free(&msg,0);

    return DLT_DAEMON_ERROR_OK;
}

void dlt_daemon_control_service_response( int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, uint32_t service_id, int8_t status , int verbose)
{
    DltMessage msg;
    DltServiceResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
		return;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceResponse);
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        return;
    }

    resp = (DltServiceResponse*) msg.databuffer;
    resp->service_id = service_id;
    resp->status = status;

    /* send message */
    dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

int dlt_daemon_control_message_unregister_context(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, char* apid, char* ctid, char* comid, int verbose)
{
    DltMessage msg;
    DltServiceUnregisterContext *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	return -1;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceUnregisterContext);
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        return -1;
    }

    resp = (DltServiceUnregisterContext*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_UNREGISTER_CONTEXT;
    resp->status = DLT_SERVICE_RESPONSE_OK;
   	dlt_set_id(resp->apid, apid);
   	dlt_set_id(resp->ctid, ctid);
   	dlt_set_id(resp->comid, comid);

    /* send message */
    if(dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,"","",  verbose))
    {
        dlt_message_free(&msg,0);
    	return -1;
    }

    /* free message */
    dlt_message_free(&msg,0);

    return 0;
}

int dlt_daemon_control_message_connection_info(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, uint8_t state, char* comid, int verbose)
{
    DltMessage msg;
    DltServiceConnectionInfo *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	return -1;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceConnectionInfo);
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        return -1;
    }

    resp = (DltServiceConnectionInfo*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_CONNECTION_INFO;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->state = state;
    dlt_set_id(resp->comid, comid);

    /* send message */
    if(dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,"","",  verbose))
    {
        dlt_message_free(&msg,0);
    	return -1;
    }

    /* free message */
    dlt_message_free(&msg,0);

    return 0;
}

int dlt_daemon_control_message_timezone(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    DltServiceTimezone *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	return -1;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceTimezone);
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        return -1;
    }

    resp = (DltServiceTimezone*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_TIMEZONE;
    resp->status = DLT_SERVICE_RESPONSE_OK;

	time_t t = time(NULL);
	struct tm lt;
	localtime_r(&t, &lt);
#if !defined(__CYGWIN__)
    resp->timezone = (int32_t) lt.tm_gmtoff;
#endif
    resp->isdst = (uint8_t) lt.tm_isdst;

    /* send message */
    if(dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,"","",  verbose))
    {
        dlt_message_free(&msg,0);
    	return -1;
    }

    /* free message */
    dlt_message_free(&msg,0);

    return 0;
}

int dlt_daemon_control_message_marker(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltMessage msg;
    DltServiceMarker *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
    	return -1;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceMarker);
    if (msg.databuffer && (msg.databuffersize < msg.datasize))
    {
        free(msg.databuffer);
        msg.databuffer=0;
    }
    if (msg.databuffer == 0){
    	msg.databuffer = (uint8_t *) malloc(msg.datasize);
    	msg.databuffersize = msg.datasize;
    }
    if (msg.databuffer==0)
    {
        return -1;
    }

    resp = (DltServiceMarker*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_MARKER;
    resp->status = DLT_SERVICE_RESPONSE_OK;

    /* send message */
    if(dlt_daemon_client_send_control_message(sock,daemon,daemon_local,&msg,"","",  verbose))
    {
        dlt_message_free(&msg,0);
    	return -1;
    }

    /* free message */
    dlt_message_free(&msg,0);

    return 0;
}

void dlt_daemon_control_callsw_cinjection(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];
    uint32_t id=0,id_tmp=0;
    uint8_t *ptr;
    DltDaemonContext *context;
	int32_t data_length_inject=0;
	uint32_t data_length_inject_tmp=0;

	int32_t datalength;

	DltUserHeader userheader;
	DltUserControlMsgInjection usercontext;
	uint8_t *userbuffer;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    datalength = msg->datasize;
    ptr = msg->databuffer;

    if (ptr==0)
    {
        return;
    }

    DLT_MSG_READ_VALUE(id_tmp,ptr,datalength,uint32_t); /* Get service id */
    id=DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

    if ((id>=DLT_DAEMON_INJECTION_MIN) && (id<=DLT_DAEMON_INJECTION_MAX))
    {
        /* This a a real SW-C injection call */
        data_length_inject=0;
        data_length_inject_tmp=0;

        DLT_MSG_READ_VALUE(data_length_inject_tmp,ptr,datalength,uint32_t); /* Get data length */
        data_length_inject=DLT_ENDIAN_GET_32(msg->standardheader->htyp, data_length_inject_tmp);

        /* Get context handle for apid, ctid (and seid) */
        /* Warning: seid is ignored in this implementation! */
        if (DLT_IS_HTYP_UEH(msg->standardheader->htyp))
        {
            dlt_set_id(apid, msg->extendedheader->apid);
            dlt_set_id(ctid, msg->extendedheader->ctid);
        }
        else
        {
            /* No extended header, and therefore no apid and ctid available */
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
            return;
        }

        /* At this point, apid and ctid is available */
        context=dlt_daemon_context_find(daemon, apid, ctid, verbose);

        if (context==0)
        {
            // dlt_log(LOG_INFO,"No context found!\n");
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
            return;
        }

        /* Send user message to handle, specified in context */
		if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_INJECTION) < DLT_RETURN_OK)
		{
			dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
			return;
		}

		usercontext.log_level_pos = context->log_level_pos;

		if(data_length_inject > msg->databuffersize)
		{
			dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
			return;
		}

		userbuffer = malloc(data_length_inject);

		if (userbuffer==0)
		{
			dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
			return;
		}

		usercontext.data_length_inject = data_length_inject;
		usercontext.service_id = id;

		memcpy(userbuffer,ptr,data_length_inject);  /* Copy received injection to send buffer */

		/* write to FIFO */
		DltReturnValue ret =
				dlt_user_log_out3(context->user_handle, &(userheader), sizeof(DltUserHeader),
				  &(usercontext), sizeof(DltUserControlMsgInjection),
				  userbuffer, data_length_inject);
		if (ret < DLT_RETURN_OK)
		{
			if (ret == DLT_RETURN_PIPE_ERROR)
			{
				/* Close connection */
				close(context->user_handle);
				context->user_handle=DLT_FD_INIT;
			}
			dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
		}
		else
		{
			dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
		}

		free(userbuffer);
		userbuffer=0;

    }
    else
    {
        /* Invalid ID */
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
    }
}

void dlt_daemon_control_set_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];
    DltServiceSetLogLevel *req;
    DltDaemonContext *context;
    int32_t id=DLT_SERVICE_ID_SET_LOG_LEVEL;

	int8_t old_log_level;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetLogLevel*) (msg->databuffer);

    dlt_set_id(apid, req->apid);
    dlt_set_id(ctid, req->ctid);

    context=dlt_daemon_context_find(daemon, apid, ctid, verbose);

    /* Set log level */
    if (context!=0)
    {
        old_log_level = context->log_level;
        context->log_level = req->log_level; /* No endianess conversion necessary*/

        if ((context->user_handle >= DLT_FD_MINIMUM) &&
                (dlt_daemon_user_send_log_level(daemon, context, verbose)==0))
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
        }
        else
        {
            //dlt_log(LOG_ERR, "Log level could not be sent!\n");
            context->log_level = old_log_level;
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        }
    }
    else
    {
        //dlt_log(LOG_ERR, "Context not found!\n");
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_trace_status(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];
    DltServiceSetLogLevel *req;             /* request uses same struct as set log level */
    DltDaemonContext *context;
    int32_t id=DLT_SERVICE_ID_SET_TRACE_STATUS;

	int8_t old_trace_status;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetLogLevel*) (msg->databuffer);

    dlt_set_id(apid, req->apid);
    dlt_set_id(ctid, req->ctid);

    context=dlt_daemon_context_find(daemon, apid, ctid, verbose);

    /* Set log level */
    if (context!=0)
    {
        old_trace_status = context->trace_status;
        context->trace_status = req->log_level;   /* No endianess conversion necessary */

        if ((context->user_handle >= DLT_FD_MINIMUM ) &&
                (dlt_daemon_user_send_log_level(daemon, context, verbose)==0))
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
        }
        else
        {
            //dlt_log(LOG_ERR, "Trace Status could not be sent!\n");
            context->trace_status = old_trace_status;
            dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        }
    }
    else
    {
        //dlt_log(LOG_ERR, "Context not found!\n");
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_default_log_level(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetDefaultLogLevel *req;
    int32_t id=DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetDefaultLogLevel*) (msg->databuffer);

    /* No endianess conversion necessary */
    if (/*(req->log_level>=0) &&*/
            (req->log_level<=DLT_LOG_VERBOSE))
    {
        daemon->default_log_level = req->log_level; /* No endianess conversion necessary */

        /* Send Update to all contexts using the default log level */
        dlt_daemon_user_send_default_update(daemon, verbose);

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_default_trace_status(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    /* Payload of request message */
    DltServiceSetDefaultLogLevel *req;
    int32_t id=DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetDefaultLogLevel*) (msg->databuffer);

    /* No endianess conversion necessary */
    if ((req->log_level==DLT_TRACE_STATUS_OFF) ||
            (req->log_level==DLT_TRACE_STATUS_ON))
    {
        daemon->default_trace_status = req->log_level; /* No endianess conversion necessary*/

        /* Send Update to all contexts using the default trace status */
        dlt_daemon_user_send_default_update(daemon, verbose);

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_timing_packets(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetVerboseMode *req;  /* request uses same struct as set verbose mode */
    int32_t id=DLT_SERVICE_ID_SET_TIMING_PACKETS;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetVerboseMode*) (msg->databuffer);
    if ((req->new_status==0) || (req->new_status==1))
    {
        daemon->timingpackets = req->new_status;

        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_OK,  verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_message_time(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	int ret;
    DltMessage msg;
    int32_t len;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0) == DLT_RETURN_ERROR)
    {
        return;
    }

    /* send message */

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader*)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader,daemon->ecuid);

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = ( msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu,daemon->ecuid);
    msg.headerextra.tmsp = dlt_uptime();

    dlt_message_set_extraparameters(&msg, verbose);

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_CONTROL_TIME;

    msg.extendedheader->noar = 0;                  /* number of arguments */
    dlt_set_id(msg.extendedheader->apid,"");       /* application id */
    dlt_set_id(msg.extendedheader->ctid,"");       /* context id */

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);

    len=msg.headersize - sizeof(DltStorageHeader) + msg.datasize;
    if (len>UINT16_MAX)
    {
        dlt_log(LOG_WARNING,"Huge control message discarded!\n");

        /* free message */
        dlt_message_free(&msg,0);

        return;
    }

    msg.standardheader->len = DLT_HTOBE_16(((uint16_t)len));

    /* Send message */
    if((ret = dlt_daemon_client_send(sock,daemon,daemon_local,msg.headerbuffer,sizeof(DltStorageHeader),msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader),
			   msg.databuffer,msg.datasize,verbose)))
    {

    }

    /* free message */
    dlt_message_free(&msg,0);
}

int dlt_daemon_process_one_s_timer(DltDaemon *daemon,
                                   DltDaemonLocal *daemon_local,
                                   int verbose)
{
    uint64_t expir = 0;
    ssize_t res = 0;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if((daemon_local == NULL) || (daemon == NULL))
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: invalid parameters",
                 __func__);
        dlt_log(LOG_ERR, local_str);
        return -1;
    }

    res = read(daemon_local->timer_one_s.fd, &expir, sizeof(expir));

    if(res < 0)
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Fail to read timer (%s)\n", __func__, strerror(errno));
        dlt_log(LOG_WARNING, str);
        /* Activity received on timer_wd, but unable to read the fd:
           let's go on sending notification */
    }

    if((daemon->state == DLT_DAEMON_STATE_SEND_BUFFER) ||
       (daemon->state == DLT_DAEMON_STATE_BUFFER_FULL))
    {
        if (dlt_daemon_send_ringbuffer_to_client(daemon,
                                                 daemon_local,
                                                 daemon_local->flags.vflag))
        {
            dlt_log(LOG_DEBUG,
                    "Can't send contents of ring buffer to clients\n");
        }
    }

    if((daemon->timingpackets) &&
       (daemon->state == DLT_DAEMON_STATE_SEND_DIRECT))
    {
        dlt_daemon_control_message_time(DLT_DAEMON_SEND_TO_ALL,
                                        daemon,
                                        daemon_local,
                                        daemon_local->flags.vflag);
    }

    dlt_log(LOG_DEBUG, "Timer timingpacket\n");

    return 0;
}

int dlt_daemon_process_sixty_s_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     int verbose)
{
    uint64_t expir = 0;
    ssize_t res = 0;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if((daemon_local == NULL) || (daemon == NULL))
    {
        snprintf(str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: invalid parameters",
                 __func__);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    res = read(daemon_local->timer_sixty_s.fd, &expir, sizeof(expir));

    if(res < 0)
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: Fail to read timer (%s)\n", __func__, strerror(errno));
        dlt_log(LOG_WARNING, str);
        /* Activity received on timer_wd, but unable to read the fd:
           let's go on sending notification */
    }

    if(daemon_local->flags.sendECUSoftwareVersion > 0)
    {
        dlt_daemon_control_get_software_version(DLT_DAEMON_SEND_TO_ALL,
                                                daemon,
                                                daemon_local,
                                                daemon_local->flags.vflag);
    }

    if(daemon_local->flags.sendTimezone > 0)
    {
        // send timezone information
        time_t t = time(NULL);
        struct tm lt;

        /*Added memset to avoid compiler warning for near initialization */
        memset((void*)&lt, 0, sizeof(lt));
        localtime_r(&t, &lt);

        dlt_daemon_control_message_timezone(DLT_DAEMON_SEND_TO_ALL,
                                            daemon,
                                            daemon_local,
                                            daemon_local->flags.vflag);
    }

    dlt_log(LOG_DEBUG, "Timer ecuversion\n");

    return 0;
}

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
int dlt_daemon_process_systemd_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     int verbose)
{
    uint64_t expir = 0;
    ssize_t res = -1;
    char local_str[DLT_DAEMON_TEXTBUFSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if((daemon_local == NULL) || (daemon == NULL))
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "%s: invalid parameters",
                 __func__);
        dlt_log(LOG_ERR, local_str);
        return res;
    }

    res = read(daemon_local->timer_wd.fd, &expir, sizeof(expir));

    if(res < 0)
    {
        snprintf(local_str,
                 DLT_DAEMON_TEXTBUFSIZE,
                 "Failed to read timer_wd; %s\n",
                 strerror(errno));
        dlt_log(LOG_WARNING, local_str);
        /* Activity received on timer_wd, but unable to read the fd:
           let's go on sending notification */
    }

    if(sd_notify(0, "WATCHDOG=1") < 0)
    {
        dlt_log(LOG_CRIT, "Could not reset systemd watchdog\n");
    }

    dlt_log(LOG_DEBUG, "Timer watchdog\n");

    return 0;
}
#else
int dlt_daemon_process_systemd_timer(DltDaemon *daemon,
                                     DltDaemonLocal *daemon_local,
                                     int verbose)
{
    (void)daemon;
    (void)daemon_local;
    (void)verbose;

    dlt_log(LOG_DEBUG, "Timer watchdog not enabled\n");

    return -1;
}
#endif

void dlt_daemon_control_service_logstorage(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, DltMessage *msg, int verbose)
{
    DltServiceOfflineLogstorage *req;
    int ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) ||(msg == 0) ||(daemon_local == 0))
    {
        dlt_log(LOG_ERR, "Invalid function parameters used for dlt_daemon_control_service_logstorage\n");
        return ;
    }

    if(daemon_local->flags.offlineLogstorageMaxDevices <= 0)
    {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE_ERROR, verbose);
        dlt_log(LOG_INFO, "Offline logstorage functionality not enabled or MAX device set is 0\n");
        return;
    }

    req = (DltServiceOfflineLogstorage*) (msg->databuffer);

     /* Check for device connection request from log storage ctrl app  */
    if (req->connection_type == DLT_OFFLINE_LOGSTORAGE_DEVICE_CONNECTED)
    {
        if(req->dev_num > daemon_local->flags.offlineLogstorageMaxDevices)
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE_ERROR, verbose);
            dlt_log(LOG_INFO, "Device number received is greater than MAX device configured \n");
            return;
        }
        /* Adding +1 to device number as ctrl app sends device number 1 less (to support indexing of storage_handle */
        ret = dlt_logstorage_device_connected(&(daemon->storage_handle[req->dev_num]), req->dev_num+1);
        if(ret != 0)
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        /* Setup logstorage with config file settings */
        ret = dlt_logstorage_load_config(&(daemon->storage_handle[req->dev_num]));
        if(ret != 0)
        {
            dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE_ERROR, verbose);
            return;
        }

        dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE,DLT_SERVICE_RESPONSE_OK, verbose);

        /* Check if log level of running application needs an update */
        dlt_daemon_logstorage_update_application_loglevel(daemon, req->dev_num, verbose);

    }
    /* Check for device disconnection request from log storage ctrl app  */
    else if(req->connection_type == DLT_OFFLINE_LOGSTORAGE_DEVICE_DISCONNECTED)
    {
        if(req->dev_num > daemon_local->flags.offlineLogstorageMaxDevices)
        {
            dlt_log(LOG_INFO, "Device number received is greater than MAX device configured \n");
            return;
        }

        /* Check if log level of running application needs to be reset */
        dlt_daemon_logstorage_reset_application_loglevel(daemon, req->dev_num, daemon_local->flags.offlineLogstorageMaxDevices, verbose);

        dlt_logstorage_device_disconnected(&(daemon->storage_handle[req->dev_num]));

        dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE_OK, verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, daemon_local, DLT_SERVICE_ID_OFFLINE_LOGSTORAGE, DLT_SERVICE_RESPONSE_ERROR, verbose);
    }
}
