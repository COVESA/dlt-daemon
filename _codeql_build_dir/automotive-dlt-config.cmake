#######
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2021, Martin Willers <M.Willers@gmx.net>
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

# Config file for the Genivi::dlt package.

# This file exports the Genivi::dlt CMake target which should be passed to the
# target_link_libraries command.
#
# In addition, the following variable is defined:
#   automotive-dlt_FOUND - TRUE if headers and library were found

include(CMakeFindDependencyMacro)

find_dependency(Threads)


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was automotive-dlt-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/automotive-dlt-targets.cmake")

check_required_components(automotive-dlt)
