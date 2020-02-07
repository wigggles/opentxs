# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  NymData.cpp
  OT_API.cpp
  OTAPI_Exec.cpp
  OTAPI_Func.cpp
  OTClient.cpp
  SwigWrap.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/client/NymData.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/client/OTAPI_Exec.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/client/OT_API.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/client/OTClient.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/client/ServerAction.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/client/SwigWrap.hpp"
)
set(cxx-headers ${cxx-install-headers} OTAPI_Func.hpp)

add_library(opentxs-client OBJECT ${cxx-sources} ${cxx-headers})
set_property(TARGET opentxs-client PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/client
  COMPONENT dev
)
