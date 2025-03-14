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

if(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD OR WITH_SYSTEMD_JOURNAL)
    message( STATUS "Added ${systemd_SRCS} to dlt-system")
endif(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD OR WITH_SYSTEMD_JOURNAL)

set(dlt_system_SRCS dlt-system.c dlt-system-options.c dlt-system-process-handling.c
       dlt-system-logfile.c dlt-system-processes.c dlt-system-shell.c
       dlt-system-syslog.c dlt-system-watchdog.c dlt-system-journal.c)

if(WITH_DLT_FILETRANSFER)
  set(dlt_system_SRCS ${dlt_system_SRCS} dlt-system-filetransfer.c)
  add_definitions(-DDLT_FILETRANSFER_ENABLE)
endif(WITH_DLT_FILETRANSFER)

add_executable(dlt-system ${dlt_system_SRCS} ${systemd_SRCS})

target_link_libraries(dlt-system dlt)

if(WITH_SYSTEMD_JOURNAL)
 if(SYSTEMD_VERSION LESS 209)
  target_link_libraries(dlt-system systemd-journal systemd-id128)
 else(SYSTEMD_VERSION LESS 209)
  target_link_libraries(dlt-system systemd)
 endif(SYSTEMD_VERSION LESS 209)
endif(WITH_SYSTEMD_JOURNAL)

if(WITH_DLT_FILETRANSFER)
 target_link_libraries(dlt-system ${ZLIB_LIBRARY})
endif(WITH_DLT_FILETRANSFER)

set_target_properties(dlt-system PROPERTIES LINKER_LANGUAGE C)


install(TARGETS dlt-system
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-system.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)
