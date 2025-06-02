# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\InvoiceApp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\InvoiceApp_autogen.dir\\ParseCache.txt"
  "InvoiceApp_autogen"
  )
endif()
