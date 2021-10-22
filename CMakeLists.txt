#######
# SPDX license identifier: MPL-2.0
#
# Copyright(C) 2011-2015, BMW AG
#
# This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License(MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
#######

cmake_minimum_required(VERSION 3.3)
project(automotive-dlt VERSION 2.18.8)

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

include(GNUInstallDirs)
# Set version parameters
set(DLT_VERSION_STATE STABLE)

execute_process(COMMAND git describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE DLT_REVISION
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
if(DLT_REVISION MATCHES "^$")
    set(PRINT_REVISION "Git revision unavailable")
else(DLT_REVISION MATCHES "")
    string(REPLACE "-" "_" DLT_REVISION ${DLT_REVISION})
    set(PRINT_REVISION ${DLT_REVISION})
endif()

# set default build type, if not defined by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
                         "Choose build type: Debug, Release, RelWithDebInfo, MinSizeRel."
                         FORCE)
    message(STATUS "Build type not defined. Using default build type 'RelWithDebInfo'.")
endif()

# Set of indiviual options
option(BUILD_SHARED_LIBS      "Set to OFF to build static libraries"                                             ON)
option(WITH_SYSTEMD           "Set to ON to create unit files and systemd check on dlt-daemon startup"           OFF)
option(WITH_SYSTEMD_WATCHDOG  "Set to ON to use the systemd watchdog in dlt-daemon"                              OFF)
option(WITH_SYSTEMD_JOURNAL   "Set to ON to use the systemd journal in dlt-system"                               OFF)
option(WITH_DOC               "Set to ON to build documentation target"                                          OFF)
option(WITH_MAN               "Set to ON to build man pages"                                                     OFF)
option(WITH_CHECK_CONFIG_FILE "Set to ON to create a configure file of CheckIncludeFiles and CheckFunctionExists" OFF)
option(WITH_TESTSCRIPTS       "Set to ON to run CMakeLists.txt in testscripts"                                   OFF)
option(WITH_GPROF             "Set -pg to compile flags"                                                         OFF)
option(WITH_DLTTEST           "Set to ON to build with modifications to test User-Daemon communication with corrupt messages" OFF)
option(WITH_DLT_SHM_ENABLE    "EXPERIMENTAL! Set to ON to use shared memory as IPC. EXPERIMENTAL!"               OFF)
option(WITH_DLT_ADAPTOR       "Set to ON to build src/adaptor binaries"                                          OFF)
option(WITH_DLT_CONSOLE       "Set to ON to build src/console binaries"                                          ON)
option(WITH_DLT_CONSOLE_WO_CTRL    "Set to ON not to build control commands under src/console"                   OFF)
option(WITH_DLT_CONSOLE_WO_SBTM    "Set to ON not to build dlt-sortbytimestamp under src/console"                OFF)
option(WITH_DLT_EXAMPLES      "Set to ON to build src/examples binaries"                                         ON)
option(WITH_DLT_FILETRANSFER  "Set to ON to build dlt-system with filetransfer support"                          OFF)
option(WITH_DLT_SYSTEM        "Set to ON to build src/system binaries"                                           OFF)
option(WITH_DLT_DBUS          "Set to ON to build src/dbus binaries"                                             OFF)
option(WITH_DLT_TESTS         "Set to ON to build src/test binaries"                                             ON)
option(WITH_DLT_UNIT_TESTS    "Set to ON to build gtest framework and tests/binaries"                            OFF)
option(WITH_DLT_QNX_SYSTEM    "Set to ON to build QNX system binary dlt-qnx-system"                              OFF)

set(DLT_IPC "FIFO" CACHE STRING "UNIX_SOCKET,FIFO,QNX_MESSAGE")
set(DLT_USER "genivi" CACHE STRING "Set user for process not run as root")

option(WITH_DLT_PKGCONFIG     "Set to ON to generate pkgconfig .pc files"                                        ON)
option(WITH_DLT_CXX11_EXT     "Set to ON to build C++11 extensions"                                              OFF)
option(WITH_DLT_COREDUMPHANDLER "EXPERIMENTAL! Set to ON to build src/core_dump_handler binaries. EXPERIMENTAL"  OFF)
option(WITH_DLT_LOGSTORAGE_CTRL_UDEV "PROTOTYPE! Set to ON to build logstorage control application with udev support" OFF)
option(WITH_DLT_USE_IPv6      "Set to ON for IPv6 support"                                                       ON)
option(WITH_DLT_KPI           "Set to ON to build src/kpi binaries"                                              OFF)
option(WITH_DLT_FATAL_LOG_TRAP "Set to ON to enable DLT_LOG_FATAL trap(trigger segv inside dlt-user library)"    OFF)
option(WITH_UDP_CONNECTION     "Set to ON to enable dlt UDP multicast SUPPORT"                                   OFF)
option(WITH_LIB_SHORT_VERSION "Set to ON to build library with only major number in version"                    OFF)
option(WITH_DLT_DISABLE_MACRO "Set to ON to build code without Macro interface support"                         OFF)
option(WITH_LEGACY_INCLUDE_PATH "Set to ON to add <prefix>/dlt to include paths for the CMake config file, in addition to only <prefix>"    ON)
option(WITH_EXTENDED_FILTERING "Set to OFF to build without extended filtering. Using json filter files is only supported for Linux based system with json-c and QNX." OFF)

option(WITH_DLT_DAEMON_VSOCK_IPC "Set to ON to enable VSOCK support in daemon"                                   OFF)
option(WITH_DLT_LIB_VSOCK_IPC    "Set to ON to enable VSOCK support in library (DLT_IPC is not used in library)" OFF)
set(DLT_VSOCK_PORT "13490" CACHE STRING "VSOCK port number for logging traffic.")

# RPM settings
set(GENIVI_RPM_RELEASE "1")  # ${DLT_REVISION}")
set(LICENSE "Mozilla Public License Version 2.0")

# Build, project and include settings
find_package(Threads REQUIRED)
if(WITH_DLT_COREDUMPHANDLER OR WITH_DLT_FILETRANSFER)
    set(ZLIB_LIBRARY "-lz")
    find_package(ZLIB REQUIRED)
else()
    set(ZLIB_LIBRARY "")
endif()

if(WITH_DLT_DBUS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DBUS REQUIRED dbus-1)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}/include/dlt
    ${PROJECT_SOURCE_DIR}/include/dlt
    ${PROJECT_SOURCE_DIR}/src/shared
    ${PROJECT_SOURCE_DIR}/src/core_dump_handler
    ${PROJECT_SOURCE_DIR}/src/offlinelogstorage
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/daemon
    ${PROJECT_SOURCE_DIR}/src/console
    ${PROJECT_SOURCE_DIR}/src/gateway
    ${PROJECT_SOURCE_DIR}/systemd/3rdparty
)

add_definitions(-D_GNU_SOURCE)

if(NOT DLT_IPC STREQUAL "UNIX_SOCKET" AND NOT DLT_IPC STREQUAL "FIFO" AND NOT DLT_IPC STREQUAL "QNX_MESSAGE")
    message(FATAL_ERROR "${DLT_IPC} is not a valid value for DLT_IPC")
endif()
add_definitions(-DDLT_DAEMON_USE_${DLT_IPC}_IPC)
if(WITH_DLT_DAEMON_VSOCK_IPC)
    add_definitions(-DDLT_DAEMON_VSOCK_IPC_ENABLE)
endif()
if(NOT WITH_DLT_LIB_VSOCK_IPC)
    add_definitions(-DDLT_LIB_USE_${DLT_IPC}_IPC)
else()
    add_definitions(-DDLT_LIB_USE_VSOCK_IPC)
endif()
if(WITH_DLT_DAEMON_VSOCK_IPC OR WITH_DLT_LIB_VSOCK_IPC)
    add_definitions(-DDLT_VSOCK_PORT=${DLT_VSOCK_PORT})
endif()

if(NOT DLT_USER_IPC_PATH)
    set(DLT_USER_IPC_PATH "/tmp")
endif()
add_definitions(-DDLT_USER_IPC_PATH="${DLT_USER_IPC_PATH}")

if(WITH_DLTTEST)
    add_definitions(-DDLT_TEST_ENABLE)
endif()

if(WITH_DLT_UNIT_TESTS)
    add_definitions(-DDLT_UNIT_TESTS)
endif()

if(WITH_DLT_SHM_ENABLE)
    add_definitions(-DDLT_SHM_ENABLE)
endif()

if(WITH_DLT_USE_IPv6)
    add_definitions(-DDLT_USE_IPv6)
endif()

if(WITH_DLT_QNX_SYSTEM AND NOT "${CMAKE_C_COMPILER}" MATCHES "nto-qnx|qcc")
    message(FATAL_ERROR "Can only compile for QNX with a QNX compiler.")
endif()

if(WITH_GPROF)
    add_compile_options(-pg)
endif()

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu99>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11>
    -Wall
    -Wextra
    # -pedantic
    -Wno-variadic-macros
    -Wno-strict-aliasing
    )

if(WITH_DOC STREQUAL "OFF")
    set(PACKAGE_DOC "#")
else()
    set(PACKAGE_DOC "")
endif()

if(WITH_DLT_PKGCONFIG)
    configure_file(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.spec.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.spec)
    configure_file(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)
endif()

if(WITH_DLT_CXX11_EXT AND WITH_DLT_PKGCONFIG)
    configure_file(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}-c++.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-c++.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-c++.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)
endif()

set(CONFIGURATION_FILES_DIR ${CMAKE_INSTALL_FULL_SYSCONFDIR})
add_definitions(-DCONFIGURATION_FILES_DIR="${CONFIGURATION_FILES_DIR}")

add_subdirectory(cmake)

# Message queue
if(HAVE_MQUEUE_H AND HAVE_FUNC_MQOPEN AND HAVE_FUNC_MQCLOSE AND
   HAVE_FUNC_MQUNLINK AND HAVE_FUNC_MQSEND AND HAVE_FUNC_MQRECEIVE)
    add_definitions(-DDLT_NETWORK_TRACE_ENABLE)
    set(DLT_NETWORK_TRACE_ENABLE 1)
else()
    message(STATUS "Disable network trace interface since message queue is not supported")
endif()

if(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD_JOURNAL)
    find_package(PkgConfig REQUIRED)
    execute_process(COMMAND pkg-config --modversion systemd OUTPUT_VARIABLE SYSTEMD_VERSION)
    string(REPLACE "\n" "" SYSTEMD_VERSION ${SYSTEMD_VERSION})

    if(WITH_SYSTEMD)
        add_definitions(-DDLT_SYSTEMD_ENABLE)
    endif()

    if(WITH_SYSTEMD_WATCHDOG)
        add_definitions(-DDLT_SYSTEMD_WATCHDOG_ENABLE)
    endif()

    if(WITH_SYSTEMD_JOURNAL)
        add_definitions(-DDLT_SYSTEMD_JOURNAL_ENABLE)
    endif()

    set(systemd_SRCS ${PROJECT_SOURCE_DIR}/systemd/3rdparty/sd-daemon.c)

    set(SYSTEMD_UNITDIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system" CACHE PATH
        "Set directory to install systemd unit files")

    add_subdirectory(systemd)
else()
    set(systemd_SRCS "")
endif()

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
    add_definitions(-DDLT_LOGSTORAGE_CTRL_UDEV_ENABLE)
endif()

if(WITH_DLT_FATAL_LOG_TRAP)
    add_definitions(-DDLT_FATAL_LOG_RESET_ENABLE)
endif()

if(WITH_DLT_FILTER_BACKEND_PROP)
   add_definitions(-DHAS_PROPRIETARY_FILTER_BACKEND)
endif()

if(WITH_EXTENDED_FILTERING)
    if(("${CMAKE_SYSTEM_NAME}" MATCHES "Linux") OR ("${CMAKE_SYSTEM_NAME}" MATCHES "Android"))
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(JSON REQUIRED json-c)
        set(DLT_JSON_LIBRARY ${JSON_LIBRARIES})
    elseif("${CMAKE_SYSTEM_NAME}" MATCHES "QNX")
        set(DLT_JSON_LIBRARY json)
    else()
        message(FATAL_ERROR "OS doesn't support extended filtering. Please build without cmake option '-DWITH_EXTENDED_FILTERING=OFF'")
    endif()
    add_definitions(-DEXTENDED_FILTERING)
else()
    set(DLT_JSON_LIBRARY "")
endif()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(testscripts)
if (WITH_DLT_UNIT_TESTS)
    find_package(GTest)

    if (NOT GTEST_FOUND)
        add_subdirectory( gtest-1.7.0 )
    endif()
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS)
message(STATUS "-------------------------------------------------------------------------------")
message(STATUS "Build for Version ${PROJECT_VERSION} build ${DLT_REVISION} version state ${DLT_VERSION_STATE}")
message(STATUS "WITH_SYSTEMD = ${WITH_SYSTEMD}")
message(STATUS "WITH_SYSTEMD_WATCHDOG = ${WITH_SYSTEMD_WATCHDOG}")
message(STATUS "WITH_SYSTEMD_JOURNAL = ${WITH_SYSTEMD_JOURNAL}")
message(STATUS "WITH_DOC = ${WITH_DOC}")

message(STATUS "WITH_MAN = ${WITH_MAN}")
message(STATUS "WITH_DLT_ADAPTOR = ${WITH_DLT_ADAPTOR}")
message(STATUS "WITH_DLT_CONSOLE = ${WITH_DLT_CONSOLE}")
message(STATUS "WITH_DLT_CONSOLE_WO_CTRL = ${WITH_DLT_CONSOLE_WO_CTRL}")
message(STATUS "WITH_DLT_CONSOLE_WO_SBTM = ${WITH_DLT_CONSOLE_WO_SBTM}")
message(STATUS "WITH_DLT_EXAMPLES = ${WITH_DLT_EXAMPLES}")
message(STATUS "WITH_DLT_SYSTEM = ${WITH_DLT_SYSTEM}")
message(STATUS "WITH_DLT_FILETRANSFER = ${WITH_DLT_FILETRANSFER}")
message(STATUS "WITH_DLT_DBUS = ${WITH_DLT_DBUS}")
message(STATUS "WITH_DLT_TESTS = ${WITH_DLT_TESTS}")
message(STATUS "WITH_DLT_UNIT_TESTS = ${WITH_DLT_UNIT_TESTS}")
message(STATUS "WITH_DLT_SHM_ENABLE = ${WITH_DLT_SHM_ENABLE}")
message(STATUS "WITH_DLTTEST = ${WITH_DLTTEST}")
message(STATUS "WITH_DLT_PKGCONFIG = ${WITH_DLT_PKGCONFIG}")
message(STATUS "WITH_DLT_CXX11_EXT = ${WITH_DLT_CXX11_EXT}")
message(STATUS "WITH_DLT_COREDUMPHANDLER = ${WITH_DLT_COREDUMPHANDLER}")
message(STATUS "WITH_DLT_KPI = ${WITH_DLT_KPI}")
message(STATUS "WITH_DLT_FATAL_LOG_TRAP = ${WITH_DLT_FATAL_LOG_TRAP}")
message(STATUS "WITH_CHECK_CONFIG_FILE = ${WITH_CHECK_CONFIG_FILE}")
message(STATUS "WITH_TESTSCRIPTS = ${WITH_TESTSCRIPTS}")
message(STATUS "WITH_GPROF = ${WITH_GPROF}")
message(STATUS "WITH_DLT_USE_IPv6 = ${WITH_DLT_USE_IPv6}")
message(STATUS "DLT_USER = ${DLT_USER}")
message(STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}")
message(STATUS "TARGET_CPU_NAME = ${TARGET_CPU_NAME}")
if(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD_JOURNAL)
    message(STATUS "SYSTEMD_VERSION = ${SYSTEMD_VERSION}")
    message(STATUS "SYSTEMD_UNITDIR = ${SYSTEMD_UNITDIR}")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR = ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "WITH_DLT_LOGSTORAGE_CTRL_UDEV = ${WITH_DLT_LOGSTORAGE_CTRL_UDEV}")
message(STATUS "DLT_IPC = ${DLT_IPC}(Path: ${DLT_USER_IPC_PATH})")
message(STATUS "WITH_DLT_DAEMON_VSOCK_IPC = ${WITH_DLT_DAEMON_VSOCK_IPC}")
message(STATUS "WITH_DLT_LIB_VSOCK_IPC = ${WITH_DLT_LIB_VSOCK_IPC}")
message(STATUS "DLT_VSOCK_PORT = ${DLT_VSOCK_PORT}")
message(STATUS "WITH_UDP_CONNECTION = ${WITH_UDP_CONNECTION}")
message(STATUS "WITH_DLT_QNX_SYSTEM = ${WITH_DLT_QNX_SYSTEM}")
message(STATUS "WITH_LIB_SHORT_VERSION = ${WITH_LIB_SHORT_VERSION}")
message(STATUS "WITH_LEGACY_INCLUDE_PATH = ${WITH_LEGACY_INCLUDE_PATH}")
message(STATUS "WITH_EXTENDED_FILTERING = ${WITH_EXTENDED_FILTERING}")
message(STATUS "WITH_DLT_DISABLE_MACRO = ${WITH_DLT_DISABLE_MACRO}")
message(STATUS "Change a value with: cmake -D<Variable>=<Value>")
message(STATUS "-------------------------------------------------------------------------------")
message(STATUS)
