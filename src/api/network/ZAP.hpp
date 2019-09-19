// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::network::implementation
{
class ZAP final : virtual public api::network::ZAP
{
public:
    bool RegisterDomain(const std::string& domain, const Callback& callback)
        const final;
    bool SetDefaultPolicy(const Policy policy) const final;

    ~ZAP() final = default;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& context_;
    OTZMQZAPCallback callback_;
    OTZMQZAPHandler zap_;

    ZAP(const opentxs::network::zeromq::Context& context);
    ZAP() = delete;
    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    ZAP& operator=(const ZAP&) = delete;
    ZAP& operator=(ZAP&&) = delete;
};
}  // namespace opentxs::api::network::implementation
