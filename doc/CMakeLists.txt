#######
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
#######

if(WITH_DOC)
    # Markdown
    FIND_PROGRAM(PANDOC_TOOL
             NAMES pandoc
             PATHS /bin
                   /usr/bin
                   /usr/local/bin)

    if(NOT PANDOC_TOOL)
        MESSAGE(FATAL_ERROR "Could not find pandoc for man page generation.")
    endif(NOT PANDOC_TOOL)

    FIND_PROGRAM(SED_TOOL
    NAMES sed
    PATHS /bin
          /usr/bin
          /usr/local/bin)

    if(NOT SED_TOOL)
        MESSAGE(FATAL_ERROR "Could not find sed for html generation.")
    endif(NOT SED_TOOL)

    file(GLOB MD_FILES "${PROJECT_SOURCE_DIR}/doc/*.md")

    add_custom_target(generate_doc ALL)
    add_custom_command(TARGET generate_doc PRE_BUILD
        COMMAND ${PANDOC_TOOL} -s -f markdown -t html5 ${PROJECT_SOURCE_DIR}/README.md -o ${PROJECT_BINARY_DIR}/README.html
        COMMAND ${SED_TOOL} -i 's|\.md|\.html|g' ${PROJECT_BINARY_DIR}/README.html)
    foreach(FILE ${MD_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        string(REPLACE ".md" ".html" HTML_FILE ${FILENAME})
        add_custom_command(TARGET generate_doc PRE_BUILD
            COMMAND ${PANDOC_TOOL} -s -f markdown -t html5 ${FILE} -o ${PROJECT_BINARY_DIR}/doc/${HTML_FILE}
            COMMAND ${SED_TOOL} -i 's|\.md|\.html|g' ${PROJECT_BINARY_DIR}/doc/${HTML_FILE})
    endforeach()

    # Doxygen
    find_package(Doxygen REQUIRED)

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
        COMMAND cp -r ${PROJECT_SOURCE_DIR}/doc/images/* ${PROJECT_BINARY_DIR}/doc/manuals/images
        COMMAND ${ASCIIDOC_TOOL} -a TOC1 -o ${PROJECT_BINARY_DIR}/doc/manuals/dlt_design_specification.html ${PROJECT_SOURCE_DIR}/doc/dlt_design_specification.md
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

    file(GLOB MD_SRC "${MAN_SRC_DIR}/*.[1-8].md")

    set(MAN_BUILD_SRC "")
    foreach(FILE ${MD_SRC})
        get_filename_component(FILENAME ${FILE} NAME)
        string(REPLACE ".md" "" MAN_FILENAME ${FILENAME})
        list(APPEND MAN_BUILD_SRC ${MAN_BUILD_DIR}/${MAN_FILENAME})
    endforeach()

    set(MAN_BUILD_GZ "")
    foreach(FILE ${MAN_BUILD_SRC})
        list(APPEND MAN_BUILD_GZ "${FILE}.gz")
    endforeach()

    add_custom_target(generate_man ALL)

    set(CNT "0")
    list(LENGTH MD_SRC NUMBER_OF_MAN_PAGES)
    while(CNT LESS ${NUMBER_OF_MAN_PAGES})
            list(GET MD_SRC ${CNT} SRC_MD_FILE)
            list(GET MAN_BUILD_SRC ${CNT} BUILD_MAN_FILE)
            list(GET MAN_BUILD_GZ ${CNT} BUILD_MAN_GZ_FILE)

            add_custom_command(TARGET generate_man PRE_BUILD
            COMMAND ${PANDOC_TOOL} -s -t man ${SRC_MD_FILE} -o ${BUILD_MAN_FILE}
            COMMAND ${GZIP_TOOL} -c ${BUILD_MAN_FILE} > ${BUILD_MAN_GZ_FILE})

            MATH(EXPR CNT "${CNT}+1")
    endwhile()

    # If user has not set the base dir for man pages, use a default location
    set(MAN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/man)

    set(MAN5_GZ_FILES "")
    set(MAN1_GZ_FILES "")
    foreach(FILE ${MAN_BUILD_GZ})
        if(${FILE} MATCHES "(\.1\.gz)$")
            list(APPEND MAN1_GZ_FILES ${FILE})
        elseif(${FILE} MATCHES "(\.5\.gz)$")
            list(APPEND MAN5_GZ_FILES ${FILE})
        endif()
    endforeach()

    install(FILES
        ${MAN5_GZ_FILES}
        DESTINATION ${MAN_INSTALL_DIR}/man5
    )

    install(FILES
        ${MAN1_GZ_FILES}
        DESTINATION ${MAN_INSTALL_DIR}/man1
    )
endif(WITH_MAN)