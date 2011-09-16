#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
 #
 # Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 # 
 # This program is free software; you can redistribute it and/or modify it under the terms of the 
 # GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 # This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 # the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 # Public License, version 2.1, for more details.
 # 
 # You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 # with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 # 
 # Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 # also be applicable to programs even in cases in which the program is not a library in the technical sense.
 # 
 # Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 # license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 # license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 # may use the program under the following exception.
 # 
 # As a special exception, the copyright holders of DLT give you permission to combine DLT 
 # with software programs or libraries that are released under any license unless such a combination is not
 # permitted by the license of such a software program or library. You may copy and distribute such a 
 # system following the terms of the GNU Lesser General Public License, version 2.1, including this
 # special exception, for DLT and the licenses of the other code concerned.
 # 
 # Note that people who make modified versions of DLT are not obligated to grant this special exception 
 # for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 # version 2.1, gives permission to release a modified version without this exception; this exception 
 # also makes it possible to release a modified version which carries forward this exception.
 #
 # @licence end@
########
if(UNIX)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	if(EXISTS "/etc/issue")
      FILE(READ "/etc/issue" LINUX_ISSUE)
	  # Fedora case
      if(LINUX_ISSUE MATCHES "Fedora")
        set(DISTRIBUTION "Fedora")      
      endif(LINUX_ISSUE MATCHES "Fedora")
      # Ubuntu case
      if(LINUX_ISSUE MATCHES "Ubuntu")
        set(DISTRIBUTION "Ubuntu")   
      endif(LINUX_ISSUE MATCHES "Ubuntu")
      # Debian case
      if(LINUX_ISSUE MATCHES "Debian")
        set(DISTRIBUTION "Debian")       
      endif(LINUX_ISSUE MATCHES "Debian")      
      # Open SuSE case
      if(LINUX_ISSUE MATCHES "SUSE") 
        set(DISTRIBUTION "SUSE")       
      endif(LINUX_ISSUE MATCHES "SUSE")
      if(LINUX_ISSUE MATCHES "Meego") 
        set(DISTRIBUTION "Meego")       
      endif(LINUX_ISSUE MATCHES "Meego")
      
      install(PROGRAMS ${DISTRIBUTION}/dlt-daemon
        DESTINATION /etc/init.d/
	COMPONENT base)
      
    endif(EXISTS "/etc/issue")      
  endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
endif(UNIX)



