# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Client.cpp Server.cpp)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/curve/Client.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/curve/Server.hpp"
)
set(cxx-headers ${cxx-install-headers} Client.hpp Server.hpp)

add_library(opentxs-network-zeromq-curve OBJECT ${cxx-sources} ${cxx-headers})

target_link_libraries(
  opentxs-network-zeromq-curve PRIVATE opentxs::messages "${OT_ZMQ_TARGET}"
)
add_dependencies(opentxs-network-zeromq-curve generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/curve
    COMPONENT dev
  )
endif()
