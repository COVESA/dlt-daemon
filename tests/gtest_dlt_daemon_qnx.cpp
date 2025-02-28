// Test Fixture for close_pipes
class ClosePipesTest : public ::testing::Test {
    protected:
        int fds[2];
        ClosePipeMock mock;

        void SetUp() override {
            // Initialize file descriptors to invalid state
            fds[0] = DLT_FD_INIT;
            fds[1] = DLT_FD_INIT;
        }

        void TearDown() override {
            // Ensure no file descriptors are left open
            if (fds[0] != DLT_FD_INIT) close(fds[0]);
            if (fds[1] != DLT_FD_INIT) close(fds[1]);
        }
};

// Test closing valid pipes
TEST_F(ClosePipesTest, ClosesValidPipes) {
    // Create a pipe
    ASSERT_EQ(pipe(fds), 0);

    // Ensure file descriptors are valid
    ASSERT_GT(fds[0], 0);
    ASSERT_GT(fds[1], 0);

    // Close the pipes
    mock.close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing one invalid and one valid pipe
TEST_F(ClosePipesTest, ClosesMixedPipes) {
    // Create a pipe
    ASSERT_EQ(pipe(fds), 0);

    // Close one pipe manually
    close(fds[0]);
    fds[0] = DLT_FD_INIT;

    // Close the pipes using the function
    mock.close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing already closed pipes
TEST_F(ClosePipesTest, HandlesAlreadyClosedPipes) {
    // Set file descriptors to invalid state
    fds[0] = DLT_FD_INIT;
    fds[1] = DLT_FD_INIT;

    // Close the pipes using the function
    mock.close_pipes(fds);

    // Verify file descriptors remain DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing pipes with negative file descriptors
TEST_F(ClosePipesTest, HandlesNegativeFileDescriptors) {
    // Set file descriptors to negative values
    fds[0] = -100;
    fds[1] = -200;

    // Close the pipes using the function
    mock.close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}

// Test closing pipes with one negative and one valid file descriptor
TEST_F(ClosePipesTest, HandlesMixedNegativeAndValidDescriptors) {
    // Create a pipe
    ASSERT_EQ(pipe(fds), 0);

    // Set one file descriptor to a negative value
    fds[0] = -100;

    // Close the pipes using the function
    mock.close_pipes(fds);

    // Verify file descriptors are reset to DLT_FD_INIT
    EXPECT_EQ(fds[0], DLT_FD_INIT);
    EXPECT_EQ(fds[1], DLT_FD_INIT);
}
