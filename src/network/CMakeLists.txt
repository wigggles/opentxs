# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(zeromq)

set(cxx-sources ServerConnection.cpp)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/OpenDHT.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/ServerConnection.hpp"
)
set(cxx-headers ${cxx-install-headers} ServerConnection.hpp)

if(DHT_EXPORT)
  list(APPEND cxx-sources OpenDHT.cpp)
  list(APPEND cxx-headers OpenDHT.hpp)
endif()

add_library(opentxs-network OBJECT ${cxx-sources} ${cxx-headers})

if(DHT_EXPORT)
  target_include_directories(
    opentxs-network SYSTEM PRIVATE ${OPENDHT_INCLUDE_DIR}
  )
  target_include_directories(
    opentxs-network SYSTEM PRIVATE ${GNUTLS_INCLUDE_DIR}
  )
endif()

target_link_libraries(opentxs-network PRIVATE opentxs::messages)
target_include_directories(
  opentxs-network SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
add_dependencies(opentxs-network generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network
    COMPONENT dev
  )
endif()
