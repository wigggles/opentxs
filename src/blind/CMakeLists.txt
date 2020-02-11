# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  mint/Lucre.cpp
  token/Lucre.cpp
  Lucre.cpp
  Mint.cpp
  Purse.cpp
  Token.cpp
)
set(cxx-install-headers "${opentxs_SOURCE_DIR}/include/opentxs/blind/Mint.hpp"
                        "${opentxs_SOURCE_DIR}/include/opentxs/blind/Purse.hpp"
                        "${opentxs_SOURCE_DIR}/include/opentxs/blind/Token.hpp")
set(
  cxx-headers
  ${cxx-install-headers}
  mint/Lucre.hpp
  token/Lucre.hpp
  Lucre.hpp
  Mint.hpp
  Purse.hpp
  Token.hpp
)

add_library(opentxs-blind OBJECT ${cxx-sources} ${cxx-headers})

if(OT_OPENSSL_FLAVOR_LIBRESSL)
  target_compile_definitions(opentxs-blind PRIVATE OT_BUNDLED_LIBRESSL)
endif()

target_include_directories(
  opentxs-blind SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_include_directories(
  opentxs-blind SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/lucre/include" "${OPENSSL_INCLUDE_DIR}"
)
set_property(TARGET opentxs-blind PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blind
  COMPONENT dev
)
