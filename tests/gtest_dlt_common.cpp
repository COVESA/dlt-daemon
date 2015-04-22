#include <stdio.h>
#include "gtest/gtest.h"
#include <limits.h>

extern "C" {
#include "dlt-daemon.h"
#include <dlt-daemon_cfg.h>
#include "dlt_user.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_user_cfg.h"
int dlt_buffer_increase_size(DltBuffer *);
int dlt_buffer_minimize_size(DltBuffer *);
int dlt_buffer_reset(DltBuffer *);
int dlt_buffer_push(DltBuffer *,const unsigned char *,unsigned int);
int dlt_buffer_push3(DltBuffer *,const unsigned char *,unsigned int,const unsigned char *,unsigned int,const unsigned char *,unsigned int);
int dlt_buffer_get(DltBuffer *,unsigned char *, int,int);
int dlt_buffer_pull(DltBuffer *,unsigned char *, int);
int dlt_buffer_remove(DltBuffer *);
}

/* Beginn Method: dlt_common::dlt_buffer_init_dynamic */
TEST(t_dlt_buffer_init_dynamic, normal)
{
    DltBuffer init_dynamic;

    // Normal Use-Case for initializing a buffer
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&init_dynamic, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&init_dynamic));

    // Min Values for a success init
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&init_dynamic, 12,12,12));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&init_dynamic));
}
TEST(t_dlt_buffer_init_dynamic, abnormal)
{
    DltBuffer buf;

    // Initialze buffer twice, expected -1 for second init
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Initialize buffer with max-value of uint32, expected 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, UINT_MAX,UINT_MAX,UINT_MAX));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Initialize buffer with min-value of uint32, expected 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, 0,0,0));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Initialize buffer min-value > max-value, expected -1
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Initialsize buffer step-value > max-value, expected -1
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE * 2));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_init_dynamic, nullpointer)
{
    DltBuffer buf;

    // NULL-Pointer, expect -1
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, NULL, NULL, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, NULL, DLT_USER_RINGBUFFER_MAX_SIZE, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, NULL, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, NULL, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(NULL, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, NULL, NULL, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, NULL, DLT_USER_RINGBUFFER_MAX_SIZE, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, NULL, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, NULL, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, NULL));
}
/* End Method: dlt_common::dlt_buffer_init_dynamic */




/* Beginn Method: dlt_common::dlt_buffer_free_dynamic */
TEST(t_dlt_buffer_free_dynamic, normal)
{
    DltBuffer buf;

    // Normal Use-Case szenario
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case szenario
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, 12,12,12));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_free_dynamic, abnormal)
{
    DltBuffer buf;

    // Free uninizialised buffer, expected -1
    EXPECT_GE(-1,dlt_buffer_free_dynamic(&buf));

    // Free buffer twice, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
    EXPECT_GE(-1,dlt_buffer_free_dynamic(&buf));

}
TEST(t_dlt_buffer_free_dynamic, nullpointer)
{
    // NULL-POinter
    EXPECT_GE(-1, dlt_buffer_free_dynamic(NULL));
}
/* End Method: dlt_common::dlt_buffer_free_dynamic */


/* Beginn Method: dlt_common::dlt_buffer_increase_size */
TEST(t_dlt_buffer_increase_size, normal)
{
    DltBuffer buf;

    // Normal Use-Case, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_increase_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Fill buffer to max-value, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i = 0; i <= (DLT_USER_RINGBUFFER_MAX_SIZE / DLT_USER_RINGBUFFER_MIN_SIZE); i += DLT_USER_RINGBUFFER_STEP_SIZE)
    {
        EXPECT_EQ(0,dlt_buffer_increase_size(&buf));
    }
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_increase_size, abnormal)
{
    DltBuffer buf;

    // Increase uninizialised buffer
    EXPECT_GE(-1, dlt_buffer_increase_size(&buf));

    // Fill buffer over max-value, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_increase_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // min-value > max-value, trying to increase buffer, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_increase_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // trying to increase buffer with 0 , expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, 0));
    EXPECT_GE(-1, dlt_buffer_increase_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_increase_size, nullpointer)
{
    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_increase_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_increase_size */




/* Beginn Method: dlt_common::dlt_buffer_minimize_size */
TEST(t_dlt_buffer_minimize_size, normal)
{
    DltBuffer buf;

    // Normal Use-Case, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_minimize_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // minimize buffer to min-value, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i = (DLT_USER_RINGBUFFER_MAX_SIZE / DLT_USER_RINGBUFFER_MIN_SIZE); i >= 0; i -= DLT_USER_RINGBUFFER_STEP_SIZE)
    {
        EXPECT_EQ(0,dlt_buffer_minimize_size(&buf));
    }
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_minimize_size, abnormal)
{
    DltBuffer buf;

    // Minimize uninizialised buffer
    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf));

    // minimize buffer under min-value, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // min-value > max-value, trying to minimize buffer, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // trying to minimize buffer with 0 , expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, 0));
    EXPECT_GE(-1, dlt_buffer_minimize_size(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_minimize_size, nullpointer)
{
    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_minimize_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_minimize_size */




/* Beginn Method: dlt_common::dlt_buffer_reset */
TEST(t_dlt_buffer_reset, normal)
{
    DltBuffer buf;

    // Normal Use-Case. expect 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_reset(&buf));

}
TEST(t_dlt_buffer_reset, abnormal)
{
    DltBuffer buf;

    //Use uninizialsied buffer, expected -1
    EXPECT_GE(-1, dlt_buffer_reset(&buf));
}
TEST(t_dlt_buffer_reset, nullpointer)
{
    //Use NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_reset(NULL));
}
/* End Method: dlt_common::dlt_buffer_reset */




/* Beginn Method: dlt_common::dlt_buffer_push*/
TEST(t_dlt_buffer_push, normal)
{
    DltBuffer buf;
    char * test;
    int size = sizeof(test);

    // Normal Use-Case, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0, dlt_buffer_push(&buf,(unsigned char *)&test,size));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Push till buffer is full, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i=0; i<= (DLT_USER_RINGBUFFER_MIN_SIZE/size); i++)
    {
        EXPECT_EQ(0, dlt_buffer_push(&buf,(unsigned char *)&test,size));
    }
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_push, abnormal)
{
    DltBuffer buf;
    char * test;
    int size = sizeof(test);

    // Use uninizialsied, expected -1
    EXPECT_GE(-1, dlt_buffer_push(&buf,(unsigned char *)&test,size));


    // set size == 0, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_push(&buf,(unsigned char *)&test,0));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // set size == 0 and char == 0 expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_push(&buf,0,0));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Push till buffer is overfilled , expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i=0; i<= (DLT_USER_RINGBUFFER_MIN_SIZE/size) + size; i++)
    {
        if(i <= DLT_USER_RINGBUFFER_MIN_SIZE)
            EXPECT_EQ(0, dlt_buffer_push(&buf,(unsigned char *)&test,size));
        else
            EXPECT_GE(-1, dlt_buffer_push(&buf,(unsigned char *)&test,size));
    }
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_push, nullpointer)
{
    DltBuffer buf;
    char * test;
    int size = sizeof(test);

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_push(NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push(NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push(NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push(NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push(&buf,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push(&buf,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push(&buf,(unsigned char *)&test,NULL));
}
/* End Method: dlt_common::dlt_buffer_push*/




/* Begin Method: dlt_common::dlt_buffer_push3 */
TEST(t_dlt_buffer_push3, normal)
{
    DltBuffer buf;
    char * test;
    int size = sizeof(test);

    // Normal Use-Case, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0, dlt_buffer_push3(&buf,(unsigned char *)&test,size,0,0,0,0));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Push till buffer is full, expected 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i=0; i<= (DLT_USER_RINGBUFFER_MIN_SIZE/size); i++)
    {
        EXPECT_EQ(0, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
    }
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_push3, abnormal)
{
    DltBuffer buf;
    char * test;
    int size = sizeof(test);

    // Use uninizialsied, expected -1
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size));


    // set size == 0, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,0, (unsigned char *)&test,0, (unsigned char *)&test,0));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // set size == 0 and char == 0 expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,0,0,0,0,0,0));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Push till buffer is overfilled , expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i=0; i<= (DLT_USER_RINGBUFFER_MIN_SIZE/size) + size; i++)
    {
        if(i <= DLT_USER_RINGBUFFER_MIN_SIZE)
            EXPECT_EQ(0, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
        else
            EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
    }
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_push3, nullpointer)
{
    DltBuffer buf;
    char * test;
    int size = sizeof(test);

    //Null Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,NULL,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,NULL,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(NULL,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,NULL,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,NULL,size,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,NULL,(unsigned char *)&test,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,NULL,size,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,NULL,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,NULL,(unsigned char *)&test,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,NULL,(unsigned char *)&test,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,NULL,NULL));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,NULL,size));
    EXPECT_GE(-1, dlt_buffer_push3(&buf,(unsigned char *)&test,size,(unsigned char *)&test,size,(unsigned char *)&test,NULL));
}
/* End Method: dlt_common::dlt_buffer_push3 */




/* Begin Method: dlt_common::dlt_buffer_pull */
TEST(t_dlt_buffer_pull, normal)
{
    //Normal Use Cases, expected 0 or -1 in return
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    // Normal Use-Case, empty pull, expected -1
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, (unsigned char*)&header, size));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case, expected > 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    EXPECT_LE(1, dlt_buffer_pull(&buf, (unsigned char*)&header, size));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_pull, abnormal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_buffer_pull(&buf, (unsigned char*)&header, size));

    // data == 0 and max_size == 0, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, 0, 0));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // no push before pull, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, 0, 0));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_pull, nullpointer)
{
    DltBuffer buf;
    DltUserHeader header;

    // NULL-Point, expected -1
    EXPECT_GE(-1, dlt_buffer_pull(NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_buffer_pull(NULL, NULL, sizeof(DltUserHeader)));
    EXPECT_GE(-1, dlt_buffer_pull(NULL, (unsigned char *)&header, NULL));
    EXPECT_GE(-1, dlt_buffer_pull(NULL, (unsigned char *)&header, sizeof(DltUserHeader)));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, NULL, NULL));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, NULL, sizeof(DltUserHeader)));
    EXPECT_GE(-1, dlt_buffer_pull(&buf, (unsigned char *)&header, NULL));
}
/* End Method: dlt_common::dlt_buffer_pull */



/* Begin Method: dlt_common::dlt_buffer_remove */
TEST(t_dlt_buffer_remove, normal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    // Normal Use-Case, empty pull, expected -1
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_remove(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case, expected > 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    EXPECT_LE(1, dlt_buffer_remove(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_remove, abnormal)
{
    DltBuffer buf;
    DltUserHeader header;

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_buffer_remove(&buf));

    // no push before remove, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_remove(&buf));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));

    // Call remove 10 time, expected > 1 till buffer is empty
    // pushed one time so expect one > 1 and 9 times < 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    for(int i=0; i<10;i++)
    {
        if(i == 0)
            EXPECT_LE(1, dlt_buffer_remove(&buf));
        else
            EXPECT_GE(-1, dlt_buffer_remove(&buf));
    }
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_remove, nullpointer)
{
    // NULL_Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_remove(NULL));
}
/* End Method: dlt_common::dlt_buffer_remove*/




/* Begin Method: dlt_common::dlt_buffer_copy */
TEST(t_dlt_buffer_copy, normal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    // Normal Use-Case, empty pull, expected -1
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_GE(-1, dlt_buffer_copy(&buf, (unsigned char *)&header, size));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case, expected > 0
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    EXPECT_LE(1, dlt_buffer_copy(&buf, (unsigned char *)&header, size));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_copy, abnormal)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    // Uninizialised buffer , expected -1
    EXPECT_LE(-1, dlt_buffer_copy(&buf, (unsigned char *)&header, size));

    // no push before copy, expected -1
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_LE(-1, dlt_buffer_copy(&buf, (unsigned char *)&header, size));
    EXPECT_EQ(0,dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_copy, nullpointer)
{
    DltBuffer buf;
    DltUserHeader header;
    int size = sizeof(DltUserHeader);

    // NULL-Pointer, expected -1
    EXPECT_LE(-1, dlt_buffer_copy(NULL,NULL,size));
    EXPECT_LE(-1, dlt_buffer_copy(NULL,NULL,NULL));
    EXPECT_LE(-1, dlt_buffer_copy(NULL,(unsigned char *)&header,size));
    EXPECT_LE(-1, dlt_buffer_copy(NULL,(unsigned char *)&header,NULL));
    EXPECT_LE(-1, dlt_buffer_copy(&buf,NULL,size));
    EXPECT_LE(-1, dlt_buffer_copy(&buf,NULL,NULL));
    EXPECT_LE(-1, dlt_buffer_copy(&buf,(unsigned char *)&header,size));
}
/* End Method: dlt_common::dlt_buffer_copy */



/* Begin Method: dlt_common::dlt_buffer_get */
/*
TEST(t_dlt_buffer_get, normal)
{
    DltBuffer get;
    DltUserHeader head_get;

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_LE(0, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_LE(0, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[0] = 50000;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[1] = 50000;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[2] = -50000;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[2] = 0;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[0] = 4000;
    ((int*)(get.shm))[1] = 5000;
    ((int*)(get.shm))[2] = 0;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[0] = 10;
    ((int*)(get.shm))[1] = 5;
    ((int*)(get.shm))[2] = 5;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[2] = 50000;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[0] = 19;
    EXPECT_EQ(-1, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    ((int*)(get.shm))[2] = 19;
    EXPECT_LE(0, dlt_buffer_get(&get,(unsigned char*)&head_get,5,1));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

}
TEST(t_dlt_buffer_get, abnormal)
{
    DltBuffer get;
    DltUserHeader head_get;

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_LE(0, dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),123456789123456789));      // Delete: suggest int to bool
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_LE(0, dlt_buffer_get(&get,NULL,sizeof(DltUserHeader),1));                                            // unsigned char = NULL still work
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));

    EXPECT_EQ(0, dlt_buffer_init_dynamic(&get, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&get,(unsigned char *)&head_get,sizeof(DltUserHeader)));
    printf("#### %i\n", dlt_buffer_get(&get,(unsigned char*)&head_get,sizeof(DltUserHeader),0));
    EXPECT_LE(0, dlt_buffer_get(&get,NULL,NULL,1));                                            // unsigned char = NULL still work && size = NULL
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&get));
}
TEST(t_dlt_buffer_get, nullpointer)
{
    DltBuffer get;
    DltUserHeader head_get;

    EXPECT_DEATH({dlt_buffer_get(NULL, (unsigned char*)&head_get,sizeof(DltUserHeader),0);}, "");
    EXPECT_DEATH({dlt_buffer_get(&get, (unsigned char*)&head_get,sizeof(DltUserHeader),0);}, "");
}
*/
/* End Method: dlt_common::dlt_buffer_get */




/* Begin MEthod: dlt_common::dlt_buffer_get_message_count */
TEST(t_dlt_buffer_get_message_count, normal)
{
    DltBuffer buf;
    DltUserHeader header;

    // Normal Usce-Case without pushing data, expected 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    //printf("##### %i\n", dlt_buffer_get_message_count(&buf));
    EXPECT_EQ(0, dlt_buffer_get_message_count(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case, with pushing data, expected > 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    //printf("#### %i\n", dlt_buffer_get_message_count(&buf));
    EXPECT_LE(0, dlt_buffer_get_message_count(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Pushing 1000 mesages, expected 10000
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i = 1; i <= 10000; i++)
    {
        EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
        //printf("#### %i\n", dlt_buffer_get_message_count(&buf));
        EXPECT_EQ(i, dlt_buffer_get_message_count(&buf));
    }
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

}
TEST(t_dlt_buffer_get_message_count, abnormal)
{
    DltBuffer buf;
    DltUserHeader header;

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_buffer_get_message_count(&buf));


}
TEST(t_dlt_buffer_get_message_count, nullpointer)
{
    //NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_buffer_get_message_count(NULL));
}
/* Begin MEthod: dlt_common::dlt_buffer_get_message_count */




/* Begin Method: dlt_common::dlt_buffer_get_total_size*/
TEST(t_dlt_buffer_get_total_size, normal)
{
    DltBuffer buf;
    DltUserHeader header;

    // Normal Use-Case, expected max buffer size (DLT_USER_RINGBUFFER_MAX_SIZE)
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    //printf("##### %i\n", dlt_buffer_get_total_size(&buf));
    EXPECT_EQ(DLT_USER_RINGBUFFER_MAX_SIZE, dlt_buffer_get_total_size(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case, 1st pushing data, expected max buffer size (DLT_USER_RINGBUFFER_MAX_SIZE)
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    //printf("##### %i\n", dlt_buffer_get_total_size(&buf));
    EXPECT_EQ(DLT_USER_RINGBUFFER_MAX_SIZE, dlt_buffer_get_total_size(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_get_total_size, abnormal)
{
    DltBuffer buf;

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_buffer_get_total_size(&buf));
}
TEST(t_dlt_buffer_get_total_size, nullpointer)
{
    // NULL-Pointer, expect -1
    EXPECT_GE(-1, dlt_buffer_get_total_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_get_total_size*/



/* Begin Method: dlt_common::dlt_buffer_get_used_size*/
TEST(t_dlt_buffer_get_used_size, normal)
{
    DltBuffer buf;
    DltUserHeader header;

    // Normal Use Cas buffer empty, expected 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    //printf("##### %i\n", dlt_buffer_get_used_size(&buf));
    EXPECT_EQ(0, dlt_buffer_get_used_size(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case with pushing data, expected > 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
    //printf("##### %i\n", dlt_buffer_get_used_size(&buf));
    EXPECT_LE(0, dlt_buffer_get_used_size(&buf));
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));

    // Normal Use-Case with pushing 10000 data, expected > 0
    EXPECT_EQ(0, dlt_buffer_init_dynamic(&buf, DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
    for(int i = 1; i <= 10000; i++)
    {
        EXPECT_EQ(0,dlt_buffer_push(&buf,(unsigned char *)&header,sizeof(DltUserHeader)));
        //printf("#### %i\n", dlt_buffer_get_used_size(&buf));
        EXPECT_LE(1, dlt_buffer_get_used_size(&buf));
    }
    EXPECT_EQ(0, dlt_buffer_free_dynamic(&buf));
}
TEST(t_dlt_buffer_get_used_size, abnormal)
{
    DltBuffer buf;

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_buffer_get_used_size(&buf));
}
TEST(t_dlt_buffer_get_used_size, nullpointer)
{
    //NULL-Pointer, expcted -1
    EXPECT_GE(-1, dlt_buffer_get_used_size(NULL));
}
/* End Method: dlt_common::dlt_buffer_get_used_size*/


/*##############################################################################################################################*/
/*##############################################################################################################################*/
/*##############################################################################################################################*/


/* Begin Method: dlt_common::dlt_message_init*/
TEST(t_dlt_message_init, normal)
{
    DltMessage msg;

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_message_init(&msg, 0));
    EXPECT_EQ(0, dlt_message_free(&msg, 0));

    EXPECT_EQ(0, dlt_message_init(&msg, 1));
    EXPECT_EQ(0, dlt_message_free(&msg, 0));
}
TEST(t_dlt_message_init, abnormal)
{
    DltMessage msg;

    // Double use init, expected -1
    EXPECT_EQ(0, dlt_message_init(&msg,0));
    EXPECT_GE(-1, dlt_message_init(&msg,0));
    EXPECT_EQ(0, dlt_message_free(&msg,0));
    EXPECT_EQ(0, dlt_message_init(&msg,1));
    EXPECT_GE(-1, dlt_message_init(&msg,1));
    EXPECT_EQ(0, dlt_message_free(&msg,1));

    // set Verbose to 12345678, expected -1
    EXPECT_GE(-1, dlt_message_init(&msg,12345678));
}
TEST(t_dlt_message_init, nullpointer)
{
    DltMessage msg;

    //NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_init(NULL, NULL));
    EXPECT_GE(-1, dlt_message_init(NULL, 0));
    EXPECT_GE(-1, dlt_message_init(NULL, 1));
    EXPECT_GE(-1, dlt_message_init(&msg, NULL));
}
/* End Method: dlt_common::dlt_message_init*/




/* Begin Method: dlt_common::dlt_message_free */
TEST(t_dlt_message_free, normal)
{
    DltMessage msg;

    // Normal Use Case, expected 0
    EXPECT_EQ(0, dlt_message_init(&msg, 0));
    EXPECT_EQ(0, dlt_message_free(&msg, 0));

    EXPECT_EQ(0, dlt_message_init(&msg, 0));
    EXPECT_EQ(0, dlt_message_free(&msg, 1));
}
TEST(t_dlt_message_free, abnormal)
{
    DltMessage msg;

    // Double use free, expected -1
    EXPECT_EQ(0, dlt_message_init(&msg,0));
    EXPECT_EQ(0, dlt_message_free(&msg,0));
    EXPECT_GE(-1, dlt_message_free(&msg,0));

    EXPECT_EQ(0, dlt_message_init(&msg,0));
    EXPECT_EQ(0, dlt_message_free(&msg,1));
    EXPECT_GE(-1, dlt_message_free(&msg,1));

    // set Verbose to 12345678, expected -1
    EXPECT_GE(-1, dlt_message_free(&msg,12345678));
}
TEST(t_dlt_message_free, nullpointer)
{
    DltMessage msg;

    //NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_free(NULL, NULL));
    EXPECT_GE(-1, dlt_message_free(NULL, 0));
    EXPECT_GE(-1, dlt_message_free(NULL, 1));
    EXPECT_GE(-1, dlt_message_free(&msg, NULL));
}
/* End Method: dlt_common::dlt_message_free */




/* Begin Method: dlt_common::dlt_file_open */
TEST(t_dlt_file_open, normal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    EXPECT_EQ(0, dlt_file_free(&file, 0));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 1));
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_file_open, abnormal)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Uninizialsied, expected -1
    EXPECT_GE(-1, dlt_file_open(&file, openfile, 0));
    EXPECT_GE(-1, dlt_file_open(&file, openfile, 1));

    // Verbose set to 12345678
    EXPECT_GE(-1, dlt_file_open(&file, openfile, 12345678));

    // Path doesn't exist, expected -1
    EXPECT_GE(-1, dlt_file_open(&file, "This Path doesn't exist!!", 0));
}
TEST(t_dlt_file_open, nullpointer)
{
    DltFile file;
    /* Get PWD so file can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_file_open(NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_file_open(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_file_open(NULL, NULL, 1));
    EXPECT_GE(-1, dlt_file_open(NULL, openfile, NULL));
    EXPECT_GE(-1, dlt_file_open(NULL, openfile, 0));
    EXPECT_GE(-1, dlt_file_open(NULL, openfile, 1));
    EXPECT_GE(-1, dlt_file_open(&file, NULL, NULL));
    EXPECT_GE(-1, dlt_file_open(&file, NULL, 0));
    EXPECT_GE(-1, dlt_file_open(&file, NULL, 1));
    EXPECT_GE(-1, dlt_file_open(&file, openfile, NULL));
}
/* End Method: dlt_common::dlt_file_open */




/* Begin Method: dlt_common::dlt_message_print_ascii*/
TEST(t_dlt_message_print_ascii, normal)
{

    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_ascii, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // No messages read, expected -1
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));

    // Set verbose to 12345678
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_ascii, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,text,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,text,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_ascii(NULL,text,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_ascii(&file.msg,text,NULL,1));
}
/* End Method: dlt_common::dlt_message_print_ascii*/




/* Begin Method: dlt_common::dlt_message_print_ascii with filter*/
TEST(t_dlt_message_print_ascii_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_filter_init(&filter, 0));
    EXPECT_EQ(0, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_EQ(0, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_ascii_with_filter, abnormal)
{
    // equal with t_dlt_message_print_ascii
}
TEST(t_dlt_message_print_ascii_with_filter, nullpointer)
{
    // equal with t_dlt_message_print_ascii
}
/* End Method: dlt_common::dlt_message_print_ascii with filter*/




/* Begin Method: dlt_common::dlt_message_print_header */
TEST(t_dlt_message_print_header, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_header, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // No messages read, expected -1
    EXPECT_GE(-1, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));

    // Set verbose to 12345678
    EXPECT_GE(-1, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_header, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_print_header(NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_header(NULL,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_header(NULL,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_header(NULL,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_header(NULL,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_header(NULL,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_header(NULL,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_header(NULL,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_header(NULL,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_header(NULL,text,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_header(NULL,text,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_header(NULL,text,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_header(&file.msg,text,DLT_DAEMON_TEXTSIZE,NULL));
}
/* End Method: dlt_common::dlt_message_print_header */




/* Begin Method: dlt_common::dlt_message_print_header with filter */
TEST(t_dlt_message_print_header_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_filter_init(&filter, 0));
    EXPECT_EQ(0, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_EQ(0, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_header_with_filter, abnormal)
{
    // equal with t_dlt_message_print_header
}
TEST(t_dlt_message_print_header_with_filter, nullpointer)
{
    // equal with t_dlt_message_print_header
}
/* End Method: dlt_common::dlt_message_print_header with filter */




/* Begin Method: dlt_common::dlt_message_print_hex */
TEST(t_dlt_message_print_hex, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_hex, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // No messages read, expected -1
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));

    // Set verbose to 12345678
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_hex, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_print_hex(NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,text,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,text,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_hex(NULL,text,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_hex(&file.msg,text,DLT_DAEMON_TEXTSIZE,NULL));
}
/* End Method: dlt_common::dlt_message_print_hex */





/* Begin Method: dlt_common::dlt_message_print_hex with filter */
TEST(t_dlt_message_print_hex_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_filter_init(&filter, 0));
    EXPECT_EQ(0, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_EQ(0, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_hex(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_hex_with_filter, abnormal)
{
    // equal with t_dlt_message_print_hex
}
TEST(t_dlt_message_print_hex_with_filter, nullpointer)
{
    // equal with t_dlt_message_print_hex
}
/* End Method: dlt_common::dlt_message_print_hex with filter */




/* Begin Method: dlt_common::dlt_message_print_mixed_plain */
TEST(t_dlt_message_print_mixed_plain, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_plain, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // No messages read, expected -1
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));

    // Set verbose to 12345678
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_plain, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,text,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,text,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(NULL,text,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_plain(&file.msg,text,DLT_DAEMON_TEXTSIZE,NULL));
}
/* End Method: dlt_common::dlt_message_print_mixed_pain */





/* Begin Method: dlt_common::dlt_message_print_mixed_plain with filter */
TEST(t_dlt_message_print_mixed_plain_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_filter_init(&filter, 0));
    EXPECT_EQ(0, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_EQ(0, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_plain(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_plain_with_filter, abnormal)
{
    // equal with t_dlt_message_print_mixed_plain
}
TEST(t_dlt_message_print_mixed_plain_with_filter, nullpointer)
{
    // equal with t_dlt_message_print_mixed_plain
}
/* End Method: dlt_common::dlt_message_print_mixed_pain with filter */



/* Begin Method: dlt_common::dlt_message_print_mixed_html */
TEST(t_dlt_message_print_mixed_html, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_html, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // No messages read, expected -1
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));

    // Set verbose to 12345678
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_html, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,text,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,text,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(NULL,text,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,NULL,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,NULL,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,NULL,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,NULL,DLT_DAEMON_TEXTSIZE,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,text,NULL,NULL));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,text,NULL,0));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,text,NULL,1));
    EXPECT_GE(-1, dlt_message_print_mixed_html(&file.msg,text,DLT_DAEMON_TEXTSIZE,NULL));
}
/* End Method: dlt_common::dlt_message_print_mixed_html */




/* Begin Method: dlt_common::dlt_message_print_mixed_html_with filter */
TEST(t_dlt_message_print_mixed_html_with_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_filter_init(&filter, 0));
    EXPECT_EQ(0, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_EQ(0, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_print_mixed_html(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_print_mixed_html_with_filter, abnormal)
{
    // equal with t_dlt_message_print_mixed_html
}
TEST(t_dlt_message_print_mixed_html_with_filter, nullpointer)
{
    // equal with t_dlt_message_print_mixed_html
}
/* End Method: dlt_common::dlt_message_print_mixed_html_with filter */




/* Begin Method:dlt_common::dlt_message_filter_check */
TEST(t_dlt_message_filter_check, normal)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected > 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_filter_init(&filter, 0));
    EXPECT_EQ(0, dlt_filter_load(&filter, openfilter, 0));
    EXPECT_EQ(0, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_LE(0, dlt_message_filter_check(&file.msg, &filter, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_LE(0, dlt_message_filter_check(&file.msg, &filter, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_filter_check, abnormal)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // No messages read, expected -1
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, &filter, 0));
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, &filter, 1));

    // Set verbose to 12345678
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, &filter, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_filter_check(&file.msg, &filter, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_filter_check, nullpointer)
{
    DltFile file;
    DltFilter filter;

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    char * openfilter;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    openfilter = (char*)malloc(100 + 17);
    sprintf(openfilter, "%s/testfilter.txt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_filter_check(NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_filter_check(NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_filter_check(NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_filter_check(NULL, &filter, NULL));
    EXPECT_GE(-1, dlt_message_filter_check(NULL, &filter, 0));
    EXPECT_GE(-1, dlt_message_filter_check(NULL, &filter, 1));
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, NULL, NULL));
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, NULL, 0));
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, NULL, 1));
    EXPECT_GE(-1, dlt_message_filter_check(&file.msg, &filter, NULL));
}
/* End Method:dlt_common::dlt_message_filter_check */




/* Begin Method:dlt_common::dlt_message _get_extraparameters */
TEST(t_dlt_message_get_extraparamters, normal)
{
    DltFile file;

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect >0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_LE(0, dlt_message_get_extraparameters(&file.msg, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_LE(0, dlt_message_get_extraparameters(&file.msg, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_get_extraparamters, abnormal)
{
    DltFile file;

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_message_get_extraparameters(&file.msg, 0));
    EXPECT_GE(-1, dlt_message_get_extraparameters(&file.msg, 1));

    // set verbose to 12345678, expected -1
    EXPECT_GE(-1, dlt_message_get_extraparameters(&file.msg, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_get_extraparameters(&file.msg, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));

}
TEST(t_dlt_message_get_extraparamters, nullpointer)
{
    DltFile file;

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_get_extraparameters(NULL, NULL));
    EXPECT_GE(-1, dlt_message_get_extraparameters(NULL, 0));
    EXPECT_GE(-1, dlt_message_get_extraparameters(NULL, 1));
    EXPECT_GE(-1, dlt_message_get_extraparameters(&file.msg, NULL));
}
/* End Method:dlt_common::dlt_message_get_extraparameters */




/* Begin Method:dlt_common::dlt_message_header */
TEST(t_dlt_message_header, normal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
        printf("%s \n",text);
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
        printf("%s \n",text);
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_header, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));

    // set verbose to 12345678, expected -1
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, 12345678));
        printf("%s \n",text);
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_header, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_header(NULL, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header(NULL, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_header(NULL, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL));
    EXPECT_GE(-1, dlt_message_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_header(NULL, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(-1, dlt_message_header(NULL, text, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header(NULL, text, NULL, 0));
    EXPECT_GE(-1, dlt_message_header(NULL, text, NULL, 1));
    EXPECT_GE(-1, dlt_message_header(NULL, text, DLT_DAEMON_TEXTSIZE, NULL));
    EXPECT_GE(-1, dlt_message_header(NULL, text, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_header(NULL, text, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(-1, dlt_message_header(&file.msg, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header(&file.msg, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_header(&file.msg, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL));
    EXPECT_GE(-1, dlt_message_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 0));
    EXPECT_GE(-1, dlt_message_header(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, 1));
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, NULL, 0));
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, NULL, 1));
    EXPECT_GE(-1, dlt_message_header(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL));
}
/* End Method:dlt_common::dlt_message_header */




/* Begin Method:dlt_common::dlt_message_header_flags */
TEST(t_dlt_message_header_flags, normal)
{
    /* Possible Flags*/
    //#define DLT_HEADER_SHOW_NONE       0x0000
    //#define DLT_HEADER_SHOW_TIME       0x0001
    //#define DLT_HEADER_SHOW_TMSTP      0x0002
    //#define DLT_HEADER_SHOW_MSGCNT     0x0004
    //#define DLT_HEADER_SHOW_ECUID      0x0008
    //#define DLT_HEADER_SHOW_APID       0x0010
    //#define DLT_HEADER_SHOW_CTID       0x0020
    //#define DLT_HEADER_SHOW_MSGTYPE    0x0040
    //#define DLT_HEADER_SHOW_MSGSUBTYPE 0x0080
    //#define DLT_HEADER_SHOW_VNVSTATUS  0x0100
    //#define DLT_HEADER_SHOW_NOARG      0x0200
    //#define DLT_HEADER_SHOW_ALL        0xFFFF
    /*########################################*/


    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
        printf("%s \n",text);
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
        printf("%s \n",text);
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_header_flags, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));

    // USE own DLT_HEADER_SHOW , expected -1
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0x1234, 0));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0x1234, 0));
        printf("%s \n",text);
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_header_flags, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, NULL, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, NULL, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, NULL, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_ALL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_NONE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_TIME, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_TMSTP, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGCNT, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_ECUID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_APID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_CTID, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_VNVSTATUS, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_NOARG, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_ALL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_NONE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_TIME, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_TMSTP, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGCNT, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_ECUID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_APID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_CTID, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_MSGSUBTYPE, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_VNVSTATUS, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_NOARG, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, NULL, DLT_HEADER_SHOW_ALL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NONE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TIME, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_TMSTP, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGCNT, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ECUID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_APID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_CTID, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_MSGSUBTYPE, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_VNVSTATUS, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_NOARG, NULL));
    EXPECT_GE(-1, dlt_message_header_flags(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_HEADER_SHOW_ALL, NULL));
}
/* End Method:dlt_common::dlt_message_header_flags */




/* Begin Method:dlt_common::dlt_message_payload */
TEST(t_dlt_message_payload, normal)
{
    /* Types */
    //#define DLT_OUTPUT_HEX              1
    //#define DLT_OUTPUT_ASCII            2
    //#define DLT_OUTPUT_MIXED_FOR_PLAIN  3
    //#define DLT_OUTPUT_MIXED_FOR_HTML   4
    //#define DLT_OUTPUT_ASCII_LIMITED    5
    /*####################################*/

    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expected 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
        printf("%s \n",text);
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
        printf("%s \n",text);
        EXPECT_EQ(0, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
        printf("%s \n",text);
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_payload, abnormal)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));

    // USE own DLT_HEADER_SHOW , expected -1
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, 99, 0));

    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, 99, 0));
        printf("%s \n",text);
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));

    // set verbose to 12345678
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 12345678));
        EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 12345678));
        EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 12345678));
        EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 12345678));
        EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_payload, nullpointer)
{
    DltFile file;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file and filter can be used*/
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expected -1
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, NULL, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, NULL, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(NULL, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, NULL, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, NULL, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_HEX, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_ASCII, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_ASCII_LIMITED, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_HEX, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_ASCII, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_MIXED_FOR_PLAIN, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_MIXED_FOR_HTML, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, NULL, DLT_OUTPUT_ASCII_LIMITED, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL, 0));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, NULL, 1));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_PLAIN, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_MIXED_FOR_HTML, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_ASCII_LIMITED, NULL));
    EXPECT_GE(-1, dlt_message_payload(&file.msg, text, DLT_DAEMON_TEXTSIZE, DLT_OUTPUT_HEX, 0));
}
/* End Method:dlt_common::dlt_message_payload */




/* Begin Method:dlt_common::dlt_message_set_extraparameters */
TEST(t_dlt_message_set_extraparamters, normal)
{
    DltFile file;
    // Get PWD so file and filter can be used
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Normal Use-Case, expect 0
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_set_extraparameters(&file.msg, 0));
    }
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_EQ(0, dlt_message_set_extraparameters(&file.msg, 1));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));
}
TEST(t_dlt_message_set_extraparamters, abnormal)
{
    DltFile file;
    // Get PWD so file and filter can be used
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // Uninizialised, expected -1
    EXPECT_GE(-1, dlt_message_set_extraparameters(&file.msg, 0));
    EXPECT_GE(-1, dlt_message_set_extraparameters(&file.msg, 1));

    // set verbos to 12345678
    EXPECT_EQ(0, dlt_file_init(&file, 0));
    EXPECT_EQ(0, dlt_file_open(&file, openfile, 0));
    while (dlt_file_read(&file,0)>=0){}
    for(int i=0;i<file.counter;i++)
    {
        EXPECT_EQ(0, dlt_file_message(&file, i, 0));
        EXPECT_GE(-1, dlt_message_set_extraparameters(&file.msg, 12345678));
    }
    EXPECT_EQ(0, dlt_file_free(&file, 0));

}
TEST(t_dlt_message_set_extraparamters, nullpointer)
{
    DltFile file;
    // Get PWD so file and filter can be used
    char pwd[100];
    getcwd(pwd, 100);
    char * openfile;
    openfile = (char*)malloc(100 + 14);
    sprintf(openfile, "%s/testfile.dlt", pwd);
    /*---------------------------------------*/

    // NULL-Pointer, expect -1
    EXPECT_GE(-1, dlt_message_set_extraparameters(NULL, NULL));
    EXPECT_GE(-1, dlt_message_set_extraparameters(NULL, 0));
    EXPECT_GE(-1, dlt_message_set_extraparameters(NULL, 1));
    EXPECT_GE(-1, dlt_message_set_extraparameters(&file.msg, NULL));
}
/* End Method:dlt_common::dlt_message_set_extraparameters */




int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::FLAGS_gtest_break_on_failure = true;
    //::testing::FLAGS_gtest_repeat = 10000;
    //::testing::FLAGS_gtest_filter = "*.nullpointer";
    return RUN_ALL_TESTS();
}
