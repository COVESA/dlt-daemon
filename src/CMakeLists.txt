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

add_subdirectory( lib )
add_subdirectory( daemon )

if( WITH_DLT_CONSOLE )
    add_subdirectory( console )
endif( WITH_DLT_CONSOLE )

if( WITH_DLT_EXAMPLES )
    add_subdirectory( examples )
endif( WITH_DLT_EXAMPLES )

if( WTIH_DLT_ADAPTOR )
    add_subdirectory( adaptor )
endif( WTIH_DLT_ADAPTOR )

if( WITH_DLT_TESTS )
    add_subdirectory( tests )
endif( WITH_DLT_TESTS )

if( WITH_DLT_SYSTEM )
    add_subdirectory( system )
endif( WITH_DLT_SYSTEM )

if( WITH_DLT_DBUS )
    add_subdirectory( dbus )
endif( WITH_DLT_DBUS )
