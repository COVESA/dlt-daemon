# Generate dlt_version.h without Cmake
py_binary(
    name = "create_dlt_version_h",
    srcs = ["util/create_dlt_version_h.py"],
)

genrule(
    name = "dlt_version_header",
    srcs = [
        "CMakeLists.txt",
        "cmake/dlt_version.h.cmake",
    ],
    outs = ["dlt_version.h"],
    cmd = "$(location :create_dlt_version_h) $(SRCS) $(OUTS)",
    tools = [":create_dlt_version_h"],
)

# Generate dlt_user.h without Cmake
py_binary(
    name = "create_dlt_user_h",
    srcs = ["util/create_dlt_user_h.py"],
)

genrule(
    name = "dlt_user_header",
    srcs = [
        "include/dlt/dlt_user.h.in",
    ],
    outs = ["dlt_user.h"],
    cmd = "$(location :create_dlt_user_h) $(SRCS) $(OUTS)",
    tools = [":create_dlt_user_h"],
)

cc_library(
    name = "dlt_library",
    srcs = [
        "src/lib/dlt_client.c",
        "src/lib/dlt_env_ll.c",
        "src/lib/dlt_filetransfer.c",
        "src/lib/dlt_user.c",
        "src/shared/dlt_common.c",
        "src/shared/dlt_log.c",
        "src/shared/dlt_multiple_files.c",
        "src/shared/dlt_protocol.c",
        "src/shared/dlt_user_shared.c",
    ],
    copts = [
        "-Wno-unused-parameter",
        "-W",
        "-Wall",
        "-pthread",
    ],
    defines = [
        "DLT_DAEMON_USE_UNIX_SOCKET_IPC",
        "DLT_LIB_USE_UNIX_SOCKET_IPC",
        r"CONFIGURATION_FILES_DIR=\"\"/vendor/etc\"\"",
        r'DLT_USER_IPC_PATH=\"/dev/socket\"',
        "DLT_WRITEV_TIMEOUT_MS=1000",
    ],
    linkopts = [
        "-pthread",
    ],
    deps = [
        ":dlt_defaults",
    ],
    alwayslink = True,
)

cc_library(
    name = "dlt_defaults",
    hdrs = glob([
        "include/dlt/*.h",
        "src/lib/*.h",
        "src/shared/*.h",
    ]) + [
        ":dlt_user_header",
        ":dlt_version_header",
    ],
    includes = [
        "include/dlt",
        "src/lib",
        "src/shared",
    ],
)

cc_binary(
    name = "libdlt.so",
    defines = [
        "DLT_DAEMON_USE_UNIX_SOCKET_IPC",
        "DLT_LIB_USE_UNIX_SOCKET_IPC",
        r"CONFIGURATION_FILES_DIR=\"/vendor/etc\"",
        r"DLT_USER_IPC_PATH=\"/dev/socket\"",
        "DLT_WRITEV_TIMEOUT_MS=1000",
    ],
    linkshared = True,
    deps = [
        ":dlt_library",
    ],
)

cc_binary(
    name = "dlt-receive",
    srcs = [
        "src/console/dlt-control-common.c",
        "src/console/dlt-control-common.h",
        "src/console/dlt-receive.c",
    ],
    defines = [
        "DLT_DAEMON_USE_UNIX_SOCKET_IPC",
        "DLT_LIB_USE_UNIX_SOCKET_IPC",
        r"CONFIGURATION_FILES_DIR=\"/vendor/etc\"",
        r"DLT_USER_IPC_PATH=\"/dev/socket\"",
        "DLT_WRITEV_TIMEOUT_MS=1000",
    ],
    deps = [
        ":dlt_library",
    ],
)
