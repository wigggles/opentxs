// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::network::zeromq::zap::implementation
{
class Handler final : virtual zap::Handler,
                      zeromq::implementation::Socket,
                      zeromq::implementation::CurveServer,
                      zeromq::implementation::Receiver<zap::Request>
{
public:
    bool Start(const std::string& endpoint) const override { return false; }

    virtual ~Handler() = default;

private:
    friend zap::Handler;
    typedef Socket ot_super;

    const zap::Callback& callback_;

    Handler* clone() const override { return new Handler(context_, callback_); }
    bool have_callback() const override { return true; }

    void process_incoming(const Lock& lock, zap::Request& message) override;

    Handler(const zeromq::Context& context, const zap::Callback& callback);
    Handler() = delete;
    Handler(const Handler&) = delete;
    Handler(Handler&&) = delete;
    Handler& operator=(const Handler&) = delete;
    Handler& operator=(Handler&&) = delete;
};
}  // namespace opentxs::network::zeromq::zap::implementation
