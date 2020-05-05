# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources Basket.cpp BasketContract.cpp BasketItem.cpp)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/basket/Basket.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/basket/BasketContract.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/basket/BasketItem.hpp"
)
set(cxx-headers ${cxx-install-headers} BasketContract.hpp)

add_library(opentxs-core-contract-basket OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-core-contract-basket SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(opentxs-core-contract-basket PRIVATE opentxs::messages)
set_property(
  TARGET opentxs-core-contract-basket
  PROPERTY POSITION_INDEPENDENT_CODE 1
)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/core/contract/basket
  COMPONENT dev
)
