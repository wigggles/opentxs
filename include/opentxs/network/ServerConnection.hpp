/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_NETWORK_SERVERCONNECTION_HPP
#define OPENTXS_NETWORK_SERVERCONNECTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace opentxs
{
namespace api
{
namespace network
{
namespace implementation
{
class ZMQ;
}  // namespace implementation
}  // namespace network
}  // namespace api

class ServerConnection
{
public:
    bool ChangeAddressType(const proto::AddressType type);
    bool ClearProxy();
    bool EnableProxy();
    NetworkReplyRaw Send(const std::string& message);
    NetworkReplyString Send(const String& message);
    NetworkReplyMessage Send(const Message& message);
    bool Status() const;

    ~ServerConnection();

private:
    friend class api::network::implementation::ZMQ;

    std::atomic<bool>& shutdown_;
    std::atomic<std::chrono::seconds>& keep_alive_;
    const api::network::ZMQ& zmq_;
    const api::Settings& config_;
    const network::zeromq::Context& context_;

    std::shared_ptr<const ServerContract> remote_contract_{nullptr};
    const std::string remote_endpoint_{""};
    std::shared_ptr<network::zeromq::RequestSocket> request_socket_;
    std::unique_ptr<std::mutex> lock_{nullptr};
    std::unique_ptr<std::thread> thread_{nullptr};
    std::atomic<std::time_t> last_activity_{0};
    std::atomic<bool> status_{false};
    std::atomic<bool> use_proxy_{true};

    std::string GetRemoteEndpoint(
        const std::string& server,
        std::shared_ptr<const ServerContract>& contract) const;

    void Init(const std::string& proxy);
    bool Receive(std::string& reply);
    void ResetSocket();
    void ResetTimer();
    void SetCurve();
    void SetProxy(const std::string& proxy);
    void SetTimeouts();
    void Thread();

    ServerConnection(
        const std::string& server,
        const std::string& proxy,
        std::atomic<bool>& shutdown,
        std::atomic<std::chrono::seconds>& keepAlive,
        const api::network::ZMQ& zmq,
        const api::Settings& config);
    ServerConnection() = delete;
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection& operator=(ServerConnection&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_NETWORK_SERVERCONNECTION_HPP
