add_subdirectory(client)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(storage)

set(cxx-sources
  Activity.cpp
  Api.cpp
  Blockchain.cpp
  ContactManager.cpp
  Identity.cpp
  Native.cpp
  Server.cpp
  Settings.cpp
  UI.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/api/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/Activity.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Api.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ContactManager.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Native.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Server.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/UI.hpp
)

set(MODULE_NAME opentxs-api)

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
    util/OTWindowsRegistryTools.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/api
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
