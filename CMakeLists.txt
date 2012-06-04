#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
#
# Copyright (C) 2011-2012, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# @licence end@
########

cmake_minimum_required( VERSION 2.6 )
project( automotive-dlt )

mark_as_advanced( CMAKE_BACKWARDS_COMPATIBILITY)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )
set( CMAKE_INSTALL_PREFIX "/usr" )

# Set version parameters
set( DLT_MAJOR_VERSION 2)
set( DLT_MINOR_VERSION 7)
set( DLT_PATCH_LEVEL 0)
set( DLT_VERSION ${DLT_MAJOR_VERSION}.${DLT_MINOR_VERSION}.${DLT_PATCH_LEVEL})
set( DLT_VERSION_STATE ALPHA )
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
option(BUILD_SHARED_LIBS      "Set to OFF to build static libraries"                 ON  )
option(WITH_SYSTEMD           "Set to ON to create unit files and systemd check on dlt-daemon startup"           ON )
option(WITH_SYSTEMD_WATCHDOG  "Set to ON to use the systemd watchdog in dlt-daemon"      ON)
option(WITH_DOC               "Set to ON to build documentation target"              OFF )
option(WITH_MAN               "Set to ON to build man pages"             ON )
option(WITH_CHECK_CONFIG_FILE "Set to ON to create a configure file of CheckIncludeFiles and CheckFunctionExists " OFF )
option(WITH_TESTSCRIPTS       "Set to ON to run CMakeLists.txt in testscripts"       OFF )
option(WITH_GPROF             "Set -pg to compile flags"                             OFF )
option(WITH_DLTTEST			  "Set to ON to build with modifications to test User-Daemon communication with corrupt messages" OFF)
option(WITH_DLT_SHM_ENABLE    "EXPERIMENTAL! Set to ON to use shared memory as IPC. EXPERIMENTAL!"    OFF )
option(WTIH_DLT_ADAPTOR       "Set ton ON to build src/adaptor binaries" ON)
option(WITH_DLT_CONSOLE       "Set ton ON to build src/console binaries" ON)
option(WITH_DLT_EXAMPLES      "Set ton ON to build src/examples binaries" ON)
option(WITH_DLT_SYSTEM        "Set ton ON to build src/system binaries" ON)
option(WITH_DLT_TESTS         "Set ton ON to build src/test binaries" ON)
# RPM settings
set( GENIVI_RPM_RELEASE "1")#${DLT_REVISION}")
set( LICENSE "Mozilla Public License Version 2.0" )

# Build, project and include settings
find_package(Threads REQUIRED)

include_directories( 
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/include/dlt
    ${CMAKE_SOURCE_DIR}/src/shared/ 
    ${CMAKE_SOURCE_DIR}/src/lib/
    ${CMAKE_SOURCE_DIR}/src/daemon/
    ${CMAKE_SOURCE_DIR}/systemd/3rdparty/
)

add_definitions( -D_GNU_SOURCE )

if(WITH_DLTTEST)
	add_definitions( -DDLT_TEST_ENABLE)
endif(WITH_DLTTEST)

if(WITH_DLT_SHM_ENABLE)
    add_definitions( -DDLT_SHM_ENABLE)
endif(WITH_DLT_SHM_ENABLE)
 
if(WITH_GPROF)
  SET(CMAKE_C_FLAGS "-pg")
endif(WITH_GPROF)    
   
add_definitions( "-Wall" )
add_definitions( "-Wextra" )

configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.spec.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.spec)
configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig COMPONENT devel)

add_subdirectory( cmake )

if(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG)
    if(WITH_SYSTEMD)
        add_definitions( -DDLT_SYSTEMD_ENABLE)
    endif(WITH_SYSTEMD)
    
    if(WITH_SYSTEMD_WATCHDOG)
        add_definitions( -DDLT_SYSTEMD_WATCHDOG_ENABLE)
    endif(WITH_SYSTEMD_WATCHDOG)
    
    set( systemd_SRCS ${CMAKE_SOURCE_DIR}/systemd/3rdparty/sd-daemon.c)
    
    add_subdirectory( systemd )
endif(WITH_SYSTEMD OR WITH_SYSTEMD_WATCHDOG)

add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( include )
add_subdirectory( testscripts )

message( STATUS )
message( STATUS "-------------------------------------------------------------------------------" )
message( STATUS "Build for Version ${DLT_VERSION} build ${DLT_REVISION} version state ${DLT_VERSION_STATE}")
message( STATUS "WITH_SYSTEMD = ${WITH_SYSTEMD}" )
message( STATUS "WITH_SYSTEMD_WATCHDOG = ${WITH_SYSTEMD_WATCHDOG}" )
message( STATUS "WITH_DOC = ${WITH_DOC}" )
message( STATUS "WITH_MAN = ${WITH_MAN}" )
message( STATUS "WTIH_DLT_ADAPTOR = ${WTIH_DLT_ADAPTOR}")
message( STATUS "WITH_DLT_CONSOLE = ${WITH_DLT_CONSOLE}")
message( STATUS "WITH_DLT_EXAMPLES = ${WITH_DLT_EXAMPLES}")
message( STATUS "WITH_DLT_SYSTEM = ${WITH_DLT_SYSTEM}")
message( STATUS "WITH_DLT_TESTS = ${WITH_DLT_TESTS}")
message( STATUS "WITH_DLT_SHM_ENABLE = ${WITH_DLT_SHM_ENABLE}" )
message( STATUS "WITH_DLTTEST = ${WITH_DLTTEST}" )
message( STATUS "WITH_CHECK_CONFIG_FILE = ${WITH_CHECK_CONFIG_FILE}" )
message( STATUS "WITH_TESTSCRIPTS = ${WITH_TESTSCRIPTS}" )
message( STATUS "WITH_GPROF = ${WITH_GPROF}" )
message( STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}" )
message( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
message( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
message( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
message( STATUS "-------------------------------------------------------------------------------" )
message( STATUS )
