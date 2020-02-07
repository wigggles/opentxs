# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Common.cpp Tag.cpp)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/core/util/Common.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/util/Tag.hpp"
)
set(cxx-headers ${cxx-install-headers})

add_library(opentxs-core-util OBJECT ${cxx-sources} ${cxx-headers})
set_property(TARGET opentxs-core-util PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/core/util
  COMPONENT dev
)
