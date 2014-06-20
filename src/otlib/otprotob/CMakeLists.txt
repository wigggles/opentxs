# Copyright (c) Monetas AG, 2014

find_package(Protobuf REQUIRED)

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER Generics.proto Bitcoin.proto Markets.proto Moneychanger.proto)

add_library(otprotob
  STATIC
  ${PROTO_SRC}
  ${PROTO_HEADER}
)
