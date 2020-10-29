# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    BailmentNotice.cpp
    BailmentReply.cpp
    BailmentRequest.cpp
    ConnectionReply.cpp
    ConnectionRequest.cpp
    NoticeAcknowledgement.cpp
    OutBailmentReply.cpp
    OutBailmentRequest.cpp
    PeerObject.cpp
    PeerReply.cpp
    PeerRequest.cpp
    StoreSecret.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/BailmentNotice.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/BailmentReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/BailmentRequest.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/ConnectionReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/ConnectionRequest.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/OutBailmentReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/OutBailmentRequest.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/PeerObject.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/PeerReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/PeerRequest.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/StoreSecret.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    BailmentNotice.cpp
    BailmentReply.cpp
    BailmentRequest.cpp
    ConnectionReply.cpp
    ConnectionRequest.cpp
    NoticeAcknowledgement.cpp
    OutBailmentReply.cpp
    OutBailmentRequest.cpp
    PeerObject.hpp
    PeerReply.cpp
    PeerRequest.cpp
    StoreSecret.cpp
)

add_library(opentxs-core-contract-peer OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-core-contract-peer PRIVATE opentxs::messages)
target_include_directories(
  opentxs-core-contract-peer SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/contract/peer
    COMPONENT dev
  )
endif()
