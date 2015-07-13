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

if(WITH_DOC)
    find_package(Doxygen)

    configure_file(${CMAKE_SOURCE_DIR}/doc/doxygen.cfg.cmake ${CMAKE_BINARY_DIR}/doc/doxygen.cfg @ONLY)
	
	add_custom_target (doc ALL
  		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/doxygen.cfg
  		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
	)
	
	add_custom_target (doc-manuals ALL
		COMMAND mkdir -p ${CMAKE_BINARY_DIR}/doc/manuals
		COMMAND mkdir -p ${CMAKE_BINARY_DIR}/doc/manuals/images
		COMMAND cp ${CMAKE_SOURCE_DIR}/doc/images/* ${CMAKE_BINARY_DIR}/doc/manuals/images
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/INSTALL.html ${CMAKE_SOURCE_DIR}/INSTALL
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/README.html ${CMAKE_SOURCE_DIR}/README
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/ReleaseNotes.html ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_user_manual.html ${CMAKE_SOURCE_DIR}/doc/dlt_user_manual.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_cheatsheet.html ${CMAKE_SOURCE_DIR}/doc/dlt_cheatsheet.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_design_specification.html ${CMAKE_SOURCE_DIR}/doc/dlt_design_specification.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_filetransfer.html ${CMAKE_SOURCE_DIR}/doc/dlt_filetransfer.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt-daemon.1.html ${CMAKE_SOURCE_DIR}/doc/dlt-daemon.1.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt-system.1.html ${CMAKE_SOURCE_DIR}/doc/dlt-system.1.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt-receive.1.html ${CMAKE_SOURCE_DIR}/doc/dlt-receive.1.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt-convert.1.html ${CMAKE_SOURCE_DIR}/doc/dlt-convert.1.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt.conf.5.html ${CMAKE_SOURCE_DIR}/doc/dlt.conf.5.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt-system.conf.5.html ${CMAKE_SOURCE_DIR}/doc/dlt-system.conf.5.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_book.html ${CMAKE_SOURCE_DIR}/doc/dlt_book.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_extended_network_trace.html ${CMAKE_SOURCE_DIR}/doc/dlt_extended_network_trace.txt
  		COMMAND asciidoc -a TOC1 -o ${CMAKE_BINARY_DIR}/doc/manuals/dlt_howto_debug.html ${CMAKE_SOURCE_DIR}/doc/dlt_howto_debug.txt
  		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
	)

	add_custom_target (doc-man
  		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
		COMMAND a2x --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/doc/dlt-daemon.1.txt
		COMMAND a2x --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/doc/dlt-system.1.txt
		COMMAND a2x --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/doc/dlt-receive.1.txt
		COMMAND a2x --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/doc/dlt-convert.1.txt
		COMMAND a2x --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/doc/dlt.conf.5.txt
		COMMAND a2x --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/doc/dlt-system.conf.5.txt
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
