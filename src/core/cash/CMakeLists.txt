# Copyright (c) Monetas AG, 2014

set(cxx-sources
  Mint.cpp
  MintLucre.cpp
  DigitalCash.cpp
  Purse.cpp
  Token.cpp
  TokenLucre.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/cash/*.hpp")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/lucre/include/
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core
)

add_library(opentxs-cash
    STATIC
    ${cxx-sources}
    ${cxx-headers}
)

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(opentxs-cash lucre ${OPENSSL_LIBRARIES})
