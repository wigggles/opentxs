# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Dht.cpp ZAP.cpp ZMQ.cpp)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/network/Dht.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/network/ZAP.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/network/ZMQ.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    Dht.hpp
    ZAP.hpp
    ZMQ.hpp
)

add_library(opentxs-api-network OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-api-network PRIVATE opentxs::messages)
target_include_directories(
  opentxs-api-network SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api/network
    COMPONENT dev
  )
endif()
