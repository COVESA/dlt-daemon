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

if(WITH_DOC)
    find_package(Doxygen)
       
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/doxygen.cfg.cmake ${CMAKE_SOURCE_DIR}/doc/doxygen.cfg @ONLY)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/filetransfer_doxygen.cfg.cmake ${CMAKE_SOURCE_DIR}/doc/filetransfer_doxygen.cfg @ONLY)
	
	add_custom_target (doc ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/doxygen.cfg 
  		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc
	)
	
	add_custom_target (doc-filetransfer ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/filetransfer_doxygen.cfg 
  		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc
	)
endif(WITH_DOC)

# Compress the man pages and install to proper place
FIND_PROGRAM(GZIP_TOOL
             NAMES gzip
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)

IF(NOT GZIP_TOOL)
  MESSAGE(FATAL_ERROR "Could not find gzip for man page compression.") 
ENDIF(NOT GZIP_TOOL)	                   

set(MAN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/dlt.conf.5 
			${CMAKE_CURRENT_SOURCE_DIR}/dlt-system.conf.5 
			${CMAKE_CURRENT_SOURCE_DIR}/dlt-convert.1 
			${CMAKE_CURRENT_SOURCE_DIR}/dlt-daemon.1 
			${CMAKE_CURRENT_SOURCE_DIR}/dlt-receive.1 
			${CMAKE_CURRENT_SOURCE_DIR}/dlt-system.1)
set(MAN_SRC_GZ dlt.conf.5.gz dlt-system.conf.5.gz dlt-convert.1.gz dlt-daemon.1.gz dlt-receive.1.gz dlt-system.1.gz)

FOREACH(MAN ${MAN_SRC})
  ADD_CUSTOM_COMMAND(OUTPUT ${MAN}.gz
                     COMMAND ${GZIP_TOOL} -c ${MAN} > ${MAN}.gz
                     DEPENDS ${MAN}
                     COMMENT "Compressing ${MAN}.gz")
ENDFOREACH(MAN)

# If user has not set the base dir for man pages, use a default location
if(MAN_INSTALL_DIR)
else()
    if(WIN32)
        set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/man)
    else()
        set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/man)
    endif()
endif()

add_library(man_pages STATIC dummy.c ${MAN_SRC_GZ})

INSTALL(FILES 	dlt.conf.5.gz
				dlt-system.conf.5.gz
	DESTINATION ${MAN_INSTALL_DIR}/man5
)

INSTALL(FILES 	dlt-convert.1.gz
				dlt-daemon.1.gz
				dlt-receive.1.gz
				dlt-system.1.gz
	DESTINATION ${MAN_INSTALL_DIR}/man1
)
	