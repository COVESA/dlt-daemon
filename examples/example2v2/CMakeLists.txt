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

#
# DLT example implementation
#

cmake_minimum_required( VERSION 2.6 )
project( automotive-dlt-example2 )

#
# find dependency packages
#

find_package(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)

#
# include directories
#

include_directories(
    ${DLT_INCLUDE_DIRS}
)

#
# build project
#

set(dlt_example2_SRCS example2.c)
add_executable(dlt-example2 ${dlt_example2_SRCS})
target_link_libraries(dlt-example2 ${DLT_LIBRARIES})
set_target_properties(dlt-example2 PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-example2
	RUNTIME DESTINATION bin
	COMPONENT base)
