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

    configure_file(${PROJECT_SOURCE_DIR}/doc/doxygen.cfg.cmake ${PROJECT_BINARY_DIR}/doc/doxygen.cfg @ONLY)

    add_custom_target (doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/doxygen.cfg
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc
    )

    FIND_PROGRAM(ASCIIDOC_TOOL
             NAMES asciidoc
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)

    if(NOT ASCIIDOC_TOOL)
        MESSAGE(FATAL_ERROR "Could not find asciidoc for doc-manuals generation.")
    endif(NOT ASCIIDOC_TOOL)

    add_custom_target (doc-manuals ALL
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/doc/manuals
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/doc/manuals/images
        COMMAND cp ${PROJECT_SOURCE_DIR}/doc/images/* ${PROJECT_BINARY_DIR}/doc/manuals/images
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/README.html ${PROJECT_SOURCE_DIR}/README.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_glossary.html ${CMAKE_SOURCE_DIR}/doc/dlt_glossary.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_for_developers.html ${CMAKE_SOURCE_DIR}/doc/dlt_for_developers.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_offline_logstorage.html ${CMAKE_SOURCE_DIR}/doc/dlt_offline_logstorage.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_multinode.html ${CMAKE_SOURCE_DIR}/doc/dlt_multinode.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_extended_network_trace.html ${CMAKE_SOURCE_DIR}/doc/dlt_extended_network_trace.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_filetransfer.html ${CMAKE_SOURCE_DIR}/doc/dlt_filetransfer.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_kpi.html ${CMAKE_SOURCE_DIR}/doc/dlt_kpi.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_cdh.html ${CMAKE_SOURCE_DIR}/doc/dlt_cdh.md
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_design_specification.html ${PROJECT_SOURCE_DIR}/doc/dlt_design_specification.txt
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc
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

    FIND_PROGRAM(PANDOC_TOOL
             NAMES pandoc
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)

    if(NOT PANDOC_TOOL)
        MESSAGE(FATAL_ERROR "Could not find pandoc for man page generation.")
    endif(NOT PANDOC_TOOL)

    set(MAN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(MAN_BUILD_DIR ${PROJECT_BINARY_DIR}/doc)

    set(MD_SRC
        ${MAN_SRC_DIR}/dlt.conf.5.md
        ${MAN_SRC_DIR}/dlt-system.conf.5.md
        ${MAN_SRC_DIR}/dlt-convert.1.md
        ${MAN_SRC_DIR}/dlt-sortbytimestamp.1.md
        ${MAN_SRC_DIR}/dlt-daemon.1.md
        ${MAN_SRC_DIR}/dlt-receive.1.md
        ${MAN_SRC_DIR}/dlt-system.1.md
        ${MAN_SRC_DIR}/dlt-logstorage-ctrl.1.md
        ${MAN_SRC_DIR}/dlt-passive-node-ctrl.1.md)

    set(MAN_SRC
        ${MAN_SRC_DIR}/dlt.conf.5
        ${MAN_SRC_DIR}/dlt-system.conf.5
        ${MAN_SRC_DIR}/dlt-convert.1
        ${MAN_SRC_DIR}/dlt-sortbytimestamp.1
        ${MAN_SRC_DIR}/dlt-daemon.1
        ${MAN_SRC_DIR}/dlt-receive.1
        ${MAN_SRC_DIR}/dlt-system.1
        ${MAN_SRC_DIR}/dlt-logstorage-ctrl.1
        ${MAN_SRC_DIR}/dlt-passive-node-ctrl.1)

    set(MAN_BUILD_SRC
        ${MAN_BUILD_DIR}/dlt.conf.5
        ${MAN_BUILD_DIR}/dlt-system.conf.5
        ${MAN_BUILD_DIR}/dlt-convert.1
        ${MAN_BUILD_DIR}/dlt-sortbytimestamp.1
        ${MAN_BUILD_DIR}/dlt-daemon.1
        ${MAN_BUILD_DIR}/dlt-receive.1
        ${MAN_BUILD_DIR}/dlt-system.1
        ${MAN_BUILD_DIR}/dlt-logstorage-ctrl.1
        ${MAN_BUILD_DIR}/dlt-passive-node-ctrl.1)

    set(MAN_BUILD_GZ
        ${MAN_BUILD_DIR}/dlt.conf.5.gz
        ${MAN_BUILD_DIR}/dlt-system.conf.5.gz
        ${MAN_BUILD_DIR}/dlt-convert.1.gz
        ${MAN_BUILD_DIR}/dlt-sortbytimestamp.1.gz
        ${MAN_BUILD_DIR}/dlt-daemon.1.gz
        ${MAN_BUILD_DIR}/dlt-receive.1.gz
        ${MAN_BUILD_DIR}/dlt-system.1.gz
        ${MAN_BUILD_DIR}/dlt-logstorage-ctrl.1.gz
        ${MAN_BUILD_DIR}/dlt-passive-node-ctrl.1.gz)

    add_custom_target(generate_man ALL
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt.conf.5.md -o ${MAN_BUILD_DIR}/dlt.conf.5
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-system.conf.5.md -o ${MAN_BUILD_DIR}/dlt-system.conf.5
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-convert.1.md -o ${MAN_BUILD_DIR}/dlt-convert.1
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-sortbytimestamp.1.md -o ${MAN_BUILD_DIR}/dlt-sortbytimestamp.1
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-daemon.1.md -o ${MAN_BUILD_DIR}/dlt-daemon.1
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-receive.1.md -o ${MAN_BUILD_DIR}/dlt-receive.1
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-system.1.md -o ${MAN_BUILD_DIR}/dlt-system.1
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-logstorage-ctrl.1.md -o ${MAN_BUILD_DIR}/dlt-logstorage-ctrl.1
        COMMAND ${PANDOC_TOOL} -s -t man ${MAN_SRC_DIR}/dlt-passive-node-ctrl.1.md -o ${MAN_BUILD_DIR}/dlt-passive-node-ctrl.1

        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt.conf.5 > ${MAN_BUILD_DIR}/dlt.conf.5.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-system.conf.5 > ${MAN_BUILD_DIR}/dlt-system.conf.5.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-convert.1  > ${MAN_BUILD_DIR}/dlt-convert.1.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-sortbytimestamp.1  > ${MAN_BUILD_DIR}/dlt-sortbytimestamp.1.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-daemon.1 > ${MAN_BUILD_DIR}/dlt-daemon.1.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-receive.1 > ${MAN_BUILD_DIR}/dlt-receive.1.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-system.1 > ${MAN_BUILD_DIR}/dlt-system.1.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-logstorage-ctrl.1 > ${MAN_BUILD_DIR}/dlt-logstorage-ctrl.1.gz
        COMMAND ${GZIP_TOOL} -c ${MAN_BUILD_DIR}/dlt-passive-node-ctrl.1 > ${MAN_BUILD_DIR}/dlt-passive-node-ctrl.1.gz)

    # If user has not set the base dir for man pages, use a default location
    set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/man)

    install(FILES
        ${MAN_BUILD_DIR}/dlt.conf.5.gz
        ${MAN_BUILD_DIR}/dlt-system.conf.5.gz
        DESTINATION ${MAN_INSTALL_DIR}/man5
    )

    install(FILES
        ${MAN_BUILD_DIR}/dlt-convert.1.gz
        ${MAN_BUILD_DIR}/dlt-sortbytimestamp.1.gz
        ${MAN_BUILD_DIR}/dlt-daemon.1.gz
        ${MAN_BUILD_DIR}/dlt-receive.1.gz
        ${MAN_BUILD_DIR}/dlt-system.1.gz
        ${MAN_BUILD_DIR}/dlt-logstorage-ctrl.1.gz
        ${MAN_BUILD_DIR}/dlt-passive-node-ctrl.1.gz
        DESTINATION ${MAN_INSTALL_DIR}/man1
    )
endif(WITH_MAN)
