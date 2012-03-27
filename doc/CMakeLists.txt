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
# @licence end@
########

if(WITH_DOC)
    find_package(Doxygen)
       
    configure_file(${CMAKE_SOURCE_DIR}/doc/doxygen.cfg.cmake ${CMAKE_BINARY_DIR}/doc/doxygen.cfg @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/doc/filetransfer_doxygen.cfg.cmake ${CMAKE_BINARY_DIR}/doc/filetransfer_doxygen.cfg @ONLY)
	
	add_custom_target (doc ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/doxygen.cfg 
  		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
	)
	
	add_custom_target (doc-filetransfer ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/filetransfer_doxygen.cfg 
  		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
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
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${MAN} ${MAN_BUILD_DIR}) 
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