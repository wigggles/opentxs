set(cxx-sources
  OpenDHT.cpp
)

file(GLOB cxx-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/network/*.hpp"
)

include_directories(SYSTEM
  ${OPENDHT_INCLUDE_DIR}
  ${GNUTLS_INCLUDE_DIR}
)

set(MODULE_NAME opentxs-network)
if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress warnings about strcpy being unsafe in anyoption.cpp
  add_definitions(/wd4996)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

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

if(OT_DHT)
  target_link_libraries(${MODULE_NAME} PUBLIC ${OPENDHT_LIBRARIES} ${GNUTLS_LIBRARIES})
endif()

set_lib_property(${MODULE_NAME})

if (LIBSECP256K1_EXPORT)
# This fixes a stupid build warning (pedantically converted to error)
add_dependencies(${MODULE_NAME} libsecp256k1)
endif()

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
