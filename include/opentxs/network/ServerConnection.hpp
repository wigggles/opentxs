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

#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/network/ZMQ.hpp"

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace opentxs
{

class ServerContract;
class Settings;
class String;
class ZMQ;

class ServerConnection
{
private:
    friend class ZMQ;

    std::atomic<bool>& shutdown_;
    std::atomic<std::chrono::seconds>& keep_alive_;
    ZMQ& zmq_;
    Settings& config_;

    std::shared_ptr<const ServerContract> remote_contract_{nullptr};
    const std::string remote_endpoint_{""};
    zsock_t* request_socket_{nullptr};
    std::unique_ptr<std::mutex> lock_{nullptr};
    std::unique_ptr<std::thread> thread_{nullptr};
    std::atomic<std::time_t> last_activity_{0};
    std::atomic<bool> status_{false};

    std::string GetRemoteEndpoint(
        const std::string& server,
        std::shared_ptr<const ServerContract>& contract) const;

    void Init();
    bool Receive(std::string& reply);
    void ResetSocket();
    void ResetTimer();
    void SetRemoteKey();
    void SetProxy();
    void SetTimeouts();
    void Thread();

    ServerConnection() = delete;
    ServerConnection(
        const std::string& server,
        std::atomic<bool>& shutdown,
        std::atomic<std::chrono::seconds>& keepAlive,
        ZMQ& zmq,
        Settings& config);
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection& operator=(ServerConnection&&) = delete;

public:
    bool ChangeAddressType(const proto::AddressType type);
    NetworkReplyRaw Send(const std::string& message);
    NetworkReplyString Send(const String& message);
    NetworkReplyMessage Send(const Message& message);
    bool Status() const;

    ~ServerConnection();
};
}  // namespace opentxs

#endif  // OPENTXS_NETWORK_SERVERCONNECTION_HPP
