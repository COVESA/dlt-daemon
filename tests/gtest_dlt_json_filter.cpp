#include <stdio.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <syslog.h>

extern "C"
{
    #include "dlt-control-common.h"
    #include "dlt-daemon.h"
}

/* Begin Method: dlt_common::dlt_message_print_ascii with json filter*/
TEST(t_dlt_message_print_ascii_with_json_filter, normal)
{
    DltFile file;
    DltFilter filter;
    static char text[DLT_DAEMON_TEXTSIZE];

    /* Get PWD so file can be used*/
    char pwd[100];
    char openfile[114];

    /* ignore returned value from getcwd */
    if (getcwd(pwd, 100) == NULL) {}

    char openfilter[117];
    sprintf(openfile, "%s/testfile_extended.dlt", pwd);
    sprintf(openfilter, "%s/testfilter.json", pwd);
    /*---------------------------------------*/

    /* Normal Use-Case, expect 0 */
    EXPECT_LE(DLT_RETURN_OK, dlt_file_init(&file, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_filter_init(&filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_json_filter_load(&filter, openfilter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_set_filter(&file, &filter, 0));
    EXPECT_LE(DLT_RETURN_OK, dlt_file_open(&file, openfile, 0));

    char tmp[DLT_ID_SIZE+1];
    strncpy(tmp, filter.apid[0], DLT_ID_SIZE);
    tmp[DLT_ID_SIZE] = {};
    EXPECT_STREQ("LOG",tmp);
    EXPECT_EQ(3,filter.log_level[0]);
    EXPECT_EQ(0,filter.payload_min[0]);
    EXPECT_EQ(INT32_MAX,filter.payload_max[0]);


    strncpy(tmp, filter.apid[1], DLT_ID_SIZE);
    EXPECT_STREQ("app",tmp);
    strncpy(tmp, filter.ctid[1], DLT_ID_SIZE);
    EXPECT_STREQ("",tmp);

    EXPECT_EQ(0,filter.log_level[2]);
    EXPECT_EQ(20,filter.payload_min[2]);
    EXPECT_EQ(50,filter.payload_max[2]);

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
TEST(t_dlt_message_print_ascii_with_json_filter, abnormal)
{
    /* equal with t_dlt_message_print_ascii */
}
TEST(t_dlt_message_print_ascii_with_json_filter, nullpointer)
{
    /* equal with t_dlt_message_print_ascii */
}
/* End Method: dlt_common::dlt_message_print_ascii with json filter*/
