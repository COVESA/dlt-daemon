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
#include <sys/uio.h>    /* for writev() */
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>

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
bool dlt_testclient_fetch_next_message_callback(void *data);

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
    int max_messages;
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
    printf("  -S            Send message with serial header (Default: Without serial header)\n");
    printf("  -R            Enable resync serial header\n");
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: ECU1)\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("  -f filename   Enable filtering of messages\n");
    printf("  -z max msgs   Print z DLT messages\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    DltTestclientData dltdata;
    int c, i;
    int index;

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
    dltdata.ohandle = -1;

    dltdata.running_test = 0;
    dltdata.max_messages = INT_MIN;

    for (i = 0; i < DLT_TESTCLIENT_NUM_TESTS; i++) {
        dltdata.test_counter_macro[i] = 0;
        dltdata.test_counter_function[i] = 0;
    }

    dltdata.tests_passed = 0;
    dltdata.tests_failed = 0;

    dltdata.sock = -1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashSRyxmf:o:e:b:z:")) != -1)
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
        case 'z':
        {
            dltdata.max_messages = atoi(optarg);
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
    dlt_client_init(&g_dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_testclient_message_callback);

    /* Register callback to be called if next message needs to be fetched */
    dlt_client_register_fetch_next_message_callback(dlt_testclient_fetch_next_message_callback);

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

    /* Update the send and resync serial header flags based on command line option */
    g_dltclient.send_serial_header = dltdata.sendSerialHeaderFlag;
    g_dltclient.resync_serial_header = dltdata.resyncSerialHeaderFlag;

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

bool dlt_testclient_fetch_next_message_callback(void *data)
{
  if (data == 0)
    return true;

  DltTestclientData *dltdata = (DltTestclientData *)data;
  if (dltdata->max_messages > INT_MIN)
  {
    dltdata->max_messages--;
    if (dltdata->max_messages <= 0)
      return false;
  }
  return true;
}

int dlt_testclient_message_callback(DltMessage *message, void *data)
{
    static char text[DLT_TESTCLIENT_TEXTBUFSIZE];
    int mtin;
    DltTestclientData *dltdata;

    uint32_t type_info, type_info_tmp;
    int16_t length, length_tmp; /* the macro can set this variable to -1 */
    uint32_t length_tmp32 = 0;
    uint8_t *ptr;
    int32_t datalength;

    uint32_t id, id_tmp;
    int slen;
    int tc_old;

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

        dlt_message_header(message, text, sizeof(text), dltdata->vflag);

        if (dltdata->aflag)
            printf("%s ", text);

        dlt_message_payload(message, text, sizeof(text), DLT_OUTPUT_ASCII, dltdata->vflag);

        if (dltdata->aflag)
            printf("[%s]\n", text);

        if (strcmp(text, "Tests starting") == 0)
            printf("Tests starting\n");

        /* check test 1m */
        if (strcmp(text, "Test1: (Macro IF) Test all log levels") == 0) {
            printf("Test1m: (Macro IF) Test all log levels\n");
            dltdata->running_test = 1;
            dltdata->test_counter_macro[0] = 0;
        }
        else if (strcmp(text, "Test1: (Macro IF) finished") == 0)
        {
            /* >=4, as "info" is default log level */
            if (dltdata->test_counter_macro[0] >= 4) {
                printf("Test1m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test1m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 1)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_LOG) {
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_LOG_FATAL)
                        dltdata->test_counter_macro[0]++;

                    if (mtin == DLT_LOG_ERROR)
                        dltdata->test_counter_macro[0]++;

                    if (mtin == DLT_LOG_WARN)
                        dltdata->test_counter_macro[0]++;

                    if (mtin == DLT_LOG_INFO)
                        dltdata->test_counter_macro[0]++;

                    if (mtin == DLT_LOG_DEBUG)
                        dltdata->test_counter_macro[0]++;

                    if (mtin == DLT_LOG_VERBOSE)
                        dltdata->test_counter_macro[0]++;
                }
            }
        }

        /* check test 2m */
        if (strcmp(text, "Test2: (Macro IF) Test all variable types (verbose)") == 0) {
            printf("Test2m: (Macro IF) Test all variable types (verbose)\n");
            dltdata->running_test = 2;
            dltdata->test_counter_macro[1] = 0;
        }
        else if (strcmp(text, "Test2: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[1] == 16) {
                printf("Test2m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test2m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 2)
        {
            /* Verbose */
            if (!(DLT_MSG_IS_NONVERBOSE(message))) {
                type_info = 0;
                type_info_tmp = 0;
                length = 0;  /* the macro can set this variable to -1 */
                length_tmp = 0;
                ptr = message->databuffer;
                datalength = (int32_t) message->datasize;

                /* Log message */
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_LOG) {
                    if (message->extendedheader->noar >= 2) {
                        /* get type of first argument: must be string */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            /* skip string */
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                            if (length >= 0) {
                                ptr += length;
                                datalength -= length;

                                /* read type of second argument: must be raw */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if ((type_info & DLT_TYPE_INFO_STRG) &&
                                    ((type_info & DLT_TYPE_INFO_SCOD) == DLT_SCOD_ASCII)) {
                                    if (datalength == (sizeof(uint16_t) + strlen("Hello world") + 1))
                                        dltdata->test_counter_macro[1]++;
                                }
                                else if ((type_info & DLT_TYPE_INFO_STRG) &&
                                         ((type_info & DLT_TYPE_INFO_SCOD) == DLT_SCOD_UTF8))
                                {
                                    if (datalength == (sizeof(uint16_t) + strlen("Hello world") + 1))
                                        dltdata->test_counter_macro[1]++;
                                }
                                else if (type_info & DLT_TYPE_INFO_BOOL)
                                {
                                    if (datalength == sizeof(uint8_t))
                                        dltdata->test_counter_macro[1]++;
                                }
                                else if (type_info & DLT_TYPE_INFO_SINT)
                                {
                                    switch (type_info & DLT_TYPE_INFO_TYLE) {
                                    case DLT_TYLE_8BIT:
                                    {
                                        if (datalength == sizeof(int8_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_16BIT:
                                    {
                                        if (datalength == sizeof(int16_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_32BIT:
                                    {
                                        if (datalength == sizeof(int32_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_64BIT:
                                    {
                                        if (datalength == sizeof(int64_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_128BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    }
                                }
                                else if (type_info & DLT_TYPE_INFO_UINT)
                                {
                                    switch (type_info & DLT_TYPE_INFO_TYLE) {
                                    case DLT_TYLE_8BIT:
                                    {
                                        if (datalength == sizeof(uint8_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_16BIT:
                                    {
                                        if (datalength == sizeof(uint16_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_32BIT:
                                    {
                                        if (datalength == sizeof(uint32_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_64BIT:
                                    {
                                        if (datalength == sizeof(uint64_t))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_128BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    }
                                }
                                else if (type_info & DLT_TYPE_INFO_FLOA)
                                {
                                    switch (type_info & DLT_TYPE_INFO_TYLE) {
                                    case DLT_TYLE_8BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    case DLT_TYLE_16BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    case DLT_TYLE_32BIT:
                                    {
                                        if (datalength == (2 * sizeof(float) + sizeof(uint32_t)))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_64BIT:
                                    {
                                        if (datalength == (2 * sizeof(double) + sizeof(uint32_t)))
                                            dltdata->test_counter_macro[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_128BIT:
                                        /* Not tested here */
                                        break;
                                    }
                                }
                                else if (type_info & DLT_TYPE_INFO_RAWD)
                                {
                                    /* Get length */
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                    if ((length == datalength) && (10 == length))
                                        dltdata->test_counter_macro[1]++;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 3m */
        if (strcmp(text, "Test3: (Macro IF) Test all variable types (non-verbose)") == 0) {
            printf("Test3m: (Macro IF) Test all variable types (non-verbose)\n");
            dltdata->running_test = 3;
            dltdata->test_counter_macro[2] = 0;
        }
        else if (strcmp(text, "Test3: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[2] == 16) {
                printf("Test3m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test3m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 3)
        {
            /* Nonverbose */
            if (DLT_MSG_IS_NONVERBOSE(message)) {
                id = 0;
                id_tmp = 0;
                ptr = message->databuffer;
                datalength = (int32_t) message->datasize;
                slen = -1;

                tc_old = dltdata->test_counter_macro[2];

                /* Get message id */
                DLT_MSG_READ_VALUE(id_tmp, ptr, datalength, uint32_t);
                id = DLT_ENDIAN_GET_32(message->standardheader->htyp, id_tmp);

                /* Length of string */
                datalength -= (int32_t) sizeof(uint16_t);
                ptr += sizeof(uint16_t);

                switch (id) {
                case  1:
                {
                    slen = strlen("string") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint16_t) + strlen("Hello world") + 1)
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  2:
                {
                    slen = strlen("utf8") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint16_t) + strlen("Hello world") + 1)
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  3:
                {
                    slen = strlen("bool") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint8_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  4:
                {
                    slen = strlen("int") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  5:
                {
                    slen = strlen("int8") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int8_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  6:
                {
                    slen = strlen("int16") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int16_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  7:
                {
                    slen = strlen("int32") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int32_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  8:
                {
                    slen = strlen("int64") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int64_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  9:
                {
                    slen = strlen("uint") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(unsigned int))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  10:
                {
                    slen = strlen("uint8") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint8_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case  11:
                {
                    slen = strlen("uint16") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint16_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case 12:
                {
                    slen = strlen("uint32") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint32_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case 13:
                {
                    slen = strlen("uint64") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint64_t))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case 14:
                {
                    slen = strlen("float32") + 1;
                    datalength -= slen;
                    ptr += slen;

                    /* 2*, as the min and the max is transfered */
                    if (datalength == 2 * sizeof(float))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case 15:
                {
                    slen = strlen("float64") + 1;
                    datalength -= slen;
                    ptr += slen;

                    /* 2*, as the min and the max is transfered */
                    if (datalength == 2 * sizeof(double))
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                case 16:
                {
                    slen = strlen("raw") + 1;
                    datalength -= slen;
                    ptr += slen;
                    datalength -= (int32_t) sizeof(uint16_t);
                    ptr += sizeof(uint16_t);

                    if (datalength == 10)
                        dltdata->test_counter_macro[2]++;

                    break;
                }
                }

                if ((slen >= 0) && (tc_old == dltdata->test_counter_macro[2]))
                    printf("ID=%d, Datalength=%d => Failed!", id, datalength);
            }
        }

        /* check test 4m */
        if (strcmp(text, "Test4: (Macro IF) Test different message sizes") == 0) {
            printf("Test4m: (Macro IF) Test different message sizes\n");
            dltdata->running_test = 4;
            dltdata->test_counter_macro[3] = 0;
        }
        else if (strcmp(text, "Test4: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[3] == 4) {
                printf("Test4m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test4m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 4)
        {
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
                            datalength = (int32_t) message->datasize;

                            /* first read the type info of the first argument: must be string */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_STRG) {
                                /* skip string */
                                DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

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
        }

        /* check test 5m */
        if (strcmp(text, "Test5: (Macro IF) Test high-level API") == 0) {
            printf("Test5m: (Macro IF) Test high-level API\n");
            dltdata->running_test = 5;
            dltdata->test_counter_macro[4] = 0;
        }
        else if (strcmp(text, "Test5: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[4] == 12) {
                printf("Test5m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test5m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 5)
        {
            if (strcmp(text, "Next line: DLT_LOG_INT") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "-42") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "Next line: DLT_LOG_UINT") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "42") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "Next line: DLT_LOG_STRING") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "String output") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "Next line: DLT_LOG_RAW") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "00\'01\'02\'03\'04\'05\'06\'07\'08\'09\'0a\'0b\'0c\'0d\'0e\'0f") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "Next line: DLT_LOG_STRING_INT") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "String output:  -42") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "Next line: DLT_LOG_STRING_UINT") == 0)
                dltdata->test_counter_macro[4]++;

            if (strcmp(text, "String output:  42") == 0)
                dltdata->test_counter_macro[4]++;
        }

        /* check test 6m */
        if (strcmp(text, "Test 6: (Macro IF) Test local printing") == 0) {
            printf("Test6m: (Macro IF) Test local printing\n");
            dltdata->running_test = 6;
            dltdata->test_counter_macro[5] = 0;
        }
        else if (strcmp(text, "Test6: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[5] == 2) {
                printf("Test6m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test6m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 6)
        {
            if (strcmp(text, "Message (visible: locally printed)") == 0) {
                printf("Message (visible: locally printed)\n");
                dltdata->test_counter_macro[5]++;
            }

            if (strcmp(text, "Message (invisible: not locally printed)") == 0) {
                printf("Message (invisible: not locally printed)\n");
                dltdata->test_counter_macro[5]++;
            }
        }

        /* check test 7m */
        if (strcmp(text, "Test 7: (Macro IF) Test network trace") == 0) {
            printf("Test7m: (Macro IF) Test network trace\n");
            dltdata->running_test = 7;
            dltdata->test_counter_macro[6] = 0;
        }
        else if (strcmp(text, "Test7: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[6] == 8) {
                printf("Test7m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test7m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 7)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_NW_TRACE) {
                    /* Check message type information*/
                    /* Each correct message type increases the counter by 1 */
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_NW_TRACE_IPC)
                        dltdata->test_counter_macro[6]++;

                    if (mtin == DLT_NW_TRACE_CAN)
                        dltdata->test_counter_macro[6]++;

                    if (mtin == DLT_NW_TRACE_FLEXRAY)
                        dltdata->test_counter_macro[6]++;

                    if (mtin == DLT_NW_TRACE_MOST)
                        dltdata->test_counter_macro[6]++;

                    /* Check payload, must be two arguments (2 raw data blocks) */
                    /* If the payload is correct, the counter is increased by 1 */
                    if (message->extendedheader->noar == 2) {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* first read the type info of the first argument: must be string */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_RAWD) {
                            /* skip string */
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

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
                                        dltdata->test_counter_macro[6]++;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 8m */
        if (strcmp(text, "Test 8: (Macro IF) Test truncated network trace") == 0) {
            printf("Test8m: (Macro IF) Test truncated network trace\n");
            dltdata->running_test = 8;
            dltdata->test_counter_macro[7] = 0;
        }
        else if (strcmp(text, "Test8: (Macro IF) finished") == 0)
        {
            if (dltdata->test_counter_macro[7] == 20) {
                printf("Test8m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test8m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 8)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_NW_TRACE) {
                    /* Check message type information*/
                    /* Each correct message type increases the counter by 1 */
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_NW_TRACE_IPC)
                        dltdata->test_counter_macro[7]++;

                    if (mtin == DLT_NW_TRACE_CAN)
                        dltdata->test_counter_macro[7]++;

                    if (mtin == DLT_NW_TRACE_FLEXRAY)
                        dltdata->test_counter_macro[7]++;

                    if (mtin == DLT_NW_TRACE_MOST)
                        dltdata->test_counter_macro[7]++;

                    /* Check payload, must be two arguments (2 raw data blocks) */
                    /* If the payload is correct, the counter is increased by 1 */
                    if (message->extendedheader->noar == 4) {
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            /* Read NWTR */
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_TRUNCATED) == 0)
                                dltdata->test_counter_macro[7]++;

                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_RAWD) {
                                char hdr[2048];
                                DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                                DLT_MSG_READ_STRING(hdr, ptr, datalength, (int)sizeof(hdr), length);

                                if ((length == 16) && (hdr[15] == 15))
                                    dltdata->test_counter_macro[7]++;

                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_UINT) {
                                    uint32_t orig_size;
                                    DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                    orig_size = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                    if (orig_size == 1024 * 5)
                                        dltdata->test_counter_macro[7]++;

                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_RAWD) {
                                        DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                        length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                        /* Size of the truncated message after headers */
                                        if (length == DLT_USER_BUF_MAX_SIZE - 41 - sizeof(uint16_t) - sizeof(uint32_t))
                                            dltdata->test_counter_macro[7]++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 9m */
        if (strcmp(text, "Test 9: (Macro IF) Test segmented network trace") == 0) {
            printf("Test9m: (Macro IF) Test segmented network trace\n");
            dltdata->running_test = 9;
            dltdata->test_counter_macro[8] = 0;
        }
        else if (strcmp(text, "Test9: (Macro IF) finished") == 0)
        {
            /* (Interface types) * (results per packet)*/
            if (dltdata->test_counter_macro[8] == 4 * 35) {
                printf("Test9m PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test9m FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 9)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_NW_TRACE) {
                    /* Check message type information*/
                    /* Each correct message type increases the counter by 1 */
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_NW_TRACE_IPC)
                        dltdata->test_counter_macro[8]++;

                    if (mtin == DLT_NW_TRACE_CAN)
                        dltdata->test_counter_macro[8]++;

                    if (mtin == DLT_NW_TRACE_FLEXRAY)
                        dltdata->test_counter_macro[8]++;

                    if (mtin == DLT_NW_TRACE_MOST)
                        dltdata->test_counter_macro[8]++;

                    /* Payload for first segmented message */
                    if (message->extendedheader->noar == 6) {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* NWST */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_START) == 0)
                                dltdata->test_counter_macro[8]++;

                            /* Streahandle */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_UINT) {
                                uint32_t handle;
                                DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                handle = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                if (handle > 0)
                                    dltdata->test_counter_macro[8]++;

                                /* Header */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_RAWD) {
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                    /* Test packet header size 16 */
                                    if (length == 16)
                                        dltdata->test_counter_macro[8]++;

                                    /* Skip data */
                                    ptr += length;
                                    datalength -= length;

                                    /* Payload size */
                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_UINT) {
                                        uint32_t pl_sz;
                                        DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                        pl_sz = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                        /* Test packet payload size. */
                                        if (pl_sz == 5120)
                                            dltdata->test_counter_macro[8]++;

                                        /* Segmentcount */
                                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                        if (type_info & DLT_TYPE_INFO_UINT) {
                                            uint16_t scount;
                                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                            scount = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                            /* Test packet segment count 5 */
                                            if (scount == 5)
                                                dltdata->test_counter_macro[8]++;

                                            /* Segment length */
                                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                            if (type_info & DLT_TYPE_INFO_UINT) {
                                                uint16_t slen;
                                                DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                                slen = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                                /* Default segment size 1024 */
                                                if (slen == 1024)
                                                    dltdata->test_counter_macro[8]++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    /* Data segment */
                    else if (message->extendedheader->noar == 4)
                    {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* NWCH */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_SEGMENT) == 0)
                                dltdata->test_counter_macro[8]++;

                            /* handle */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_UINT) {
                                uint32_t handle;
                                DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                handle = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                if (handle > 0)
                                    dltdata->test_counter_macro[8]++;

                                /* Sequence */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_UINT) {
                                    /*uint16_t seq; */
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    /*seq=DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp); */
                                    dltdata->test_counter_macro[8]++;

                                    /* Data */
                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_RAWD) {
                                        DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                        length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                        /* Segment size by default, 1024 */
                                        if (length == 1024)
                                            dltdata->test_counter_macro[8]++;
                                    }
                                }
                            }
                        }
                    }
                    /* End segment */
                    else if (message->extendedheader->noar == 2)
                    {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* NWEN */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_END) == 0)
                                dltdata->test_counter_macro[8]++;

                            /* handle */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_UINT) {
                                uint32_t handle;
                                DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                handle = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                if (handle > 0)
                                    dltdata->test_counter_macro[8]++;
                            }
                        }
                    }
                }
            }
        }

        /* check test 1f */
        if (strcmp(text, "Test1: (Function IF) Test all log levels") == 0) {
            printf("Test1f: (Function IF) Test all log levels\n");
            dltdata->running_test = 10;
            dltdata->test_counter_function[0] = 0;
        }
        else if (strcmp(text, "Test1: (Function IF) finished") == 0)
        {
            /* >=4, as "info" is default log level */
            if (dltdata->test_counter_function[0] >= 4) {
                printf("Test1f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test1f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 10)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_LOG) {
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_LOG_FATAL)
                        dltdata->test_counter_function[0]++;

                    if (mtin == DLT_LOG_ERROR)
                        dltdata->test_counter_function[0]++;

                    if (mtin == DLT_LOG_WARN)
                        dltdata->test_counter_function[0]++;

                    if (mtin == DLT_LOG_INFO)
                        dltdata->test_counter_function[0]++;

                    if (mtin == DLT_LOG_DEBUG)
                        dltdata->test_counter_function[0]++;

                    if (mtin == DLT_LOG_VERBOSE)
                        dltdata->test_counter_function[0]++;
                }
            }
        }

        /* check test 2f */
        if (strcmp(text, "Test2: (Function IF) Test all variable types (verbose)") == 0) {
            printf("Test2f: (Function IF) Test all variable types (verbose)\n");
            dltdata->running_test = 11;
            dltdata->test_counter_function[1] = 0;
        }
        else if (strcmp(text, "Test2: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[1] == 14) {
                printf("Test2f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test2f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 11)
        {
            /* Verbose */
            if (!(DLT_MSG_IS_NONVERBOSE(message))) {
                type_info = 0;
                type_info_tmp = 0;
                length = 0;
                length_tmp = 0; /* the macro can set this variable to -1 */
                ptr = message->databuffer;
                datalength = (int32_t) message->datasize;

                /* Log message */
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_LOG) {
                    if (message->extendedheader->noar >= 2) {
                        /* get type of first argument: must be string */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            /* skip string */
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                            if (length >= 0) {
                                ptr += length;
                                datalength -= length;

                                /* read type of second argument: must be raw */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_BOOL) {
                                    if (datalength == sizeof(uint8_t))
                                        dltdata->test_counter_function[1]++;
                                }
                                else if (type_info & DLT_TYPE_INFO_SINT)
                                {
                                    switch (type_info & DLT_TYPE_INFO_TYLE) {
                                    case DLT_TYLE_8BIT:
                                    {
                                        if (datalength == sizeof(int8_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_16BIT:
                                    {
                                        if (datalength == sizeof(int16_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_32BIT:
                                    {
                                        if (datalength == sizeof(int32_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_64BIT:
                                    {
                                        if (datalength == sizeof(int64_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_128BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    }
                                }
                                else if (type_info & DLT_TYPE_INFO_UINT)
                                {
                                    switch (type_info & DLT_TYPE_INFO_TYLE) {
                                    case DLT_TYLE_8BIT:
                                    {
                                        if (datalength == sizeof(uint8_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_16BIT:
                                    {
                                        if (datalength == sizeof(uint16_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_32BIT:
                                    {
                                        if (datalength == sizeof(uint32_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_64BIT:
                                    {
                                        if (datalength == sizeof(uint64_t))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_128BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    }
                                }
                                else if (type_info & DLT_TYPE_INFO_FLOA)
                                {
                                    switch (type_info & DLT_TYPE_INFO_TYLE) {
                                    case DLT_TYLE_8BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    case DLT_TYLE_16BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    case DLT_TYLE_32BIT:
                                    {
                                        if (datalength == (2 * sizeof(float) + sizeof(uint32_t)))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_64BIT:
                                    {
                                        if (datalength == (2 * sizeof(double) + sizeof(uint32_t)))
                                            dltdata->test_counter_function[1]++;

                                        break;
                                    }
                                    case DLT_TYLE_128BIT:
                                    {
                                        /* Not tested here */
                                        break;
                                    }
                                    }
                                }
                                else if (type_info & DLT_TYPE_INFO_RAWD)
                                {
                                    /* Get length */
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                    if ((length == datalength) && (length == 10))
                                        dltdata->test_counter_function[1]++;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 3f */
        if (strcmp(text, "Test3: (Function IF) Test all variable types (non-verbose)") == 0) {
            printf("Test3f: (Function IF) Test all variable types (non-verbose)\n");
            dltdata->running_test = 12;
            dltdata->test_counter_function[2] = 0;
        }
        else if (strcmp(text, "Test3: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[2] == 14) {
                printf("Test3f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test3f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 12)
        {
            /* Nonverbose */
            if (DLT_MSG_IS_NONVERBOSE(message)) {
                id = 0;
                id_tmp = 0;
                ptr = message->databuffer;
                datalength = (int32_t) message->datasize;
                slen = -1;

                tc_old = dltdata->test_counter_function[2];

                /* Get message id */
                DLT_MSG_READ_VALUE(id_tmp, ptr, datalength, uint32_t);
                id = DLT_ENDIAN_GET_32(message->standardheader->htyp, id_tmp);

                /* Length of string */
                datalength -= sizeof(uint16_t);
                ptr += sizeof(uint16_t);

                switch (id) {
                case  1:
                {
                    slen = strlen("bool") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint8_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  2:
                {
                    slen = strlen("int") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  3:
                {
                    slen = strlen("int8") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int8_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  4:
                {
                    slen = strlen("int16") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int16_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  5:
                {
                    slen = strlen("int32") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int32_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  6:
                {
                    slen = strlen("int64") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(int64_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  7:
                {
                    slen = strlen("uint") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(unsigned int))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  8:
                {
                    slen = strlen("uint8") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint8_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case  9:
                {
                    slen = strlen("uint16") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint16_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case 10:
                {
                    slen = strlen("uint32") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint32_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case 11:
                {
                    slen = strlen("uint64") + 1;
                    datalength -= slen;
                    ptr += slen;

                    if (datalength == sizeof(uint64_t))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case 12:
                {
                    slen = strlen("float32") + 1;
                    datalength -= slen;
                    ptr += slen;

                    /* 2*, as the min and the max is transfered */
                    if (datalength == 2 * sizeof(float))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case 13:
                {
                    slen = strlen("float64") + 1;
                    datalength -= slen;
                    ptr += slen;

                    /* 2*, as the min and the max is transfered */
                    if (datalength == 2 * sizeof(double))
                        dltdata->test_counter_function[2]++;

                    break;
                }
                case 14:
                {
                    slen = strlen("raw") + 1;
                    datalength -= slen;
                    ptr += slen;
                    datalength -= sizeof(uint16_t);
                    ptr += sizeof(uint16_t);

                    if (datalength == 10)
                        dltdata->test_counter_function[2]++;

                    break;
                }
                }

                if ((slen >= 0) && (tc_old == dltdata->test_counter_function[2]))
                    printf("ID=%d, Datalength=%d => Failed!", id, datalength);
            }
        }

        /* check test 4f */
        if (strcmp(text, "Test4: (Function IF) Test different message sizes") == 0) {
            printf("Test4f: (Function IF) Test different message sizes\n");
            dltdata->running_test = 13;
            dltdata->test_counter_function[3] = 0;
        }
        else if (strcmp(text, "Test4: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[3] == 4) {
                printf("Test4f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test4f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 13)
        {
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
                            datalength = (int32_t) message->datasize;

                            /* first read the type info of the first argument: should be string */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_STRG) {
                                /* skip string */
                                DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                if (length >= 0) {
                                    ptr += length;
                                    datalength -= length;

                                    /* read type of second argument: should be raw */
                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_RAWD) {
                                        /* get length of raw data block */
                                        DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                        length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                        if ((length >= 0) && (length == datalength))
                                            /*printf("Raw data found in payload, length="); */
                                            /*printf("%d, datalength=%d \n", length, datalength); */
                                            dltdata->test_counter_function[3]++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 5f */
        if (strcmp(text, "Test5: (Function IF) Test high-level API") == 0) {
            printf("Test5f: (Function IF) Test high-level API\n");
            dltdata->running_test = 14;
            dltdata->test_counter_function[4] = 0;
        }
        else if (strcmp(text, "Test5: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[4] == 12) {
                printf("Test5f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test5f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 14)
        {
            if (strcmp(text, "Next line: dlt_log_int()") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "-42") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "Next line: dlt_log_uint()") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "42") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "Next line: dlt_log_string()") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "String output") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "Next line: dlt_log_raw()") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "00\'01\'02\'03\'04\'05\'06\'07\'08\'09\'0a\'0b\'0c\'0d\'0e\'0f") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "Next line: dlt_log_string_int()") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "String output:  -42") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "Next line: dlt_log_string_uint()") == 0)
                dltdata->test_counter_function[4]++;

            if (strcmp(text, "String output:  42") == 0)
                dltdata->test_counter_function[4]++;
        }

        /* check test 6f */
        if (strcmp(text, "Test 6: (Function IF) Test local printing") == 0) {
            printf("Test6f: (Function IF) Test local printing\n");
            dltdata->running_test = 15;
            dltdata->test_counter_function[5] = 0;
        }
        else if (strcmp(text, "Test6: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[5] == 2) {
                printf("Test6f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test6f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 15)
        {
            if (strcmp(text, "Message (visible: locally printed)") == 0) {
                printf("Message (visible: locally printed)\n");
                dltdata->test_counter_function[5]++;
            }

            if (strcmp(text, "Message (invisible: not locally printed)") == 0) {
                printf("Message (invisible: not locally printed)\n");
                dltdata->test_counter_function[5]++;
            }
        }

        /* check test 7f */
        if (strcmp(text, "Test 7: (Function IF) Test network trace") == 0) {
            printf("Test7f: (Function IF) Test network trace\n");
            dltdata->running_test = 16;
            dltdata->test_counter_function[6] = 0;
        }
        else if (strcmp(text, "Test7: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[6] == 8) {
                printf("Test7f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test7f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 16)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_NW_TRACE) {
                    /* Check message type information*/
                    /* Each correct message type increases the counter by 1 */
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_NW_TRACE_IPC)
                        dltdata->test_counter_function[6]++;

                    if (mtin == DLT_NW_TRACE_CAN)
                        dltdata->test_counter_function[6]++;

                    if (mtin == DLT_NW_TRACE_FLEXRAY)
                        dltdata->test_counter_function[6]++;

                    if (mtin == DLT_NW_TRACE_MOST)
                        dltdata->test_counter_function[6]++;

                    /* Check payload, must be two arguments (2 raw data blocks) */
                    /* If the payload is correct, the counter is increased by 1 */
                    if (message->extendedheader->noar == 2) {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0;
                        length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* first read the type info of the first argument: should be string */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_RAWD) {
                            /* skip string */
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                            if (length >= 0) {
                                ptr += length;
                                datalength -= length;

                                /* read type of second argument: should be raw */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_RAWD) {
                                    /* get length of raw data block */
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                    if ((length >= 0) && (length == datalength))
                                        /*printf("Raw data found in payload, length="); */
                                        /*printf("%d, datalength=%d \n", length, datalength); */
                                        dltdata->test_counter_function[6]++;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 8f */
        if (strcmp(text, "Test 8: (Function IF) Test truncated network trace") == 0) {
            printf("Test8f: (Function IF) Test truncated network trace\n");
            dltdata->running_test = 17;
            dltdata->test_counter_function[7] = 0;
        }
        else if (strcmp(text, "Test8: (Function IF) finished") == 0)
        {
            if (dltdata->test_counter_function[7] == 20) {
                printf("Test8f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test8f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 17)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_NW_TRACE) {
                    /* Check message type information*/
                    /* Each correct message type increases the counter by 1 */
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_NW_TRACE_IPC)
                        dltdata->test_counter_function[7]++;

                    if (mtin == DLT_NW_TRACE_CAN)
                        dltdata->test_counter_function[7]++;

                    if (mtin == DLT_NW_TRACE_FLEXRAY)
                        dltdata->test_counter_function[7]++;

                    if (mtin == DLT_NW_TRACE_MOST)
                        dltdata->test_counter_function[7]++;

                    /* Check payload, must be two arguments (2 raw data blocks) */
                    /* If the payload is correct, the counter is increased by 1 */
                    if (message->extendedheader->noar == 4) {
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            /* Read NWTR */
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_TRUNCATED) == 0)
                                dltdata->test_counter_function[7]++;

                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_RAWD) {
                                char hdr[2048];
                                DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                                DLT_MSG_READ_STRING(hdr, ptr, datalength, (int)sizeof(hdr), length);

                                if ((length == 16) && (hdr[15] == 15))
                                    dltdata->test_counter_function[7]++;

                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_UINT) {
                                    uint32_t orig_size;
                                    DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                    orig_size = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                    if (orig_size == 1024 * 5)
                                        dltdata->test_counter_function[7]++;

                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_RAWD) {
                                        DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                        length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                        /* Size of the truncated message after headers */
                                        if (length == DLT_USER_BUF_MAX_SIZE - 41 - sizeof(uint16_t) - sizeof(uint32_t))
                                            dltdata->test_counter_function[7]++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* check test 9f */
        if (strcmp(text, "Test 9: (Function IF) Test segmented network trace") == 0) {
            printf("Test9f: (Function IF) Test segmented network trace\n");
            dltdata->running_test = 18;
            dltdata->test_counter_function[8] = 0;
        }
        else if (strcmp(text, "Test9: (Function IF) finished") == 0)
        {
            /* (Interface types) * (number of messages per complete message) */
            if (dltdata->test_counter_function[8] == 4 * 35) {
                printf("Test9f PASSED\n");
                dltdata->tests_passed++;
            }
            else {
                printf("Test9f FAILED\n");
                dltdata->tests_failed++;
            }

            dltdata->running_test = 0;
        }
        else if (dltdata->running_test == 18)
        {
            if (DLT_IS_HTYP_UEH(message->standardheader->htyp)) {
                if ((DLT_GET_MSIN_MSTP(message->extendedheader->msin)) == DLT_TYPE_NW_TRACE) {
                    /* Check message type information*/
                    /* Each correct message type increases the counter by 1 */
                    mtin = DLT_GET_MSIN_MTIN(message->extendedheader->msin);

                    if (mtin == DLT_NW_TRACE_IPC)
                        dltdata->test_counter_function[8]++;

                    if (mtin == DLT_NW_TRACE_CAN)
                        dltdata->test_counter_function[8]++;

                    if (mtin == DLT_NW_TRACE_FLEXRAY)
                        dltdata->test_counter_function[8]++;

                    if (mtin == DLT_NW_TRACE_MOST)
                        dltdata->test_counter_function[8]++;

                    /* Payload for first segmented message */
                    if (message->extendedheader->noar == 6) {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* NWST */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_START) == 0)
                                dltdata->test_counter_function[8]++;

                            /* Streahandle */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_UINT) {
                                uint32_t handle;
                                DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                handle = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                if (handle > 0)
                                    dltdata->test_counter_function[8]++;

                                /* Header */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_RAWD) {
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                    /* Test packet header size 16 */
                                    if (length == 16)
                                        dltdata->test_counter_function[8]++;

                                    /* Skip data */
                                    ptr += length;
                                    datalength -= length;

                                    /* Payload size */
                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_UINT) {
                                        uint32_t pl_sz;
                                        DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                        pl_sz = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                        /* Test packet payload size. */
                                        if (pl_sz == 5120)
                                            dltdata->test_counter_function[8]++;

                                        /* Segmentcount */
                                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                        if (type_info & DLT_TYPE_INFO_UINT) {
                                            uint16_t scount;
                                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                            scount = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                            /* Test packet segment count 5 */
                                            if (scount == 5)
                                                dltdata->test_counter_function[8]++;

                                            /* Segment length */
                                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                            if (type_info & DLT_TYPE_INFO_UINT) {
                                                uint16_t slen;
                                                DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                                slen = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                                /* Default segment size 1024 */
                                                if (slen == 1024)
                                                    dltdata->test_counter_function[8]++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    /* Data segment */
                    else if (message->extendedheader->noar == 4)
                    {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* NWCH */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_SEGMENT) == 0)
                                dltdata->test_counter_function[8]++;

                            /* handle */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_UINT) {
                                uint32_t handle;
                                DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                handle = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                if (handle > 0)
                                    dltdata->test_counter_function[8]++;

                                /* Sequence */
                                DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                if (type_info & DLT_TYPE_INFO_UINT) {
                                    /*uint16_t seq; */
                                    DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                    /*seq=DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp); */
                                    dltdata->test_counter_function[8]++;

                                    /* Data */
                                    DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                                    type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                                    if (type_info & DLT_TYPE_INFO_RAWD) {
                                        DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                                        length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);

                                        /* Segment size by default, 1024 */
                                        if (length == 1024)
                                            dltdata->test_counter_function[8]++;
                                    }
                                }
                            }
                        }
                    }
                    /* End segment */
                    else if (message->extendedheader->noar == 2)
                    {
                        /* verbose mode */
                        type_info = 0;
                        type_info_tmp = 0;
                        length = 0, length_tmp = 0; /* the macro can set this variable to -1 */

                        ptr = message->databuffer;
                        datalength = (int32_t) message->datasize;

                        /* NWEN */
                        DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                        type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                        if (type_info & DLT_TYPE_INFO_STRG) {
                            char chdr[10];
                            DLT_MSG_READ_VALUE(length_tmp, ptr, datalength, uint16_t);
                            length = DLT_ENDIAN_GET_16(message->standardheader->htyp, length_tmp);
                            DLT_MSG_READ_STRING(chdr, ptr, datalength, (int)sizeof(chdr), length);

                            if (strcmp((char *)chdr, DLT_TRACE_NW_END) == 0)
                                dltdata->test_counter_function[8]++;

                            /* handle */
                            DLT_MSG_READ_VALUE(type_info_tmp, ptr, datalength, uint32_t);
                            type_info = DLT_ENDIAN_GET_32(message->standardheader->htyp, type_info_tmp);

                            if (type_info & DLT_TYPE_INFO_UINT) {
                                uint32_t handle;
                                DLT_MSG_READ_VALUE(length_tmp32, ptr, datalength, uint32_t);
                                handle = DLT_ENDIAN_GET_32(message->standardheader->htyp, length_tmp32);

                                if (handle > 0)
                                    dltdata->test_counter_function[8]++;
                            }
                        }
                    }
                }
            }
        }

        if (strcmp(text, "Tests finished") == 0) {
            printf("Tests finished\n");
            dltdata->running_test = 1;

            printf("%d tests passed\n", dltdata->tests_passed);
            printf("%d tests failed\n", dltdata->tests_failed);

            if (dltdata->sock != -1)
                close(dltdata->sock);

            g_testsFailed = dltdata->tests_failed;

            return 0;
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
                printf("dlt_testclient_message_callback, error in: writev(dltdata->ohandle, iov, 2)\n");
                return -1;
            }
        }
    }

    return 0;
}
