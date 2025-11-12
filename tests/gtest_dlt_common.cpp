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
#define BINARY_FILE_NAME "/testfile.dlt"
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


/* Begin Method: dlt_common::dlt_buffer_init_dynamic */
TEST(t_dlt_buffer_init_dynamic, normal)
{
    DltBuffer init_dynamic;

    /* Normal Use-Case for initializing a buffer */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&init_dynamic, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&init_dynamic));

    /* Min Values for a success init */
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&init_dynamic, 12, 12, 12));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&init_dynamic));
}
TEST(t_dlt_buffer_init_dynamic, abnormal)
{
/*    DltBuffer buf; */

    /* Initialze buffer twice, expected -1 for second init */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*  EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_free_dynamic(&buf)); */

    /* Initialize buffer with max-value of uint32, expected 0 */
    /* TODO: what should the maximum parameter values be? UINT_MAX is too large and leads to segfault */
/*  EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, UINT_MAX,UINT_MAX,UINT_MAX)); */
/*  EXPECT_LE(DLT_RETURN_OK,dlt_buffer_free_dynamic(&buf)); */

    /* Initialize buffer with min-value of uint32, expected 0 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, 0,0,0)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_free_dynamic(&buf)); */

    /* Initialize buffer min-value > max-value, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_free_dynamic(&buf)); */

    /* Initialsize buffer step-value > max-value, expected -1 */
/*    EXPECT_GE(-1,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE * 2)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_init_dynamic, nullpointer)
{
    DltBuffer buf;

    /* NULL-Pointer, expect -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(NULL, 0, 0, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(NULL, 0, DLT_USER_RINGBUFFER_MAX_SIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(NULL, 0, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, 0, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(&buf, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(&buf, 0, 0, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(&buf, 0, DLT_USER_RINGBUFFER_MAX_SIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(&buf, 0, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, 0, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, 0));
}
/* End Method: dlt_common::dlt_buffer_init_dynamic */




/* Begin Method: dlt_common::dlt_buffer_free_dynamic */
TEST(t_dlt_buffer_free_dynamic, normal)
{
    DltBuffer buf;

    /* Normal Use-Case szenario */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case szenario */
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, 12, 12, 12));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_free_dynamic, abnormal)
{
/*    DltBuffer buf; */

    /* Free uninizialised buffer, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_free_dynamic(&buf)); */

    /* Free buffer twice, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_GE(-1, dlt_buffer_free_dynamic(&buf)); */

}
TEST(t_dlt_buffer_free_dynamic, nullpointer)
{
    /* NULL-POinter */
    EXPECT_GE(-1, dlt_buffer_free_dynamic(NULL));
}
/* End Method: dlt_common::dlt_buffer_free_dynamic */




/* Begin Method: dlt_common::dlt_buffer_increase_size */
TEST(t_dlt_buffer_increase_size, normal)
{
    DltBuffer buf;

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(0, dlt_buffer_increase_size(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Fill buffer to max-value, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));

    for (int i = 0;
         i <= (DLT_USER_RINGBUFFER_MAX_SIZE / DLT_USER_RINGBUFFER_MIN_SIZE);
         i += DLT_USER_RINGBUFFER_STEP_SIZE)
        EXPECT_LE(0, dlt_buffer_increase_size(&buf));

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_increase_size, abnormal)
{
/*    DltBuffer buf; */

    /* Increase uninizialised buffer */
/*    EXPECT_GE(-1, dlt_buffer_increase_size(&buf)); */

    /* Fill buffer over max-value, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(-1, dlt_buffer_increase_size(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* min-value > max-value, trying to increase buffer, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(-1, dlt_buffer_increase_size(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* trying to increase buffer with 0 , expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, 0)); */
/*    EXPECT_GE(-1, dlt_buffer_increase_size(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_increase_size, nullpointer)
{
    /* NULL-Pointer, expected -1 */
    EXPECT_GE(-1, dlt_buffer_increase_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_increase_size */




/* Begin Method: dlt_common::dlt_buffer_minimize_size */
TEST(t_dlt_buffer_minimize_size, normal)
{
    DltBuffer buf;

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(0, dlt_buffer_minimize_size(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* minimize buffer to min-value, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));

    for (int i = (DLT_USER_RINGBUFFER_MAX_SIZE / DLT_USER_RINGBUFFER_MIN_SIZE);
         i >= 0;
         i -= DLT_USER_RINGBUFFER_STEP_SIZE)
        EXPECT_LE(0, dlt_buffer_minimize_size(&buf));

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_minimize_size, abnormal)
{
/*    DltBuffer buf; */

    /* Minimize uninizialised buffer */
/*    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf)); */

    /* minimize buffer under min-value, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* min-value > max-value, trying to minimize buffer, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* trying to minimize buffer with 0 , expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, 0)); */
/*    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_minimize_size, nullpointer)
{
    /* NULL-Pointer, expected -1 */
    EXPECT_GE(-1, dlt_buffer_minimize_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_minimize_size */




/* Begin Method: dlt_common::dlt_buffer_reset */
TEST(t_dlt_buffer_reset, normal)
{
    DltBuffer buf;

    /* Normal Use-Case. expect 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(0, dlt_buffer_reset(&buf));

}
TEST(t_dlt_buffer_reset, abnormal)
{
/*    DltBuffer buf; */

    /*Use uninizialsied buffer, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_reset(&buf)); */
}
TEST(t_dlt_buffer_reset, nullpointer)
{
    /*Use NULL-Pointer, expected -1 */
    EXPECT_GE(-1, dlt_buffer_reset(NULL));
}
/* End Method: dlt_common::dlt_buffer_reset */




/* Begin Method: dlt_common::dlt_buffer_push*/
TEST(t_dlt_buffer_push, normal)
{
    DltBuffer buf;
    char *test;
    unsigned int size = sizeof(test);

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&test, size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Push till buffer is full, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));

    for (unsigned int i = 0; i <= (DLT_USER_RINGBUFFER_MIN_SIZE / (size + sizeof(DltBufferBlockHead))); i++)
    {
        EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&test, size));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_push, abnormal)
{
/*    DltBuffer buf; */
/*    char * test; */
/*    int size = sizeof(test); */

    /* Use uninizialsied, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_push(&buf,(unsigned char *)&test,size)); */


    /* set size == 0, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(&buf,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* set size == 0 and char == 0 expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(&buf,0,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* Push till buffer is overfilled , expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    for(int i=0; i<= (DLT_USER_RINGBUFFER_MIN_SIZE/size) + size; i++) */
/*    { */
/*        if(i <= DLT_USER_RINGBUFFER_MIN_SIZE) */
/*            EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf,(unsigned char *)&test,size)); */
/*        else */
/*            EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(&buf,(unsigned char *)&test,size)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* All use-case, wich works with null pointer, has to discuss */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(&buf,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(&buf,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

}
TEST(t_dlt_buffer_push, nullpointer)
{
    char *test;
    int size = sizeof(test);

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(NULL, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(NULL, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(NULL, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push(NULL, (unsigned char *)&test, size));
}
/* End Method: dlt_common::dlt_buffer_push*/




/* Begin Method: dlt_common::dlt_buffer_push3 */
TEST(t_dlt_buffer_push3, normal)
{
    DltBuffer buf;
    char *test;
    int size = sizeof(test);

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push3(&buf, (unsigned char *)&test, size, 0, 0, 0, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_push3(&buf, (unsigned char *)&test, size, (unsigned char *)&test, size, (unsigned char *)&test,
                               size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Push till buffer is full, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));

    for (unsigned int i = 0; i <= (DLT_USER_RINGBUFFER_MIN_SIZE / (size * 3 + sizeof(DltBufferBlockHead))); i++)
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_buffer_push3(&buf, (unsigned char *)&test, size, (unsigned char *)&test, size,
                                   (unsigned char *)&test,
                                   size));

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_push3, abnormal)
{
/*    DltBuffer buf; */
/*    char * test; */
/*    int size = sizeof(test); */

    /* Use uninizialsied, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size)); */


    /* set size == 0, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0, (unsigned char *)&test,0, (unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* set size == 0 and char == 0 expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,0,0,0,0,0,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* works with null pointer, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* Push till buffer is overfilled , expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    for(int i=0; i<= (DLT_USER_RINGBUFFER_MIN_SIZE/size) + size; i++) */
/*    { */
/*        if(i <= DLT_USER_RINGBUFFER_MIN_SIZE) */
/*            EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size)); */
/*        else */
/*            EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* All use-case, wich works with null pointer, has to discuss */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,0,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,0,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,size,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,NULL,size,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,0,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,0,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,size,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,0,(unsigned char *)&test,size,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,0,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,0,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,size,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,NULL,size,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,0,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,0,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,0,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,0,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,size,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,NULL,size,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,(unsigned char *)&test,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,(unsigned char *)&test,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,(unsigned char *)&test,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,0,(unsigned char *)&test,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,0,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,0,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,(unsigned char *)&test,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,(unsigned char *)&test,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,0,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,0,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,NULL,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,NULL,size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_push3, nullpointer)
{
    char *test;
    int size = sizeof(test);

    /*Null Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, 0, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, size, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, NULL, size, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, 0, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, size, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, 0, (unsigned char *)&test, size, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, 0, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, size, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, NULL, size, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, 0, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, size, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, NULL, size, (unsigned char *)&test, size, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, 0, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, size, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, NULL, size, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, 0, (unsigned char *)&test,
                               size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, size, (unsigned char *)&test,
                               0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, 0, (unsigned char *)&test, size, (unsigned char *)&test,
                               size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, 0, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, 0, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, size, (unsigned char *)&test, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, NULL, size, (unsigned char *)&test, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, 0, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, 0, (unsigned char *)&test,
                               0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, 0, (unsigned char *)&test,
                               size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, size, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, size, NULL, size));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, size, (unsigned char *)&test,
                               0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_buffer_push3(NULL, (unsigned char *)&test, size, (unsigned char *)&test, size, (unsigned char *)&test,
                               size));
}
/* End Method: dlt_common::dlt_buffer_push3 */




/* Begin Method: dlt_common::dlt_buffer_pull */
TEST(t_dlt_buffer_pull, normal)
{
    /*Normal Use Cases, expected 0 or -1 in return */
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    /* Normal Use-Case, empty pull, expected -1 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, (unsigned char *)&header, size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case, expected > 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
    EXPECT_LE(1, dlt_buffer_pull(&buf, (unsigned char *)&header, size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_pull, abnormal)
{
/*    DltBuffer buf; */
/*    DltUserHeader header; */

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_pull(&buf, (unsigned char*)&header, sizeof(DltUserHeader))); */

    /* data == 0 and max_size == 0, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader))); */
/*    EXPECT_GE(-1, dlt_buffer_pull(&buf, 0, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* no push before pull, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(-1, dlt_buffer_pull(&buf, 0, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_pull, nullpointer)
{
    DltBuffer buf;
    DltUserHeader header;

    /* NULL-Point, expected -1 */
    EXPECT_GE(-1, dlt_buffer_pull(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_buffer_pull(NULL, NULL, sizeof(DltUserHeader)));
    EXPECT_GE(-1, dlt_buffer_pull(NULL, (unsigned char *)&header, 0));
    EXPECT_GE(-1, dlt_buffer_pull(NULL, (unsigned char *)&header, sizeof(DltUserHeader)));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, NULL, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, NULL, sizeof(DltUserHeader)));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
/* End Method: dlt_common::dlt_buffer_pull */



/* Begin Method: dlt_common::dlt_buffer_remove */
TEST(t_dlt_buffer_remove, normal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    /* Normal Use-Case, empty pull, expected -1 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_remove(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case, expected > 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, size));
    EXPECT_LE(0, dlt_buffer_remove(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_remove, abnormal)
{
/*    DltBuffer buf; */
/*    DltUserHeader header; */
/*    int size = sizeof(DltUserHeader); */

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_remove(&buf)); */

    /* no push before remove, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_GE(-1, dlt_buffer_remove(&buf)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */

    /* Call remove 10 time, expected > 1 till buffer is empty */
    /* pushed one time so expect one > 1 and 9 times < 0 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf,(unsigned char *)&header,size)); */
/*    for(int i=0; i<10;i++) */
/*    { */
/*        if(i == 0) */
/*            EXPECT_LE(1, dlt_buffer_remove(&buf)); */
/*        else */
/*            EXPECT_GE(-1, dlt_buffer_remove(&buf)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_remove, nullpointer)
{
    /* NULL_Pointer, expected -1 */
    EXPECT_GE(-1, dlt_buffer_remove(NULL));
}
/* End Method: dlt_common::dlt_buffer_remove*/




/* Begin Method: dlt_common::dlt_buffer_copy */
TEST(t_dlt_buffer_copy, normal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    /* Normal Use-Case, empty pull, expected -1 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_copy(&buf, (unsigned char *)&header, size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case, expected > 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
    EXPECT_LE(1, dlt_buffer_copy(&buf, (unsigned char *)&header, size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_copy, abnormal)
{
/*    DltBuffer buf; */
/*    DltUserHeader header; */
/*    int size = sizeof(DltUserHeader); */

    /* Uninizialised buffer , expected -1 */
/*    EXPECT_LE(-1, dlt_buffer_copy(&buf, (unsigned char *)&header, size)); */

    /* no push before copy, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(-1, dlt_buffer_copy(&buf, (unsigned char *)&header, size)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_copy, nullpointer)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    /* NULL-Pointer, expected -1 */
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_buffer_copy(NULL, NULL, size));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_buffer_copy(NULL, NULL, 0));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_buffer_copy(NULL, (unsigned char *)&header, size));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_buffer_copy(NULL, (unsigned char *)&header, 0));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_buffer_copy(&buf, NULL, size));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_WRONG_PARAMETER, dlt_buffer_copy(&buf, NULL, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
/* End Method: dlt_common::dlt_buffer_copy */



/* Begin Method: dlt_common::dlt_buffer_get */

TEST(t_dlt_buffer_get, normal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    /* Normal Use-Case */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, size));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    EXPECT_LE(0, dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, size));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    EXPECT_LE(0, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[0] = 50000;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[1] = 50000;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[2] = -50000;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[2] = 0;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[0] = 4000;
    ((int *)(buf.shm))[1] = 5000;
    ((int *)(buf.shm))[2] = 0;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[0] = 10;
    ((int *)(buf.shm))[1] = 5;
    ((int *)(buf.shm))[2] = 5;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[2] = 50000;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, size));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[0] = 19;
    EXPECT_GE(-1, dlt_buffer_get(&buf, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, size));
    printf("#### %i\n", dlt_buffer_get(&buf, (unsigned char *)&header, size, 0));
    ((int *)(buf.shm))[2] = 19;
    EXPECT_LE(0, dlt_buffer_get(&buf, (unsigned char *)&header, 5, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_get, abnormal)
{
/*    DltBuffer buf; */
/*    DltUserHeader header; */
/*    int size = sizeof(DltUserHeader); */

    /* Uninizialsied, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_get(&buf,(unsigned char *)&header,size, 0)); */

    /* Integer with 12345678 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf,(unsigned char *)&header,size)); */
/*    printf("#### %i\n", dlt_buffer_get(&buf,(unsigned char*)&header,size,0)); */
/*    EXPECT_LE(0, dlt_buffer_get(&buf,(unsigned char*)&header,size,12345678)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf)); */
}
TEST(t_dlt_buffer_get, nullpointer)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    /* NULL-Pointer */
    EXPECT_GE(-1, dlt_buffer_get(NULL, NULL, 0, 0));
    EXPECT_GE(-1, dlt_buffer_get(NULL, NULL, 0, 1));
    EXPECT_GE(-1, dlt_buffer_get(NULL, NULL, size, 0));
    EXPECT_GE(-1, dlt_buffer_get(NULL, NULL, size, 1));
    EXPECT_GE(-1, dlt_buffer_get(NULL, (unsigned char *)&header, 0, 0));
    EXPECT_GE(-1, dlt_buffer_get(NULL, (unsigned char *)&header, 0, 1));
    EXPECT_GE(-1, dlt_buffer_get(NULL, (unsigned char *)&header, size, 0));
    EXPECT_GE(-1, dlt_buffer_get(NULL, (unsigned char *)&header, size, 1));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_get(&buf, NULL, 0, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_get(&buf, NULL, 0, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_get(&buf, NULL, size, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_get(&buf, NULL, size, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
/* End Method: dlt_common::dlt_buffer_get */




/* Begin MEthod: dlt_common::dlt_buffer_get_message_count */
TEST(t_dlt_buffer_get_message_count, normal)
{
    DltBuffer buf;
    DltUserHeader header;

    /* Normal Usce-Case without pushing data, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    /*printf("##### %i\n", dlt_buffer_get_message_count(&buf)); */
    EXPECT_EQ(0, dlt_buffer_get_message_count(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case, with pushing data, expected 1 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
    /*printf("#### %i\n", dlt_buffer_get_message_count(&buf)); */
    EXPECT_EQ(1, dlt_buffer_get_message_count(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Pushing DLT_USER_RINGBUFFER_MIN_SIZE / (sizeof(DltUserHeader) + sizeof(DltBufferBlockHead)) mesages */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));

    for (unsigned int i = 1; i <= DLT_USER_RINGBUFFER_MIN_SIZE / (sizeof(DltUserHeader) + sizeof(DltBufferBlockHead)); i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
        /*printf("#### %i\n", dlt_buffer_get_message_count(&buf)); */
        EXPECT_EQ(i, dlt_buffer_get_message_count(&buf));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

}
TEST(t_dlt_buffer_get_message_count, abnormal)
{
/*    DltBuffer buf; */

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_get_message_count(&buf)); */
}
TEST(t_dlt_buffer_get_message_count, nullpointer)
{
    /*NULL-Pointer, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_get_message_count(NULL)); */
}
/* Begin MEthod: dlt_common::dlt_buffer_get_message_count */




/* Begin Method: dlt_common::dlt_buffer_get_total_size*/
TEST(t_dlt_buffer_get_total_size, normal)
{
    DltBuffer buf;
    DltUserHeader header;

    /* Normal Use-Case, expected max buffer size (DLT_USER_RINGBUFFER_MAX_SIZE) */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    /*printf("##### %i\n", dlt_buffer_get_total_size(&buf)); */
    EXPECT_LE(DLT_USER_RINGBUFFER_MAX_SIZE, dlt_buffer_get_total_size(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case, 1st pushing data, expected max buffer size (DLT_USER_RINGBUFFER_MAX_SIZE) */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
    /*printf("##### %i\n", dlt_buffer_get_total_size(&buf)); */
    EXPECT_LE(DLT_USER_RINGBUFFER_MAX_SIZE, dlt_buffer_get_total_size(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_get_total_size, abnormal)
{
/*    DltBuffer buf; */

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_get_total_size(&buf)); */
}
TEST(t_dlt_buffer_get_total_size, nullpointer)
{
    /* NULL-Pointer, expect -1 */
    EXPECT_GE(-1, dlt_buffer_get_total_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_get_total_size*/



/* Begin Method: dlt_common::dlt_buffer_get_used_size*/
TEST(t_dlt_buffer_get_used_size, normal)
{
    DltBuffer buf;
    DltUserHeader header;
    int sum = 0;

    /* Normal Use Cas buffer empty, expected 0 */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    /*printf("##### %i\n", dlt_buffer_get_used_size(&buf)); */
    EXPECT_EQ(0, dlt_buffer_get_used_size(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case with pushing data, expected sum of DltUserHeader and DltBufferBlockHead */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
    /*printf("##### %i\n", dlt_buffer_get_used_size(&buf)); */
    EXPECT_EQ(sizeof(DltUserHeader) + sizeof(DltBufferBlockHead), dlt_buffer_get_used_size(&buf));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    /* Normal Use-Case with pushing DLT_USER_RINGBUFFER_MIN_SIZE / (sizeof(DltUserHeader) + sizeof(DltBufferBlockHead)) data */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));

    for (unsigned int i = 1; i <= DLT_USER_RINGBUFFER_MIN_SIZE / (sizeof(DltUserHeader) + sizeof(DltBufferBlockHead)); i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_buffer_push(&buf, (unsigned char *)&header, sizeof(DltUserHeader)));
        /*printf("#### %i\n", dlt_buffer_get_used_size(&buf)); */
        sum += sizeof(DltUserHeader) + sizeof(DltBufferBlockHead);
        EXPECT_EQ(sum, dlt_buffer_get_used_size(&buf));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_get_used_size, abnormal)
{
/*    DltBuffer buf; */

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(-1, dlt_buffer_get_used_size(&buf)); */
}
TEST(t_dlt_buffer_get_used_size, nullpointer)
{
    /*NULL-Pointer, expcted -1 */
    EXPECT_GE(-1, dlt_buffer_get_used_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_get_used_size*/




/* Begin Method: dlt_common::dlt_buffer_write_block */
TEST(t_dlt_buffer_write_block, normal)
{
    DltBuffer buf;
    unsigned char *data = NULL;
    int write;
    int size1 = 516;
    int size2 = 1024;

    /* Normal Use-Case, void method, expected no error */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, data, size1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, data, size2));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    int tmp = 0;

    for (int i = 0; i <= 10000; i += 10) {
        tmp += i;
        EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, data, i));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_write_block, abnormal)
{
    /* Boundary check of write position */
    DltBuffer buf;
    const char *data = "data";
    int write = DLT_USER_RINGBUFFER_MIN_SIZE;
    write -= sizeof(DltBufferHead);
    int size = sizeof(data);
    // when write = buf->size, it should not throw any warning
    // and write should equal to size.
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf,DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, (unsigned char *)&data, size));
    EXPECT_EQ(size , write);
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_write_block, nullpointer)
{
    DltBuffer buf;
    char *data;
    int write;
    int test1 = 1000;

    /* NULL-Pointer, expected < 0 */
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, NULL, NULL, 0));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, NULL, NULL, test1));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, NULL, (unsigned char *)&data, 0));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, NULL, (unsigned char *)&data, test1));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, &write, NULL, 0));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, &write, NULL, test1));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, &write, (unsigned char *)&data, 0));
    EXPECT_NO_THROW(dlt_buffer_write_block(NULL, &write, (unsigned char *)&data, test1));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, NULL, NULL, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, NULL, NULL, test1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, NULL, (unsigned char *)&data, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, NULL, (unsigned char *)&data, test1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, NULL, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, NULL, test1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
/* End Method: dlt_common::dlt_buffer_write_block */




/* Begin Method: dlt_common::dlt_buffer_read_block */
TEST(t_dlt_buffer_read_block, normal)
{
    DltBuffer buf;
    unsigned char *data = NULL;
    int write, read;
    int size1 = 516;
    int size2 = 1024;

    /* Normal Use-Case, void method, expected no error */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &write, data, size1));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, &write, data, size1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_write_block(&buf, &read, data, size2));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, &write, data, size2));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_read_block, abnormal)
{
    /* Boundary check of read position */
    DltBuffer buf;
    /* Buffer to read data from DltBuffer */
    unsigned char *data_read;
    data_read = (unsigned char *) calloc(1000, sizeof(char));
    int read = DLT_USER_RINGBUFFER_MIN_SIZE;
    read -= sizeof(DltBufferHead);
    int size = 1000;
    // when read = buf->size, it should not throw any warning
    // and read position should equal to size.
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, &read, data_read, size));
    EXPECT_EQ(size,read);
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    free(data_read);
}
TEST(t_dlt_buffer_read_block, nullpointer)
{
    DltBuffer buf;
    char *data;
    int read = -1;
    int test1 = 1000;

    /* NULL-Pointer, expected < 0 */
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, NULL, NULL, 0));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, NULL, NULL, test1));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, NULL, (unsigned char *)&data, 0));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, NULL, (unsigned char *)&data, test1));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, &read, NULL, 0));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, &read, NULL, test1));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, &read, (unsigned char *)&data, 0));
    EXPECT_NO_THROW(dlt_buffer_read_block(NULL, &read, (unsigned char *)&data, test1));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, NULL, NULL, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, NULL, NULL, test1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, NULL, (unsigned char *)&data, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, NULL, (unsigned char *)&data, test1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, &read, NULL, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, &read, NULL, test1));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_read_block(&buf, &read, (unsigned char *)&data, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
/* End Method: dlt_common::dlt_buffer_read_block */




/* Begin Method: dlt_common::dlt_buffer_info */
TEST(t_dlt_buffer_info, normal)
{
    DltBuffer buf;

    /* Normal Use-Case */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_info(&buf));
}
TEST(t_dlt_buffer_info, abnormal)
{
    /* actually no abnormal test cases */
    /* because of void function and missing gtest tools for that */
}
TEST(t_dlt_buffer_info, nullpointer)
{
    /* NULL-Pointer, no throw */
    EXPECT_NO_THROW(dlt_buffer_info(NULL));
}
/* End Method: dlt_common::dlt_buffer_info */




/* Begin Method: dlt_common::dlt_buffer_status */
TEST(t_dlt_buffer_status, normal)
{
    DltBuffer buf;

    /* Normal Use-Case */
    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_NO_THROW(dlt_buffer_status(&buf));
}
TEST(t_dlt_buffer_status, abnormal)
{
    /* actual no abnormal test cases */
    /* because of void funktion and missing gtest tools for that */
}
TEST(t_dlt_buffer_status, nullpointer)
{
    /* NULL-Pointer, expected -1 */
    EXPECT_NO_THROW(dlt_buffer_status(NULL));
}
/* End Method: dlt_common::dlt_buffer_status */




/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/




/* Begin Method: dlt_common::dlt_message_init*/
TEST(t_dlt_message_init, normal)
{
    DltMessage msg;

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg, 0));
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




/* Begin Method: dlt_common::dlt_message_free */
TEST(t_dlt_message_free, normal)
{
    DltMessage msg;

    /* Normal Use Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg, 1));
}
TEST(t_dlt_message_free, abnormal)
{
/*    DltMessage msg; */

    /* Double use free, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg,0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free(&msg,0)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_init(&msg,0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_message_free(&msg,1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free(&msg,1)); */

    /* set Verbose to 12345678, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free(&msg,12345678)); */
}
TEST(t_dlt_message_free, nullpointer)
{
    /*NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free(NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_free(NULL, 1));
}
/* End Method: dlt_common::dlt_message_free */




/* Begin Method: dlt_common::dlt_file_open */
TEST(t_dlt_file_open, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 1));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_file_open, abnormal)
{
/*    DltFile file; */
/*    / * Get PWD so file can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)]; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* Uninizialsied, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(&file, openfile, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(&file, openfile, 1)); */

    /* Verbose set to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(&file, openfile, 12345678)); */

    /* Path doesn't exist, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(&file, "This Path doesn't exist!!", 0)); */
}
TEST(t_dlt_file_open, nullpointer)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(NULL, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(NULL, NULL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(NULL, openfile, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(NULL, openfile, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(&file, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_file_open(&file, NULL, 1));
}
/* End Method: dlt_common::dlt_file_open */




/* Begin Method: dlt_common::dlt_file_quick_parsing */
TEST(t_dlt_file_quick_parsing, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];
    char output[128] = "/tmp/output_testfile.txt";

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_quick_parsing(&file, output, DLT_OUTPUT_ASCII, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
    unlink(output);
}

TEST(t_dlt_file_quick_parsing, abnormal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];
    char output[128] = "/tmp/output_testfile.txt";

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Abnormal Use-Case, expected DLT_RETURN_WRONG_PARAMETER (-5) */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));
    EXPECT_GE(DLT_RETURN_WRONG_PARAMETER, dlt_file_quick_parsing(&file, NULL, DLT_OUTPUT_ASCII, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));
    EXPECT_GE(DLT_RETURN_WRONG_PARAMETER, dlt_file_quick_parsing(NULL, output, DLT_OUTPUT_ASCII, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
/* End Method: dlt_common::dlt_file_quick_parsing */




/* Begin Method: dlt_common::dlt_message_print_ascii*/
TEST(t_dlt_message_print_ascii, normal)
{

    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_ascii, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

/*    / * Get PWD so file and filter can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)]; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* No messages read, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1)); */

    /* Set verbose to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_print_ascii, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_ascii(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_ascii*/




/* Begin Method: dlt_common::dlt_message_print_ascii with filter*/
TEST(t_dlt_message_print_ascii_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_ascii_with_filter, abnormal)
{
    /* equal with t_dlt_message_print_ascii */
}
TEST(t_dlt_message_print_ascii_with_filter, nullpointer)
{
    /* equal with t_dlt_message_print_ascii */
}
/* End Method: dlt_common::dlt_message_print_ascii with filter*/




/* Begin Method: dlt_common::dlt_message_print_header */
TEST(t_dlt_message_print_header, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_header, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

/*    / * Get PWD so file and filter can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* No messages read, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1)); */

    /* Set verbose to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_print_header, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));

}
/* End Method: dlt_common::dlt_message_print_header */




/* Begin Method: dlt_common::dlt_message_print_header with filter */
TEST(t_dlt_message_print_header_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_header_with_filter, abnormal)
{
    /* equal with t_dlt_message_print_header */
}
TEST(t_dlt_message_print_header_with_filter, nullpointer)
{
    /* equal with t_dlt_message_print_header */
}
/* End Method: dlt_common::dlt_message_print_header with filter */




/* Begin Method: dlt_common::dlt_message_print_hex */
TEST(t_dlt_message_print_hex, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_hex, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

/*    / * Get PWD so file and filter can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* No messages read, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1)); */

    /* Set verbose to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_print_hex, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_hex(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_hex */





/* Begin Method: dlt_common::dlt_message_print_hex with filter */
TEST(t_dlt_message_print_hex_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_hex_with_filter, abnormal)
{
    /* equal with t_dlt_message_print_hex */
}
TEST(t_dlt_message_print_hex_with_filter, nullpointer)
{
    /* equal with t_dlt_message_print_hex */
}
/* End Method: dlt_common::dlt_message_print_hex with filter */




/* Begin Method: dlt_common::dlt_message_print_mixed_plain */
TEST(t_dlt_message_print_mixed_plain, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_plain, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

/*    / * Get PWD so file and filter can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* No messages read, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1)); */

    /* Set verbose to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_print_mixed_plain, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_plain(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_mixed_pain */





/* Begin Method: dlt_common::dlt_message_print_mixed_plain with filter */
TEST(t_dlt_message_print_mixed_plain_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_plain_with_filter, abnormal)
{
    /* equal with t_dlt_message_print_mixed_plain */
}
TEST(t_dlt_message_print_mixed_plain_with_filter, nullpointer)
{
    /* equal with t_dlt_message_print_mixed_plain */
}
/* End Method: dlt_common::dlt_message_print_mixed_pain with filter */



/* Begin Method: dlt_common::dlt_message_print_mixed_html */
TEST(t_dlt_message_print_mixed_html, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_html, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

/*    / * Get PWD so file and filter can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* No messages read, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1)); */

    /* Set verbose to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_print_mixed_html, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_print_mixed_html(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));
}
/* End Method: dlt_common::dlt_message_print_mixed_html */




/* Begin Method: dlt_common::dlt_message_print_mixed_html_with filter */
TEST(t_dlt_message_print_mixed_html_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_html_with_filter, abnormal)
{
    /* equal with t_dlt_message_print_mixed_html */
}
TEST(t_dlt_message_print_mixed_html_with_filter, nullpointer)
{
    /* equal with t_dlt_message_print_mixed_html */
}
/* End Method: dlt_common::dlt_message_print_mixed_html_with filter */




/* Begin Method:dlt_common::dlt_message_filter_check */
TEST(t_dlt_message_filter_check, normal)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected > 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_filter_check(&file.msg, &filter, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_filter_check(&file.msg, &filter, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_filter_check, abnormal)
{
/*    DltFile file; */
/*    DltFilter filter; */

    /* Get PWD so file and filter can be used*/
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)]; */
/*    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)]; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
/*    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* No messages read, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(&file.msg, &filter, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(&file.msg, &filter, 1)); */

    /* Set verbose to 12345678 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(&file.msg, &filter, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(&file.msg, &filter, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_filter_check, nullpointer)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char openfilter[MAX_LINE+sizeof(FILTER_FILE_NAME)];
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    sprintf(openfilter, "%s" FILTER_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(NULL, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(NULL, NULL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(NULL, &filter, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(NULL, &filter, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(&file.msg, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_filter_check(&file.msg, NULL, 1));
}
/* End Method:dlt_common::dlt_message_filter_check */




/* Begin Method:dlt_common::dlt_message _get_extraparameters */
TEST(t_dlt_message_get_extraparamters, normal)
{
    DltFile file;

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect >0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_get_extraparameters(&file.msg, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_get_extraparameters(&file.msg, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_get_extraparamters, abnormal)
{
/*    DltFile file; */

    /* Get PWD so file and filter can be used*/
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_get_extraparameters(&file.msg, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_get_extraparameters(&file.msg, 1)); */

    /* set verbose to 12345678, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_get_extraparameters(&file.msg, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_get_extraparameters(&file.msg, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_get_extraparamters, nullpointer)
{
    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_get_extraparameters(NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_get_extraparameters(NULL, 1));
}
/* End Method:dlt_common::dlt_message_get_extraparameters */




/* Begin Method:dlt_common::dlt_message_header */
TEST(t_dlt_message_header, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
        printf("%s \n", text);
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
        printf("%s \n", text);
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_header, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

    /* Get PWD so file and filter can be used*/
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1)); */

    /* set verbose to 12345678, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678)); */
/*        printf("%s \n",text); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_header, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, text, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, NULL, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, text, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header(&file.msg, text, 0, 1));
}
/* End Method:dlt_common::dlt_message_header */




/* Begin Method:dlt_common::dlt_message_header_flags */
TEST(t_dlt_message_header_flags, normal)
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
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
        printf("%s \n", text);
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
        printf("%s \n", text);
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_header_flags, abnormal)
{
/*    DltFile file; */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */

/*    / * Get PWD so file and filter can be used* / */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];;; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1)); */

    /* USE own DLT_HEADER_SHOW , expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0x1234, 0)); */

/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0x1234, 0)); */
/*        printf("%s \n",text); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_header_flags, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, 0, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, 0, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, 0, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_header_flags(&file.msg, text, 0, DLT_HEADER_SHOW_ALL, 1));
}
/* End Method:dlt_common::dlt_message_header_flags */




/* Begin Method:dlt_common::dlt_message_payload */
TEST(t_dlt_message_payload, normal)
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
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expected 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
        printf("%s \n", text);
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
        printf("%s \n", text);
        EXPECT_LE(DLT_RETURN_OK,
                  dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
        printf("%s \n", text);
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_payload, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[MAX_LINE];
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];;
    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Uninizialised, expected -1 */
    memset(&file, 0x00, sizeof(DltFile));

    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));

    /* USE own DLT_HEADER_SHOW , expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, 99, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, 99, 0));
        printf("%s \n",text);
    }
    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_payload, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, 0, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, 0, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR,
              dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_payload(&file.msg, text, 0, DLT_OUTPUT_ASCII_LIMITED, 1));

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
TEST(t_dlt_message_set_extraparamters, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_set_extraparameters(&file.msg, 0));
    }

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_OK, dlt_message_set_extraparameters(&file.msg, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_set_extraparamters, abnormal)
{
/*    DltFile file; */
/*    // Get PWD so file and filter can be used */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
    /*---------------------------------------*/

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_set_extraparameters(&file.msg, 0)); */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_set_extraparameters(&file.msg, 1)); */

    /* set verbos to 12345678 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_set_extraparameters(&file.msg, 12345678)); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_set_extraparamters, nullpointer)
{
    /* NULL-Pointer, expect -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_set_extraparameters(NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_set_extraparameters(NULL, 1));
}
/* End Method:dlt_common::dlt_message_set_extraparameters */





/* Begin Method:dlt_common::dlt_message_read */
TEST(t_dlt_message_read, normal)
{
    DltFile file;
    /* Get PWD so file can be used */
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    DltBuffer buf;
    char *buffer = NULL;

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_ERROR, dlt_message_read(&file.msg, (unsigned char *)buffer, 255, 0, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));

    EXPECT_LE(DLT_RETURN_OK,
              dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE,
                                      DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        EXPECT_LE(DLT_RETURN_ERROR, dlt_message_read(&file.msg, (unsigned char *)buffer, 255, 1, 1));
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_message_read, abnormal)
{}
TEST(t_dlt_message_read, nullpointer)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    /*---------------------------------------*/

    DltBuffer buf;

    /* NULL_Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read(NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read(NULL, (uint8_t *)&buf, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read(&file.msg, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_read(&file.msg, (uint8_t *)&buf, 0, 0, 0));

}
/* End Method:dlt_common::dlt_message_read */




/* Begin Method:dlt_common::dlt_message_argument_print */
TEST(t_dlt_message_argument_print, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    static char text[DLT_DAEMON_TEXTSIZE];
    /*---------------------------------------*/
    uint8_t *ptr;
    int32_t datalength;
    uint8_t **pptr;
    int32_t *pdatalength;

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        ptr = file.msg.databuffer;
        datalength = file.msg.datasize;
        pptr = &ptr;
        pdatalength = &datalength;
        EXPECT_GE(DLT_RETURN_OK,
                  dlt_message_argument_print(&file.msg, DLT_TYPE_INFO_BOOL, pptr, pdatalength, text,
                                             DLT_DAEMON_TEXTSIZE, 0, 1));
        /*printf("### ARGUMENT:%s\n", text); */
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));

    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    while (dlt_file_read(&file, 0) >= 0) {}

    for (int i = 0; i < file.counter; i++) {
        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0));
        ptr = file.msg.databuffer;
        datalength = file.msg.datasize;
        pptr = &ptr;
        pdatalength = &datalength;
        EXPECT_GE(DLT_RETURN_OK,
                  dlt_message_argument_print(&file.msg, DLT_TYPE_INFO_RAWD, pptr, pdatalength, text,
                                             DLT_DAEMON_TEXTSIZE, 0, 1));
        /*printf("### ARGUMENT:%s\n", text); */
    }

    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0));

}
TEST(t_dlt_message_argument_print, abnormal)
{
/*    DltFile file; */
    /* Get PWD so file and filter can be used */
/*    char pwd[MAX_LINE]; */
/*    getcwd(pwd, MAX_LINE); */
/*    char  openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];; */
/*    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd); */
/*    static char text[DLT_DAEMON_TEXTSIZE]; */
    /*---------------------------------------*/
/*    uint8_t *ptr; */
/*    int32_t datalength; */
/*    uint8_t **pptr; */
/*    int32_t *pdatalength; */

    /* Uninizialised, expected -1 */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg,12345678,pptr,pdatalength,text,DLT_DAEMON_TEXTSIZE,0,1)); */

    /* Use a non defined type_info, expected -1 */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0)); */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0)); */
/*    while (dlt_file_read(&file,0)>=0){} */
/*    for(int i=0;i<file.counter;i++) */
/*    { */
/*        EXPECT_LE(DLT_RETURN_OK, dlt_file_message(&file, i, 0)); */
/*        ptr = file.msg.databuffer; */
/*        datalength = file.msg.datasize; */
/*        pptr = &ptr; */
/*        pdatalength = &datalength; */
/*        EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg,12345678,pptr,pdatalength,text,DLT_DAEMON_TEXTSIZE,0,1)); */
/*        //printf("### ARGUMENT:%s\n", text); */
/*    } */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_file_free(&file, 0)); */
}
TEST(t_dlt_message_argument_print, nullpointer)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[MAX_LINE];
    char openfile[MAX_LINE+sizeof(BINARY_FILE_NAME)];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, MAX_LINE) == NULL) {}

    sprintf(openfile, "%s" BINARY_FILE_NAME, pwd);
    static char text[DLT_DAEMON_TEXTSIZE];
    /*---------------------------------------*/
    uint8_t *ptr;
    int32_t datalength;
    uint8_t **pptr;
    int32_t *pdatalength;
    pptr = &ptr;
    pdatalength = &datalength;

    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, NULL, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, NULL, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, NULL, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, NULL, pdatalength, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, pptr, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, pptr, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, pptr, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(NULL, 0, pptr, pdatalength, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, NULL, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, NULL, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, NULL, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, NULL, pdatalength, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, pptr, NULL, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, pptr, NULL, text, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, pptr, pdatalength, NULL, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_message_argument_print(&file.msg, 0, pptr, pdatalength, text, 0, 0, 0));
}
/* End Method:dlt_common::dlt_message_argument_print */




/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/




/* Begin Method:dlt_common::dlt_log_set_level */
TEST(t_dlt_log_set_level, normal)
{
    /* Possible Falgs */
    /* DLT_LOG_TO_CONSOLE=0, */
    /* DLT_LOG_TO_SYSLOG=1, */
    /* DLT_LOG_TO_FILE=2, */
    /* DLT_LOG_TO_STDERR=3, */
    /* DLT_LOG_DROPPED=4 */
    /*####################### */

    /* Normal Use-Case, expect 0-4 */
    EXPECT_NO_THROW(dlt_log_set_level(DLT_LOG_TO_CONSOLE));
    EXPECT_NO_THROW(dlt_log_set_level(DLT_LOG_TO_SYSLOG));
    EXPECT_NO_THROW(dlt_log_set_level(DLT_LOG_TO_FILE));
    EXPECT_NO_THROW(dlt_log_set_level(DLT_LOG_TO_STDERR));
    EXPECT_NO_THROW(dlt_log_set_level(DLT_LOG_DROPPED));
}
TEST(t_dlt_log_set_level, abnormal)
{
    /* actual no test cases */
    /* because of void method and missing gtest function */
}
TEST(t_dlt_log_set_level, nullpointer)
{
    /* NULL-Pointer, expected -1 */
}
/* End Method:dlt_common::dlt_log_set_level */





/* Begin MEthod:dlt_common::dlt_log_set_filename */
TEST(dlt_log_set_filename, normal)
{
    /* Normal Use-Case, exptected PATH */
    const char *filename = "/tmp/dlt.log";
    EXPECT_NO_THROW(dlt_log_set_filename(filename));
}
TEST(dlt_log_set_filename, abnormal)
{
    /* actual no test cases */
    /* because of void method and missing gtest function */
}
TEST(dlt_log_set_filename, nullpointer)
{
    /* NULL-Pointer, expected -1 or "no file" */
    EXPECT_NO_THROW(dlt_log_set_filename(NULL));
}
/* End MEthod:dlt_common::dlt_log_set_filename */




/* Begin Method: dlt_common::dlt_log_init */
TEST(t_dlt_log_init, normal)
{
    /* Possible Falgs */
    /* DLT_LOG_TO_CONSOLE=0, */
    /* DLT_LOG_TO_SYSLOG=1, */
    /* DLT_LOG_TO_FILE=2, */
    /* DLT_LOG_TO_STDERR=3, */
    /* DLT_LOG_DROPPED=4 */
    /*####################### */

    /* Normal Use-Case, exptect 0-3 */
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_CONSOLE));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_SYSLOG));
    EXPECT_NO_THROW(dlt_log_set_filename("/tmp/dlt.log"));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_FILE));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_FILE));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_STDERR));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_DROPPED));
}
TEST(t_dlt_log_init, abnormal)
{
    /* actual no test cases */
    /* because of void method and missing gtest function */
}
TEST(t_dlt_log_init, nullpointer)
{
    /* NULL-Pointer, expected -1 */
}
/* End Method: dlt_common::dlt_log_init */




/* Begin Method:dlt_common::dlt_log_free */
TEST(t_dlt_log_free, normal)
{
    /* Possible Falgs */
    /* DLT_LOG_TO_CONSOLE=0, */
    /* DLT_LOG_TO_SYSLOG=1, */
    /* DLT_LOG_TO_FILE=2, */
    /* DLT_LOG_TO_STDERR=3, */
    /* DLT_LOG_DROPPED=4 */
    /*####################### */

    /* Normal Use-Case, expected 0 */
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_CONSOLE));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_SYSLOG));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_TO_STDERR));
    EXPECT_NO_THROW(dlt_log_init(DLT_LOG_DROPPED));
}
TEST(t_dlt_log_free, abnormal)
{
    /* actual no test cases */
    /* because of void method and missing gtest function */
}
TEST(t_dlt_log_free, nullpointer)
{
    /* NULL-Pointer, expected -1 */
}
/* End Method:dlt_common::dlt_log_free */




/* Begin Method: dlt_common::dlt_log */
TEST(t_dlt_log, normal)
{
    /* Possible Falgs */
    /* #define  LOG_EMERG   0 */
    /* #define  LOG_ALERT   1 */
    /* #define  LOG_CRIT    2 */
    /* #define  LOG_ERR     3 */
    /* #define  LOG_WARNING 4 */
    /* #define  LOG_NOTICE  5 */
    /* #define  LOG_INFO    6 */
    /* #define  LOG_DEBUG   7 */

    const char *EMERG = "SYSLOG EMERG\n";
    const char *ALERT = "SYSLOG ALERT\n";
    const char *CRIT = "SYSLOG CRIT\n";
    const char *ERR = "SYSLOG ERR\n";
    const char *WARNING = "SYSLOG WARNING\n";
    const char *NOTICE = "SYSLOG NOTICE\n";
    const char *INFO = "SYSLOG INFO\n";
    const char *DEBUG = "SYSLOG DEBUG\n";

    /* Normal Use-Case, expected 0 */
    dlt_log_init(DLT_LOG_TO_CONSOLE);
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_EMERG, (char *)EMERG));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_ALERT, (char *)ALERT));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_CRIT, (char *)CRIT));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_ERR, (char *)ERR));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_WARNING, (char *)WARNING));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_NOTICE, (char *)NOTICE));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_INFO, (char *)INFO));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_DEBUG, (char *)DEBUG));

    dlt_log_init(DLT_LOG_DROPPED);
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_EMERG, (char *)EMERG));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_ALERT, (char *)ALERT));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_CRIT, (char *)CRIT));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_ERR, (char *)ERR));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_WARNING, (char *)WARNING));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_NOTICE, (char *)NOTICE));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_INFO, (char *)INFO));
    EXPECT_LE(DLT_RETURN_OK, dlt_log(LOG_DEBUG, (char *)DEBUG));
}
TEST(t_dlt_log, abnormal)
{
    /* LOG MODE don't exists, expected -1 */
/*    int DLT_LOG_DONT_EXISTS = 123456789; */
/*    const char * EXIST = "SYSLOG DONT EXISTS\n"; */
/*    EXPECT_GE(DLT_RETURN_ERROR, dlt_log(DLT_LOG_DONT_EXISTS, (char *) EXIST)); */
}
TEST(t_dlt_log, nullpointer)
{
    /* NULL-Pointer, expected -1 */
    EXPECT_GE(DLT_RETURN_ERROR, dlt_log(0, NULL));
}
/* End Method: dlt_common::dlt_log_init_init */




/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/



/* Begin Method:dlt_common::dlt_uptime */
TEST(t_dlt_uptime, normal)
{
    EXPECT_LE(1, dlt_uptime());
}
TEST(t_dlt_uptime, abnormal)
{}
TEST(t_dlt_uptime, nullpointer)
{}
/* End Method:dlt_common::dlt_uptime */




/* Begin Method:dlt_common::dlt_set_id */
TEST(t_dlt_set_id, normal)
{
/*    char id[4]; */
/*    const char * text = "DLTD"; */
/*    dlt_set_id(id, text); */
/*    EXPECT_STREQ(text, id); */
}
TEST(t_dlt_set_id, abnormal)
{
/*    char id[10]; */
/*    const char * text = "1234567890"; */
/*    dlt_set_id(id, text); */
/*    EXPECT_STRNE(text, id); */
}
TEST(t_dlt_set_id, nullpointer)
{
    char id[4];
    const char *text = "TEST";

    EXPECT_NO_THROW(dlt_set_id(NULL, NULL));
    EXPECT_NO_THROW(dlt_set_id(NULL, text));
    EXPECT_NO_THROW(dlt_set_id(id, NULL));
}
/* End Method:dlt_common::dlt_set_id */




/* Begin Method:dlt_common::dlt_print_hex_string */
TEST(t_dlt_print_hex_string, normal)
{
    /* Normal Use-Case, exptect 0 */
    const char *test1 = "HELLO_HEX";
    char text1[DLT_DAEMON_TEXTSIZE];
    EXPECT_LE(DLT_RETURN_OK, dlt_print_hex_string(text1, DLT_DAEMON_TEXTSIZE, (unsigned char *)test1, strlen(test1)));
    /*printf("text:%s\n", text1); */
    /* convert text1 to an ascii string to compare with the original */
    char *converted = (char *)malloc(strlen(test1) + 1);
    int t = 0;

    for (unsigned int i = 0; i < strlen(text1); i += 3) {
        char tmp[3] = { '\0' };
        tmp[0] = text1[i];
        tmp[1] = text1[i + 1];
        char k = (int)strtol(tmp, NULL, 16);
        converted[i - t] = k;
        t += 2;
    }

    converted[strlen(test1)] = '\0';
    /*printf("%s\n", converted); */
    EXPECT_STREQ(test1, converted);
    free(converted);

    const char *test2 = "qwertzuiopasdfghjklyxcvbnm1234567890";
    char text2[DLT_DAEMON_TEXTSIZE];
    EXPECT_LE(DLT_RETURN_OK, dlt_print_hex_string(text2, DLT_DAEMON_TEXTSIZE, (unsigned char *)test2, strlen(test2)));
    /*printf("text:%s\n", text2); */
    /* convert text2 to an ascii string to compare with the original */
    converted = (char *)malloc(strlen(test2) + 1);
    t = 0;

    for (unsigned int i = 0; i < strlen(text2); i += 3) {
        char tmp[3] = { '\0' };
        tmp[0] = text2[i];
        tmp[1] = text2[i + 1];
        char k = (int)strtol(tmp, NULL, 16);
        converted[i - t] = k;
        t += 2;
    }

    converted[strlen(test2)] = '\0';
    /*printf("%s\n", converted); */
    EXPECT_STREQ(test2, converted);
    free(converted);
}
TEST(t_dlt_print_hex_string, abnormal)
{
    /* print special characters, expected 0 */
/*    const char * test3 = "^°!\"§$%&/()=?`´¹²³¼½¬{[]}\\¸@€üöä+#*'~`,.-;:_·…–<>|"; */
/*    char text3[DLT_DAEMON_TEXTSIZE]; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_hex_string(text3,DLT_DAEMON_TEXTSIZE,(unsigned char *)test3, strlen(test3))); */
    /*printf("text:%s\n", text3); */
    /* convert text3 to an ascii string to compare with the original */
/*    char * converted = (char*) malloc(strlen(test3) +1); */
/*    int t = 0; */
/*    for(unsigned int i=0;i<strlen(text3);i+=3) */
/*    { */
/*        char tmp[2]; */
/*        tmp[0] = text3[i]; */
/*        tmp[1] = text3[i+1]; */
/*        char k =  (int) strtol(tmp, NULL, 16); */
/*        converted[i-t] = k; */
/*        t +=2; */
/*    } */
/*    converted[strlen(test3)] = '\0'; */
    /*printf("%s\n", converted); */
/*    EXPECT_STREQ(test3, converted); */
/*  free(converted); */

    /* Empty char *, expect 0 */
/*    const char * test4 = ""; */
/*    char text4[DLT_DAEMON_TEXTSIZE]; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_hex_string(text4,DLT_DAEMON_TEXTSIZE,(unsigned char *)test4, strlen(test4))); */
    /*printf("text:%s\n", text4); */
    /* convert text4 to an ascii string to compare with the original */
/*    converted = (char*) malloc(strlen(test4) +1); */
/*    t = 0; */
/*    for(unsigned int i=0;i<strlen(text4);i+=3) */
/*    { */
/*        char tmp[2]; */
/*        tmp[0] = text4[i]; */
/*        tmp[1] = text4[i+1]; */
/*        char k =  (int) strtol(tmp, NULL, 16); */
/*        converted[i-t] = k; */
/*        t +=2; */
/*    } */
/*    converted[strlen(test4)] = '\0'; */
    /*printf("%s\n", converted); */
/*    EXPECT_STREQ(test4, converted); */
/*    free(converted); */
}
TEST(t_dlt_print_hex_string, nullpointer)
{
    const char *test5 = "HELLO";
    char text5[DLT_DAEMON_TEXTSIZE];

    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_hex_string(NULL, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_hex_string(NULL, 0, (unsigned char *)test5, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_hex_string(text5, 0, NULL, 0));
}
/* End Method:dlt_common::dlt_print_hex_string */




/* Begin Method:dlt_common::dlt_print_mixed_string */
TEST(t_dlt_print_mixed_string, normal)
{
    const char *test1 = "HELLO_MIXED";
    char text1[DLT_DAEMON_TEXTSIZE];
    EXPECT_LE(DLT_RETURN_OK,
              dlt_print_mixed_string(text1, DLT_DAEMON_TEXTSIZE, (unsigned char *)test1, strlen(test1), 0));
    printf("%s\n", text1);

    const char *test2 = "HELLO_MIXED";
    char text2[DLT_DAEMON_TEXTSIZE];
    EXPECT_LE(DLT_RETURN_OK,
              dlt_print_mixed_string(text2, DLT_DAEMON_TEXTSIZE, (unsigned char *)test2, strlen(test2), 1));
    printf("%s\n", text2);

    const char *test3 = "qwertzuiopasdfghjklyxcvbnm1234567890";
    char text3[DLT_DAEMON_TEXTSIZE];
    EXPECT_LE(DLT_RETURN_OK,
              dlt_print_mixed_string(text3, DLT_DAEMON_TEXTSIZE, (unsigned char *)test3, strlen(test3), 0));
    printf("%s\n", text3);

    const char *test4 = "qwertzuiopasdfghjklyxcvbnm1234567890";
    char text4[DLT_DAEMON_TEXTSIZE];
    EXPECT_LE(DLT_RETURN_OK,
              dlt_print_mixed_string(text4, DLT_DAEMON_TEXTSIZE, (unsigned char *)test4, strlen(test4), 1));
    printf("%s\n", text4);
}
TEST(t_dlt_print_mixed_string, abnormal)
{
/*    const char * test5 = "^°!\"§$%&/()=?`´¹²³¼½¬{[]}\\¸@€üöä+#*'~`,.-;:_·…–<>|"; */
/*    char text5[DLT_DAEMON_TEXTSIZE]; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_mixed_string(text5,DLT_DAEMON_TEXTSIZE,(unsigned char *)test5,strlen(test5),0)); */
/*    printf("%s\n", text5); */

/*    const char * test6 = "^°!\"§$%&/()=?`´¹²³¼½¬{[]}\\¸@€üöä+#*'~`,.-;:_·…–<>|"; */
/*    char text6[DLT_DAEMON_TEXTSIZE]; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_mixed_string(text6,DLT_DAEMON_TEXTSIZE,(unsigned char *)test6,strlen(test6),1)); */
/*    printf("%s\n", text6); */

/*    const char * test7 = ""; */
/*    char text7[DLT_DAEMON_TEXTSIZE]; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_mixed_string(text7,DLT_DAEMON_TEXTSIZE,(unsigned char *)test7,strlen(test7),0)); */
/*    printf("%s\n", text7); */

/*    const char * test8 = ""; */
/*    char text8[DLT_DAEMON_TEXTSIZE]; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_mixed_string(text8,DLT_DAEMON_TEXTSIZE,(unsigned char *)test8,strlen(test8),1)); */
/*    printf("%s\n", text8); */
}
TEST(t_dlt_print_mixed_string, nullpointer)
{
    const char *test9 = "";
    char text9[DLT_DAEMON_TEXTSIZE];

    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_mixed_string(NULL, 0, 0, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_mixed_string(NULL, 0, 0, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_mixed_string(NULL, 0, (unsigned char *)test9, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_mixed_string(NULL, 0, (unsigned char *)test9, 0, 1));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_mixed_string(text9, 0, NULL, 0, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_mixed_string(text9, 0, NULL, 0, 1));
}
/* End Method:dlt_common::dlt_print_mixed_string */




/* Begin Method:dlt_common::dlt_print_char_string */
TEST(t_dlt_print_char_string, normal)
{
    /* Normal Use-Case, expect 0 */
    const char *test1 = "HELLO";
    char text1[DLT_DAEMON_TEXTSIZE];
    char *ptr1 = text1;
    EXPECT_LE(DLT_RETURN_OK, dlt_print_char_string(&ptr1, DLT_DAEMON_TEXTSIZE, (unsigned char *)test1, strlen(test1)));
    printf("text:%s\n", text1);
    EXPECT_STREQ(text1, test1);

    const char *test2 = "qwertzuiopasdfghjklyxcvbnm1234567890";
    char text2[DLT_DAEMON_TEXTSIZE];
    char *ptr2 = text2;
    EXPECT_LE(DLT_RETURN_OK, dlt_print_char_string(&ptr2, DLT_DAEMON_TEXTSIZE, (unsigned char *)test2, strlen(test2)));
    printf("text:%s\n", text2);
    EXPECT_STREQ(text2, test2);
}
TEST(t_dlt_print_char_string, abnormal)
{
    /* print special characters, expected 0 */
/*    const char * test3 = "^°!\"§$%&/()=?`´¹²³¼½¬{[]}\\¸@€üöä+#*'~`,.-;:_·…–<>|"; */
/*    char text3[DLT_DAEMON_TEXTSIZE]; */
/*    char * ptr3 = text3; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_char_string(&ptr3,DLT_DAEMON_TEXTSIZE,(unsigned char *)test3, strlen(test3))); */
/*    printf("text:%s\n", text3); */
/*    EXPECT_STREQ(text3, test3); */

    /* Empty char *, expect 0 */
/*    const char * test4 = ""; */
/*    char text4[DLT_DAEMON_TEXTSIZE]; */
/*    char * ptr4 = text4; */
/*    EXPECT_LE(DLT_RETURN_OK, dlt_print_char_string(&ptr4,DLT_DAEMON_TEXTSIZE,(unsigned char *)test4, strlen(test4))); */
/*    printf("text:%s\n", text4); */
/*    EXPECT_STREQ(text4, test4); */
}
TEST(t_dlt_print_char_string, nullpointer)
{
    const char *test5 = "HELLO";
    char text5[DLT_DAEMON_TEXTSIZE];
    char *ptr5 = text5;

    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_char_string(NULL, 0, NULL, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_char_string(NULL, 0, (unsigned char *)test5, 0));
    EXPECT_GE(DLT_RETURN_ERROR, dlt_print_char_string(&ptr5, 0, NULL, 0));
}
/* End Method:dlt_common::dlt_print_char_string */




/* Begin Method:dlt_common::dlt_strnlen_s*/
TEST(t_dlt_strnlen_s, nullpointer)
{
    size_t len = dlt_strnlen_s(NULL, 0);
    EXPECT_EQ(len, 0);
}

TEST(t_dlt_strnlen_s, len_zero)
{
    const char text[] = "The Quick Brown Fox";

    size_t len = dlt_strnlen_s(text, 0);
    EXPECT_EQ(len, 0);
}

TEST(t_dlt_strnlen_s, len_smaller)
{
    const char text[] = "The Quick Brown Fox";

    size_t len = dlt_strnlen_s(text, 10);
    EXPECT_EQ(len, 10);
}

TEST(t_dlt_strnlen_s, len_equal)
{
    const char text[] = "The Quick Brown Fox";

    size_t len = dlt_strnlen_s(text, 19);
    EXPECT_EQ(len, 19);
}

TEST(t_dlt_strnlen_s, len_larger)
{
    const char text[] = "The Quick Brown Fox";

    size_t len = dlt_strnlen_s(text, 100);
    EXPECT_EQ(len, 19);
}
/* End Method:dlt_common::dlt_strnlen_s*/




/* Begin Method:dlt_common::dlt_print_id */
TEST(t_dlt_print_id, normal)
{
    /* Normal Use-Case, expect text==id */
    const char *id = "DLTD";
    char text[DLT_DAEMON_TEXTSIZE];
    dlt_print_id(text, id);
    EXPECT_STREQ(text, id);
}
TEST(t_dlt_print_id, abnormal)
{
    /* id to long, expect only first 4 chars */
/*    const char* id = "DLTD123456789"; */
/*    char text[DLT_DAEMON_TEXTSIZE]; */
/*    dlt_print_id(text,id); */
/*    EXPECT_STREQ(text,"DLTD"); */

    /* id to short, expect expend with "-" to 4 chars */
/*    id = "DL"; */
/*    dlt_print_id(text,id); */
/*    EXPECT_STREQ(text,"DL--"); */
}
TEST(t_dlt_print_id, nullpointer)
{
    const char *id = "DLTD";
    char text[DLT_DAEMON_TEXTSIZE];

    /* NULL-Pointer, expected nothing in return */
    EXPECT_NO_THROW(dlt_print_id(NULL, NULL));
    EXPECT_NO_THROW(dlt_print_id(NULL, id));
    EXPECT_NO_THROW(dlt_print_id(text, NULL));
}
/* End Method:dlt_common::dlt_print_id */




/* Begin Method:dlt_common::dlt_get_version */
TEST(t_dlt_get_version, normal)
{
    /* Normal Use-Case */
    char ver[255];
    dlt_get_version(ver, 255);
    printf("%s\n", ver);
}
TEST(t_dlt_get_version, abnormal)
{
    /* Change default length of ver to 1 */
/*    char ver[1]; */
/*    dlt_get_version(ver, DLT_USER_MAX_LIB_VERSION_LENGTH); */
/*    printf("%s\n", ver); */

    /* Change default length of ver to 1 and reduce second para to 1, too */
/*    dlt_get_version(ver, 1); */
/*    printf("%s\n", ver); */
}
TEST(t_dlt_get_version, nullpointer)
{
    EXPECT_NO_THROW(dlt_get_version(NULL, 0));
}
/* End Method:dlt_common::dlt_get_version */




/* Begin Method:dlt_common::dlt_get_major_version */
TEST(dlt_get_major_version, normal)
{
    char ver[DLT_USER_MAX_LIB_VERSION_LENGTH];
    dlt_get_major_version(ver, DLT_USER_MAX_LIB_VERSION_LENGTH);
    EXPECT_STREQ(ver, _DLT_PACKAGE_MAJOR_VERSION);
}
TEST(dlt_get_major_version, abnormal)
{
    /* Change default length of ver to 1 */
/*    char ver[1]; */
/*    dlt_get_major_version(ver, DLT_USER_MAX_LIB_VERSION_LENGTH); */
/*    EXPECT_STREQ(ver, _DLT_PACKAGE_MAJOR_VERSION); */

    /* Change default length of ver to 1 and reduce second para to 1, too */
/*    dlt_get_major_version(ver, 1); */
/*    EXPECT_STREQ(ver, _DLT_PACKAGE_MAJOR_VERSION); */
}
TEST(dlt_get_major_version, nullpointer)
{
    /* NULL-Pointer, expect exeption */
    EXPECT_NO_THROW(dlt_get_major_version(NULL, 0));
}
/* End Method:dlt_common::dlt_get_major_version */




/* Begin Method:dlt_common::dlt_get_minor_version */
TEST(dlt_get_minor_version, normal)
{
    char ver[DLT_USER_MAX_LIB_VERSION_LENGTH];
    dlt_get_minor_version(ver, DLT_USER_MAX_LIB_VERSION_LENGTH);
    EXPECT_STREQ(ver, _DLT_PACKAGE_MINOR_VERSION);
}
TEST(dlt_get_minor_version, abnormal)
{
    /* Change default length of ver to 1 */
/*    char ver[1]; */
/*    dlt_get_minor_version(ver, DLT_USER_MAX_LIB_VERSION_LENGTH); */
/*    EXPECT_STREQ(ver, _DLT_PACKAGE_MINOR_VERSION); */

    /* Change default length of ver to 1 and reduce second para to 1, too */
/*    dlt_get_minor_version(ver, 1); */
/*    EXPECT_STREQ(ver, _DLT_PACKAGE_MINOR_VERSION); */
}
TEST(dlt_get_minor_version, nullpointer)
{
    /* NULL-Pointer, expect exeption */
    EXPECT_NO_THROW(dlt_get_minor_version(NULL, 0));
}
/* End Method:dlt_common::dlt_get_minor_version */


TEST(dlt_client_parse_get_log_info_resp_text, normal)
{
    char input[] = "get_log_info, 07, 02 00 4c 4f 47 00 03 00 54 45 53 54 ff ff 18 00 54 65 73 74 20 43 6f 6e 74 65 78 74 20 66 6f 72 20 4c 6f 67 67 69 6e 67 54 53 31 00 ff ff 1b 00 54 65 73 74 20 43 6f 6e 74 65 78 74 31 20 66 6f 72 20 69 6e 6a 65 63 74 69 6f 6e 54 53 32 00 ff ff 1b 00 54 65 73 74 20 43 6f 6e 74 65 78 74 32 20 66 6f 72 20 69 6e 6a 65 63 74 69 6f 6e 1c 00 54 65 73 74 20 41 70 70 6c 69 63 61 74 69 6f 6e 20 66 6f 72 20 4c 6f 67 67 69 6e 67 53 59 53 00 02 00 4a 4f 55 52 ff ff 0f 00 4a 6f 75 72 6e 61 6c 20 41 64 61 70 74 65 72 4d 47 52 00 ff ff 22 00 43 6f 6e 74 65 78 74 20 6f 66 20 6d 61 69 6e 20 64 6c 74 20 73 79 73 74 65 6d 20 6d 61 6e 61 67 65 72 12 00 44 4c 54 20 53 79 73 74 65 6d 20 4d 61 6e 61 67 65 72 72 65 6d 6f";
    /* expected output:
     * APID:LOG- Test Application for Logging
     * CTID:TEST -1 -1 Test Context for Logging
     * CTID:TS1- -1 -1 Test Context1 for injection
     * CTID:TS2- -1 -1 Test Context2 for injection
     * APID:SYS- DLT System Manager
     * CTID:JOUR -1 -1 Journal Adapter
     * CTID:MGR- -1 -1 Context of main dlt system manager
     */

    DltServiceGetLogInfoResponse * resp = (DltServiceGetLogInfoResponse *)malloc(sizeof(DltServiceGetLogInfoResponse ));
    DltReturnValue ret = (DltReturnValue) dlt_set_loginfo_parse_service_id(
            input, &resp->service_id, &resp->status);
    EXPECT_EQ(DLT_RETURN_OK, ret);
    EXPECT_EQ(DLT_SERVICE_ID_GET_LOG_INFO, resp->service_id);
    EXPECT_EQ(GET_LOG_INFO_STATUS_MAX, resp->status);

    ret = dlt_client_parse_get_log_info_resp_text(resp, input);
    EXPECT_EQ(DLT_RETURN_OK, ret);

    EXPECT_EQ(2,resp->log_info_type.count_app_ids);

    EXPECT_EQ(0, memcmp( "LOG", resp->log_info_type.app_ids[0].app_id,4));
    EXPECT_EQ(0, strcmp("Test Application for Logging", resp->log_info_type.app_ids[0].app_description));
    EXPECT_EQ(28, resp->log_info_type.app_ids[0].len_app_description);

    EXPECT_EQ(3, resp->log_info_type.app_ids[0].count_context_ids);

    EXPECT_EQ(0, memcmp( "TEST", resp->log_info_type.app_ids[0].context_id_info[0].context_id,4));
    EXPECT_EQ(0, strcmp( "Test Context for Logging",resp->log_info_type.app_ids[0].context_id_info[0].context_description ));
    EXPECT_EQ(24,resp->log_info_type.app_ids[0].context_id_info[0].len_context_description);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[0].context_id_info[0].log_level);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[0].context_id_info[0].trace_status);

    EXPECT_EQ(0, memcmp( "TS1", resp->log_info_type.app_ids[0].context_id_info[1].context_id,4));
    EXPECT_EQ(0, strcmp( "Test Context1 for injection",resp->log_info_type.app_ids[0].context_id_info[1].context_description ));
    EXPECT_EQ(27,resp->log_info_type.app_ids[0].context_id_info[1].len_context_description);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[0].context_id_info[1].log_level);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[0].context_id_info[1].trace_status);

    EXPECT_EQ(0, memcmp( "TS2", resp->log_info_type.app_ids[0].context_id_info[2].context_id,4));
    EXPECT_EQ(0, strcmp( "Test Context2 for injection",resp->log_info_type.app_ids[0].context_id_info[2].context_description ));
    EXPECT_EQ(27,resp->log_info_type.app_ids[0].context_id_info[2].len_context_description);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[0].context_id_info[2].log_level);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[0].context_id_info[2].trace_status);

    EXPECT_EQ(0, memcmp( "SYS", resp->log_info_type.app_ids[1].app_id,4));
    EXPECT_EQ(0, strcmp("DLT System Manager", resp->log_info_type.app_ids[1].app_description));
    EXPECT_EQ(18, resp->log_info_type.app_ids[1].len_app_description);

    EXPECT_EQ(2, resp->log_info_type.app_ids[1].count_context_ids);

    EXPECT_EQ(0, memcmp( "JOUR", resp->log_info_type.app_ids[1].context_id_info[0].context_id,4));
    EXPECT_EQ(0, strcmp( "Journal Adapter",resp->log_info_type.app_ids[1].context_id_info[0].context_description ));
    EXPECT_EQ(15,resp->log_info_type.app_ids[1].context_id_info[0].len_context_description);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[1].context_id_info[0].log_level);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[1].context_id_info[0].trace_status);

    EXPECT_EQ(0, memcmp( "MGR", resp->log_info_type.app_ids[1].context_id_info[1].context_id,4));
    EXPECT_EQ(0, strcmp( "Context of main dlt system manager",resp->log_info_type.app_ids[1].context_id_info[1].context_description ));
    EXPECT_EQ(34,resp->log_info_type.app_ids[1].context_id_info[1].len_context_description);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[1].context_id_info[1].log_level);
    EXPECT_EQ(-1,resp->log_info_type.app_ids[1].context_id_info[1].trace_status);

    ret = (DltReturnValue)dlt_client_cleanup_get_log_info(resp);
    EXPECT_EQ(DLT_RETURN_OK,ret);
}

TEST(dlt_getloginfo_conv_ascii_to_string, normal)
{
    /* NULL-Pointer, expect exeption */
    char rp_1[] = "72 65 6d 6f";
    char rp_2[] = "123456789 72 65 6d 6f";
    int rp_count = 0;
    char rp_str[] =
    { 0x72, 0x65, 0x6d, 0x6f, 0x00 };
    char wp[5];

    dlt_getloginfo_conv_ascii_to_string(rp_1, &rp_count, wp, 4);
    EXPECT_EQ(0, strcmp(rp_str, wp));
    EXPECT_EQ(12, rp_count);

    rp_count = 10;
    dlt_getloginfo_conv_ascii_to_string(rp_2, &rp_count, wp, 4);
    EXPECT_EQ(0, strcmp(rp_str, wp));
    EXPECT_EQ(22, rp_count);

}

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
