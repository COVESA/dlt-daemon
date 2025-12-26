#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Genivi::dlt" for configuration "Debug"
set_property(TARGET Genivi::dlt APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Genivi::dlt PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libdlt.so.2.18.10"
  IMPORTED_SONAME_DEBUG "libdlt.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS Genivi::dlt )
list(APPEND _IMPORT_CHECK_FILES_FOR_Genivi::dlt "${_IMPORT_PREFIX}/lib/libdlt.so.2.18.10" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
