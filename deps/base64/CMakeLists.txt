set(c-sources
  base64.c
)

file(GLOB c-headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(MODULE_NAME base64)
add_library(${MODULE_NAME} OBJECT
  ${c-sources}
  ${c-headers}
)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
