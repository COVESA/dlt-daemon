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

cmake_minimum_required(VERSION 2.8.5)
project(automotive-dlt)

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

include(GNUInstallDirs)
# Set version parameters
set(DLT_MAJOR_VERSION 2)
set(DLT_MINOR_VERSION 18)
set(DLT_PATCH_LEVEL 3)
set(DLT_VERSION ${DLT_MAJOR_VERSION}.${DLT_MINOR_VERSION}.${DLT_PATCH_LEVEL})
set(DLT_VERSION_STATE STABLE)
set(DLT_REVISION "")

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
option(WITH_DLT_EXAMPLES      "Set to ON to build src/examples binaries"                                         ON)
option(WITH_DLT_SYSTEM        "Set to ON to build src/system binaries"                                           OFF)
option(WITH_DLT_DBUS          "Set to ON to build src/dbus binaries"                                             OFF)
option(WITH_DLT_TESTS         "Set to ON to build src/test binaries"                                             ON)
option(WITH_DLT_UNIT_TESTS    "Set to ON to build gtest framework and tests/binaries"                            OFF)

set(DLT_IPC "FIFO" CACHE STRING "UNIX_SOCKET,FIFO")
set(DLT_USER "genivi" CACHE STRING "Set user for process not run as root")

option(WITH_DLT_PKGCONFIG     "Set to ON to generate pkgconfig .pc files"                                        ON)
option(WITH_DLT_CXX11_EXT     "Set to ON to build C++11 extensions"                                              OFF)
option(WITH_DLT_COREDUMPHANDLER "EXPERIMENTAL! Set to ON to build src/core_dump_handler binaries. EXPERIMENTAL"  OFF)
option(WITH_DLT_LOGSTORAGE_CTRL_UDEV "PROTOTYPE! Set to ON to build logstorage control application with udev support" OFF)
option(WITH_DLT_USE_IPv6      "Set to ON for IPv6 support"                                                       ON)
option(WITH_DLT_KPI           "Set to ON to build src/kpi binaries"                                              OFF)
option(WITH_DLT_FATAL_LOG_TRAP "Set to ON to enable DLT_LOG_FATAL trap(trigger segv inside dlt-user library)"    OFF)

# RPM settings
set(GENIVI_RPM_RELEASE "1")  # ${DLT_REVISION}")
set(LICENSE "Mozilla Public License Version 2.0")

# Build, project and include settings
find_package(Threads REQUIRED)
if(WITH_DLT_SYSTEM)
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
    ${PROJECT_SOURCE_DIR}/include/dlt
    ${PROJECT_BINARY_DIR}/include/dlt
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

if(DLT_IPC STREQUAL "UNIX_SOCKET")
    add_definitions(-DDLT_USE_UNIX_SOCKET_IPC)
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

if(CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(CONFIGURATION_FILES_DIR "/etc")
else()
    set(CONFIGURATION_FILES_DIR "${CMAKE_INSTALL_PREFIX}/etc")
endif()

add_definitions(-DCONFIGURATION_FILES_DIR="${CONFIGURATION_FILES_DIR}")

add_subdirectory(cmake)

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
endif()

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
    add_definitions(-DDLT_LOGSTORAGE_CTRL_UDEV_ENABLE)
endif()

if(WITH_DLT_FATAL_LOG_TRAP)
    add_definitions(-DDLT_FATAL_LOG_RESET_ENABLE)
endif()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(include/dlt)
add_subdirectory(testscripts)
if(WITH_DLT_UNIT_TESTS)
    add_subdirectory( gtest-1.7.0 )
    add_subdirectory(tests)
endif()

message(STATUS)
message(STATUS "-------------------------------------------------------------------------------")
message(STATUS "Build for Version ${DLT_VERSION} build ${DLT_REVISION} version state ${DLT_VERSION_STATE}")
message(STATUS "WITH_SYSTEMD = ${WITH_SYSTEMD}")
message(STATUS "WITH_SYSTEMD_WATCHDOG = ${WITH_SYSTEMD_WATCHDOG}")
message(STATUS "WITH_SYSTEMD_JOURNAL = ${WITH_SYSTEMD_JOURNAL}")
message(STATUS "WITH_DOC = ${WITH_DOC}")

message(STATUS "WITH_MAN = ${WITH_MAN}")
message(STATUS "WITH_DLT_ADAPTOR = ${WITH_DLT_ADAPTOR}")
message(STATUS "WITH_DLT_CONSOLE = ${WITH_DLT_CONSOLE}")
message(STATUS "WITH_DLT_EXAMPLES = ${WITH_DLT_EXAMPLES}")
message(STATUS "WITH_DLT_SYSTEM = ${WITH_DLT_SYSTEM}")
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
message(STATUS "Change a value with: cmake -D<Variable>=<Value>")
message(STATUS "-------------------------------------------------------------------------------")
message(STATUS)
