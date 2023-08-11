/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-stress-client.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-stress-client.c                                      **
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
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 */

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for strcmp() */
#include <fcntl.h>
#include <sys/uio.h>    /* for writev() */

#include "dlt_client.h"
#include "dlt_protocol.h"
#include "dlt_user.h"

#define DLT_TESTCLIENT_TEXTBUFSIZE 10024  /* Size of buffer for text output */
#define DLT_TESTCLIENT_ECU_ID     "ECU1"

#define DLT_TESTCLIENT_NUM_TESTS       7

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
    char *ovalue;
    char *fvalue;
    char *tvalue;
    char *evalue;
    int nvalue;
    int bvalue;
    int sendSerialHeaderFlag;
    int resyncSerialHeaderFlag;

    char ecuid[4];
    int ohandle;

    DltFile file;
    DltFilter filter;

    int running_test;

    int test_counter_macro[DLT_TESTCLIENT_NUM_TESTS];
    int test_counter_function[DLT_TESTCLIENT_NUM_TESTS];

    int tests_passed;
    int tests_failed;

    int sock;

    /* test values */
    unsigned long bytes_received;
    unsigned long time_elapsed;
    int last_value;
    int count_received_messages;
    int count_not_received_messages;

} DltTestclientData;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-test-stress-client [options] hostname/serial_device_name\n");
    printf("Test against received data from dlt-test-stress-user.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -a            Print DLT messages; payload as ASCII\n");
    printf("  -x            Print DLT messages; payload as hex\n");
    printf("  -m            Print DLT messages; payload as hex and ASCII\n");
    printf("  -s            Print DLT messages; only headers\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -S            Send message with serial header (Default: Without serial header)\n");
    printf("  -R            Enable resync serial header\n");
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: ECU1)\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("  -f filename   Enable filtering of messages\n");
    printf("  -n messages   Number of messages to be received per test(Default: 10000)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    DltClient dltclient;
    DltTestclientData dltdata;
    int c, i;
    int index;

    /* Initialize dltclient */
    memset(&dltclient, 0x0, sizeof(DltClient));

    /* Initialize dltdata */
    dltdata.aflag = 0;
    dltdata.sflag = 0;
    dltdata.xflag = 0;
    dltdata.mflag = 0;
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.ovalue = 0;
    dltdata.fvalue = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;
    dltdata.sendSerialHeaderFlag = 0;
    dltdata.resyncSerialHeaderFlag = 0;
    dltdata.nvalue = 10000;
    dltdata.ohandle = -1;

    dltdata.running_test = 0;

    for (i = 0; i < DLT_TESTCLIENT_NUM_TESTS; i++) {
        dltdata.test_counter_macro[i] = 0;
        dltdata.test_counter_function[i] = 0;
    }

    dltdata.tests_passed = 0;
    dltdata.tests_failed = 0;

    dltdata.bytes_received = 0;
    dltdata.time_elapsed = dlt_uptime();

    dltdata.last_value = 0;
    dltdata.count_received_messages = 0;
    dltdata.count_not_received_messages = 0;


    dltdata.sock = -1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashSRyxmf:o:e:b:n:")) != -1)
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
        case 'S':
        {
            dltdata.sendSerialHeaderFlag = 1;
            break;
        }
        case 'R':
        {
            dltdata.resyncSerialHeaderFlag = 1;
            break;
        }
        case 'y':
        {
            dltdata.yflag = 1;
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
        case 'n':
        {
            dltdata.nvalue = atoi(optarg);
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
    dlt_client_init(&dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_testclient_message_callback);

    /* Setup DLT Client structure */
    dltclient.mode = dltdata.yflag;

    if (dltclient.mode == 0) {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_server_ip(&dltclient, argv[index]) == -1) {
                fprintf(stderr, "set server ip didn't succeed\n");
                return -1;
            }



        if (dltclient.servIP == 0) {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr, "ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&dltclient, dltdata.vflag);
            return -1;
        }
    }
    else {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_serial_device(&dltclient, argv[index]) == -1) {
                fprintf(stderr, "set serial device didn't succeed\n");
                return -1;
            }



        if (dltclient.serialDevice == 0) {
            /* no serial device name selected, show usage and terminate */
            fprintf(stderr, "ERROR: No serial device name specified\n");
            usage();
            return -1;
        }

        dlt_client_setbaudrate(&dltclient, dltdata.bvalue);
    }

    /* Update the send and resync serial header flags based on command line option */
    dltclient.send_serial_header = dltdata.sendSerialHeaderFlag;
    dltclient.resync_serial_header = dltdata.resyncSerialHeaderFlag;

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
    if (dlt_client_connect(&dltclient, dltdata.vflag) != DLT_RETURN_ERROR) {
        dltdata.sock = dltclient.sock;

        /* Dlt Client Main Loop */
        dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&dltclient, dltdata.vflag);
    }

    /* dlt-receive cleanup */
    if (dltdata.ovalue)
        close(dltdata.ohandle);

    dlt_file_free(&(dltdata.file), dltdata.vflag);

    dlt_filter_free(&(dltdata.filter), dltdata.vflag);

    return 0;
}

int dlt_testclient_message_callback(DltMessage *message, void *data)
{
    static char text[DLT_TESTCLIENT_TEXTBUFSIZE];
    DltTestclientData *dltdata;

    uint32_t type_info, type_info_tmp;
    int16_t length, length_tmp; /* the macro can set this variable to -1 */
    uint8_t *ptr;
    int32_t datalength;
    int32_t value;
    uint32_t value_tmp = 0;

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

    if ((dltdata->fvalue == 0) ||
        (dltdata->fvalue &&
         (dlt_message_filter_check(message, &(dltdata->filter), dltdata->vflag) == DLT_RETURN_TRUE))) {

        /*dlt_message_header(message,text,sizeof(text),dltdata->vflag); */
        if (dltdata->aflag) {
            /*printf("%s ",text); */
        }

        /*dlt_message_payload(message,text,sizeof(text),DLT_OUTPUT_ASCII,dltdata->vflag); */
        if (dltdata->aflag) {
            /*printf("[%s]\n",text); */
        }

        /* do something here */

        /* Count number of received bytes */
        dltdata->bytes_received += message->datasize + message->headersize - sizeof(DltStorageHeader);

        /* print number of received bytes */
        if ((dlt_uptime() - dltdata->time_elapsed) > 10000) {
            printf("Received %lu Bytes/s\n", dltdata->bytes_received /**10000/(dlt_uptime()-dltdata->time_elapsed)*/);
            /*printf("Received %lu Bytes received\n",dltdata->bytes_received); */
            dltdata->time_elapsed = dlt_uptime();
            dltdata->bytes_received = 0;
        }

        /* Extended header */
        if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
            /* Log message */
            if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_LOG) {
                /* Verbose */
                if (DLT_IS_MSIN_VERB(message->extendedheader->msin)) {
                    /* 2 arguments */
                    if (message->extendedheader->noar == 2) {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0;
                        length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = message->datasize;

                        /* first read the type info of the first argument: must be string */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_SINT) {
                            /* read value */
                            DLT_MSG_READ_VALUE(value_tmp, ptr, datalength, int32_t);
                            value = DLT_ENDIAN_GET_32(message->standardheader->htyp, value_tmp);
                            /*printf("%d\n",value); */

                            if (value < dltdata->last_value) {
                                if (dltdata->nvalue == dltdata->count_received_messages)
                                    printf("PASSED: %d Msg received, %d not received\n",
                                           dltdata->count_received_messages,
                                           dltdata->count_not_received_messages);
                                else
                                    printf("FAILED: %d Msg received, %d not received\n",
                                           dltdata->count_received_messages,
                                           dltdata->count_not_received_messages);

                                dltdata->last_value = 0;
                                dltdata->count_received_messages = 0;
                                dltdata->count_not_received_messages = value - 1;
                            }
                            else {
                                dltdata->count_not_received_messages += value - dltdata->last_value - 1;
                            }

                            dltdata->last_value = value;
                            dltdata->count_received_messages++;

                            if (length >= 0) {
                                ptr += length;
                                datalength -= length;

                                /* read type of second argument: must be raw */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_RAWD) {
                                    /* get length of raw data block */
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                    if ((length >= 0) && (length == datalength))
                                        /*printf("Raw data found in payload, length="); */
                                        /*printf("%d, datalength=%d \n", length, datalength); */
                                        dltdata->test_counter_macro[3]++;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* if no filter set or filter is matching display message */
        if (dltdata->xflag)
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

            bytes_written = (int) writev(dltdata->ohandle, iov, 2);

            if (0 > bytes_written) {
                printf("dlt_testclient_message_callback, error when: writev(dltdata->ohandle, iov, 2) \n");
                return -1;
            }
        }
    }

    return 0;
}
