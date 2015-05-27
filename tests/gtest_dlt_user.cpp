#include <stdio.h>
#include "gtest/gtest.h"
#include <limits.h>

extern "C" {
#include "dlt_common.h"
#include "dlt_user.h"
}



// TEST COMMENTED OUT WITH
// TODO:
// DO FAIL!



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

/*
int dlt_register_app(const char *appid, const char * description);
int dlt_unregister_app(void);
int dlt_register_context(DltContext *handle, const char *contextid, const char * description);
int dlt_register_context_ll_ts(DltContext *handle, const char *contextid, const char * description, int loglevel, int tracestatus);
int dlt_unregister_context(DltContext *handle);
int dlt_register_injection_callback(DltContext *handle, uint32_t service_id, int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length));
int dlt_register_log_level_changed_callback(DltContext *handle, void (*dlt_log_level_changed_callback)(char context_id[DLT_ID_SIZE],uint8_t log_level, uint8_t trace_status));
*/

/*
int dlt_user_trace_network(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);
int dlt_user_trace_network_truncated(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload, int allow_truncate);
int dlt_user_trace_network_segmented(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);
*/

/*
int dlt_set_log_mode(DltUserLogMode mode);
int dlt_get_log_state();
*/

/*
int dlt_verbose_mode(void);
int dlt_nonverbose_mode(void);
*/




// define some min and max values (if not present in <limits.h>)
#ifndef UINT8_MIN
#define UINT8_MIN 0
#endif

#ifndef UINT16_MIN
#define UINT16_MIN 0
#endif

#ifndef UINT32_MIN
#define UINT32_MIN 0
#endif

#ifndef UINT64_MIN
#define UINT64_MIN 0
#endif

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295
#endif

#ifndef UINT64_MAX
#define UINT64_MAX 18446744073709551615
#endif

#ifndef INT8_MIN
#define INT8_MIN -128
#endif

#ifndef INT16_MIN
#define INT16_MIN -32768
#endif

#ifndef INT32_MIN
#define INT32_MIN -2147483648
#endif

#ifndef INT64_MIN
#define INT64_MIN -9223372036854775808
#endif

#ifndef INT8_MAX
#define INT8_MAX 127
#endif

#ifndef INT16_MAX
#define INT16_MAX 32767
#endif

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif

#ifndef INT64_MAX
#define INT64_MAX 9223372036854775807
#endif

#ifndef UINT_MIN
#define UINT_MIN UINT32_MIN
#endif

#ifndef UINT_MAX
#define UINT_MAX UINT32_MAX
#endif

#ifndef INT_MIN
#define INT_MIN INT32_MIN
#endif

#ifndef INT_MAX
#define INT_MAX INT32_MAX
#endif


/////////////////////////////////////////
// t_dlt_user_log_write_start
TEST(t_dlt_user_log_write_start, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start normal"));
    
    // the defined enum values for log level
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_OFF));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_FATAL));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_ERROR));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_WARN));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_INFO));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEBUG));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_VERBOSE));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_start, abnormal)
{
    DltContext context;
    // TODO: DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)-100));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)-10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, (DltLogLevelType)100));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_start, startstartfinish)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start startstartfinish"));
    
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // shouldn't it return -1, because it is already started?
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_start, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start nullpointer"));

    // NULL's
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(NULL, &contextData, DLT_LOG_DEFAULT));
    //EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start(NULL, NULL, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_start(&context, NULL, DLT_LOG_DEFAULT));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_start_id
TEST(t_dlt_user_log_write_start_id, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id normal"));
    
    // the defined enum values for log level
    messageid = UINT32_MIN;
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    messageid = UINT32_MAX;
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_OFF, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_FATAL, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_ERROR, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_WARN, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_INFO, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEBUG, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_VERBOSE, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_start_id, abnormal)
{
    DltContext context;
    // TODO: DltContextData contextData;
    // TODO: uint32_t messageid;

    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: messageid = UINT32_MIN;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)-100, messageid));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)-10, messageid));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)10, messageid));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, (DltLogLevelType)100, messageid));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_start_id, startstartfinish)
{
    DltContext context;
    DltContextData contextData;
    uint32_t messageid;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id startstartfinish"));
    
    messageid = UINT32_MIN;
    EXPECT_LE(0,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    // shouldn't it return -1, because it is already started?
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(&context, &contextData, DLT_LOG_DEFAULT, messageid));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_start_id, nullpointer)
{
    DltContext context;
    uint32_t messageid;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start_id nullpointer"));

    // NULL's
    messageid = UINT32_MIN;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(NULL, &contextData, DLT_LOG_DEFAULT, messageid));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_start_id(NULL, NULL, DLT_LOG_DEFAULT, messageid));
    EXPECT_GE(-1,dlt_user_log_write_start_id(&context, NULL, DLT_LOG_DEFAULT, messageid));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_finish
TEST(t_dlt_user_log_write_finish, finish)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_start finish"));
    
    // finish without start
    // TODO: EXPECT_GE(-1,dlt_user_log_write_finish(NULL));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    
    // finish without start, but initialized context
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_finish finish"));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    
    // finish with start and initialized context
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    // 2nd finish
    // TODO: EXPECT_GE(-1,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_bool
TEST(t_dlt_user_log_write_bool, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = true;
    EXPECT_LE(0,dlt_user_log_write_bool(&contextData, data));
    data = false;
    EXPECT_LE(0,dlt_user_log_write_bool(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_bool, abnormal)
{
    DltContext context;
    DltContextData contextData;
    // TODO: uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // TODO: data = 2;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_bool(&contextData, data));
    // TODO: data = 100;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_bool(&contextData, data));
    // TODO: data = UINT8_MAX;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_bool(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_bool, nullpointer)
{
    DltContext context;
    uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_bool nullpointer"));

    // NULL
    data = true;
    EXPECT_GE(-1,dlt_user_log_write_bool(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_float32
TEST(t_dlt_user_log_write_float32, normal)
{
    DltContext context;
    DltContextData contextData;
    float32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32 normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_float32, nullpointer)
{
    DltContext context;
    float32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float32 nullpointer"));

    // NULL
    data = 1.;
    EXPECT_GE(-1,dlt_user_log_write_float32(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_float64
TEST(t_dlt_user_log_write_float64, normal)
{
    DltContext context;
    DltContextData contextData;
    double data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64 normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_float64, nullpointer)
{
    DltContext context;
    double data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_float64 nullpointer"));

    // NULL
    data = 1.;
    EXPECT_GE(-1,dlt_user_log_write_float64(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint
TEST(t_dlt_user_log_write_uint, normal)
{
    DltContext context;
    DltContextData contextData;
    unsigned int data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT_MIN;
    EXPECT_LE(0,dlt_user_log_write_uint(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint(&contextData, data));
    data = UINT_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint, abnormal)
{
    DltContext context;
    DltContextData contextData;
    // TODO: unsigned int data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // TODO: data = -1;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint, nullpointer)
{
    DltContext context;
    unsigned int data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint8
TEST(t_dlt_user_log_write_uint8, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT8_MIN;
    EXPECT_LE(0,dlt_user_log_write_uint8(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint8(&contextData, data));
    data = UINT8_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint8(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint8, nullpointer)
{
    DltContext context;
    uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint8(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint16
TEST(t_dlt_user_log_write_uint16, normal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT16_MIN;
    EXPECT_LE(0,dlt_user_log_write_uint16(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint16(&contextData, data));
    data = UINT16_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint16(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint16, nullpointer)
{
    DltContext context;
    uint16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint16(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint32
TEST(t_dlt_user_log_write_uint32, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT32_MIN;
    EXPECT_LE(0,dlt_user_log_write_uint32(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint32(&contextData, data));
    data = UINT32_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint32(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint32, nullpointer)
{
    DltContext context;
    uint32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint32(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint64
TEST(t_dlt_user_log_write_uint64, normal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64 normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT64_MIN;
    EXPECT_LE(0,dlt_user_log_write_uint64(&contextData, data));
    data = 1;
    EXPECT_LE(0,dlt_user_log_write_uint64(&contextData, data));
    data = UINT64_MAX;
    EXPECT_LE(0,dlt_user_log_write_uint64(&contextData, data));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint64, nullpointer)
{
    DltContext context;
    uint64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint64(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint8_formatted
TEST(t_dlt_user_log_write_uint8_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT8_MIN;
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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint8_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    // TODO: uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // TODO: data = 1;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)-100));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)-10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint8_formatted, nullpointer)
{
    DltContext context;
    uint8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint8_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint8_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint16_formatted
TEST(t_dlt_user_log_write_uint16_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT16_MIN;
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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint16_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    // TODO: uint16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // TODO: data = 1;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)-100));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)-10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint16_formatted, nullpointer)
{
    DltContext context;
    uint16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint16_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint16_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint32_formatted
TEST(t_dlt_user_log_write_uint32_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT32_MIN;
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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint32_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    // TODO: uint32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // TODO: data = 1;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)-100));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)-10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint32_formatted, nullpointer)
{
    DltContext context;
    uint32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint32_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint32_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_uint64_formatted
TEST(t_dlt_user_log_write_uint64_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    uint64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    data = UINT64_MIN;
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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint64_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    // TODO: uint64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted abnormal"));

    // abnormal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    // TODO: data = 1;
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)-100));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)-10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)10));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(&contextData, data, (DltFormatType)100));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_uint64_formatted, nullpointer)
{
    DltContext context;
    uint64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_uint64_formatted nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX8));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX16));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX32));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_HEX64));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_BIN8));
    EXPECT_GE(-1,dlt_user_log_write_uint64_formatted(NULL, data, DLT_FORMAT_BIN16));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int
TEST(t_dlt_user_log_write_int, normal)
{
    DltContext context;
    DltContextData contextData;
    int data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_int, nullpointer)
{
    DltContext context;
    int data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int8
TEST(t_dlt_user_log_write_int8, normal)
{
    DltContext context;
    DltContextData contextData;
    int8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8 normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_int8, nullpointer)
{
    DltContext context;
    int8_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int8 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int8(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int16
TEST(t_dlt_user_log_write_int16, normal)
{
    DltContext context;
    DltContextData contextData;
    int16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16 normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_int16, nullpointer)
{
    DltContext context;
    int16_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int16 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int16(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int32
TEST(t_dlt_user_log_write_int32, normal)
{
    DltContext context;
    DltContextData contextData;
    int32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32 normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_int32, nullpointer)
{
    DltContext context;
    int32_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int32 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int32(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_int64
TEST(t_dlt_user_log_write_int64, normal)
{
    DltContext context;
    DltContextData contextData;
    int64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64 normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_int64, nullpointer)
{
    DltContext context;
    int64_t data;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_int64 nullpointer"));

    // NULL
    data = 1;
    EXPECT_GE(-1,dlt_user_log_write_int64(NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_string
TEST(t_dlt_user_log_write_string, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(0,dlt_user_log_write_string(&contextData, text2));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_string nullpointer"));

    // NULL
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_string(NULL, text1));
    EXPECT_GE(-1,dlt_user_log_write_string(NULL, NULL));
    EXPECT_GE(-1,dlt_user_log_write_string(&contextData, NULL));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_constant_string
TEST(t_dlt_user_log_write_constant_string, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_constant_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(0,dlt_user_log_write_constant_string(&contextData, text2));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_constant_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_constant_string nullpointer"));

    // NULL
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_constant_string(NULL, text1));
    EXPECT_GE(-1,dlt_user_log_write_constant_string(NULL, NULL));
    EXPECT_GE(-1,dlt_user_log_write_constant_string(&contextData, NULL));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_utf8_string
TEST(t_dlt_user_log_write_utf8_string, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_utf8_string(&contextData, text1));
    const char *text2 = "";
    EXPECT_LE(0,dlt_user_log_write_utf8_string(&contextData, text2));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_utf8_string, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_utf8_string nullpointer"));

    // NULL
    const char *text1 = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_utf8_string(NULL, text1));
    EXPECT_GE(-1,dlt_user_log_write_utf8_string(NULL, NULL));
    EXPECT_GE(-1,dlt_user_log_write_utf8_string(&contextData, NULL));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_raw
TEST(t_dlt_user_log_write_raw, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw normal"));

    // normal values
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_raw(&contextData, text1, 6));
    char text2[1] = "";
    EXPECT_LE(0,dlt_user_log_write_raw(&contextData, text2, 0));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_raw, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw nullpointer"));

    // NULL
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw(NULL, text1, 6));
    EXPECT_GE(-1,dlt_user_log_write_raw(NULL, NULL, 0));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_raw(&contextData, NULL, 0));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_raw(&contextData, NULL, 1));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_user_log_write_raw_formatted
TEST(t_dlt_user_log_write_raw_formatted, normal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted normal"));

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
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_raw_formatted, abnormal)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted abnormal"));
    
    // undefined values for DltFormatType
    // shouldn't it return -1?
    char text1[6] = "test1";
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)-100));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)-10));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)10));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, text1, 6, (DltFormatType)100));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_log_write_raw_formatted, nullpointer)
{
    DltContext context;
    DltContextData contextData;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_log_write_raw_formatted nullpointer"));

    // NULL
    char text1[6] = "test1";
    EXPECT_LE(0,dlt_user_log_write_start(&context, &contextData, DLT_LOG_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(NULL, text1, 6, DLT_FORMAT_DEFAULT));
    EXPECT_GE(-1,dlt_user_log_write_raw_formatted(NULL, NULL, 0, DLT_FORMAT_DEFAULT));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, NULL, 0, DLT_FORMAT_DEFAULT));
    // TODO: EXPECT_GE(-1,dlt_user_log_write_raw_formatted(&contextData, NULL, 1, DLT_FORMAT_DEFAULT));
    EXPECT_LE(0,dlt_user_log_write_finish(&contextData));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
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
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string normal"));

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

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_string, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: const char text1[6] = "test1";
    // TODO: EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)-100, text1));
    // TODO: EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)-10, text1));
    // TODO: EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)10, text1));
    // TODO: EXPECT_GE(-1,dlt_log_string(&context, (DltLogLevelType)100, text1));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_string, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string nullpointer"));

    // NULL
    char text1[6] = "test1";
    EXPECT_GE(-1,dlt_log_string(NULL, DLT_LOG_DEFAULT, text1));
    EXPECT_GE(-1,dlt_log_string(NULL, DLT_LOG_DEFAULT, NULL));
    EXPECT_GE(-1,dlt_log_string(&context, DLT_LOG_DEFAULT, NULL));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_string_int
TEST(t_dlt_log_string_int, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int normal"));

    // normal values
    const char text1[6] = "test1";
    int data = INT_MIN;
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
    data = INT_MAX;
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(0,dlt_log_string_int(&context, DLT_LOG_VERBOSE, text2, data));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_string_int, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: const char text1[6] = "test1";
    // TODO: int data = 1;
    // TODO: EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)-100, text1, data));
    // TODO: EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)-10, text1, data));
    // TODO: EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)10, text1, data));
    // TODO: EXPECT_GE(-1,dlt_log_string_int(&context, (DltLogLevelType)100, text1, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_string_int, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_int nullpointer"));

    // NULL
    char text1[6] = "test1";
    int data = 0;
    EXPECT_GE(-1,dlt_log_string_int(NULL, DLT_LOG_DEFAULT, text1, data));
    EXPECT_GE(-1,dlt_log_string_int(NULL, DLT_LOG_DEFAULT, NULL, data));
    EXPECT_GE(-1,dlt_log_string_int(&context, DLT_LOG_DEFAULT, NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_string_uint
TEST(t_dlt_log_string_uint, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint normal"));

    // normal values
    const char text1[6] = "test1";
    unsigned int data = UINT_MIN;
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
    data = UINT_MAX;
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_DEFAULT, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_OFF, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_FATAL, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_ERROR, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_WARN, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_INFO, text2, data));
    EXPECT_LE(0,dlt_log_string_uint(&context, DLT_LOG_VERBOSE, text2, data));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_string_uint, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: const char text1[6] = "test1";
    // TODO: unsigned int data = 1;
    // TODO: EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)-100, text1, data));
    // TODO: EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)-10, text1, data));
    // TODO: EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)10, text1, data));
    // TODO: EXPECT_GE(-1,dlt_log_string_uint(&context, (DltLogLevelType)100, text1, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_string_uint, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_string_uint nullpointer"));

    // NULL
    char text1[6] = "test1";
    unsigned int data = 0;
    EXPECT_GE(-1,dlt_log_string_uint(NULL, DLT_LOG_DEFAULT, text1, data));
    EXPECT_GE(-1,dlt_log_string_uint(NULL, DLT_LOG_DEFAULT, NULL, data));
    EXPECT_GE(-1,dlt_log_string_uint(&context, DLT_LOG_DEFAULT, NULL, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_int
TEST(t_dlt_log_int, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int normal"));

    // normal values
    int data = INT_MIN;
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
    data = INT_MAX;
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_int(&context, DLT_LOG_VERBOSE, data));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_int, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: int data = 1;
    // TODO: EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)-100, data));
    // TODO: EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)-10, data));
    // TODO: EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)10, data));
    // TODO: EXPECT_GE(-1,dlt_log_int(&context, (DltLogLevelType)100, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_int, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_int nullpointer"));

    // NULL
    int data = 0;
    EXPECT_GE(-1,dlt_log_int(NULL, DLT_LOG_DEFAULT, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_uint
TEST(t_dlt_log_uint, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint normal"));

    // normal values
    unsigned int data = UINT_MIN;
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
    data = UINT_MAX;
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_DEFAULT, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_OFF, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_FATAL, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_ERROR, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_WARN, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_INFO, data));
    EXPECT_LE(0,dlt_log_uint(&context, DLT_LOG_VERBOSE, data));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_uint, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    // TODO: unsigned int data = 1;
    // TODO: EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)-100, data));
    // TODO: EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)-10, data));
    // TODO: EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)10, data));
    // TODO: EXPECT_GE(-1,dlt_log_uint(&context, (DltLogLevelType)100, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_uint, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_uint nullpointer"));

    // NULL
    unsigned int data = 0;
    EXPECT_GE(-1,dlt_log_uint(NULL, DLT_LOG_DEFAULT, data));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_raw
TEST(t_dlt_log_raw, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw normal"));

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

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_raw, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw abnormal"));
    
    // undefined values for DltLogLevelType
    // shouldn't it return -1?
    char data[5] = "test";
    // TODO: uint16_t length = 4;
    // TODO: EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)-100, data, length));
    // TODO: EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)-10, data, length));
    // TODO: EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)10, data, length));
    // TODO: EXPECT_GE(-1,dlt_log_raw(&context, (DltLogLevelType)100, data, length));

    // zero length
    // TODO: EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, 0));
    
    // negative length
    EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, -1));
    EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, data, -100));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_log_raw, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_raw nullpointer"));

    // NULL
    // TODO: char data[5] = "test";
    // TODO: uint16_t length = 4;
    // TODO: EXPECT_GE(-1,dlt_log_raw(NULL, DLT_LOG_DEFAULT, data, length));
    // TODO: EXPECT_GE(-1,dlt_log_raw(NULL, DLT_LOG_DEFAULT, NULL, length));
    // TODO: EXPECT_GE(-1,dlt_log_raw(&context, DLT_LOG_DEFAULT, NULL, length));
    
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_log_marker
TEST(t_dlt_log_marker, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_log_marker normal"));

    // normal
    EXPECT_LE(0,dlt_log_marker());

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_register_app
TEST(t_dlt_register_app, normal)
{
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("T", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_register_app("TU", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_register_app("TUS", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_register_app, abnormal)
{
    EXPECT_LE(0,dlt_init());
    EXPECT_GE(-1,dlt_register_app("", "dlt_user.c tests"));
    EXPECT_GE(-1,dlt_unregister_app());
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR1", "dlt_user.c tests"));
    // TODO: EXPECT_GE(-1,dlt_unregister_app());
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR123445667", "dlt_user.c tests"));
    // TODO: EXPECT_GE(-1,dlt_unregister_app());
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_register_app, nullpointer)
{
    EXPECT_LE(0,dlt_init());
    EXPECT_GE(-1,dlt_register_app(NULL, NULL));
    EXPECT_GE(-1,dlt_register_app(NULL, "dlt_user.c tests"));
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR", NULL));
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_unregister_app
TEST(t_dlt_unregister_app, normal)
{
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("T", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_register_app("TU", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_register_app("TUS", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_unregister_app, abnormal)
{
    EXPECT_LE(0,dlt_init());
    EXPECT_GE(-1,dlt_unregister_app());
    EXPECT_GE(-1,dlt_register_app("", "dlt_user.c tests"));
    EXPECT_GE(-1,dlt_unregister_app());
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR1", "dlt_user.c tests"));
    // TODO: EXPECT_GE(-1,dlt_unregister_app());
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR123445667", "dlt_user.c tests"));
    // TODO: EXPECT_GE(-1,dlt_unregister_app());
    // TODO: EXPECT_GE(-1,dlt_register_app("TUSR", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_register_context
TEST(t_dlt_register_context, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    EXPECT_LE(0,dlt_unregister_context(&context));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_register_context, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(-1,dlt_register_context(&context, "", "d"));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "T", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context(&context, "", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1", "1"));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1234567890", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1234567890", "1"));
    
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_context normal"));
    EXPECT_LE(0,dlt_unregister_context(&context));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_register_context, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", NULL));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context(&context, NULL, "dlt_user.c t_dlt_register_context normal"));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context(&context, NULL, NULL));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(NULL, "TEST", NULL));
    EXPECT_GE(-1,dlt_register_context(NULL, NULL, "dlt_user.c t_dlt_register_context normal"));
    EXPECT_GE(-1,dlt_register_context(NULL, NULL, NULL));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_register_context_ll_ts
TEST(t_dlt_register_context_ll_ts, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_FATAL, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_ERROR, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_WARN, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_INFO, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEBUG, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_VERBOSE, DLT_TRACE_STATUS_OFF));
    EXPECT_LE(0,dlt_unregister_context(&context));

    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_FATAL, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_ERROR, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_WARN, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_INFO, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEBUG, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_VERBOSE, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_register_context_ll_ts, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "", "d", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "T", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST1", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST1", "1", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST1234567890", "", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST1234567890", "1", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    
    EXPECT_LE(0,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_ON));
    EXPECT_LE(0,dlt_unregister_context(&context));

    // DLT_LOG_DEFAULT and DLT_TRACE_STATUS_DEFAULT not allowed
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEFAULT, DLT_TRACE_STATUS_OFF));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_DEFAULT, DLT_TRACE_STATUS_DEFAULT));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_DEFAULT));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    
    // abnormal values for loglevel and tracestatus
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", -1, DLT_TRACE_STATUS_OFF));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", 100, DLT_TRACE_STATUS_OFF));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, -1));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, 100));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_register_context_ll_ts, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(&context, "TEST", NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, NULL, "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context_ll_ts(&context, NULL, NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context_ll_ts(NULL, "TEST", NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_GE(-1,dlt_register_context_ll_ts(NULL, NULL, "dlt_user.c t_dlt_register_context_ll_ts normal", DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));
    EXPECT_GE(-1,dlt_register_context_ll_ts(NULL, NULL, NULL, DLT_LOG_OFF, DLT_TRACE_STATUS_OFF));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

/////////////////////////////////////////
// t_dlt_unregister_context
TEST(t_dlt_unregister_context, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    EXPECT_LE(0,dlt_unregister_context(&context));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_unregister_context, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    EXPECT_GE(-1,dlt_register_context(&context, "", "d"));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "T", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context(&context, "", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1", "1"));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1234567890", ""));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST1234567890", "1"));
    
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_unregister_context normal"));
    EXPECT_LE(0,dlt_unregister_context(&context));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_unregister_context, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));

    // TODO: EXPECT_GE(-1,dlt_register_context(&context, "TEST", NULL));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(&context, NULL, "dlt_user.c t_dlt_unregister_context normal"));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    EXPECT_GE(-1,dlt_register_context(&context, NULL, NULL));
    // TODO: EXPECT_GE(-1,dlt_unregister_context(&context));
    // TODO: EXPECT_GE(-1,dlt_register_context(NULL, "TEST", NULL));
    EXPECT_GE(-1,dlt_register_context(NULL, NULL, "dlt_user.c t_dlt_unregister_context normal"));
    EXPECT_GE(-1,dlt_register_context(NULL, NULL, NULL));

    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_register_injection_callback
int dlt_user_injection_callback(uint32_t /*service_id*/, void */*data*/, uint32_t /*length*/)
{
    return 0;
}

TEST(t_dlt_register_injection_callback, normal)
{
    DltContext context;
    // TODO: uint32_t service_id;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_injection_callback normal"));

    // TODO: service_id = 0x123;
    // TODO: EXPECT_LE(0,dlt_register_injection_callback(&context, service_id, dlt_user_injection_callback));    

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());

}


/////////////////////////////////////////
// t_dlt_register_log_level_changed_callback
void dlt_user_log_level_changed_callback(char /*context_id*/[DLT_ID_SIZE], uint8_t /*log_level*/, uint8_t /*trace_status*/)
{
    
}

TEST(t_dlt_register_log_level_changed_callback, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_register_log_level_changed_callback normal"));

    EXPECT_LE(0,dlt_register_log_level_changed_callback(&context, dlt_user_log_level_changed_callback));    

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());

}


/////////////////////////////////////////
// t_dlt_user_trace_network
TEST(t_dlt_user_trace_network, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network normal"));

    char header[16];
    for(char i = 0; i < 16; ++i)
    {
        header[(int)i] = i;
    }
    char payload[32];
    for(char i = 0; i < 32; ++i)
    {
        payload[(int)i] = i;
    }

    EXPECT_LE(0,dlt_user_trace_network(&context, DLT_NW_TRACE_IPC, 16, header, 32, payload));
    EXPECT_LE(0,dlt_user_trace_network(&context, DLT_NW_TRACE_CAN, 16, header, 32, payload));
    EXPECT_LE(0,dlt_user_trace_network(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 32, payload));
    EXPECT_LE(0,dlt_user_trace_network(&context, DLT_NW_TRACE_MOST, 16, header, 32, payload));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_trace_network, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network abnormal"));

    // TODO: char header[16];
    // TODO: for(char i = 0; i < 16; ++i)
    // TODO: {
    // TODO:     header[(int)i] = i;
    // TODO: }
    // TODO: char payload[32];
    // TODO: for(char i = 0; i < 32; ++i)
    // TODO: {
    // TODO:     payload[(int)i] = i;
    // TODO: }

    // data length = 0. Does this make sense?
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload));

    // invalid DltNetworkTraceType value
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, (DltNetworkTraceType)-100, 16, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, (DltNetworkTraceType)-10, 16, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, (DltNetworkTraceType)10, 16, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, (DltNetworkTraceType)100, 16, header, 32, payload));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_trace_network, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network nullpointer"));

    // TODO: char header[16];
    // TODO: for(char i = 0; i < 16; ++i)
    // TODO: {
    // TODO:     header[(int)i] = i;
    // TODO: }
    // TODO: char payload[32];
    // TODO: for(char i = 0; i < 32; ++i)
    // TODO: {
    // TODO:     payload[(int)i] = i;
    // TODO: }

    // what to expect when giving in NULL pointer?
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, DLT_NW_TRACE_IPC, 16, NULL, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, DLT_NW_TRACE_CAN, 16, header, 32, NULL));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network(&context, DLT_NW_TRACE_FLEXRAY, 16, NULL, 32, NULL));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_user_trace_network_truncated
TEST(t_dlt_user_trace_network_truncated, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated normal"));

    char header[16];
    for(char i = 0; i < 16; ++i)
    {
        header[(int)i] = i;
    }
    char payload[32];
    for(char i = 0; i < 32; ++i)
    {
        payload[(int)i] = i;
    }

    EXPECT_LE(0,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_IPC, 16, header, 32, payload, 0));
    EXPECT_LE(0,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_CAN, 16, header, 32, payload, 1));
    EXPECT_LE(0,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 32, payload, -1));
    EXPECT_LE(0,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_MOST, 16, header, 32, payload, 10));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_trace_network_truncated, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated abnormal"));

    // TODO: char header[16];
    // TODO: for(char i = 0; i < 16; ++i)
    // TODO: {
    // TODO:     header[(int)i] = i;
    // TODO: }
    // TODO: char payload[32];
    // TODO: for(char i = 0; i < 32; ++i)
    // TODO: {
    // TODO:     payload[(int)i] = i;
    // TODO: }

    // data length = 0. Does this make sense?
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload, 0));

    // invalid DltNetworkTraceType value
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)-100, 16, header, 32, payload, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)-10, 16, header, 32, payload, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)10, 16, header, 32, payload, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, (DltNetworkTraceType)100, 16, header, 32, payload, 0));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_trace_network_truncated, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_truncated nullpointer"));

    // TODO: char header[16];
    // TODO: for(char i = 0; i < 16; ++i)
    // TODO: {
    // TODO:     header[(int)i] = i;
    // TODO: }
    // TODO: char payload[32];
    // TODO: for(char i = 0; i < 32; ++i)
    // TODO: {
    // TODO:     payload[(int)i] = i;
    // TODO: }

    // what to expect when giving in NULL pointer?
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_IPC, 16, NULL, 32, payload, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_CAN, 16, header, 32, NULL, 0));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_truncated(&context, DLT_NW_TRACE_FLEXRAY, 16, NULL, 32, NULL, 0));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_user_trace_network_segmented
TEST(t_dlt_user_trace_network_segmented, normal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented normal"));

    char header[16];
    for(char i = 0; i < 16; ++i)
    {
        header[(int)i] = i;
    }
    char payload[32];
    for(char i = 0; i < 32; ++i)
    {
        payload[(int)i] = i;
    }

    EXPECT_LE(0,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_IPC, 16, header, 32, payload));
    EXPECT_LE(0,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_CAN, 16, header, 32, payload));
    EXPECT_LE(0,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 32, payload));
    EXPECT_LE(0,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_MOST, 16, header, 32, payload));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_trace_network_segmented, abnormal)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented abnormal"));

    // TODO: char header[16];
    // TODO: for(char i = 0; i < 16; ++i)
    // TODO: {
    // TODO:     header[(int)i] = i;
    // TODO: }
    // TODO: char payload[32];
    // TODO: for(char i = 0; i < 32; ++i)
    // TODO: {
    // TODO:     payload[(int)i] = i;
    // TODO: }

    // data length = 0. Does this make sense?
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_IPC, 0, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_CAN, 0, header, 0, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_FLEXRAY, 16, header, 0, payload));

    // invalid DltNetworkTraceType value
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)-100, 16, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)-10, 16, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)10, 16, header, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, (DltNetworkTraceType)100, 16, header, 32, payload));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_user_trace_network_segmented, nullpointer)
{
    DltContext context;
    
    EXPECT_LE(0,dlt_init());
    EXPECT_LE(0,dlt_register_app("TUSR", "dlt_user.c tests"));
    EXPECT_LE(0,dlt_register_context(&context, "TEST", "dlt_user.c t_dlt_user_trace_network_segmented nullpointer"));

    // TODO: char header[16];
    // TODO: for(char i = 0; i < 16; ++i)
    // TODO: {
    // TODO:     header[(int)i] = i;
    // TODO: }
    // TODO: char payload[32];
    // TODO: for(char i = 0; i < 32; ++i)
    // TODO: {
    // TODO:     payload[(int)i] = i;
    // TODO: }

    // what to expect when giving in NULL pointer?
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_IPC, 16, NULL, 32, payload));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_CAN, 16, header, 32, NULL));
    // TODO: EXPECT_GE(-1,dlt_user_trace_network_segmented(&context, DLT_NW_TRACE_FLEXRAY, 16, NULL, 32, NULL));

    EXPECT_LE(0,dlt_unregister_context(&context));
    EXPECT_LE(0,dlt_unregister_app());
    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_set_log_mode
TEST(t_dlt_set_log_mode, normal)
{
    EXPECT_LE(0,dlt_init());

    EXPECT_LE(0,dlt_set_log_mode(DLT_USER_MODE_OFF));
    EXPECT_LE(0,dlt_set_log_mode(DLT_USER_MODE_EXTERNAL));
    EXPECT_LE(0,dlt_set_log_mode(DLT_USER_MODE_INTERNAL));
    EXPECT_LE(0,dlt_set_log_mode(DLT_USER_MODE_BOTH));

    EXPECT_LE(0,dlt_free());
}

TEST(t_dlt_set_log_mode, abnormal)
{
    EXPECT_LE(0,dlt_init());

    // TODO: EXPECT_GE(-1,dlt_set_log_mode(DLT_USER_MODE_UNDEFINED));
    // TODO: EXPECT_GE(-1,dlt_set_log_mode((DltUserLogMode)-100));
    // TODO: EXPECT_GE(-1,dlt_set_log_mode((DltUserLogMode)-10));
    // TODO: EXPECT_GE(-1,dlt_set_log_mode((DltUserLogMode)10));
    // TODO: EXPECT_GE(-1,dlt_set_log_mode((DltUserLogMode)100));

    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_get_log_state
TEST(t_dlt_get_log_state, normal)
{
    EXPECT_LE(0,dlt_init());

    EXPECT_GE(-1,dlt_get_log_state());

    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_verbose_mode
TEST(t_dlt_verbose_mode, normal)
{
    EXPECT_LE(0,dlt_init());

    EXPECT_LE(0,dlt_verbose_mode());

    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// t_dlt_nonverbose_mode
TEST(t_dlt_nonverbose_mode, normal)
{
    EXPECT_LE(0,dlt_init());

    EXPECT_LE(0,dlt_nonverbose_mode());

    EXPECT_LE(0,dlt_free());
}


/////////////////////////////////////////
// main
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

