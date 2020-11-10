# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    LMDB.cpp
    ScopeGuard.cpp
    Signals.cpp
    Sodium.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Signals.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/WorkType.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    Blank.hpp
    Container.hpp
    HDIndex.hpp
    LMDB.hpp
    Polarity.hpp
    ScopeGuard.hpp
    Sodium.hpp
    Work.hpp
)

add_library(opentxs-util OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(
  opentxs-util PRIVATE opentxs::messages unofficial-sodium::sodium
)
add_dependencies(opentxs-util generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/util
    COMPONENT dev
  )
endif()

if(OT_STORAGE_LMDB)
  target_link_libraries(opentxs-util PRIVATE lmdb)
endif()
