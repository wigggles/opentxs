// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::implementation
{
class ServerConnection : virtual public opentxs::network::ServerConnection,
                         Lockable
{
public:
    bool ChangeAddressType(const proto::AddressType type) override;
    bool ClearProxy() override;
    bool EnableProxy() override;
    NetworkReplyRaw Send(const std::string& message) override;
    NetworkReplyString Send(const String& message) override;
    NetworkReplyMessage Send(const Message& message) override;
    bool Status() const override;

    ~ServerConnection();

private:
    friend opentxs::network::ServerConnection;

    const api::network::ZMQ& zmq_;
    const api::Wallet& wallet_;
    const zeromq::PublishSocket& updates_;
    const std::string server_id_{};
    proto::AddressType address_type_{proto::ADDRESSTYPE_ERROR};
    std::shared_ptr<const ServerContract> remote_contract_{nullptr};
    std::unique_ptr<std::thread> thread_{nullptr};
    OTZMQRequestSocket socket_;
    std::atomic<std::time_t> last_activity_{0};
    OTFlag socket_ready_;
    OTFlag status_;
    OTFlag use_proxy_;

    ServerConnection* clone() const override { return nullptr; }
    std::string endpoint() const;
    void publish() const;
    void set_curve(const Lock& lock, zeromq::RequestSocket& socket) const;
    void set_proxy(const Lock& lock, zeromq::RequestSocket& socket) const;
    void set_timeouts(const Lock& lock, zeromq::RequestSocket& socket) const;
    OTZMQRequestSocket socket(const Lock& lock) const;

    void activity_timer();
    zeromq::RequestSocket& get_socket(const Lock& lock);
    void reset_socket(const Lock& lock);
    void reset_timer();

    ServerConnection(
        const api::network::ZMQ& zmq,
        const api::Wallet& wallet,
        const zeromq::PublishSocket& updates,
        const std::shared_ptr<const ServerContract>& contract);
    ServerConnection() = delete;
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection& operator=(ServerConnection&&) = delete;
};
}  // namespace opentxs::network::implementation
