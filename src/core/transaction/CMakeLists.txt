# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Helpers.cpp)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/transaction/Helpers.hpp"
)
set(cxx-headers ${cxx-install-headers})

add_library(opentxs-core-transaction OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-core-transaction SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(opentxs-core-transaction PRIVATE opentxs::messages)
add_dependencies(opentxs-core-transaction generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/transaction
    COMPONENT dev
  )
endif()
