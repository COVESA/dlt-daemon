#ifndef GMOCK_DLT_DAEMON_H
#define GMOCK_DLT_DAEMON_H

#include <cstdio>
#include <gmock/gmock.h>
using ::testing::_;

// Mock class for dlt_get_version
class DltVersionMock {
    public:
        virtual ~DltVersionMock() = default;
        virtual void dlt_get_version(char *buf, int size) = 0; // Pure virtual
};

// Concrete implementation with a default response
class DltVersionMockImpl : public DltVersionMock {
    public:
        void dlt_get_version(char *buf, int size) override {
            snprintf(buf, size, "DLT Daemon Version 2.18.0");
        }
};

// Mock for dlt_log_set_fifo_basedir
class DltLogSetFifoBasedirMock {
    public:
        virtual ~DltLogSetFifoBasedirMock() = default;
        MOCK_METHOD(void, dlt_log_set_fifo_basedir, (const char *dir), ());
};

// Mock for usage
class UsageMock {
    public:
        virtual ~UsageMock() = default;
        MOCK_METHOD(void, usage, (), ());
};

// Mock for fprintf
class FprintfMock {
    public:
        MOCK_METHOD(int, fprintf_wrapper, (FILE *stream, const char *format), ());

        // Static helper function for default behavior
        static int DefaultFprintfWrapper(FILE *stream, const char *format) {
            return fprintf(stream, "%s", format);
        }
};

#endif // GMOCK_DLT_DAEMON_H
