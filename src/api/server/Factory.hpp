// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::server::implementation
{
class Factory final : public opentxs::api::implementation::Factory
{
public:
    std::unique_ptr<OTCron> Cron() const final;

    ~Factory() final = default;

private:
    friend opentxs::Factory;

    const api::server::internal::Manager& server_;

    Factory(const api::server::internal::Manager& server);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::server::implementation
