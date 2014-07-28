# Copyright (c) Monetas AG, 2014

find_package(Protobuf REQUIRED)

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER Generics.proto Bitcoin.proto Markets.proto Moneychanger.proto)

if (WIN32)
  # suppress std::_Copy_impl being unsafe warnings 
  add_Definitions("/wd4996")

  include_directories(${PROTOBUF_INCLUDE_DIR})
endif()

add_library(otprotob
  STATIC
  ${PROTO_SRC}
  ${PROTO_HEADER}
)
