# Copyright (c) 2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(zeromq)

set(cxx-sources OpenDHT.cpp ServerConnection.cpp)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/network/OpenDHT.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/ServerConnection.hpp"
)
set(cxx-headers ${cxx-install-headers} OpenDHT.hpp ServerConnection.hpp)

add_library(opentxs-network OBJECT ${cxx-sources} ${cxx-headers})

if(DHT_EXPORT)
  target_include_directories(
    opentxs-network SYSTEM
    PRIVATE ${OPENDHT_INCLUDE_DIR}
  )
  target_include_directories(
    opentxs-network SYSTEM
    PRIVATE ${GNUTLS_INCLUDE_DIR}
  )
endif()

set_property(TARGET opentxs-network PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/network
  COMPONENT dev
)
