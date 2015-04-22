#include <stdio.h>
#include "gtest/gtest.h"
#include <limits.h>

extern "C" {
#include "dlt_common.h"
#include "dlt_user.h"
/*#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_user_cfg.h"*/
}


// tested functions
/*
int dlt_user_log_write_start(DltContext *handle, DltContextData *log, DltLogLevelType loglevel);
int dlt_user_log_write_start_id(DltContext *handle, DltContextData *log, DltLogLevelType loglevel, uint32_t messageid);
int dlt_user_log_write_finish(DltContextData *log);
int dlt_user_log_write_bool(DltContextData *log, uint8_t data);
int dlt_user_log_write_float32(DltContextData *log, float32_t data);
int dlt_user_log_write_float64(DltContextData *log, double data);
int dlt_user_log_write_uint(DltContextData *log, unsigned int data);
int dlt_user_log_write_uint8(DltContextData *log, uint8_t data);
int dlt_user_log_write_uint16(DltContextData *log, uint16_t data);
int dlt_user_log_write_uint32(DltContextData *log, uint32_t data);
int dlt_user_log_write_uint64(DltContextData *log, uint64_t data);
int dlt_user_log_write_uint8_formatted(DltContextData *log, uint8_t data, DltFormatType type);
int dlt_user_log_write_uint16_formatted(DltContextData *log, uint16_t data, DltFormatType type);
int dlt_user_log_write_uint32_formatted(DltContextData *log, uint32_t data, DltFormatType type);
int dlt_user_log_write_uint64_formatted(DltContextData *log, uint64_t data, DltFormatType type);
int dlt_user_log_write_int(DltContextData *log, int data);
int dlt_user_log_write_int8(DltContextData *log, int8_t data);
int dlt_user_log_write_int16(DltContextData *log, int16_t data);
int dlt_user_log_write_int32(DltContextData *log, int32_t data);
int dlt_user_log_write_int64(DltContextData *log, int64_t data);
int dlt_user_log_write_string( DltContextData *log, const char *text);
int dlt_user_log_write_constant_string( DltContextData *log, const char *text);
int dlt_user_log_write_utf8_string(DltContextData *log, const char *text);
int dlt_user_log_write_raw(DltContextData *log,void *data,uint16_t length);
int dlt_user_log_write_raw_formatted(DltContextData *log,void *data,uint16_t length,DltFormatType type);
*/

/*
int dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text);
int dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data);
int dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data);
int dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data);
int dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data);
int dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length);
int dlt_log_marker();
*/

/////////////////////////////////////////
// t_dlt_user_log_write_start
TEST(t_dlt_user_log_write_start, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start normal"));
    
    // the defined enum values for log level
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_OFF));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_FATAL));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_ERROR));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_WARN));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_INFO));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEBUG));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_VERBOSE));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_start, abnormal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)-100));
    EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)-10));
    EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)10));
    EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)100));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_start, startstartfinish)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start startstartfinish"));
    
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // shouldn't it return -1, because it is already started?
    EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_start, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start nullpointer"));

    // NULL's
    //EXPECT_GE(-1,dlt_user_log_write_start(NULL, &contextData, DLT_LOG_DEFAULT));  SEGMENTATION FAULT!
    //EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    //EXPECT_GE(-1,dlt_user_log_write_start(NULL, NULL, DLT_LOG_DEFAULT));     SEGMENTATION FAULT!
    EXPECT_GE(-1,dlt_user_log_write_start(&context, NULL, DLT_LOG_DEFAULT));
    //EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_start_id
TEST(t_dlt_user_log_write_start_id, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id normal"));
    
    // the defined enum values for log level
    for (messageid = 0; messageid <= UINT32_MAX - (UINT32_MAX / 10000); messageid += (UINT32_MAX / 10000)) // 10.000 tests
    {
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    }    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_start_id, abnormal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;

    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    for (messageid = 0; messageid <= UINT32_MAX - (UINT32_MAX / 10000); messageid += (UINT32_MAX / 10000)) // 10.000 tests
    {
        EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)-100, messageid));
        EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)-10, messageid));
        EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)10, messageid));
        EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)100, messageid));
    }    

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_start_id, startstartfinish)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id startstartfinish"));
    
    for (messageid = 0; messageid <= UINT32_MAX - (UINT32_MAX / 10000); messageid += (UINT32_MAX / 10000)) // 10.000 tests
    {
        EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
        // shouldn't it return -1, because it is already started?
        EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
        EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    }    
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_start_id, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint32_t messageid;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id nullpointer"));

    // NULL's
    for (messageid = 0; messageid <= UINT32_MAX - (UINT32_MAX / 10000); messageid += (UINT32_MAX / 10000)) // 10.000 tests
    {
        //EXPECT_GE(-1,dlt_user_log_write_start_id(NULL, &contextData, DLT_LOG_DEFAULT, messageid));  SEGMENTATION FAULT!
        //EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
        //EXPECT_GE(-1,dlt_user_log_write_start_id(NULL, NULL, DLT_LOG_DEFAULT, messageid));     SEGMENTATION FAULT!
        EXPECT_GE(-1,dlt_user_log_write_start_id(&context, NULL, DLT_LOG_DEFAULT, messageid));
    }    
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_finish
TEST(t_dlt_user_log_write_finish, finish)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start finish"));
    
    // finish without start
    //EXPECT_GE(-1,dlt_user_log_write_finish(NULL));  SEGMENTATION FAULT!
    //EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));  SEGMENTATION FAULT!
    
    // finish without start, but initialized context
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_finish finish"));
    //EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));  SEGMENTATION FAULT!
    
    // finish with start and initialized context
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    // 2nd finish
    EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_bool
TEST(t_dlt_user_log_write_bool, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = true;
    EXPECT_LE(0,dlt_user_log_write_bool(&contextData, data));
    data = false;
    EXPECT_LE(0,dlt_user_log_write_bool(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_bool, abnormal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 2;
    EXPECT_GE(-1,dlt_user_log_write_bool(&contextData, data));
    data = 100;
    EXPECT_GE(-1,dlt_user_log_write_bool(&contextData, data));
    data = UINT8_MAX;
    EXPECT_GE(-1,dlt_user_log_write_bool(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_bool, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool nullpointer"));

    // NULL
    data = true;
    EXPECT_GE(-1,dlt_user_log_write_bool(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_float32
TEST(t_dlt_user_log_write_float32, normal)
{
    DltContext context;
    DltContextData contextData;
    float32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 3.141592653589793238;
    EXPECT_LE(0,dlt_user_log_write_float32(&contextData, data));
    data = -3.141592653589793238;
    EXPECT_LE(0,dlt_user_log_write_float32(&contextData, data));
    data = 0.;
    EXPECT_LE(0,dlt_user_log_write_float32(&contextData, data));
    data = -0.;
    EXPECT_LE(0,dlt_user_log_write_float32(&contextData, data));
    data = FLT_MIN;
    EXPECT_LE(0,dlt_user_log_write_float32(&contextData, data));
    data = FLT_MAX;
    EXPECT_LE(0,dlt_user_log_write_float32(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_float32, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    float32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32 nullpointer"));

    // NULL
    data = 1.;
    EXPECT_GE(-1,dlt_user_log_write_float32(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_float64
TEST(t_dlt_user_log_write_float64, normal)
{
    DltContext context;
    DltContextData contextData;
    double data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 3.14159265358979323846;
    EXPECT_LE(0,dlt_user_log_write_float64(&contextData, data));
    data = -3.14159265358979323846;
    EXPECT_LE(0,dlt_user_log_write_float64(&contextData, data));
    data = 0.;
    EXPECT_LE(0,dlt_user_log_write_float64(&contextData, data));
    data = -0.;
    EXPECT_LE(0,dlt_user_log_write_float64(&contextData, data));
    data = DBL_MIN;
    EXPECT_LE(0,dlt_user_log_write_float64(&contextData, data));
    data = DBL_MAX;
    EXPECT_LE(0,dlt_user_log_write_float64(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_float64, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    double data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64 nullpointer"));

    // NULL
    data = 1.;
    EXPECT_GE(-1,dlt_user_log_write_float64(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint
TEST(t_dlt_user_log_write_uint, normal)
{
    DltContext context;
    DltContextData contextData;
    unsigned int data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint(&contextData, data));
    data = UINT_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint, abnormal)
{
    DltContext context;
    DltContextData contextData;
    unsigned int data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_GE(-1,dlt_user_log_write_uint(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    unsigned int data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint8
TEST(t_dlt_user_log_write_uint8, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint8(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint8(&contextData, data));
    data = UINT8_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint8(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint8, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint8(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint16
TEST(t_dlt_user_log_write_uint16, normal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint16(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint16(&contextData, data));
    data = UINT16_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint16(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint16, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint16(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint32
TEST(t_dlt_user_log_write_uint32, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint32(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint32(&contextData, data));
    data = UINT32_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint32(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint32, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint32(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint64
TEST(t_dlt_user_log_write_uint64, normal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint64(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint64(&contextData, data));
    data = UINT64_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint64(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint64, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint64(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint8_formatted
TEST(t_dlt_user_log_write_uint8_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT8_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint8_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint8_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)-100));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)-10));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)10));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint8_formatted, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint16_formatted
TEST(t_dlt_user_log_write_uint16_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT16_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint16_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint16_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)-100));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)-10));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)10));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint16_formatted, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint32_formatted
TEST(t_dlt_user_log_write_uint32_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT32_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint32_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint32_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)-100));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)-10));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)10));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint32_formatted, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint64_formatted
TEST(t_dlt_user_log_write_uint64_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN16));
    data = UINT64_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_uint64_formatted(&contextData, data, DLT_FORMAT_BIN16));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint64_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)-100));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)-10));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)10));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint64_formatted, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    uint64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int
TEST(t_dlt_user_log_write_int, normal)
{
    DltContext context;
    DltContextData contextData;
    int data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(0,dlt_user_log_write_int(&contextData, data));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_int(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_int(&contextData, data));
    data = INT_MIN;
    EXPECT_LE(0,dlt_user_log_write_int(&contextData, data));
    data = INT_MAX;
    EXPECT_LE(0,dlt_user_log_write_int(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_int, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    int data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int8
TEST(t_dlt_user_log_write_int8, normal)
{
    DltContext context;
    DltContextData contextData;
    int8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(0,dlt_user_log_write_int8(&contextData, data));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_int8(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_int8(&contextData, data));
    data = INT8_MIN;
    EXPECT_LE(0,dlt_user_log_write_int8(&contextData, data));
    data = INT8_MAX;
    EXPECT_LE(0,dlt_user_log_write_int8(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_int8, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    int8_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int8(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int16
TEST(t_dlt_user_log_write_int16, normal)
{
    DltContext context;
    DltContextData contextData;
    int16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(0,dlt_user_log_write_int16(&contextData, data));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_int16(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_int16(&contextData, data));
    data = INT16_MIN;
    EXPECT_LE(0,dlt_user_log_write_int16(&contextData, data));
    data = INT16_MAX;
    EXPECT_LE(0,dlt_user_log_write_int16(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_int16, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    int16_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int16(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int32
TEST(t_dlt_user_log_write_int32, normal)
{
    DltContext context;
    DltContextData contextData;
    int32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(0,dlt_user_log_write_int32(&contextData, data));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_int32(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_int32(&contextData, data));
    data = INT32_MIN;
    EXPECT_LE(0,dlt_user_log_write_int32(&contextData, data));
    data = INT32_MAX;
    EXPECT_LE(0,dlt_user_log_write_int32(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_int32, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    int32_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int32(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int64
TEST(t_dlt_user_log_write_int64, normal)
{
    DltContext context;
    DltContextData contextData;
    int64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = -1;
    EXPECT_LE(0,dlt_user_log_write_int64(&contextData, data));
    data = 0;
    EXPECT_LE(0,dlt_user_log_write_int64(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_int64(&contextData, data));
    data = INT64_MIN;
    EXPECT_LE(0,dlt_user_log_write_int64(&contextData, data));
    data = INT64_MAX;
    EXPECT_LE(0,dlt_user_log_write_int64(&contextData, data));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_int64, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    int64_t data;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int64(NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_string
TEST(t_dlt_user_log_write_string, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(0,dlt_user_log_write_string(&contextData, text2));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string nullpointer"));

    // NULL
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_string(NULL, text1));
    EXPECT_GE(-1,dlt_user_log_write_string(NULL, NULL));
    EXPECT_GE(-1,dlt_user_log_write_string(&contextData, NULL));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_constant_string
TEST(t_dlt_user_log_write_constant_string, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_constant_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(0,dlt_user_log_write_constant_string(&contextData, text2));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_constant_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string nullpointer"));

    // NULL
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_constant_string(NULL, text1));
    EXPECT_GE(-1,dlt_user_log_write_constant_string(NULL, NULL));
    EXPECT_GE(-1,dlt_user_log_write_constant_string(&contextData, NULL));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_utf8_string
TEST(t_dlt_user_log_write_utf8_string, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_utf8_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(0,dlt_user_log_write_utf8_string(&contextData, text2));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_utf8_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string nullpointer"));

    // NULL
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_utf8_string(NULL, text1));
    EXPECT_GE(-1,dlt_user_log_write_utf8_string(NULL, NULL));
    EXPECT_GE(-1,dlt_user_log_write_utf8_string(&contextData, NULL));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_raw
TEST(t_dlt_user_log_write_raw, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_raw(&contextData, text1, 6));
    char text2[1] = "";
    EXPECT_LE(0,dlt_user_log_write_raw(&contextData, text2, 0));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_raw, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw nullpointer"));

    // NULL
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw(NULL, text1, 6));
    EXPECT_GE(-1,dlt_user_log_write_raw(NULL, NULL, 0));
    EXPECT_GE(-1,dlt_user_log_write_raw(&contextData, NULL, 0));
    //EXPECT_GE(-1,dlt_user_log_write_raw(&contextData, NULL, 1)); // SEGMENTATION FAULT
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_raw_formatted
TEST(t_dlt_user_log_write_raw_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text1, 6, DLT_FORMAT_BIN16));
    char text2[1] = "";
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX8));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX16));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX32));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_HEX64));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_BIN8));
    EXPECT_LE(0,dlt_user_log_write_raw_formatted(&contextData, text2, 0, DLT_FORMAT_BIN16));
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_raw_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted abnormal"));
    
    // undefined values for DltFormatType
    // shouldn't it return -1?
    char text1[6] = "test1";
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)-100));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)-10));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)10));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)100));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_user_log_write_raw_formatted, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted nullpointer"));

    // NULL
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(NULL, text1, 6, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(NULL, NULL, 0, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, NULL, 0, DLT_FORMAT_DEFAULT));
    //EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, NULL, 1, DLT_FORMAT_DEFAULT)); // SEGMENTATION FAULT
    EXPECT_EQ(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/*
int dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text);
int dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data);
int dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data);
int dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data);
int dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data);
int dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length);
int dlt_log_marker();
*/

/////////////////////////////////////////
// t_dlt_log_string
TEST(t_dlt_log_string, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string normal"));

    // normal values
    const char text1[6] = "test1";
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_DEFAULT, text1));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_OFF, text1));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_FATAL, text1));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_ERROR, text1));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_WARN, text1));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_INFO, text1));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_VERBOSE, text1));
    const char text2[1] = "";
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_DEFAULT, text2));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_OFF, text2));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_FATAL, text2));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_ERROR, text2));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_WARN, text2));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_INFO, text2));
    EXPECT_LE(0,dlt_log_string(&context, DLT_LOG_VERBOSE, text2));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_string, abnormal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    const char text1[6] = "test1";
    EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)-100, text1));
    EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)-10, text1));
    EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)10, text1));
    EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)100, text1));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_string, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string nullpointer"));

    // NULL
    char text1[6] = "test1";
    EXPECT_GE(-1,dlt_log_string(NULL, DLT_LOG_DEFAULT, text1));
    EXPECT_GE(-1,dlt_log_string(NULL, DLT_LOG_DEFAULT, NULL));
    EXPECT_GE(-1,dlt_log_string(&context, DLT_LOG_DEFAULT, NULL));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_string_int
TEST(t_dlt_log_string_int, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int normal"));

    // normal values
    const char text1[6] = "test1";
    int data = 1;
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_DEFAULT, text1, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_OFF, text1, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_FATAL, text1, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_ERROR, text1, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_WARN, text1, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_INFO, text1, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_VERBOSE, text1, data));
    const char text2[1] = "";
    data = 0;
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_VERBOSE, text2, data));
    data = -1;
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_VERBOSE, text2, data));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_string_int, abnormal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    const char text1[6] = "test1";
    int data = 1;
    EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)-100, text1, data));
    EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)-10, text1, data));
    EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)10, text1, data));
    EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)100, text1, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_string_int, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int nullpointer"));

    // NULL
    char text1[6] = "test1";
    int data = 0;
    EXPECT_GE(-1,dlt_log_string_int(NULL, DLT_LOG_DEFAULT, text1, data));
    EXPECT_GE(-1,dlt_log_string_int(NULL, DLT_LOG_DEFAULT, NULL, data));
    EXPECT_GE(-1,dlt_log_string_int(&context, DLT_LOG_DEFAULT, NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_string_uint
TEST(t_dlt_log_string_uint, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint normal"));

    // normal values
    const char text1[6] = "test1";
    unsigned int data = 1;
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text1, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_OFF, text1, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_FATAL, text1, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_ERROR, text1, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_WARN, text1, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_INFO, text1, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text1, data));
    const char text2[1] = "";
    data = 0;
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text2, data));
    data = -1;
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text2, data));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_string_uint, abnormal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    const char text1[6] = "test1";
    unsigned int data = 1;
    EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)-100, text1, data));
    EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)-10, text1, data));
    EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)10, text1, data));
    EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)100, text1, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_string_uint, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint nullpointer"));

    // NULL
    char text1[6] = "test1";
    unsigned int data = 0;
    EXPECT_GE(-1,dlt_log_string_uint(NULL, DLT_LOG_DEFAULT, text1, data));
    EXPECT_GE(-1,dlt_log_string_uint(NULL, DLT_LOG_DEFAULT, NULL, data));
    EXPECT_GE(-1,dlt_log_string_uint(&context, DLT_LOG_DEFAULT, NULL, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_int
TEST(t_dlt_log_int, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int normal"));

    // normal values
    int data = 1;
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_VERBOSE, data));
    data = 0;
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_VERBOSE, data));
    data = -1;
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_VERBOSE, data));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_int, abnormal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    int data = 1;
    EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)-100, data));
    EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)-10, data));
    EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)10, data));
    EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)100, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_int, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int nullpointer"));

    // NULL
    int data = 0;
    EXPECT_GE(-1,dlt_log_int(NULL, DLT_LOG_DEFAULT, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_uint
TEST(t_dlt_log_uint, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint normal"));

    // normal values
    unsigned int data = 1;
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_VERBOSE, data));
    data = 0;
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_VERBOSE, data));
    data = -1;
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_VERBOSE, data));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_uint, abnormal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    unsigned int data = 1;
    EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)-100, data));
    EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)-10, data));
    EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)10, data));
    EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)100, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_uint, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint nullpointer"));

    // NULL
    unsigned int data = 0;
    EXPECT_GE(-1,dlt_log_uint(NULL, DLT_LOG_DEFAULT, data));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_raw
TEST(t_dlt_log_raw, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw normal"));

    // normal values
    char data[5] = "test";
    uint16_t length = 4;
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, length));
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_OFF, data, length));
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_FATAL, data, length));
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_ERROR, data, length));
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_WARN, data, length));
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_INFO, data, length));
    EXPECT_LE(0,dlt_log_raw(&context, DLT_LOG_VERBOSE, data, length));

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_raw, abnormal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    char data[5] = "test";
    uint16_t length = 4;
    EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)-100, data, length));
    EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)-10, data, length));
    EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)10, data, length));
    EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)100, data, length));

    // zero length
    EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, 0));
    
    // negative length
    EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, -1));
    EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, -100));
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

TEST(t_dlt_log_raw, nullpointer)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw nullpointer"));

    // NULL
    //char data[5] = "test";
    //uint16_t length = 4;
    //EXPECT_GE(-1,dlt_log_raw(NULL, DLT_LOG_DEFAULT, data, length)); // SEGMENTATION FAULT
    //EXPECT_GE(-1,dlt_log_raw(NULL, DLT_LOG_DEFAULT, NULL, length)); // SEGMENTATION FAULT
    //EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, NULL, length)); // SEGMENTATION FAULT
    
    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_marker
TEST(t_dlt_log_marker, normal)
{
    DltContext context;
    //DltContextData contextData;
    
    EXPECT_EQ(0,dlt_init());
    EXPECT_EQ(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_EQ(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_marker normal"));

    // normal
    EXPECT_LE(0,dlt_log_marker());

    EXPECT_EQ(0,dlt_unregister_context(&context));
    EXPECT_EQ(0,dlt_unregister_app());
    EXPECT_EQ(0,dlt_free());
}







/////////////////////////////////////////
// main
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

