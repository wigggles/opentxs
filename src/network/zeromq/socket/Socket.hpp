// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

#include <functional>
#include <map>
#include <mutex>
#include <set>

#define CURVE_KEY_BYTES 32
#define CURVE_KEY_Z85_BYTES 40
#define SHUTDOWN                                                               \
    {                                                                          \
        running_->Off();                                                       \
        Lock lock(lock_);                                                      \
        shutdown(lock);                                                        \
    }

namespace opentxs::network::zeromq::socket::implementation
{
class Socket : virtual public zeromq::socket::Socket, public Lockable
{
public:
    using SocketCallback = std::function<bool(const Lock& lock)>;

    static bool send_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message) noexcept;
    static std::string random_inproc_endpoint() noexcept;
    static bool receive_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message) noexcept;

    SocketType Type() const noexcept final;

    operator void*() const noexcept final;

    virtual bool apply_socket(SocketCallback&& cb) const noexcept;
    bool Close() const noexcept override;
    const zeromq::Context& Context() const noexcept final { return context_; }
    bool SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const noexcept final;
    bool Start(const std::string& endpoint) const noexcept override;

    Socket& get() { return *this; }

    ~Socket() override = default;

protected:
    const zeromq::Context& context_;
    const Socket::Direction direction_;
    mutable void* socket_{nullptr};
    mutable std::atomic<int> linger_{0};
    mutable std::atomic<int> send_timeout_{-1};
    mutable std::atomic<int> receive_timeout_{-1};
    mutable std::mutex endpoint_lock_;
    mutable std::set<std::string> endpoints_;
    mutable OTFlag running_;

    void add_endpoint(const std::string& endpoint) const noexcept;
    bool apply_timeouts(const Lock& lock) const noexcept;
    bool bind(const Lock& lock, const std::string& endpoint) const noexcept;
    bool connect(const Lock& lock, const std::string& endpoint) const noexcept;
    bool receive_message(const Lock& lock, zeromq::Message& message) const
        noexcept;
    bool send_message(const Lock& lock, zeromq::Message& message) const
        noexcept;
    bool set_socks_proxy(const std::string& proxy) const noexcept;
    bool start(const Lock& lock, const std::string& endpoint) const noexcept;

    virtual void init() noexcept {}
    virtual void shutdown(const Lock& lock) noexcept;

    explicit Socket(
        const zeromq::Context& context,
        const SocketType type,
        const Socket::Direction direction) noexcept;

private:
    static const std::map<SocketType, int> types_;

    const SocketType type_{SocketType::Error};

    Socket() = delete;
    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
