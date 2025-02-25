#include "gmock_dlt_daemon.h"
#include <cstdio>

void SetDefaultMockBehaviors(FprintfMock& fprintfMock, DltVersionMock& dltVersionMock) {
    // Default implementation for dlt_get_version
    ON_CALL(dltVersionMock, dlt_get_version)
        .WillByDefault([](char *buf, int size) {
            snprintf(buf, size, "DLT Daemon Version 2.18.0");
        });

    // Default implementation for fprintf_wrapper
    ON_CALL(fprintfMock, fprintf_wrapper)
        .WillByDefault([](FILE *stream, const char *format) {
            return FprintfMock::DefaultFprintfWrapper(stream, format);
        });
}
