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

#include "opentxs/core/app/Wallet.hpp"

#include <thread>

#include "opentxs/core/app/App.hpp"

namespace opentxs
{
bool Wallet::RemoveServer(const Identifier& id)
{
    std::string server(String(id).Get());
    std::unique_lock<std::mutex> mapLock(server_map_lock_);
    auto deleted = server_map_.erase(server);

    if (0 != deleted) {
        return App::Me().DB().RemoveServer(server);
    }

    return false;
}

ConstServerContract Wallet::Server(
    const Identifier& id,
    const std::chrono::milliseconds& timeout)
{
    const String strID(id);
    const std::string server = strID.Get();
    std::unique_lock<std::mutex> mapLock(server_map_lock_);
    bool inMap = (server_map_.find(server) != server_map_.end());
    bool valid = false;

    if (!inMap) {
        std::shared_ptr<proto::ServerContract> serialized;

        std::string alias;
        bool loaded = App::Me().DB().Load(server, serialized, alias, true);

        if (loaded) {
            server_map_[server].reset(ServerContract::Factory(*serialized));

            if (server_map_[server]) {
                valid = true; // Factory() performs validation
                server_map_[server]->SetAlias(alias);
            }
        } else {
            App::Me().DHT().GetServerContract(server,
                [&](const ServerContract& contract)
                    -> void { Server(contract.PublicContract()); }
            );

            if (timeout > std::chrono::milliseconds(0)) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = std::chrono::milliseconds(100);

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (server_map_.find(server) != server_map_.end());
                    mapLock.unlock();

                    if (found) { break; }
                }

                return Server(id); // timeout of zero prevents infinite recursion
            }
        }
    } else {
        if (server_map_[server]) {
            valid = server_map_[server]->Validate();
        }
    }

    if (valid) {
        return server_map_[server];
    }

    return nullptr;
}

ConstServerContract Wallet::Server(
    const proto::ServerContract& contract)
{
    auto server = contract.nymid();
    std::unique_ptr<ServerContract>
        candidate(ServerContract::Factory(contract));

    if (candidate) {
        if (candidate->Validate()) {
            candidate->Save();
            std::unique_lock<std::mutex> mapLock(server_map_lock_);
            server_map_[server].reset(candidate.release());
            mapLock.unlock();
        }
    }

    return Server(server);
}

ConstServerContract Wallet::Server(std::unique_ptr<ServerContract>& contract)
{
    Identifier id;

    if (contract) {
        if (contract->Validate()) {
            id = contract->ID();
            std::unique_lock<std::mutex> mapLock(server_map_lock_);
            server_map_[String(id).Get()].reset(contract.release());
            mapLock.unlock();
        }
    }

    return Server(id);
}

Storage::ObjectList Wallet::ServerList()
{
    return App::Me().DB().ServerList();
}

bool Wallet::SetServerAlias(const Identifier& id, const std::string alias)
{
    return App::Me().DB().SetServerAlias(String(id).Get(), alias);
}

} // namespace opentxs
