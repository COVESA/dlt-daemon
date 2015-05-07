#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG"
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

if(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD OR WITH_SYSTEMD_JOURNAL)
    message( STATUS "Added ${systemd_SRCS} to dlt-system")
endif(WITH_SYSTEMD_WATCHDOG OR WITH_SYSTEMD OR WITH_SYSTEMD_JOURNAL)

set(dlt_system_SRCS dlt-system.c dlt-system-options.c dlt-system-process-handling.c 
	dlt-system-filetransfer.c dlt-system-logfile.c dlt-system-processes.c dlt-system-shell.c
	dlt-system-syslog.c dlt-system-watchdog.c dlt-system-journal.c)
add_executable(dlt-system ${dlt_system_SRCS} ${systemd_SRCS})
if(WITH_SYSTEMD_JOURNAL)
	target_link_libraries(dlt-system dlt ${ZLIB_LIBRARIES} systemd-journal systemd-id128)
else(WITH_SYSTEMD_JOURNAL)
	target_link_libraries(dlt-system dlt ${ZLIB_LIBRARIES})
endif(WITH_SYSTEMD_JOURNAL)
	
set_target_properties(dlt-system PROPERTIES LINKER_LANGUAGE C)


install(TARGETS dlt-system
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-system.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)
