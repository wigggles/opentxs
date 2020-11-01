# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources OTPayment.cpp)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ext/Helpers.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ext/OTPayment.hpp"
)
set(cxx-headers ${cxx-install-headers})

add_library(opentxs-ext OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-ext SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(opentxs-ext PRIVATE opentxs::messages)
add_dependencies(opentxs-ext generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ext
    COMPONENT dev
  )
endif()
