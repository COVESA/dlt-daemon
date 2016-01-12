#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG
#
# This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
# @licence end@
#######

cmake_minimum_required( VERSION 2.8.5 )
project( automotive-dlt )

mark_as_advanced( CMAKE_BACKWARDS_COMPATIBILITY)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

include ( GNUInstallDirs )
# Set version parameters
set( DLT_MAJOR_VERSION 2)
set( DLT_MINOR_VERSION 14)
set( DLT_PATCH_LEVEL 1)
set( DLT_VERSION ${DLT_MAJOR_VERSION}.${DLT_MINOR_VERSION}.${DLT_PATCH_LEVEL})
set( DLT_VERSION_STATE STABLE )
set( DLT_REVISION "")

execute_process(COMMAND git describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE DLT_REVISION
	ERROR_VARIABLE GIT_ERROR
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_STRIP_TRAILING_WHITESPACE)
if( DLT_REVISION MATCHES "^$")
	set( PRINT_REVISION "Git revision unavailable")
else( DLT_REVISION MATCHES "")
	string(REPLACE "-" "_" DLT_REVISION ${DLT_REVISION})
	set( PRINT_REVISION ${DLT_REVISION})	
endif( DLT_REVISION MATCHES "^$")

# set default build type, if not defined by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
                         "Choose build type: Debug, Release, RelWithDebInfo, MinSizeRel."
                         FORCE)
    message(STATUS "Build type not defined. Using default build type 'RelWithDebInfo'.")
endif(NOT CMAKE_BUILD_TYPE)

# Set of indiviual options
option(BUILD_SHARED_LIBS      "Set to OFF to build static libraries"                                             ON  )
option(WITH_SYSTEMD           "Set to ON to create unit files and systemd check on dlt-daemon startup"           OFF )
option(WITH_SYSTEMD_WATCHDOG  "Set to ON to use the systemd watchdog in dlt-daemon"                              OFF)
option(WITH_SYSTEMD_JOURNAL   "Set to ON to use the systemd journal in dlt-system"                               OFF)
option(WITH_DOC               "Set to ON to build documentation target"                                          OFF )
option(WITH_MAN               "Set to ON to build man pages"                                                     ON )
option(WITH_CHECK_CONFIG_FILE "Set to ON to create a configure file of CheckIncludeFiles and CheckFunctionExists " OFF )
option(WITH_TESTSCRIPTS       "Set to ON to run CMakeLists.txt in testscripts"                                   OFF )
option(WITH_GPROF             "Set -pg to compile flags"                                                         OFF )
option(WITH_DLTTEST			  "Set to ON to build with modifications to test User-Daemon communication with corrupt messages" OFF)
option(WITH_DLT_SHM_ENABLE    "EXPERIMENTAL! Set to ON to use shared memory as IPC. EXPERIMENTAL!"               OFF )
option(WTIH_DLT_ADAPTOR       "Set to ON to build src/adaptor binaries"                                         ON)
option(WITH_DLT_CONSOLE       "Set to ON to build src/console binaries"                                         ON)
option(WITH_DLT_EXAMPLES      "Set to ON to build src/examples binaries"                                        ON)
option(WITH_DLT_SYSTEM        "Set to ON to build src/system binaries"                                          ON)
option(WITH_DLT_DBUS          "Set to ON to build src/dbus binaries"                                            ON)
option(WITH_DLT_TESTS         "Set to ON to build src/test binaries"                                            ON)
option(WITH_DLT_USER          "Set user for process not run as root"                                            genivi)
option(SYSTEMD_UNITDIR        "Set directory to install systemd unit files"                                     ${CMAKE_INSTALL_PREFIX}/lib/systemd/system)
option(WITH_DLT_CXX11_EXT     "Set to ON to build C++11 extensions"                                             OFF)
option(WITH_DLT_COREDUMPHANDLER     "EXPERIMENTAL! Set to ON to build src/core_dump_handler binaries. EXPERIMENTAL"                   OFF)
option(WITH_DLT_LOGSTORAGE_CTRL_UDEV "PROTOTYPE! Set to ON to build logstorage control application with udev support" OFF)
option(WITH_DLT_LOGSTORAGE_CTRL_PROP "PROTOTYPE! Set to ON to build logstorage control application with proprietary support" OFF)
option(WITH_DLT_USE_IPv6       "Set to ON for IPv6 support"                                                     ON)
option(WITH_DLT_KPI        	  "Set to ON to build src/kpi binaries"                                             ON)
# RPM settings
set( GENIVI_RPM_RELEASE "1")#${DLT_REVISION}")
set( LICENSE "Mozilla Public License Version 2.0" )

# Build, project and include settings
find_package(Threads REQUIRED)
if(WITH_DLT_SYSTEM)
  find_package(ZLIB REQUIRED)
endif(WITH_DLT_SYSTEM)

find_package(PkgConfig)
if(WITH_DLT_DBUS)
  pkg_check_modules(DBUS REQUIRED dbus-1)
endif(WITH_DLT_DBUS)

include_directories(
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/include/dlt
    ${CMAKE_SOURCE_DIR}/src/shared/
    ${CMAKE_SOURCE_DIR}/src/core_dump_handler/
    ${CMAKE_SOURCE_DIR}/src/offlinelogstorage/
    ${CMAKE_SOURCE_DIR}/src/lib/
    ${CMAKE_SOURCE_DIR}/src/daemon/
    ${CMAKE_SOURCE_DIR}/src/console/
    ${CMAKE_SOURCE_DIR}/src/gateway/
    ${CMAKE_SOURCE_DIR}/systemd/3rdparty/
)

add_definitions( -D_GNU_SOURCE )

if(WITH_DLTTEST)
	add_definitions( -DDLT_TEST_ENABLE)
endif(WITH_DLTTEST)

if(WITH_DLT_SHM_ENABLE)
    add_definitions( -DDLT_SHM_ENABLE)
endif(WITH_DLT_SHM_ENABLE)

if(WITH_DLT_USE_IPv6)
    add_definitions( -DDLT_USE_IPv6)
endif(WITH_DLT_USE_IPv6)

if(WITH_GPROF)
  SET(CMAKE_C_FLAGS "-pg")
endif(WITH_GPROF)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=gnu99")

add_definitions( "-Wall" )
add_definitions( "-Wextra" )
#add_definitions( "-pedantic" )
add_definitions( "-Wno-variadic-macros" )
add_definitions( "-Wno-strict-aliasing" )

IF(${WITH_DOC} STREQUAL "OFF")
	SET(PACKAGE_DOC "#")
ELSE (${WITH_DOC} STREQUAL "OFF")
	SET(PACKAGE_DOC "")
ENDIF(${WITH_DOC} STREQUAL "OFF")

configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.spec.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.spec)
configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)

if(${WITH_DLT_CXX11_EXT})
    configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}-c++.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-c++.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-c++.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)
endif()

IF(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	set(CONFIGURATION_FILES_DIR "/etc")
ELSE()
	set(CONFIGURATION_FILES_DIR "${CMAKE_INSTALL_PREFIX}/etc")
ENDIF()

add_definitions(-DCONFIGURATION_FILES_DIR="${CONFIGURATION_FILES_DIR}")

add_subdirectory( cmake )

if(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD_JOURNAL)
    execute_process(COMMAND pkg-config --modversion systemd OUTPUT_VARIABLE SYSTEMD_VERSION)

    if(WITH_SYSTEMD)
        add_definitions( -DDLT_SYSTEMD_ENABLE)
    endif(WITH_SYSTEMD)

    if(WITH_SYSTEMD_WATCHDOG)
        add_definitions( -DDLT_SYSTEMD_WATCHDOG_ENABLE)
    endif(WITH_SYSTEMD_WATCHDOG)

    if(WITH_SYSTEMD_JOURNAL)
        add_definitions( -DDLT_SYSTEMD_JOURNAL_ENABLE)
    endif(WITH_SYSTEMD_JOURNAL)

    set( systemd_SRCS ${CMAKE_SOURCE_DIR}/systemd/3rdparty/sd-daemon.c)

    add_subdirectory( systemd )
endif(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD_JOURNAL)

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
	add_definitions( -DDLT_LOGSTORAGE_CTRL_UDEV_ENABLE)
endif(WITH_DLT_LOGSTORAGE_CTRL_UDEV)

if(WITH_DLT_LOGSTORAGE_CTRL_PROP)
	add_definitions( -DDLT_LOGSTORAGE_CTRL_PROP)
endif(WITH_DLT_LOGSTORAGE_CTRL_PROP)

add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( include )
add_subdirectory( testscripts )
add_subdirectory( gtest-1.7.0 )
add_subdirectory( tests )

message( STATUS )
message( STATUS "-------------------------------------------------------------------------------" )
message( STATUS "Build for Version ${DLT_VERSION} build ${DLT_REVISION} version state ${DLT_VERSION_STATE}")
message( STATUS "WITH_SYSTEMD = ${WITH_SYSTEMD}" )
message( STATUS "WITH_SYSTEMD_WATCHDOG = ${WITH_SYSTEMD_WATCHDOG}" )
message( STATUS "WITH_SYSTEMD_JOURNAL = ${WITH_SYSTEMD_JOURNAL}" )
message( STATUS "SYSTEMD_UNITDIR = ${SYSTEMD_UNITDIR}" )
message( STATUS "WITH_DLT_USER = ${WITH_DLT_USER}" )
message( STATUS "WITH_DOC = ${WITH_DOC}" )

message( STATUS "WITH_MAN = ${WITH_MAN}" )
message( STATUS "WTIH_DLT_ADAPTOR = ${WTIH_DLT_ADAPTOR}")
message( STATUS "WITH_DLT_CONSOLE = ${WITH_DLT_CONSOLE}")
message( STATUS "WITH_DLT_EXAMPLES = ${WITH_DLT_EXAMPLES}")
message( STATUS "WITH_DLT_SYSTEM = ${WITH_DLT_SYSTEM}")
message( STATUS "WITH_DLT_DBUS = ${WITH_DLT_DBUS}")
message( STATUS "WITH_DLT_TESTS = ${WITH_DLT_TESTS}")
message( STATUS "WITH_DLT_SHM_ENABLE = ${WITH_DLT_SHM_ENABLE}" )
message( STATUS "WITH_DLTTEST = ${WITH_DLTTEST}" )
message( STATUS "WITH_DLT_CXX11_EXT = ${WITH_DLT_CXX11_EXT}" )
message( STATUS "WITH_DLT_COREDUMPHANDLER = ${WITH_DLT_COREDUMPHANDLER}" )
message( STATUS "WITH_DLT_KPI = ${WITH_DLT_KPI}" )
message( STATUS "WITH_CHECK_CONFIG_FILE = ${WITH_CHECK_CONFIG_FILE}" )
message( STATUS "WITH_TESTSCRIPTS = ${WITH_TESTSCRIPTS}" )
message( STATUS "WITH_GPROF = ${WITH_GPROF}" )
message( STATUS "WITH_DLT_USE_IPv6 = ${WITH_DLT_USE_IPv6}" )
message( STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}" )
message( STATUS "TARGET_CPU_NAME = ${TARGET_CPU_NAME}" )
if(WITH_SYSTEMD_JOURNAL)
    message( STATUS "SYSTEMD_VERSION = ${SYSTEMD_VERSION}")
endif(WITH_SYSTEMD_JOURNAL)
message( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
message( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
message( STATUS "CMAKE_HOST_SYSTEM_PROCESSOR = ${CMAKE_HOST_SYSTEM_PROCESSOR}" )
message( STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}" )
message( STATUS "WITH_DLT_LOGSTORAGE_CTRL_UDEV = ${WITH_DLT_LOGSTORAGE_CTRL_UDEV}" )
message( STATUS "WITH_DLT_LOGSTORAGE_CTRL_PROP = ${WITH_DLT_LOGSTORAGE_CTRL_PROP}" )
message( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
message( STATUS "-------------------------------------------------------------------------------" )
message( STATUS )
