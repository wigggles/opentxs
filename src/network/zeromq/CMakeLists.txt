set(cxx-sources
  Context.cpp
  Message.cpp
  ReplySocket.cpp
  RequestSocket.cpp
  Socket.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/implementation/Context.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/implementation/Message.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/implementation/ReplySocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/implementation/RequestSocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/implementation/Socket.hpp
)

set(MODULE_NAME opentxs-network-zeromq)

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
