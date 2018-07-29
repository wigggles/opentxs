# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME otprotob)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow -Wno-error")
endif()

if(OT_GENERATE_PROTO)
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

  add_library(${MODULE_NAME}
    OBJECT
    ${PROTO_SRC}
    ${PROTO_HEADER}
  )

else()
  file(GLOB proto-files "${CMAKE_CURRENT_SOURCE_DIR}/generated/*.pb.*")
  add_library(${MODULE_NAME} OBJECT "${proto-files}")
  set(ProtobufIncludePath "${CMAKE_CURRENT_SOURCE_DIR}/generated/"
        CACHE INTERNAL "Path to generated protobuf files.")
endif()

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(${MODULE_NAME} PRIVATE ${OT_PROTOBUF_HEADERS})
