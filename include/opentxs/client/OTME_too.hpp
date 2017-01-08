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

#ifndef OPENTXS_CLIENT_OTME_TOO_HPP
#define OPENTXS_CLIENT_OTME_TOO_HPP

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace opentxs
{

class Api;
class MadeEasy;
class OTAPI_Exec;

class OTME_too
{
private:
    friend class Api;

    std::recursive_mutex& api_lock_;
    OTAPI_Exec& exec_;
    MadeEasy& made_easy_;
    mutable std::atomic<bool> refreshing_;
    mutable std::unique_ptr<std::thread> refresh_thread_;

    void refresh_thread() const;
    void Shutdown() const;

    OTME_too(
        std::recursive_mutex& lock,
        OTAPI_Exec& exec,
        MadeEasy& madeEasy);
    OTME_too() = delete;
    OTME_too(const OTME_too&) = delete;
    OTME_too(const OTME_too&&) = delete;
    OTME_too& operator=(const OTME_too&) = delete;
    OTME_too& operator=(const OTME_too&&) = delete;

public:
    void Refresh(const std::string& wallet = "") const;

    ~OTME_too();
};
} // namespace opentxs

#endif // OPENTXS_CLIENT_OTME_TOO_HPP
