# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/vckake_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/vckake_autogen.dir/ParseCache.txt"
  "vckake_autogen"
  )
endif()
