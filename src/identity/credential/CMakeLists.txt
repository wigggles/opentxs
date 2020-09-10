# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources "Base.cpp" "Contact.cpp" "Key.cpp" "Primary.cpp"
                "Secondary.cpp" "Verification.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Key.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Primary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Secondary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/identity/credential/Verification.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/identity/credential/Credential.hpp"
    "Base.tpp"
    "Contact.hpp"
    "Key.hpp"
    "Primary.hpp"
    "Secondary.hpp"
    "Verification.hpp"
)

add_library(opentxs-identity-credential OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-identity-credential PRIVATE opentxs::messages)
target_include_directories(
  opentxs-identity-credential SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(
  TARGET opentxs-identity-credential PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/identity/credential
  COMPONENT dev
)
