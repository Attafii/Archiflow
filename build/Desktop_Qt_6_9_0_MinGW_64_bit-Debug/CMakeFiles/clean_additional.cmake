# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ContractManagementApp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ContractManagementApp_autogen.dir\\ParseCache.txt"
  "ContractManagementApp_autogen"
  )
endif()
