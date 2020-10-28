# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(curve)
add_subdirectory(socket)
add_subdirectory(zap)

set(cxx-sources
    Context.cpp
    Frame.cpp
    FrameIterator.cpp
    FrameSection.cpp
    ListenCallback.cpp
    ListenCallbackSwig.cpp
    Message.cpp
    PairEventCallback.cpp
    PairEventCallbackSwig.cpp
    PairEventListener.cpp
    Proxy.cpp
    ReplyCallback.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Context.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Frame.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/FrameIterator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/FrameSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ListenCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ListenCallbackSwig.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Message.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/PairEventCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/PairEventCallbackSwig.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Pipeline.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/Proxy.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/ReplyCallback.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    Context.hpp
    Frame.hpp
    ListenCallback.hpp
    ListenCallbackSwig.hpp
    PairEventCallback.hpp
    PairEventCallbackSwig.hpp
    PairEventListener.hpp
    Proxy.hpp
    ReplyCallback.hpp
)

add_library(opentxs-network-zeromq OBJECT ${cxx-sources} ${cxx-headers})

target_link_libraries(
  opentxs-network-zeromq PRIVATE opentxs::messages "${OT_ZMQ_TARGET}"
)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq
    COMPONENT dev
  )
endif()
