#include <stdio.h>
#include "gtest/gtest.h"

extern "C" {
#include "dlt_common.h"
#include "dlt_user.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_user_cfg.h"
}

TEST(t_dlt_buffer_init_dynamic, returns_0)
{
    DltUser dlt_user;
    EXPECT_EQ(0,dlt_buffer_init_dynamic(&(dlt_user.startup_buffer), DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

