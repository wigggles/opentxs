# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(basket)
add_subdirectory(peer)

set(cxx-sources
    CurrencyContract.cpp
    SecurityContract.cpp
    ServerContract.cpp
    Signable.cpp
    UnitDefinition.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/CurrencyContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/SecurityContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/ServerContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/Signable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/UnitDefinition.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/Contract.hpp"
    CurrencyContract.hpp
    SecurityContract.hpp
    ServerContract.hpp
    Signable.hpp
    UnitDefinition.hpp
)
add_library(opentxs-core-contract OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-core-contract PRIVATE opentxs::messages)
target_include_directories(
  opentxs-core-contract SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
add_dependencies(opentxs-core-contract generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/contract
    COMPONENT dev
  )
endif()
