# Copyright (c) Monetas AG, 2014

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow -Wno-error")
endif()

PROTOBUF_GENERATE_CPP(
    PROTO_SRC PROTO_HEADER
    Generics.proto
    Bitcoin.proto
    Markets.proto
    Moneychanger.proto)

set(ProtobufIncludePath ${CMAKE_CURRENT_BINARY_DIR}
        CACHE INTERNAL "Path to generated protobuf files.")

if (WIN32)
  # suppress std::_Copy_impl being unsafe warnings
  add_definitions("/wd4996")
endif()

include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIR})

set(MODULE_NAME otprotob)

add_library(${MODULE_NAME}
  OBJECT
  ${PROTO_SRC}
  ${PROTO_HEADER}
)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
