# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\game_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\game_autogen.dir\\ParseCache.txt"
  "game_autogen"
  )
endif()
