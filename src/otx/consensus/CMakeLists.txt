# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Base.cpp
    Client.cpp
    ManagedNumber.cpp
    Server.cpp
    TransactionStatement.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/Client.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/ManagedNumber.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/Server.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/consensus/TransactionStatement.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/otx/consensus/Consensus.hpp"
    Base.hpp
    Client.hpp
    ManagedNumber.hpp
    Server.hpp
)

add_library(opentxs-otx-consensus OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-otx-consensus PRIVATE opentxs::messages)
target_include_directories(
  opentxs-otx-consensus SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-otx-consensus PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/otx/consensus
  COMPONENT dev
)
