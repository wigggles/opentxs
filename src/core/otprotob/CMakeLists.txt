# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

protobuf_generate_cpp(
  cxx-sources
  cxx-headers
  Bitcoin.proto
  Generics.proto
  Markets.proto
  Moneychanger.proto
)

add_library(otprotob OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-api SYSTEM
  INTERFACE "${Protobuf_INCLUDE_DIRS}"
)

if(NOT MSVC)
  target_compile_options(otprotob PRIVATE "-Wno-effc++")
endif()

set_property(TARGET otprotob PROPERTY POSITION_INDEPENDENT_CODE 1)
