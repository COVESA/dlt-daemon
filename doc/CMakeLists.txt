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

if(WITH_MAN)
	# Compress the man pages and install to proper place
	FIND_PROGRAM(GZIP_TOOL
	             NAMES gzip
	             PATHS /bin
	                   /usr/bin
	                   /usr/local/bin)
	
	if(NOT GZIP_TOOL)
	  MESSAGE(FATAL_ERROR "Could not find gzip for man page compression.") 
	endif(NOT GZIP_TOOL)	                   
	
	set(MAN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	set(MAN_BUILD_DIR ${CMAKE_BINARY_DIR}/doc)
	
	set(MAN_SRC ${MAN_SRC_DIR}/dlt.conf.5 
				${MAN_SRC_DIR}/dlt-system.conf.5 
				${MAN_SRC_DIR}/dlt-convert.1 
				${MAN_SRC_DIR}/dlt-daemon.1 
				${MAN_SRC_DIR}/dlt-receive.1 
				${MAN_SRC_DIR}/dlt-system.1)
	set(MAN_BUILD_SRC 
				${MAN_BUILD_DIR}/dlt.conf.5 
				${MAN_BUILD_DIR}/dlt-system.conf.5 
				${MAN_BUILD_DIR}/dlt-convert.1 
				${MAN_BUILD_DIR}/dlt-daemon.1 
				${MAN_BUILD_DIR}/dlt-receive.1 
				${MAN_BUILD_DIR}/dlt-system.1)
	set(MAN_BUILD_GZ 
				${MAN_BUILD_DIR}/dlt.conf.5.gz 
				${MAN_BUILD_DIR}/dlt-system.conf.5.gz
				${MAN_BUILD_DIR}/dlt-convert.1.gz
				${MAN_BUILD_DIR}/dlt-daemon.1.gz
				${MAN_BUILD_DIR}/dlt-receive.1.gz
				${MAN_BUILD_DIR}/dlt-system.1.gz)
	
	foreach(MAN ${MAN_SRC})
		file(COPY ${MAN} DESTINATION ${MAN_BUILD_DIR})
	endforeach(MAN)
	
	add_custom_target(compress_man ALL 
				COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt.conf.5 > ${MAN_BUILD_DIR}/dlt.conf.5.gz
				COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-system.conf.5 > ${MAN_BUILD_DIR}/dlt-system.conf.5.gz
				COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-convert.1  > ${MAN_BUILD_DIR}/dlt-convert.1.gz
				COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-daemon.1 > ${MAN_BUILD_DIR}/dlt-daemon.1.gz
				COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-receive.1 > ${MAN_BUILD_DIR}/dlt-receive.1.gz
				COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-system.1 > ${MAN_BUILD_DIR}/dlt-system.1.gz)
	
	# If user has not set the base dir for man pages, use a default location
	set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/man)
	
	install(FILES 	${MAN_BUILD_DIR}/dlt.conf.5.gz
					${MAN_BUILD_DIR}/dlt-system.conf.5.gz
			DESTINATION ${MAN_INSTALL_DIR}/man5
	)
	
	install(FILES 	${MAN_BUILD_DIR}/dlt-convert.1.gz
					${MAN_BUILD_DIR}/dlt-daemon.1.gz
					${MAN_BUILD_DIR}/dlt-receive.1.gz
					${MAN_BUILD_DIR}/dlt-system.1.gz
			DESTINATION ${MAN_INSTALL_DIR}/man1
	)
endif(WITH_MAN)