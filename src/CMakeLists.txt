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

add_subdirectory( lib )
add_subdirectory( daemon )
add_subdirectory( gateway )

if( WITH_DLT_CONSOLE )
    add_subdirectory( console )
endif( WITH_DLT_CONSOLE )

if( WITH_DLT_EXAMPLES )
    add_subdirectory( examples )
endif( WITH_DLT_EXAMPLES )

if( WITH_DLT_ADAPTOR_STDIN OR WITH_DLT_ADAPTOR_UDP OR WITH_DLT_ADAPTOR )
    add_subdirectory( adaptor )
endif( WITH_DLT_ADAPTOR_STDIN OR WITH_DLT_ADAPTOR_UDP OR WITH_DLT_ADAPTOR )

if( WITH_DLT_TESTS )
    add_subdirectory( tests )
endif( WITH_DLT_TESTS )

if( WITH_DLT_SYSTEM )
    add_subdirectory( system )
endif( WITH_DLT_SYSTEM )

if( WITH_DLT_DBUS )
    add_subdirectory( dbus )
endif( WITH_DLT_DBUS )

if( WITH_DLT_COREDUMPHANDLER )
    add_subdirectory( core_dump_handler )
endif( WITH_DLT_COREDUMPHANDLER )

if( WITH_DLT_KPI )
    add_subdirectory( kpi )
endif( WITH_DLT_KPI )

if( WITH_DLT_QNX_SYSTEM )
    add_subdirectory( dlt-qnx-system )
endif( WITH_DLT_QNX_SYSTEM )
