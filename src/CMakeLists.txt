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
#
# @licence end@
########

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
