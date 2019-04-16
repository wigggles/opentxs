# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-identity-credential)

set(cxx-sources
  "Base.cpp"
  "Contact.cpp"
  "Key.cpp"
  "Primary.cpp"
  "Secondary.cpp"
  "Verification.cpp"
)

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/identity/credential/Base.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/identity/credential/Contact.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/identity/credential/Key.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/identity/credential/Primary.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/identity/credential/Secondary.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/identity/credential/Verification.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/../../internal/identity/credential/Credential.hpp"
  "Base.tpp"
  "Contact.hpp"
  "Key.hpp"
  "Primary.hpp"
  "Secondary.hpp"
  "Verification.hpp"
)

if(WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings
  add_definitions("/wd4503")

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
        DESTINATION include/opentxs/identity/credential
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/")
