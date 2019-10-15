# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  Bidirectional.tpp
  Dealer.cpp
  Pair.cpp
  Pipeline.cpp
  Publish.cpp
  Pull.cpp
  Push.cpp
  Reply.cpp
  Request.cpp
  Router.cpp
  Sender.tpp
  Socket.cpp
  Subscribe.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Dealer.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Pair.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Publish.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Pull.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Push.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Reply.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Request.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Router.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Sender.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Socket.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/socket/Subscribe.hpp"
)
set(
  cxx-headers
  ${cxx-install-headers}
  Bidirectional.hpp
  Dealer.hpp
  Pair.hpp
  Pipeline.hpp
  Publish.hpp
  Pull.hpp
  Push.hpp
  Receiver.hpp
  Receiver.tpp
  Reply.hpp
  Request.hpp
  Router.hpp
  Sender.hpp
  Socket.hpp
  Subscribe.hpp
)

add_library(opentxs-network-zeromq-socket OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-network-zeromq-socket SYSTEM
  PRIVATE ${ZMQ_INCLUDE_DIRS}
)
set_property(
  TARGET opentxs-network-zeromq-socket
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/network/zeromq/socket
  COMPONENT dev
)
