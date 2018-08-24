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
    NetworkReplyMessage Send(
        const ServerContext& context,
        const Message& message) override;
    bool Status() const override;

    ~ServerConnection();

private:
    friend opentxs::network::ServerConnection;

    const api::network::ZMQ& zmq_;
    const api::Core& api_;
    const zeromq::PublishSocket& updates_;
    const std::string server_id_{};
    proto::AddressType address_type_{proto::ADDRESSTYPE_ERROR};
    std::shared_ptr<const ServerContract> remote_contract_{nullptr};
    std::thread thread_;
    OTZMQListenCallback callback_;
    OTZMQDealerSocket socket_;
    OTZMQPushSocket notification_socket_;
    std::atomic<std::time_t> last_activity_{0};
    OTFlag socket_ready_;
    OTFlag status_;
    OTFlag use_proxy_;
    std::mutex incoming_lock_;
    std::map<RequestNumber, std::unique_ptr<Message>> incoming_;
    mutable std::mutex registation_lock_;
    std::map<OTIdentifier, bool> registered_for_push_;

    static std::pair<bool, proto::ServerReply> check_for_protobuf(
        const zeromq::Frame& frame);

    ServerConnection* clone() const override { return nullptr; }
    std::string endpoint() const;
    std::string form_endpoint(
        proto::AddressType type,
        std::string hostname,
        std::uint32_t port) const;
    std::chrono::time_point<std::chrono::system_clock> get_timeout();
    void publish() const;
    void set_curve(const Lock& lock, zeromq::DealerSocket& socket) const;
    void set_proxy(const Lock& lock, zeromq::DealerSocket& socket) const;
    void set_timeouts(const Lock& lock, zeromq::DealerSocket& socket) const;
    OTZMQDealerSocket socket(const Lock& lock) const;

    void activity_timer();
    zeromq::DealerSocket& get_socket(const Lock& lock);
    void process_incoming(const zeromq::Message& in);
    void process_incoming(const proto::ServerReply& in);
    void register_for_push(const ServerContext& context);
    void reset_socket(const Lock& lock);
    void reset_timer();

    ServerConnection(
        const api::Core& api,
        const api::network::ZMQ& zmq,
        const zeromq::PublishSocket& updates,
        const std::shared_ptr<const ServerContract>& contract);
    ServerConnection() = delete;
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection& operator=(ServerConnection&&) = delete;
};
}  // namespace opentxs::network::implementation
