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
ConstNym Wallet::Nym(
    const Identifier& id,
    const std::chrono::milliseconds& timeout)
{
    const std::string nym = String(id).Get();
    std::unique_lock<std::mutex> mapLock(nym_map_lock_);
    bool inMap = (nym_map_.find(nym) != nym_map_.end());
    bool valid = false;

    if (!inMap) {
        std::shared_ptr<proto::CredentialIndex> serialized;

        std::string alias;
        bool loaded = App::Me().DB().Load(nym, serialized, alias, true);

        if (loaded) {
            nym_map_[nym].reset(new class Nym(id));
            if (nym_map_[nym]) {
                if (nym_map_[nym]->LoadCredentialIndex(*serialized)) {
                    valid = nym_map_[nym]->VerifyPseudonym();
                    nym_map_[nym]->SetAlias(alias);
                }
            }
        } else {
            App::Me().DHT().GetPublicNym(nym);

            if (timeout > std::chrono::milliseconds(0)) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = std::chrono::milliseconds(100);

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (nym_map_.find(nym) != nym_map_.end());
                    mapLock.unlock();

                    if (found) { break; }
                }

                return Nym(id); // timeout of zero prevents infinite recursion
            }
        }
    } else {
        if (nym_map_[nym]) {
            valid = nym_map_[nym]->VerifyPseudonym();
        }
    }

    if (valid) {
        return nym_map_[nym];
    }

    return nullptr;
}

ConstNym Wallet::Nym(
    const proto::CredentialIndex& publicNym)
{
    std::string nym = publicNym.nymid();
    std::unique_ptr<class Nym>
        candidate(new class Nym(Identifier(nym)));

    if (candidate) {
        if (candidate->VerifyPseudonym()) {
            candidate->SaveCredentialIDs();
            SetNymAlias(nym, candidate->GetNymName().Get());
            std::unique_lock<std::mutex> mapLock(nym_map_lock_);
            nym_map_[nym].reset(candidate.release());
            mapLock.unlock();
        }
    }

    return Nym(nym);
}

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

bool Wallet::RemoveUnitDefinition(const Identifier& id)
{
    std::string unit(String(id).Get());
    std::unique_lock<std::mutex> mapLock(unit_map_lock_);
    auto deleted = unit_map_.erase(unit);

    if (0 != deleted) {
        return App::Me().DB().RemoveUnitDefinition(unit);
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
            App::Me().DHT().GetServerContract(server);

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
    auto server = contract.id();
    std::unique_ptr<ServerContract>
        candidate(ServerContract::Factory(contract));

    if (candidate) {
        if (candidate->Validate()) {
            candidate->Save();
            SetServerAlias(server, candidate->Name().Get());
            std::unique_lock<std::mutex> mapLock(server_map_lock_);
            server_map_[server].reset(candidate.release());
            mapLock.unlock();
        }
    }

    return Server(server);
}

Storage::ObjectList Wallet::ServerList()
{
    return App::Me().DB().ServerList();
}

bool Wallet::SetNymAlias(const Identifier& id, const std::string alias)
{
    return App::Me().DB().SetNymAlias(String(id).Get(), alias);
}

bool Wallet::SetServerAlias(const Identifier& id, const std::string alias)
{
    return App::Me().DB().SetServerAlias(String(id).Get(), alias);
}

bool Wallet::SetUnitDefinitionAlias(
    const Identifier& id,
    const std::string alias)
{
    return App::Me().DB().SetUnitDefinitionAlias(String(id).Get(), alias);
}

Storage::ObjectList Wallet::UnitDefinitionList()
{
    return App::Me().DB().UnitDefinitionList();
}

ConstUnitDefinition Wallet::UnitDefinition(
    const Identifier& id,
    const std::chrono::milliseconds& timeout)
{
    const String strID(id);
    const std::string unit = strID.Get();
    std::unique_lock<std::mutex> mapLock(unit_map_lock_);
    bool inMap = (unit_map_.find(unit) != unit_map_.end());
    bool valid = false;

    if (!inMap) {
        std::shared_ptr<proto::UnitDefinition> serialized;

        std::string alias;
        bool loaded = App::Me().DB().Load(unit, serialized, alias, true);

        if (loaded) {
            unit_map_[unit].reset(UnitDefinition::Factory(*serialized));

            if (unit_map_[unit]) {
                valid = true; // Factory() performs validation
                unit_map_[unit]->SetAlias(alias);
            }
        } else {
            App::Me().DHT().GetUnitDefinition(unit);

            if (timeout > std::chrono::milliseconds(0)) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = std::chrono::milliseconds(100);

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (unit_map_.find(unit) != unit_map_.end());
                    mapLock.unlock();

                    if (found) { break; }
                }

                return UnitDefinition(id); // timeout of zero prevents
                                           // infinite recursion
            }
        }
    } else {
        if (unit_map_[unit]) {
            valid = unit_map_[unit]->Validate();
        }
    }

    if (valid) {
        return unit_map_[unit];
    }

    return nullptr;
}

ConstUnitDefinition Wallet::UnitDefinition(
    const proto::UnitDefinition& contract)
{
    auto unit = contract.id();
    std::unique_ptr<class UnitDefinition>
        candidate(UnitDefinition::Factory(contract));

    if (candidate) {
        if (candidate->Validate()) {
            candidate->Save();
            SetUnitDefinitionAlias(unit, candidate->Name().Get());
            std::unique_lock<std::mutex> mapLock(unit_map_lock_);
            unit_map_[unit].reset(candidate.release());
            mapLock.unlock();
        }
    }

    return UnitDefinition(unit);
}

} // namespace opentxs
