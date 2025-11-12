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
 * \author
 * Stefan Held <stefan_held@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_common.cpp
 */

#include <stdio.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <syslog.h>

#define MAX_LINE 200
#define BINARY_FILE_V2_NAME "/testfile-v2.dlt"
#define FILTER_FILE_NAME "/testfilter.txt"

extern "C"
{
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_user_cfg.h"
#include "dlt_version.h"
#include "dlt_client.h"
#include "dlt_protocol.h"

int dlt_buffer_increase_size(DltBuffer *);
int dlt_buffer_minimize_size(DltBuffer *);
int dlt_buffer_reset(DltBuffer *);
DltReturnValue dlt_buffer_push(DltBuffer *, const unsigned char *, unsigned int);
DltReturnValue dlt_buffer_push3(DltBuffer *,
                                const unsigned char *,
                                unsigned int,
                                const unsigned char *,
                                unsigned int,
                                const unsigned char *,
                                unsigned int);
int dlt_buffer_get(DltBuffer *, unsigned char *, int, int);
int dlt_buffer_pull(DltBuffer *, unsigned char *, int);
int dlt_buffer_remove(DltBuffer *);
void dlt_buffer_status(DltBuffer *);
void dlt_buffer_write_block(DltBuffer *, int *, const unsigned char *, unsigned int);
void dlt_buffer_read_block(DltBuffer *, int *, unsigned char *, unsigned int);
void dlt_buffer_info(DltBuffer *);
}


/* Begin Method: dlt_common::dlt_message_init*/
TEST(t_dlt_message_init_v2, normal)
{
    DltMessageV2 msg;

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_message_init_v2(&msg, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free_v2(&msg, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_message_init_v2(&msg, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free_v2(&msg, 0));
}
TEST(t_dlt_message_init, abnormal)
{
/*    DltMessage msg; */

    /* Double use init, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg,0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init(&msg,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg,1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init(&msg,1)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg,1)); */

    /* set Verbose to 12345678, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init(&msg,12345678)); */
}
TEST(t_dlt_message_init, nullpointer)
{
    /*NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init(NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init(NULL, 1));
}
/* End Method: dlt_common::dlt_message_init*/


TEST(t_dlt_message_init_v2, nullpointer)
{
    /*NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init_v2(NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_init_v2(NULL, 1));
}
/* End Method: dlt_common::dlt_message_init*/



/* Begin Method: dlt_common::dlt_message_free */
TEST(t_dlt_message_free_v2, normal)
{
    DltMessageV2 msg;

    /* Normal Use Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_message_init_v2(&msg, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free_v2(&msg, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_message_init_v2(&msg, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free_v2(&msg, 1));
}



TEST(t_dlt_message_free_v2, nullpointer)
{
    /*NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free_v2(NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free_v2(NULL, 1));
}
/* End Method: dlt_common::dlt_message_free */


/* Begin Method: dlt_common::dlt_message_print_ascii*/
TEST(t_dlt_message_print_ascii_v2, normal)
{

    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


TEST(t_dlt_message_print_ascii_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(&file.msgv2, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(&file.msgv2, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_ascii*/


/* Begin Method: dlt_common::dlt_message_print_ascii with filter*/
TEST(t_dlt_message_print_ascii_with_filter_v2, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load_v2(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


/* Begin Method: dlt_common::dlt_message_print_header */
TEST(t_dlt_message_print_header_v2, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}

/* End Method: dlt_common::dlt_message_print_header */
TEST(t_dlt_message_print_header_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(&file.msgv2, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(&file.msgv2, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 1));

}
/* End Method: dlt_common::dlt_message_print_header */


/* Begin Method: dlt_common::dlt_message_print_header with filter */
TEST(t_dlt_message_print_header_with_filter_v2, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load_v2(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


/* Begin Method: dlt_common::dlt_message_print_hex */
TEST(t_dlt_message_print_hex_v2, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}

TEST(t_dlt_message_print_hex_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(&file.msgv2, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(&file.msgv2, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_hex */


/* Begin Method: dlt_common::dlt_message_print_hex with filter */
TEST(t_dlt_message_print_hex_with_filter_v2, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load_v2(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}

/* Begin Method: dlt_common::dlt_message_print_mixed_plain */
TEST(t_dlt_message_print_mixed_plain_v2, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


TEST(t_dlt_message_print_mixed_plain_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(&file.msgv2, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(&file.msgv2, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_mixed_pain */


/* Begin Method: dlt_common::dlt_message_print_mixed_plain with filter */
TEST(t_dlt_message_print_mixed_plain_with_filter_v2, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load_v2(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


/* Begin Method:dlt_common::dlt_message_filter_check */
TEST(t_dlt_message_filter_check_v2, normal)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected > 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load_v2(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_filter_check_v2(&file.msgv2, &filter, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_filter_check_v2(&file.msgv2, &filter, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}

TEST(t_dlt_message_filter_check_v2, nullpointer)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check_v2(NULL, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check_v2(NULL, NULL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check_v2(NULL, &filter, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check_v2(NULL, &filter, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check_v2(&file.msgv2, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check_v2(&file.msgv2, NULL, 1));
}
/* End Method:dlt_common::dlt_message_filter_check */


/* Begin Method:dlt_common::dlt_message _get_extraparameters */
TEST(t_dlt_message_get_extraparamters_v2, normal)
{
    DltFile file;

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect >0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_get_extraparameters_v2(&file.msgv2, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_get_extraparameters_v2(&file.msgv2, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


/* Begin Method:dlt_common::dlt_message_header */
TEST(t_dlt_message_header_v2, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_header_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 0));
        printf("%s \n", text);
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_header_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 1));
        printf("%s \n", text);
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}

TEST(t_dlt_message_header_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(&file.msgv2, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(&file.msgv2, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(&file.msgv2, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_v2(&file.msgv2, text, 0, 1));
}
/* End Method:dlt_common::dlt_message_header */


/* Begin Method:dlt_common::dlt_message_header_flags */
TEST(t_dlt_message_header_flags_v2, normal)
{
    /* Possible Flags*/
    /*#define DLT_HEADER_SHOW_NONE       0x0000 */
    /*#define DLT_HEADER_SHOW_TIME       0x0001 */
    /*#define DLT_HEADER_SHOW_TMSTP      0x0002 */
    /*#define DLT_HEADER_SHOW_MSGCNT     0x0004 */
    /*#define DLT_HEADER_SHOW_ECUID      0x0008 */
    /*#define DLT_HEADER_SHOW_APID       0x0010 */
    /*#define DLT_HEADER_SHOW_CTID       0x0020 */
    /*#define DLT_HEADER_SHOW_MSGTYPE    0x0040 */
    /*#define DLT_HEADER_SHOW_MSGSUBTYPE 0x0080 */
    /*#define DLT_HEADER_SHOW_VNVSTATUS  0x0100 */
    /*#define DLT_HEADER_SHOW_NOARG      0x0200 */
    /*#define DLT_HEADER_SHOW_ALL        0xFFFF */
    /*########################################*/


    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
        printf("%s \n", text);
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
        printf("%s \n", text);
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


TEST(t_dlt_message_header_flags_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, 0, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, 0, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, 0, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags_v2(&file.msgv2, text, 0, DLT_HEADER_SHOW_ALL, 1));
}
/* End Method:dlt_common::dlt_message_header_flags */


/* Begin Method:dlt_common::dlt_message_payload */
TEST(t_dlt_message_payload_v2, normal)
{
    /* Types */
    /*#define DLT_OUTPUT_HEX              1 */
    /*#define DLT_OUTPUT_ASCII            2 */
    /*#define DLT_OUTPUT_MIXED_FOR_PLAIN  3 */
    /*#define DLT_OUTPUT_MIXED_FOR_HTML   4 */
    /*#define DLT_OUTPUT_ASCII_LIMITED    5 */
    /*####################################*/

    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
        printf("%s \n", text);
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
        printf("%s \n", text);
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}

TEST(t_dlt_message_payload_v2, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[MAX_LINE];
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char  openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];;
    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Uninizialised, expected -1 */
    memset(&file, 0x00, sizeof(DltFile));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));

    /* USE own DLT_HEADER_SHOW , expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 99, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, DLT_DAEMON_TEXTSIZE, 99, 0));
        printf("%s \n",text);
    }
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}

/* End Method:dlt_common::dlt_message_payload */

TEST(t_dlt_message_payload_v2, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, 0, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload_v2(&file.msgv2, text, 0, DLT_OUTPUT_ASCII_LIMITED, 1));

    /* file.msg is not initialised which causes problems when textsize is > 0 but */
    /* we don't have text: */
    /* dlt_common.c line 943: ptr = msg->databuffer; */
    /* (gdb) p ptr */
    /*    $28 = (uint8_t *) 0x5124010337d46c00 <error: Cannot access memory at address 0x5124010337d46c00> */

/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0, 1)); */
}
/* End Method:dlt_common::dlt_message_payload */


/* Begin Method:dlt_common::dlt_message_set_extraparameters */
TEST(t_dlt_message_set_extraparamters_v2, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_set_extraparameters_v2(&file.msgv2, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_set_extraparameters_v2(&file.msgv2, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));
}


/* Begin Method:dlt_common::dlt_message_read */
TEST(t_dlt_message_read_v2, normal)
{
    DltFile file;
    /* Get PWD so file can be used */
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    DltBuffer buf;
    char *buffer = NULL;

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_ERROR, dlt_message_read_v2(&file.msgv2, (unsigned char *)buffer, 255, 0, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_ERROR, dlt_message_read_v2(&file.msgv2, (unsigned char *)buffer, 255, 1, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}

/* End Method:dlt_common::dlt_message_read */
TEST(t_dlt_message_read_v2, nullpointer)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    /*---------------------------------------*/

    DltBuffer buf;

    /* NULL_Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read_v2(NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read_v2(NULL, (uint8_t *)&buf, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read_v2(&file.msgv2, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read_v2(&file.msgv2, (uint8_t *)&buf, 0, 0, 0));

}
/* End Method:dlt_common::dlt_message_read_v2 */


/* Begin Method:dlt_common::dlt_message_argument_print */
TEST(t_dlt_message_argument_print_v2, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    static char text[DLT_DAEMON_TEXTSIZE];
    /*---------------------------------------*/
    uint8_t *ptr;
    int32_t datalength;
    uint8_t **pptr;
    int32_t *pdatalength;

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        ptr = file.msg.databuffer;
        datalength = file.msg.datasize;
        pptr = &ptr;
        pdatalength = &datalength;
        EXPECT_GE(DLT_RETURN_OK,
                  dlt_message_argument_print_v2(&file.msgv2, DLT_TYPE_INFO_BOOL, pptr, pdatalength, text,
                                             DLT_DAEMON_TEXTSIZE, 0, 1));
        /*printf("### ARGUMENT:%s\n", text); */
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_file_init_v2(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        ptr = file.msg.databuffer;
        datalength = file.msg.datasize;
        pptr = &ptr;
        pdatalength = &datalength;
        EXPECT_GE(DLT_RETURN_OK,
                  dlt_message_argument_print_v2(&file.msgv2, DLT_TYPE_INFO_RAWD, pptr, pdatalength, text,
                                             DLT_DAEMON_TEXTSIZE, 0, 1));
        /*printf("### ARGUMENT:%s\n", text); */
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free_v2(&file, 0));

}


TEST(t_dlt_message_argument_print_v2, nullpointer)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_V2_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_V2_NAME, pwd);
    static char text[DLT_DAEMON_TEXTSIZE];
    /*---------------------------------------*/
    uint8_t *ptr;
    int32_t datalength;
    uint8_t **pptr;
    int32_t *pdatalength;
    pptr = &ptr;
    pdatalength = &datalength;

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, NULL, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, NULL, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, NULL, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, NULL, pdatalength, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, pptr, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, pptr, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, pptr, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(NULL, 0, pptr, pdatalength, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, NULL, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, NULL, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, NULL, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, NULL, pdatalength, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, pptr, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, pptr, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, pptr, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print_v2(&file.msgv2, 0, pptr, pdatalength, text, 0, 0, 0));
}
/* End Method:dlt_common::dlt_message_argument_print_v2 */



/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/




int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_break_on_failure = true;
    /*::testing::FLAGS_gtest_filter = "*.normal"; */
    return RUN_ALL_TESTS();
}
