#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Genivi::dlt" for configuration "Release"
set_property(TARGET Genivi::dlt APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Genivi::dlt PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libdlt.so.3.0.1"
  IMPORTED_SONAME_RELEASE "libdlt.so.3"
  )

list(APPEND _cmake_import_check_targets Genivi::dlt )
list(APPEND _cmake_import_check_files_for_Genivi::dlt "${_IMPORT_PREFIX}/lib/libdlt.so.3.0.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
