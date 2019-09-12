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
 * \file dlt-control.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-control.c                                                 **
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

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */
#include <sys/ioctl.h>  /* for ioctl(), SIOCOUTQ */
#include <string.h>     /* for open() */

#include "dlt_client.h"
#include "dlt_user.h"
#include "dlt-control-common.h"



#define DLT_GLOGINFO_APID_NUM_MAX   150
#define DLT_GLOGINFO_DATA_MAX       800
#define DLT_GET_LOG_INFO_HEADER     18      /*Get log info header size in response text */
#define DLT_INVALID_LOG_LEVEL       0xF
#define DLT_INVALID_TRACE_STATUS    0xF
/* Option of GET_LOG_INFO */
#define DLT_SERVICE_GET_LOG_INFO_OPT7    7    /* get Apid, ApDescription, Ctid, CtDescription, loglevel, tracestatus */

typedef struct
{
    uint32_t service_id;            /**< service ID */
} PACKED DltServiceGetDefaultLogLevel;

DltClient g_dltclient;
DltServiceGetLogInfoResponse *g_resp = NULL;

/* Function prototypes */
int dlt_receive_message_callback(DltMessage *message, void *data);

typedef struct {
    int vflag;
    int yflag;
    char *evalue;

    char *avalue;
    char *cvalue;
    int svalue;
    char *mvalue;
    char *xvalue;
    int tvalue;
    int lvalue;
    int rvalue;
    int dvalue;
    int fvalue;
    int ivalue;
    int oflag;
    int gflag;
    int jvalue;
    int bvalue;
    char ecuid[4];
    DltFile file;
    DltFilter filter;
} DltReceiveData;


void wait_send_buffer_empty(int fd)
{
#ifdef __linux__
    struct timespec ts = {0};
    /* Wait until send buffer is empty */
    while (1)
    {
        int unsent;
        ioctl(fd, TIOCOUTQ, &unsent);
        if (!unsent)
        {
            break;
        }

        ts.tv_sec = 0;
        ts.tv_nsec = 100000 * 1000;
        nanosleep(&ts, NULL);
    }
#endif
}

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-control [options] hostname/serial_device_name\n");
    printf("Send control message to DLT daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("\n");
    printf("  -a id		    Control message application id\n");
    printf("  -c id    		Control message context id\n");
    printf("  -s id    		Control message injection service id\n");
    printf("  -m message    Control message injection in ASCII\n");
    printf("  -x message    Control message injection in Hex e.g. 'ad 01 24 ef'\n");
    printf("  -t milliseconds Timeout to terminate application (Default:1000)'\n");
    printf("  -l loglevel      Set the log level (0=off - 6=verbose default= -1)\n");
    printf("      supported options:\n");
    printf("       -l level -a apid -c ctid\n");
    printf("       -l level -a abc* (set level for all ctxts of apps name starts with abc)\n");
    printf("       -l level -a apid (set level for all ctxts of this app)\n");
    printf("       -l level -c xyz* (set level for all ctxts whose name starts with xyz)\n");
    printf("       -l level -c ctid (set level for the particular ctxt)\n");
    printf("       -l level (set level for all the registered contexts)\n");
    printf("  -r tracestatus   Set the trace status (0=off - 1=on,255=default)\n");
    printf("      supported options:\n");
    printf("       -r tracestatus -a apid -c ctid\n");
    printf("       -r tracestatus -a abc* (set status for all ctxts of apps name starts with abc)\n");
    printf("       -r tracestatus -a apid (set status for all ctxts of this app)\n");
    printf("       -r tracestatus -c xyz* (set status for all ctxts whose name starts with xyz)\n");
    printf("       -r tracestatus -c ctid (set status for the particular ctxt)\n");
    printf("       -r tracestatus (set status for all the registered contexts)\n");
    printf("  -d loglevel	  Set the default log level (0=off - 5=verbose)\n");
    printf("  -f tracestatus  Set the default trace status (0=off - 1=on)\n");
    printf("  -i enable  	  Enable timing packets (0=off - 1=on)\n");
    printf("  -o 		  	  Store configuration\n");
    printf("  -g 		  	  Reset to factory default\n");
    printf("  -j               Get log info\n");
    printf("  -u               unix port\n");
}
/**
 * Function for sending get log info ctrl msg and printing the response.
 */
void dlt_process_get_log_info(void)
{
    char apid[DLT_ID_SIZE + 1] = { 0 };
    char ctid[DLT_ID_SIZE + 1] = { 0 };
    AppIDsType app;
    ContextIDsInfoType con;
    int i = 0;
    int j = 0;

    g_resp = (DltServiceGetLogInfoResponse *)calloc(1, sizeof(DltServiceGetLogInfoResponse));

    if (g_resp == NULL) {
        fprintf(stderr, "%s: Calloc failed for resp..\n", __func__);
        return;
    }

    /* send control message*/
    if (0 != dlt_client_get_log_info(&g_dltclient)) {
        fprintf(stderr, "ERROR: Could not get log info\n");
        dlt_client_cleanup_get_log_info(g_resp);
        return;
    }

    if (dlt_client_main_loop(&g_dltclient, NULL, 0) == DLT_RETURN_TRUE)
        fprintf(stdout, "DLT-daemon's response is invalid.\n");

    for (i = 0; i < g_resp->log_info_type.count_app_ids; i++) {
        app = g_resp->log_info_type.app_ids[i];

        dlt_print_id(apid, app.app_id);

        if (app.app_description != 0)
            printf("APID:%4s %s\n", apid, app.app_description);
        else
            printf("APID:%4s \n", apid);

        for (j = 0; j < app.count_context_ids; j++) {
            con = app.context_id_info[j];

            dlt_print_id(ctid, con.context_id);

            if (con.context_description != 0)

                printf("CTID:%4s %2d %2d %s\n",
                       ctid,
                       con.log_level,
                       con.trace_status,
                       con.context_description);
            else
                printf("CTID:%4s %2d %2d\n",
                       ctid,
                       con.log_level,
                       con.trace_status);
        }
    }

    dlt_client_cleanup_get_log_info(g_resp);
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    DltReceiveData dltdata;
    int c;
    int index;
    char *endptr = NULL;
    struct timespec ts;

    /* Initialize dltdata */
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;

    dltdata.avalue = 0;
    dltdata.cvalue = 0;
    dltdata.svalue = 0;
    dltdata.mvalue = 0;
    dltdata.xvalue = 0;
    dltdata.tvalue = 1000;
    dltdata.lvalue = DLT_INVALID_LOG_LEVEL;
    dltdata.rvalue = DLT_INVALID_TRACE_STATUS;
    dltdata.dvalue = -1;
    dltdata.fvalue = -1;
    dltdata.ivalue = -1;
    dltdata.oflag = -1;
    dltdata.gflag = -1;
    dltdata.jvalue = 0;
    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vhye:b:a:c:s:m:x:t:l:r:d:f:i:ogju")) != -1)
        switch (c) {
        case 'v':
        {
            dltdata.vflag = 1;
            break;
        }
        case 'h':
        {
            usage();
            return -1;
        }
        case 'y':
        {
            dltdata.yflag = DLT_CLIENT_MODE_SERIAL;
            break;
        }
        case 'e':
        {
            dltdata.evalue = optarg;
            break;
        }
        case 'b':
        {
            dltdata.bvalue = atoi(optarg);
            break;
        }

        case 'a':
        {
            dltdata.avalue = optarg;

            if (strlen(dltdata.avalue) > DLT_ID_SIZE) {
                fprintf (stderr, "Invalid application id\n");
                return -1;
            }

            break;
        }
        case 'c':
        {
            dltdata.cvalue = optarg;

            if (strlen(dltdata.cvalue) > DLT_ID_SIZE) {
                fprintf (stderr, "Invalid context id\n");
                return -1;
            }

            break;
        }
        case 's':
        {
            dltdata.svalue = atoi(optarg);
            break;
        }
        case 'm':
        {
            dltdata.mvalue = optarg;
            break;
        }
        case 'x':
        {
            dltdata.xvalue = optarg;
            break;
        }
        case 't':
        {
            dltdata.tvalue = atoi(optarg);
            break;
        }
        case 'l':
        {
            dltdata.lvalue = strtol(optarg, &endptr, 10);

            if ((dltdata.lvalue < DLT_LOG_DEFAULT) || (dltdata.lvalue > DLT_LOG_VERBOSE)) {
                fprintf (stderr, "invalid log level, supported log level 0-6\n");
                return -1;
            }

            break;
        }
        case 'r':
        {
            dltdata.rvalue = strtol(optarg, &endptr, 10);

            if ((dltdata.rvalue < DLT_TRACE_STATUS_DEFAULT) || (dltdata.rvalue > DLT_TRACE_STATUS_ON)) {
                fprintf (stderr, "invalid trace status, supported trace status -1, 0, 1\n");
                return -1;
            }

            break;
        }
        case 'd':
        {
            dltdata.dvalue = atoi(optarg);
            break;
        }
        case 'f':
        {
            dltdata.fvalue = atoi(optarg);
            break;
        }
        case 'i':
        {
            dltdata.ivalue = atoi(optarg);
            break;
        }
        case 'o':
        {
            dltdata.oflag = 1;
            break;
        }
        case 'g':
        {
            dltdata.gflag = 1;
            break;
        }
        case 'j':
        {
            dltdata.jvalue = 1;
            break;
        }
        case 'u':
        {
            dltdata.yflag = DLT_CLIENT_MODE_UNIX;
            break;
        }
        case '?':
        {
            if ((optopt == 'o') || (optopt == 'f'))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            abort ();
            return -1;    /*for parasoft */
        }
        }

    /* Initialize DLT Client */
    dlt_client_init(&g_dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_receive_message_callback);

    /* Setup DLT Client structure */
    if (dltdata.yflag == DLT_CLIENT_MODE_SERIAL) {
        g_dltclient.mode = DLT_CLIENT_MODE_SERIAL;
    }
    else if (dltdata.yflag == DLT_CLIENT_MODE_UNIX)
    {
        g_dltclient.mode = DLT_CLIENT_MODE_UNIX;
        g_dltclient.socketPath = NULL;
        dlt_parse_config_param("ControlSocketPath", &g_dltclient.socketPath);
    }
    else {
        g_dltclient.mode = DLT_CLIENT_MODE_TCP;
    }

    if (g_dltclient.mode == DLT_CLIENT_MODE_TCP) {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_server_ip(&g_dltclient, argv[index]) == -1) {
                pr_error("set server ip didn't succeed\n");
                return -1;
            }



        if (g_dltclient.servIP == 0) {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr, "ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&g_dltclient, dltdata.vflag);
            return -1;
        }
    }
    else if (g_dltclient.mode == DLT_CLIENT_MODE_SERIAL)
    {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_serial_device(&g_dltclient, argv[index]) == -1) {
                pr_error("set serial device didn't succeed\n");
                return -1;
            }



        if (g_dltclient.serialDevice == 0) {
            /* no serial device name selected, show usage and terminate */
            fprintf(stderr, "ERROR: No serial device name specified\n");
            usage();
            return -1;
        }

        dlt_client_setbaudrate(&g_dltclient, dltdata.bvalue);
    }

    /* initialise structure to use DLT file */
    dlt_file_init(&(dltdata.file), dltdata.vflag);

    /* first parse filter file if filter parameter is used */
    dlt_filter_init(&(dltdata.filter), dltdata.vflag);

    if (dltdata.evalue) {
        dlt_set_id(dltdata.ecuid, dltdata.evalue);
        dlt_set_id(g_dltclient.ecuid, dltdata.evalue);
    }
    else {
        dltdata.evalue = NULL;

        if (dlt_parse_config_param("ECUId", &dltdata.evalue) == 0) {
            dlt_set_id(dltdata.ecuid, dltdata.evalue);
            dlt_set_id(g_dltclient.ecuid, dltdata.evalue);
            free (dltdata.evalue);
        }
        else {
            fprintf(stderr, "ERROR: Failed to read ECUId from dlt.conf \n");
        }
    }

    /* Connect to TCP socket or open serial device */
    if (dlt_client_connect(&g_dltclient, dltdata.vflag) != DLT_RETURN_ERROR) {
        /* send injection message */
        if (dltdata.mvalue && dltdata.avalue && dltdata.cvalue) {
            /* ASCII */
            printf("Send injection message:\n");
            printf("AppId: %s\n", dltdata.avalue);
            printf("ConId: %s\n", dltdata.cvalue);
            printf("ServiceId: %d\n", dltdata.svalue);
            printf("Message: %s\n", dltdata.mvalue);

            /* send control message in ascii */
            if (dlt_client_send_inject_msg(&g_dltclient,
                                           dltdata.avalue,
                                           dltdata.cvalue,
                                           dltdata.svalue,
                                           (uint8_t *)dltdata.mvalue,
                                           strlen(dltdata.mvalue)) != DLT_RETURN_OK)
                fprintf(stderr, "ERROR: Could not send inject message\n");
        }
        else if (dltdata.xvalue && dltdata.avalue && dltdata.cvalue)
        {
            /* Hex */
            uint8_t buffer[1024];
            int size = 1024;
            printf("Send injection message:\n");
            printf("AppId: %s\n", dltdata.avalue);
            printf("ConId: %s\n", dltdata.cvalue);
            printf("ServiceId: %d\n", dltdata.svalue);
            printf("Message: %s\n", dltdata.xvalue);
            dlt_hex_ascii_to_binary(dltdata.xvalue, buffer, &size);
            printf("Size: %d\n", size);

            /* send control message in hex */
            if (dlt_client_send_inject_msg(&g_dltclient,
                                           dltdata.avalue,
                                           dltdata.cvalue,
                                           dltdata.svalue,
                                           buffer, size) != DLT_RETURN_OK)
                fprintf(stderr, "ERROR: Could not send inject message\n");
        }
        else if (dltdata.lvalue != DLT_INVALID_LOG_LEVEL)  /*&& dltdata.avalue && dltdata.cvalue)*/
        {
            if ((dltdata.avalue == 0) && (dltdata.cvalue == 0)) {
                if (dltdata.vflag) {
                    printf("Set all log level:\n");
                    printf("Loglevel: %d\n", dltdata.lvalue);
                }

                if (0 != dlt_client_send_all_log_level(&g_dltclient,
                                                       dltdata.lvalue))
                    fprintf(stderr, "ERROR: Could not send log level\n");
            }
            else {
                /* log level */
                if (dltdata.vflag) {
                    printf("Set log level:\n");
                    printf("AppId: %s\n", dltdata.avalue);
                    printf("ConId: %s\n", dltdata.cvalue);
                    printf("Loglevel: %d\n", dltdata.lvalue);
                }

                /* send control message*/
                if (0 != dlt_client_send_log_level(&g_dltclient,
                                                   dltdata.avalue,
                                                   dltdata.cvalue,
                                                   dltdata.lvalue))
                    fprintf(stderr, "ERROR: Could not send log level\n");
            }
        }
        else if (dltdata.rvalue != DLT_INVALID_TRACE_STATUS)
        {
            if ((dltdata.avalue == 0) && (dltdata.cvalue == 0)) {
                if (dltdata.vflag) {
                    printf("Set all trace status:\n");
                    printf("Tracestatus: %d\n", dltdata.rvalue);
                }

                if (0 != dlt_client_send_all_trace_status(&g_dltclient,
                                                          dltdata.rvalue))
                    fprintf(stderr, "ERROR: Could not send trace status\n");
            }
            else {
                /* trace status */
                if (dltdata.vflag) {
                    printf("Set trace status:\n");
                    printf("AppId: %s\n", dltdata.avalue);
                    printf("ConId: %s\n", dltdata.cvalue);
                    printf("Tracestatus: %d\n", dltdata.rvalue);
                }

                /* send control message*/
                if (0 != dlt_client_send_trace_status(&g_dltclient,
                                                      dltdata.avalue,
                                                      dltdata.cvalue,
                                                      dltdata.rvalue))
                    fprintf(stderr, "ERROR: Could not send trace status\n");
            }
        }
        else if (dltdata.dvalue != -1)
        {
            /* default log level */
            printf("Set default log level:\n");
            printf("Loglevel: %d\n", dltdata.dvalue);

            /* send control message in*/
            if (dlt_client_send_default_log_level(&g_dltclient, dltdata.dvalue) != DLT_RETURN_OK)
                fprintf (stderr, "ERROR: Could not send default log level\n");
        }
        else if (dltdata.fvalue != -1)
        {
            /* default trace status */
            printf("Set default trace status:\n");
            printf("TraceStatus: %d\n", dltdata.fvalue);

            /* send control message in*/
            if (dlt_client_send_default_trace_status(&g_dltclient, dltdata.fvalue) != DLT_RETURN_OK)
                fprintf (stderr, "ERROR: Could not send default trace status\n");
        }
        else if (dltdata.ivalue != -1)
        {
            /* timing pakets */
            printf("Set timing pakets:\n");
            printf("Timing packets: %d\n", dltdata.ivalue);

            /* send control message in*/
            if (dlt_client_send_timing_pakets(&g_dltclient, dltdata.ivalue) != DLT_RETURN_OK)
                fprintf (stderr, "ERROR: Could not send timing packets\n");
        }
        else if (dltdata.oflag != -1)
        {
            /* default trace status */
            printf("Store config\n");

            /* send control message in*/
            if (dlt_client_send_store_config(&g_dltclient) != DLT_RETURN_OK)
                fprintf (stderr, "ERROR: Could not send store config\n");
        }
        else if (dltdata.gflag != -1)
        {
            /* reset to factory default */
            printf("Reset to factory default\n");

            /* send control message in*/
            if (dlt_client_send_reset_to_factory_default(&g_dltclient) != DLT_RETURN_OK)
                fprintf (stderr, "ERROR: Could send reset to factory default\n");
        }
        else if (dltdata.jvalue == 1)
        {
            /* get log info */
            printf("Get log info:\n");
            dlt_process_get_log_info();
        }

        /* Dlt Client Main Loop */
        /*dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag); */

        /* Wait timeout */
        ts.tv_sec = (dltdata.tvalue * NANOSEC_PER_MILLISEC) / NANOSEC_PER_SEC;
        ts.tv_nsec = (dltdata.tvalue * NANOSEC_PER_MILLISEC) % NANOSEC_PER_SEC;
        nanosleep(&ts, NULL);

        /* Wait for all the data is sent to dlt-daemon */
        wait_send_buffer_empty(g_dltclient.sock);
    }

    /* Dlt Client Cleanup */
    dlt_client_cleanup(&g_dltclient, dltdata.vflag);

    if (g_dltclient.socketPath != NULL)
        free(g_dltclient.socketPath);

    dlt_file_free(&(dltdata.file), dltdata.vflag);

    dlt_filter_free(&(dltdata.filter), dltdata.vflag);

    return 0;
}

int dlt_receive_message_callback(DltMessage *message, void *data)
{
    static char resp_text[DLT_RECEIVE_BUFSIZE];
    int ret = DLT_RETURN_ERROR;

    /* parameter check */
    if (message == NULL)
        return -1;

    /* to avoid warning */
    (void)data;

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    else
        dlt_set_storageheader(message->storageheader, "LCTL");

    /* get response data */
    ret = dlt_message_header(message, resp_text, DLT_RECEIVE_BUFSIZE, 0);

    if (ret < 0) {
        fprintf(stderr, "GET_LOG_INFO message_header result failed..\n");
        dlt_client_cleanup(&g_dltclient, 0);
        return -1;
    }

    ret = dlt_message_payload(message, resp_text, DLT_RECEIVE_BUFSIZE, DLT_OUTPUT_ASCII, 0);

    if (ret < 0) {
        fprintf(stderr, "GET_LOG_INFO message_payload result failed..\n");
        dlt_client_cleanup(&g_dltclient, 0);
        return -1;
    }

    /* check service id */
    if (g_resp == NULL) {
        fprintf(stderr, "%s: g_resp isn't allocated.\n", __func__);
        dlt_client_cleanup(&g_dltclient, 0);
        return -1;
    }

    ret = dlt_set_loginfo_parse_service_id(resp_text, &g_resp->service_id, &g_resp->status);

    if ((ret == 0) && (g_resp->service_id == DLT_SERVICE_ID_GET_LOG_INFO)) {
        ret = dlt_client_parse_get_log_info_resp_text(g_resp, resp_text);

        if (ret != 0)
            fprintf(stderr, "GET_LOG_INFO result failed..\n");

        dlt_client_cleanup(&g_dltclient, 0);
    }

    return ret;
}
