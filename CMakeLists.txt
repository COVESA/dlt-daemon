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

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

PROJECT( automotive-dlt )

############## Set version parameters ####################### 
SET( ${PROJECT_NAME}_MAJOR_VERSION 2 )
SET( ${PROJECT_NAME}_MINOR_VERSION 5 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )
SET( ${PROJECT_NAME}_VERSION_STATE BETA )
SET( GENIVI_PROJECT_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})

SET( PRINT_MAJOR_VERSION ${${PROJECT_NAME}_MAJOR_VERSION})
SET( PRINT_MINOR_VERSION ${${PROJECT_NAME}_MINOR_VERSION})
SET( PRINT_PATCH_LEVEL ${${PROJECT_NAME}_PATCH_LEVEL})
SET( PRINT_VERSION ${GENIVI_PROJECT_VERSION})
SET( PRINT_VERSION_STATE ${${PROJECT_NAME}_VERSION_STATE})
SET( DLT_REVISION "")

EXECUTE_PROCESS(COMMAND git describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	OUTPUT_VARIABLE DLT_REVISION
	ERROR_VARIABLE GIT_ERROR 
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_STRIP_TRAILING_WHITESPACE)
IF( DLT_REVISION MATCHES "^$")
	SET( PRINT_REVISION "Git revision unavailable")
ELSE( DLT_REVISION MATCHES "")
	STRING(REPLACE "-" "_" DLT_REVISION ${DLT_REVISION})
	SET( PRINT_REVISION ${DLT_REVISION})	
ENDIF( DLT_REVISION MATCHES "^$")

##################### RPM SPEC CONFIG ########################
SET( GENIVI_RPM_RELEASE "1${DLT_REVISION}")
SET( LICENSE "LGPL v2.1 with special exception" )

#########################################################
SET(GENIVI_BUILDSYSTEM_AVAILABLE $ENV{GENIVI_BUILDSYSTEM})
IF(GENIVI_BUILDSYSTEM_AVAILABLE)
	INCLUDE($ENV{GENIVI_BUILDSYSTEM}/cmake_extensions/GeniviCMakeExtensions.cmake)
ENDIF(GENIVI_BUILDSYSTEM_AVAILABLE)


###################### GPROF Settings ###################
# Adds compile flag for gprof to target
# Run the binary
# Output: gmon.out in build directory
# Run: gprof <path/to/binary> > profile.output
SET( GPROF_DLT_DAEMON OFF )
SET( GPROF_DLT_LIB OFF )
SET( GPROF_DLT_EXAMPLES OFF) 
SET( GPROF_DLT_TESTS OFF) 
SET( GPROF_DLT_CONSOLE OFF)
SET( GPROF_DLT_ADAPTOR OFF)

###################### systemd Settings #################
SET( DLT_SYSLOG_APPID "SYS" )
SET( DLT_SYSLOG_CTID "LOG" )
SET( DLT_SYSLOG_PORT 4712 )

SET( prefix ${CMAKE_INSTALL_PREFIX})
SET( libdir "\${exec_prefix}/lib" )
SET( includedir "\${exec_prefix}/include" )

SET( DLTBINDIR ${CMAKE_INSTALL_PREFIX}/bin )
SET( DLTLIBDIR ${CMAKE_INSTALL_PREFIX}/lib )
SET( DLTLIBSTATIC ${CMAKE_INSTALL_PREFIX}/lib )
SET( DLTINCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include )
SET( DLTDOCDIR ${CMAKE_INSTALL_PREFIX}/doc ) 

IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF(NOT CMAKE_BUILD_TYPE)

FIND_PACKAGE(Doxygen)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES( 
${CMAKE_SOURCE_DIR}/
${CMAKE_SOURCE_DIR}/include/dlt
${CMAKE_SOURCE_DIR}/src/shared/ 
${CMAKE_SOURCE_DIR}/src/lib/
${CMAKE_SOURCE_DIR}/src/daemon/ )

ADD_DEFINITIONS( "-Wall" )

###################### IPC settings #################
# Disable following definition, if old Fifo version
# should be used
# ADD_DEFINITIONS( -DDLT_SHM_ENABLE=1 )

INCLUDE(TestBigEndian)

TEST_BIG_ENDIAN( DLT_BIGENDIAN )

IF ( DLT_BIGENDIAN )
	ADD_DEFINITIONS( -DBYTE_ORDER=BIG_ENDIAN )
ELSE ( DLT_BIGENDIAN )
	ADD_DEFINITIONS( -DBYTE_ORDER=LITTLE_ENDIAN )
ENDIF ( DLT_BIGENDIAN )

INCLUDE(CheckIncludeFiles)  

CHECK_INCLUDE_FILES( arpa/inet.h HAVE_ARPAINET_H)
CHECK_INCLUDE_FILES( fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILES( float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILES( limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILES( netdb.h HAVE_NETDB_H)
CHECK_INCLUDE_FILES( netinet/in.h HAVE_NETINETIN_H)
CHECK_INCLUDE_FILES( stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILES( stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES( stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES( string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES( strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES( sys/ioctl.h HAVE_SYSIOCTL_H)
CHECK_INCLUDE_FILES( sys/socket.h HAVE_SYSSOCKET_H)
CHECK_INCLUDE_FILES( sys/time.h HAVE_SYSTIME_H)
CHECK_INCLUDE_FILES( unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES( sys/ipc.h HAVE_SYSIPC_H)
CHECK_INCLUDE_FILES( netdb.h HAVE_NETDB_H)
CHECK_INCLUDE_FILES( ctype.h HAVE_CTYPE_H)
CHECK_INCLUDE_FILES( signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILES( syslog.h HAVE_SYSLOG_H)
CHECK_INCLUDE_FILES( sys/stat.h HAVE_SYSSTAT_H)
CHECK_INCLUDE_FILES( linux/stat.h HAVE_LINUXSTAT_H)
CHECK_INCLUDE_FILES( sys/uio.h HAVE_SYSUIO_H)
CHECK_INCLUDE_FILES( termios.h HAVE_TERMIOS_H)
CHECK_INCLUDE_FILES( unistd.h HAVE_UNISTD_H)

INCLUDE(CheckFunctionExists) 

CHECK_FUNCTION_EXISTS( bzero HAVE_FUNC_BZERO)
CHECK_FUNCTION_EXISTS( clock_gettime HAVE_FUNC_CLOCKGETTIME)
CHECK_FUNCTION_EXISTS( floor HAVE_FUNC_FLOOR)
CHECK_FUNCTION_EXISTS( fork HAVE_FUNC_FORK)
CHECK_FUNCTION_EXISTS( gethostbyname HAVE_FUNC_GETHOSTBYNAME)
CHECK_FUNCTION_EXISTS( gettimeofday HAVE_FUNC_GETTIMEBYDAY)
CHECK_FUNCTION_EXISTS( inet_ntoa HAVE_FUNC_INETNTOA)
CHECK_FUNCTION_EXISTS( malloc HAVE_FUNC_MALLOC)
CHECK_FUNCTION_EXISTS( memmove HAVE_FUNC_MEMMOVE)
CHECK_FUNCTION_EXISTS( memset HAVE_FUNC_MEMSET)
CHECK_FUNCTION_EXISTS( mkfifo HAVE_FUNC_MKFIFO)
CHECK_FUNCTION_EXISTS( select HAVE_FUNC_SELECT)
CHECK_FUNCTION_EXISTS( socket HAVE_FUNC_SOCKET)
CHECK_FUNCTION_EXISTS( strchr HAVE_FUNC_STRCHR)
CHECK_FUNCTION_EXISTS( strerror HAVE_FUNC_STRERROR)
CHECK_FUNCTION_EXISTS( strstr HAVE_FUNC_STRSTR)
CHECK_FUNCTION_EXISTS( strtol HAVE_FUNC_STRTOL)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/dlt/dlt_version.h.cmake ${CMAKE_SOURCE_DIR}/include/dlt/dlt_version.h @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/automotive-dlt.pc.cmake ${CMAKE_SOURCE_DIR}/automotive-dlt.pc @ONLY)
#CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/systemd/dlt.service.cmake ${CMAKE_SOURCE_DIR}/systemd/dlt.service @ONLY)
#CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/systemd/dlt-syslog.service.cmake ${CMAKE_SOURCE_DIR}/systemd/dlt-syslog.service @ONLY)
#CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/testscripts/Meego/dlt-daemon.cmake ${CMAKE_SOURCE_DIR}/testscripts/Meego/dlt-daemon @ONLY)
#CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/testscripts/Ubuntu/dlt-daemon.cmake ${CMAKE_SOURCE_DIR}/testscripts/Ubuntu/dlt-daemon @ONLY)


OPTION(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON )
OPTION(ENABLE_BUILD_DOC "Set to ON to build Documentation" OFF)

MESSAGE( STATUS )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "Build for Version ${PRINT_VERSION} build ${PRINT_REVISION}")
MESSAGE( STATUS "VERSION_STATE ${PRINT_VERSION_STATE}")
MESSAGE( STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
MESSAGE( STATUS "ENABLE_BUILD_DOC = ${ENABLE_BUILD_DOC}" )
MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS )

SET( BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}" CACHE BOOL "Set to OFF to build static libraries" FORCE )
SET( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Where to install ${PROJECT_NAME}" FORCE )
SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
#SET( ENABLE_BUILD_DOC "${ENABLE_BUILD_DOC}" CACHE BOOL "Set to ON to build Documentation" FORCE )

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( include )
ADD_SUBDIRECTORY( testscripts )

IF (ENABLE_BUILD_DOC)
	ADD_SUBDIRECTORY( doc )
ENDIF(ENABLE_BUILD_DOC)

INSTALL(FILES automotive-dlt.pc
        DESTINATION lib/pkgconfig 
	COMPONENT devel)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"

  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

#ADD_CUSTOM_TARGET(uninstall
#  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

EXPORT_LIBRARY_DEPENDENCIES( "${PROJECT_NAME}LibDeps.cmake" )
