#include <gtest/gtest.h>
extern "C"
{
    #include "dlt_shm.h"
}

DltShm *server_buf = (DltShm *)calloc(1, sizeof(DltShm));
DltShm *client_buf = (DltShm *)calloc(1, sizeof(DltShm));

char *dltShmNameTest = (char *)"dlt-shm-test";
int size = 1000;

/* Method: dlt_shm::t_dlt_shm_init_server */
TEST(t_dlt_shm_init_server, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, dlt_shm_init_server(server_buf, dltShmNameTest, size));
}

/* Method: dlt_shm::t_dlt_shm_init_server */
TEST(t_dlt_shm_init_server, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_shm_init_server(NULL, NULL, size));
}

/* Method: dlt_shm::t_dlt_shm_init_client */
TEST(t_dlt_shm_init_client, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, dlt_shm_init_client(client_buf, dltShmNameTest));
}

/* Method: dlt_shm::t_dlt_shm_init_client */
TEST(t_dlt_shm_init_client, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_shm_init_client(NULL, NULL));
}

/* Method: dlt_shm::t_dlt_shm_free_client */
TEST(t_dlt_shm_free_client, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, dlt_shm_free_client(client_buf));
}

/* Method: dlt_shm::t_dlt_shm_free_client */
TEST(t_dlt_shm_free_client, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_shm_free_client(NULL));
}

/* Method: dlt_shm::t_dlt_shm_free_server */
TEST(t_dlt_shm_free_server, normal)
{
    EXPECT_EQ(DLT_RETURN_OK, dlt_shm_free_server(server_buf, dltShmNameTest));
}

/* Method: dlt_shm::t_dlt_shm_free_server */
TEST(t_dlt_shm_free_server, nullpointer)
{
    EXPECT_EQ(DLT_RETURN_WRONG_PARAMETER, dlt_shm_free_server(NULL, NULL));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_break_on_failure = false;

    return RUN_ALL_TESTS();
}
