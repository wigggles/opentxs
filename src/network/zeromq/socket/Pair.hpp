// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::network::zeromq::socket::implementation
{
class Pair final : public Bidirectional<zeromq::socket::Pair>
{
public:
    const std::string& Endpoint() const noexcept final;
    bool Start(const std::string& endpoint) const noexcept final
    {
        return false;
    }

    ~Pair();

private:
    friend opentxs::Factory;
    friend opentxs::network::zeromq::socket::Pair;

    const ListenCallback& callback_;
    const std::string endpoint_;

    Pair* clone() const noexcept final;
    bool have_callback() const noexcept final;
    void process_incoming(const Lock& lock, Message& message) noexcept final;

    void init() noexcept final;

    Pair(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const std::string& endpoint,
        const Socket::Direction direction,
        const bool startThread) noexcept;
    Pair(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const bool startThread = true) noexcept;
    Pair(
        const zeromq::ListenCallback& callback,
        const zeromq::socket::Pair& peer,
        const bool startThread = true) noexcept;
    Pair(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const std::string& endpoint) noexcept;
    Pair() = delete;
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
