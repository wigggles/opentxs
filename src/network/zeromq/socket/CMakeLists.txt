# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-network-zeromq-socket)

set(cxx-sources
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

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Dealer.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Pair.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Publish.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Pull.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Push.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Reply.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Request.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Router.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Sender.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Socket.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/network/zeromq/socket/Subscribe.hpp"
)

set(cxx-headers
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

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/network/zeromq/socket
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
target_include_directories(${MODULE_NAME} PRIVATE ${OT_ZEROMQ_HEADERS})
