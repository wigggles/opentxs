# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(credential)
add_subdirectory(wot)

set(cxx-sources "Authority.cpp" "Nym.cpp" "Source.cpp")
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/identity/Authority.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/identity/Nym.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/identity/Source.hpp"
)
set(cxx-headers ${cxx-install-headers} "Authority.hpp" "Nym.hpp")

add_library(opentxs-identity OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-identity SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/" "${sodium_INCLUDE_DIR}"
)
set_property(TARGET opentxs-identity PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/identity
  COMPONENT dev
)
