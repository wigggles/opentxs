# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources DepositPayment.cpp Operation.cpp PaymentTasks.cpp
                StateMachine.cpp
)
set(cxx-install-headers "")
set(cxx-headers
    ${cxx-install-headers}
    ${opentxs_SOURCE_DIR}/src/internal/otx/client/Client.hpp
    DepositPayment.hpp
    Operation.hpp
    PaymentTasks.hpp
    StateMachine.hpp
    StateMachine.tpp
)

add_library(opentxs-otx-client OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-otx-client PRIVATE opentxs::messages)
target_include_directories(
  opentxs-otx-client SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-otx-client PROPERTY POSITION_INDEPENDENT_CODE 1)
