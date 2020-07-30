# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Mint.cpp Purse.cpp Token.cpp)
set(cxx-install-headers "${opentxs_SOURCE_DIR}/include/opentxs/blind/Mint.hpp"
                        "${opentxs_SOURCE_DIR}/include/opentxs/blind/Purse.hpp"
                        "${opentxs_SOURCE_DIR}/include/opentxs/blind/Token.hpp")
set(
  cxx-headers
  ${cxx-install-headers}
  Mint.hpp
  Purse.hpp
  Token.hpp
)

if(CASH_LUCRE_EXPORT)
  list(
    APPEND
      cxx-sources
      mint/Lucre.cpp
      token/Lucre.cpp
      Lucre.cpp
  )
  list(
    APPEND
      cxx-headers
      mint/Lucre.hpp
      token/Lucre.hpp
      Lucre.hpp
  )
endif()

add_library(opentxs-blind OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-blind PRIVATE opentxs::messages)

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

if(OT_LUCRE_DEBUG)
  target_compile_definitions(opentxs-blind PRIVATE OT_LUCRE_DEBUG=1)
else()
  target_compile_definitions(opentxs-blind PRIVATE OT_LUCRE_DEBUG=0)
endif()

set_property(TARGET opentxs-blind PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/blind
  COMPONENT dev
)
