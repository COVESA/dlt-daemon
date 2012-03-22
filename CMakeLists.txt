#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
 #
 # Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 # 
 # This program is free software; you can redistribute it and/or modify it under the terms of the 
 # GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 # This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 # the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 # Public License, version 2.1, for more details.
 # 
 # You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 # with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 # 
 # Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 # also be applicable to programs even in cases in which the program is not a library in the technical sense.
 # 
 # Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 # license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 # license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 # may use the program under the following exception.
 # 
 # As a special exception, the copyright holders of DLT give you permission to combine DLT 
 # with software programs or libraries that are released under any license unless such a combination is not
 # permitted by the license of such a software program or library. You may copy and distribute such a 
 # system following the terms of the GNU Lesser General Public License, version 2.1, including this
 # special exception, for DLT and the licenses of the other code concerned.
 # 
 # Note that people who make modified versions of DLT are not obligated to grant this special exception 
 # for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 # version 2.1, gives permission to release a modified version without this exception; this exception 
 # also makes it possible to release a modified version which carries forward this exception.
 #
 # @licence end@
########

cmake_minimum_required( VERSION 2.6 )
project( automotive-dlt )

mark_as_advanced( CMAKE_BACKWARDS_COMPATIBILITY)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

# Set version parameters
set( DLT_MAJOR_VERSION 2)
set( DLT_MINOR_VERSION 5)
set( DLT_PATCH_LEVEL 1)
set( DLT_VERSION ${DLT_MAJOR_VERSION}.${DLT_MINOR_VERSION}.${DLT_PATCH_LEVEL})
set( DLT_VERSION_STATE RELEASE )
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
option(WITH_DLT_SHM_ENABLE    "Set to OFF to use FIFO as IPC from user to daemon"    OFF )
option(WITH_DOC               "Set to ON to build documentation target"              OFF )
option(WITH_MAN               "Set to OFF to skip building of man pages"             ON )
option(WITH_CHECK_CONFIG_FILE "Set to ON to create a configure file of CheckIncludeFiles and CheckFunctionExists " OFF )
option(WITH_TESTSCRIPTS       "Set to ON to run CMakeLists.txt in testscripts"       OFF )
option(WITH_SYSTEMD           "Set to ON to run CMakeLists.txt in systemd"           OFF )
option(WITH_GPROF             "Set -pg to compile flags"                             OFF )
option(WITH_DLTTEST			  "Set to ON to build with modifications to test Uuser-Daemon communication with corrupt messages" OFF)

# RPM settings
set( GENIVI_RPM_RELEASE "1")#${DLT_REVISION}")
set( LICENSE "LGPL v2.1 with special exception" )

# Build, project and include settings
find_package(Threads REQUIRED)

include_directories( 
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/include/dlt
    ${CMAKE_SOURCE_DIR}/src/shared/ 
    ${CMAKE_SOURCE_DIR}/src/lib/
    ${CMAKE_SOURCE_DIR}/src/daemon/
)

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

add_subdirectory( cmake )

message( STATUS )
message( STATUS "-------------------------------------------------------------------------------" )
message( STATUS "Build for Version ${DLT_VERSION} build ${DLT_REVISION} version state ${DLT_VERSION_STATE}")
message( STATUS "WITH_DLT_SHM_ENABLE = ${WITH_DLT_SHM_ENABLE}" )
message( STATUS "WITH_CHECK_CONFIG_FILE = ${WITH_CHECK_CONFIG_FILE}" )
message( STATUS "WITH_DOC = ${WITH_DOC}" )
message( STATUS "WITH_TESTSCRIPTS = ${WITH_TESTSCRIPTS}" )
message( STATUS "WITH_SYSTEMD = ${WITH_SYSTEMD}" )
message( STATUS "WITH_GPROF = ${WITH_GPROF}" )
message( STATUS "WITH_MAN = ${WITH_MAN}" )
message( STATUS "WITH_DLTTEST = ${WITH_DLTTEST}" )
message( STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}" )
message( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
message( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
message( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
message( STATUS "-------------------------------------------------------------------------------" )
message( STATUS )

configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.spec.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.spec)
configure_file(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.pc.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig COMPONENT devel)

add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( include )
add_subdirectory( testscripts )
add_subdirectory( systemd )
