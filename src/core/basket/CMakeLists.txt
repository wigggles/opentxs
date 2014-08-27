# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTBasket.cpp
  OTBasketItem.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/basket/*.hpp")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core
)

add_library(opentxs-basket
    STATIC
    ${cxx-sources}
    ${cxx-headers}
)
