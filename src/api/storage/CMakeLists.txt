# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Storage.cpp)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/api/storage/Driver.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/storage/Multiplex.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/storage/Plugin.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/api/storage/Storage.hpp"
)
set(cxx-headers ${cxx-install-headers}
                "${opentxs_SOURCE_DIR}/src/internal/api/storage/Storage.hpp"
                Storage.hpp)

add_library(opentxs-api-storage OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-api-storage PRIVATE opentxs::messages)
set_property(TARGET opentxs-api-storage PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/api/storage
  COMPONENT dev
)
