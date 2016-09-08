# Copyright (c) Monetas AG, 2014

set(cxx-sources
  ServerSettings.cpp
  ConfigLoader.cpp
  PayDividendVisitor.cpp
  ClientConnection.cpp
  MessageProcessor.cpp
  MainFile.cpp
  UserCommandProcessor.cpp
  Notary.cpp
  Transactor.cpp
  OTServer.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/server/*.hpp")

include_directories(SYSTEM ${CZMQ_INCLUDE_DIRS})

set(MODULE_NAME opentxs-server)
if(WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

target_link_libraries(${MODULE_NAME} PUBLIC opentxs-ext opentxs-cash opentxs-core opentxs-storage)

if(WIN32)
  target_link_libraries(${MODULE_NAME} PUBLIC irrxml)
endif()

add_library(opentxs-proto SHARED IMPORTED)
add_library(opentxs-verify SHARED IMPORTED)

set_property(TARGET opentxs-proto PROPERTY IMPORTED_LOCATION ${OPENTXS_PROTO})
set_property(TARGET opentxs-verify PROPERTY IMPORTED_LOCATION ${OPENTXS_VERIFY})

target_link_libraries(${MODULE_NAME} PRIVATE opentxs-proto)
target_link_libraries(${MODULE_NAME} PRIVATE opentxs-verify)

set_lib_property(${MODULE_NAME})

if(WIN32)
  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/${MODULE_NAME}.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/${MODULE_NAME}.dll")

  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/${MODULE_NAME}.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/${MODULE_NAME}.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS ${MODULE_NAME}
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
