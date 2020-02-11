# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources OTOffer.cpp OTMarket.cpp OTTrade.cpp)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/core/trade/OTMarket.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/trade/OTOffer.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/trade/OTTrade.hpp"
)
set(cxx-headers ${cxx-install-headers})

add_library(opentxs-trade OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-trade SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-trade PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/core/trade
  COMPONENT dev
)
