# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-recurring)

set(cxx-sources
  OTAgreement.cpp
  OTPaymentPlan.cpp
)

file(GLOB cxx-install-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/recurring/*.hpp")

set(cxx-headers
  ${cxx-install-headers}
)

add_library(${MODULE_NAME}
  OBJECT
  ${cxx-sources}
  ${cxx-headers}
)

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/core/recurring
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/")
