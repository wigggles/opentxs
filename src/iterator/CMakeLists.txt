# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-iterator)

set(cxx-sources "")
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/iterator/Bidirectional.hpp"
)

set(cxx-headers "${cxx-install-headers}")

add_library(opentxs-iterator OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-iterator PRIVATE opentxs::messages)
set_target_properties(opentxs-iterator PROPERTIES LINKER_LANGUAGE CXX)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/iterator
    COMPONENT dev
  )
endif()
