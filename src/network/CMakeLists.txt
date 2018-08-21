# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-network)

add_subdirectory(zeromq)

set(cxx-sources
  OpenDHT.cpp
  ServerConnection.cpp
)

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/network/OpenDHT.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/network/ServerConnection.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/OpenDHT.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ServerConnection.hpp"
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
        DESTINATION include/opentxs/network
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})

if(DHT_EXPORT)
  target_include_directories(${MODULE_NAME} PRIVATE ${OPENDHT_INCLUDE_DIR})
  target_include_directories(${MODULE_NAME} PRIVATE ${GNUTLS_INCLUDE_DIR})
endif()
