# Copyright (c) Monetas AG, 2014

set(cxx-sources
  Mint.cpp
  MintLucre.cpp
  DigitalCash.cpp
  Purse.cpp
  Token.cpp
  TokenLucre.cpp
)

file(GLOB cxx-install-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/cash/*.hpp")

set(cxx-headers
  ${cxx-install-headers}
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/lucre/include/
)

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
)

set(MODULE_NAME opentxs-cash)
if(WIN32)
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
        DESTINATION include/opentxs/cash
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
