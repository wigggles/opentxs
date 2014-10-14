# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTAgreement.cpp
  OTPaymentPlan.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/recurring/*.hpp")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
)

add_library(opentxs-recurring
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)
