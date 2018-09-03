# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-network-zeromq)

add_subdirectory(zap)

set(cxx-sources
  Bidirectional.cpp
  Context.cpp
  CurveClient.cpp
  CurveServer.cpp
  DealerSocket.cpp
  Frame.cpp
  FrameIterator.cpp
  FrameSection.cpp
  ListenCallback.cpp
  ListenCallbackSwig.cpp
  Message.cpp
  PairEventCallback.cpp
  PairEventCallbackSwig.cpp
  PairEventListener.cpp
  PairSocket.cpp
  PublishSocket.cpp
  PullSocket.cpp
  PushSocket.cpp
  Proxy.cpp
  Receiver.cpp
  ReplyCallback.cpp
  ReplySocket.cpp
  RequestSocket.cpp
  RouterSocket.cpp
  Socket.cpp
  SubscribeSocket.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/Bidirectional.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Context.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CurveClient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CurveServer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DealerSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Frame.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ListenCallback.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ListenCallbackSwig.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PairEventCallback.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PairEventCallbackSwig.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PairEventListener.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PairSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Proxy.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PublishSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PullSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PushSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Receiver.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ReplyCallback.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ReplySocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/RequestSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/RouterSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Socket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SubscribeSocket.hpp
)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
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
        DESTINATION include/opentxs/network/zeromq
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
target_include_directories(${MODULE_NAME} PRIVATE ${OT_ZEROMQ_HEADERS})
