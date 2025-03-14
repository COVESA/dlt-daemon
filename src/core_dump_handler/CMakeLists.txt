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

if(WITH_DLT_COREDUMPHANDLER)

    set(PLATFORM_DIR ${PROJECT_SOURCE_DIR}/src/core_dump_handler/${TARGET_CPU_NAME})
    set(dlt_cdh_SRCS dlt_cdh.c dlt_cdh_context.c dlt_cdh_coredump.c ${PLATFORM_DIR}/dlt_cdh_cpuinfo.c dlt_cdh_crashid.c dlt_cdh_streamer.c)

    set(COREDUMP_CONF_DIR "/usr/lib/sysctl.d/")

    #add_definitions( -fno-strict-aliasing )

    if(WITH_CITYHASH)
        set(CITYHASH_DIR ${PROJECT_SOURCE_DIR}/src/core_dump_handler/cityhash_c)
        add_definitions(-DHAS_CITYHASH_C)
        set(dlt_cdh_SRCS ${CITYHASH_DIR}/city_c.c ${dlt_cdh_SRCS})
    endif(WITH_CITYHASH)

    add_executable(dlt-cdh ${dlt_cdh_SRCS})
    target_link_libraries(dlt-cdh ${ZLIB_LIBRARY})
    set_target_properties(dlt-cdh PROPERTIES LINKER_LANGUAGE C)

    configure_file(${PROJECT_SOURCE_DIR}/src/core_dump_handler/50-coredump.conf.cmake ${PROJECT_BINARY_DIR}/core_dump_handler/50-coredump.conf)

    install(TARGETS     dlt-cdh
            RUNTIME DESTINATION bin
            COMPONENT base)

    install(FILES ${PROJECT_BINARY_DIR}/core_dump_handler/50-coredump.conf DESTINATION ${COREDUMP_CONF_DIR})

endif(WITH_DLT_COREDUMPHANDLER)
