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

#include "opentxs/client/OTME_too.hpp"

#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/app/Wallet.hpp"

#include <list>
#include <map>

namespace opentxs
{

OTME_too::OTME_too(
    std::recursive_mutex& lock,
    OTAPI_Exec& exec,
    MadeEasy& madeEasy)
        : api_lock_(lock)
        , exec_(exec)
        , made_easy_(madeEasy)
{
    refreshing_.store(false);
}

void OTME_too::refresh_thread() const
{
    typedef std::map<std::string, std::list<std::string>> nymAccountMap;
    typedef std::map<std::string, nymAccountMap> serverNymMap;

    serverNymMap accounts;

    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    const auto serverList = App::Me().Contract().ServerList();
    const auto nymCount = exec_.GetNymCount();
    const auto accountCount = exec_.GetAccountCount();

    for (const auto server : serverList) {
        const auto& serverID = server.first;

        for (std::int32_t n = 0; n < nymCount; n++ ) {
            const auto nymID = exec_.GetNym_ID(n);

            if (exec_.IsNym_RegisteredAtServer(nymID, serverID)) {
                accounts[serverID].insert({nymID, {}});
            }
        }
    }

    for (std::int32_t n = 0; n < accountCount; n++ ) {
        const auto accountID = exec_.GetAccountWallet_ID(n);
        const auto serverID = exec_.GetAccountWallet_NotaryID(accountID);
        const auto nymID = exec_.GetAccountWallet_NymID(accountID);

        auto& server = accounts[serverID];
        auto& nym = server[nymID];
        nym.push_back(accountID);
    }

    apiLock.unlock();
    std::this_thread::yield();

    for (const auto server : accounts) {
        const auto& serverID = server.first;

        for (const auto nym : server.second) {
            const auto& nymID = nym.first;
            bool notUsed = false;
            made_easy_.retrieve_nym(serverID, nymID, notUsed, true);
            std::this_thread::yield();

            for (auto& account : nym.second) {
                made_easy_.retrieve_account(serverID, nymID, account, true);
                std::this_thread::yield();
            }
        }
    }

    refreshing_.store(false);
}

void OTME_too::Refresh(const std::string&) const
{
    if (!refreshing_.load()) {
        refreshing_.store(true);

        if (refresh_thread_) {
            refresh_thread_->join();
            refresh_thread_.reset();
        }

        refresh_thread_.reset(new std::thread(&OTME_too::refresh_thread, this));
    }
}

void OTME_too::Shutdown() const
{
    while (refreshing_.load()) {
        Log::Sleep(std::chrono::milliseconds(250));
    }

    if (refresh_thread_) {
        refresh_thread_->join();
        refresh_thread_.reset();
    }
}

OTME_too::~OTME_too()
{
    Shutdown();
}
} // namespace opentxs
