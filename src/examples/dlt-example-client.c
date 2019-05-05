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
 * \file dlt-test-client.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-client.c                                             **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**              Avi Yeger                                                     **
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
**  ay          Avi Yeger                  Argus                              **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 * ay          05.05.2019   change the test into a utility
 */

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <string.h>     /* for strcmp() */
#include <sys/uio.h>    /* for writev() */

#include "dlt_client.h"
#include "dlt_protocol.h"
#include "dlt_user.h"

#define DLT_TESTCLIENT_TEXTBUFSIZE 10024  /* Size of buffer for text output */
#define DLT_TESTCLIENT_ECU_ID     "ECU1"

#define DLT_TESTCLIENT_NUM_TESTS       9

static int g_testsFailed = 0;
DltClient g_dltclient;
/* Function prototypes */
int dlt_testclient_message_callback(DltMessage *message, void *data);

typedef struct
{
    int aflag;
    int sflag;
    int xflag;
    int mflag;
    int vflag;
    int yflag;
    int port;
    char *ovalue;
    char *fvalue;
    char *tvalue;
    char *evalue;
    int bvalue;

    char ecuid[4];
    int ohandle;

    DltFile file;
    DltFilter filter;

    int sock;
} DltTestclientData;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-test-client [options] hostname/serial_device_name\n");
    printf("Test against received data from dlt-test-user.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -a            Print DLT messages; payload as ASCII\n");
    printf("  -x            Print DLT messages; payload as hex\n");
    printf("  -m            Print DLT messages; payload as hex and ASCII\n");
    printf("  -s            Print DLT messages; only headers\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -y            Serial device mode\n");
    printf("  -p port 	    Port (Default: 3490)\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: ECU1)\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("  -f filename   Enable filtering of messages\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    DltTestclientData dltdata;
    int c;
    int index;

    /* Initialize dltdata */
    dltdata.aflag = 0;
    dltdata.sflag = 0;
    dltdata.xflag = 0;
    dltdata.mflag = 0;
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.port = 0;
    dltdata.ovalue = 0;
    dltdata.fvalue = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;
    dltdata.ohandle = -1;

    dltdata.sock = -1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashyxmf:o:e:b:p:")) != -1)
        switch (c) {
        case 'v':
        {
            dltdata.vflag = 1;
            break;
        }
        case 'a':
        {
            dltdata.aflag = 1;
            break;
        }
        case 's':
        {
            dltdata.sflag = 1;
            break;
        }
        case 'x':
        {
            dltdata.xflag = 1;
            break;
        }
        case 'm':
        {
            dltdata.mflag = 1;
            break;
        }
        case 'h':
        {
            usage();
            return -1;
        }
        case 'y':
        {
            dltdata.yflag = 1;
            break;
        }
        case 'p':
        {
            dltdata.port = atoi(optarg);
            break;
        }
        case 'f':
        {
            dltdata.fvalue = optarg;
            break;
        }
        case 'o':
        {
            dltdata.ovalue = optarg;
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
        case '?':
        {
            if ((optopt == 'o') || (optopt == 'f') || (optopt == 't'))
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
            return -1;/*for parasoft */
        }
        }

    /* Initialize DLT Client */
    if (dltdata.port) {
	dlt_client_init_port(&g_dltclient, dltdata.port, dltdata.vflag);
    } else {
    	dlt_client_init(&g_dltclient, dltdata.vflag);
    }
    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_testclient_message_callback);

    /* Setup DLT Client structure */
    g_dltclient.mode = dltdata.yflag;

    if (g_dltclient.mode == 0) {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_server_ip(&g_dltclient, argv[index]) == -1) {
                fprintf(stderr, "set server ip didn't succeed\n");
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
    else {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_serial_device(&g_dltclient, argv[index]) == -1) {
                fprintf(stderr, "set serial device didn't succeed\n");
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

    if (dltdata.fvalue) {
        if (dlt_filter_load(&(dltdata.filter), dltdata.fvalue, dltdata.vflag) < DLT_RETURN_OK) {
            dlt_file_free(&(dltdata.file), dltdata.vflag);
            return -1;
        }

        dlt_file_set_filter(&(dltdata.file), &(dltdata.filter), dltdata.vflag);
    }

    /* open DLT output file */
    if (dltdata.ovalue) {
        dltdata.ohandle = open(dltdata.ovalue, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

        if (dltdata.ohandle == -1) {
            dlt_file_free(&(dltdata.file), dltdata.vflag);
            fprintf(stderr, "ERROR: Output file %s cannot be opened!\n", dltdata.ovalue);
            return -1;
        }
    }

    if (dltdata.evalue)
        dlt_set_id(dltdata.ecuid, dltdata.evalue);
    else
        dlt_set_id(dltdata.ecuid, DLT_TESTCLIENT_ECU_ID);

    /* Connect to TCP socket or open serial device */
    if (dlt_client_connect(&g_dltclient, dltdata.vflag) != DLT_RETURN_ERROR) {
        dltdata.sock = g_dltclient.sock;

        /* Dlt Client Main Loop */
        dlt_client_main_loop(&g_dltclient, &dltdata, dltdata.vflag);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&g_dltclient, dltdata.vflag);
    }

    /* dlt-receive cleanup */
    if (dltdata.ovalue)
        close(dltdata.ohandle);

    dlt_file_free(&(dltdata.file), dltdata.vflag);

    dlt_filter_free(&(dltdata.filter), dltdata.vflag);

    return g_testsFailed == 0 ? 0 : 1;
}

int dlt_testclient_message_callback(DltMessage *message, void *data)
{
    static char text[DLT_TESTCLIENT_TEXTBUFSIZE];
    DltTestclientData *dltdata;


    struct iovec iov[2];
    int bytes_written;

    if ((message == 0) || (data == 0))
        return -1;

    dltdata = (DltTestclientData *)data;

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    else
        dlt_set_storageheader(message->storageheader, dltdata->ecuid);

    /* if no filter set or filter is matching display message */
    if ((dltdata->fvalue == 0) ||
        (dltdata->fvalue &&
         (dlt_message_filter_check(message, &(dltdata->filter), dltdata->vflag) == DLT_RETURN_TRUE))) {

        dlt_message_header(message, text, sizeof(text), dltdata->vflag);

        if (dltdata->aflag)
            printf("%s ", text);

        dlt_message_payload(message, text, sizeof(text), DLT_OUTPUT_ASCII, dltdata->vflag);

        if (dltdata->aflag)
            printf("[%s]\n", text);
	else if (dltdata->xflag)
            dlt_message_print_hex(message, text, DLT_TESTCLIENT_TEXTBUFSIZE, dltdata->vflag);
        else if (dltdata->mflag)
            dlt_message_print_mixed_plain(message, text, DLT_TESTCLIENT_TEXTBUFSIZE, dltdata->vflag);
        else if (dltdata->sflag)
            dlt_message_print_header(message, text, sizeof(text), dltdata->vflag);

        /* if file output enabled write message */
        if (dltdata->ovalue) {
            iov[0].iov_base = message->headerbuffer;
            iov[0].iov_len = message->headersize;
            iov[1].iov_base = message->databuffer;
            iov[1].iov_len = message->datasize;

            bytes_written = writev(dltdata->ohandle, iov, 2);

            if (0 > bytes_written) {
                printf("dlt_testclient_message_callback, error in: writev(dltdata->ohandle, iov, 2)\n");
                return -1;
            }
        }
    }

    return 0;
}
