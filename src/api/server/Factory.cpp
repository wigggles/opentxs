// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "api/server/Factory.hpp"  // IWYU pragma: associated

#include "api/Factory.hpp"
#include "internal/api/server/Server.hpp"
#include "opentxs/core/cron/OTCron.hpp"

//#define OT_METHOD "opentxs::api::server::implementation::Factory::"

namespace opentxs
{
api::internal::Factory* Factory::FactoryAPIServer(
    const api::server::internal::Manager& api)
{
    return new api::server::implementation::Factory(api);
}
}  // namespace opentxs

namespace opentxs::api::server::implementation
{
Factory::Factory(const api::server::internal::Manager& server)
    : api::implementation::Factory(server)
    , server_(server)
{
}

std::unique_ptr<OTCron> Factory::Cron() const
{
    auto output = std::unique_ptr<opentxs::OTCron>{};
    output.reset(new opentxs::OTCron(server_));

    return output;
}
}  // namespace opentxs::api::server::implementation
