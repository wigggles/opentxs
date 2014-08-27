# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTOffer.cpp
  OTMarket.cpp
  OTTrade.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/trade/*.hpp")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/
)

add_library(opentxs-trade
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)
