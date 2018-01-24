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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_API_IMPLEMENTATION_SERVER_HPP
#define OPENTXS_API_IMPLEMENTATION_SERVER_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Server.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace opentxs
{
class Identifier;

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace server
{
class MessageProcessor;
class Server;
}  // namespace server

namespace api
{
class Crypto;
class Settings;
class Wallet;

namespace storage
{
class Storage;
}  // namespace storage

namespace implementation
{
class Native;

class Server : virtual public opentxs::api::Server
{
public:
    const std::string GetCommandPort() const override;
    const std::string GetDefaultBindIP() const override;
    const std::string GetEEP() const override;
    const std::string GetExternalIP() const override;
    const std::string GetListenCommand() const override;
    const std::string GetListenNotify() const override;
    const std::string GetOnion() const override;
#if OT_CASH
    std::shared_ptr<Mint> GetPrivateMint(
        const Identifier& unitID,
        std::uint32_t series) const override;
    std::shared_ptr<const Mint> GetPublicMint(
        const Identifier& unitID) const override;
#endif  // OT_CASH
    const std::string GetUserName() const override;
    const std::string GetUserTerms() const override;
    const Identifier& ID() const override;
    const Identifier& NymID() const override;
#if OT_CASH
    void ScanMints() const override;
    void UpdateMint(const Identifier& unitID) const override;
#endif  // OT_CASH

    ~Server();

private:
    friend class implementation::Native;

#if OT_CASH
    typedef std::map<std::string, std::shared_ptr<Mint>> MintSeries;
#endif  // OT_CASH

    const ArgList& args_;
    api::Settings& config_;
    api::Crypto& crypto_;
    api::storage::Storage& storage_;
    api::Wallet& wallet_;
    std::atomic<bool>& shutdown_;
    const opentxs::network::zeromq::Context& zmq_context_;
    std::unique_ptr<server::Server> server_p_;
    server::Server& server_;
    std::unique_ptr<server::MessageProcessor> message_processor_p_;
    server::MessageProcessor& message_processor_;
#if OT_CASH
    std::unique_ptr<std::thread> mint_thread_;
    mutable std::mutex mint_lock_;
    mutable std::mutex mint_update_lock_;
    mutable std::mutex mint_scan_lock_;
    mutable std::map<std::string, MintSeries> mints_;
    mutable std::deque<std::string> mints_to_check_;
#endif  // OT_CASH

#if OT_CASH
    void generate_mint(
        const std::string& serverID,
        const std::string& unitID,
        const std::uint32_t series) const;
#endif  // OT_CASH
    const std::string get_arg(const std::string& argName) const;
#if OT_CASH
    std::int32_t last_generated_series(
        const std::string& serverID,
        const std::string& unitID) const;
    std::shared_ptr<Mint> load_private_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const;
    std::shared_ptr<Mint> load_public_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const;
    void mint() const;
#endif  // OT_CASH
    bool verify_lock(const Lock& lock, const std::mutex& mutex) const;
#if OT_CASH
    std::shared_ptr<Mint> verify_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID,
        std::shared_ptr<Mint>& mint) const;
    bool verify_mint_directory(const std::string& serverID) const;
#endif  // OT_CASH

    void Cleanup();
    void Init();
    void Start();

    Server(
        const ArgList& args,
        api::Crypto& crypto,
        api::Settings& config,
        api::storage::Storage& storage,
        api::Wallet& wallet,
        std::atomic<bool>& shutdown,
        const opentxs::network::zeromq::Context& context);
    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace implementation
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_IMPLEMENTATION_SERVER_HPP
