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

#ifndef OPENTXS_UTIL_SIGNALS_HPP
#define OPENTXS_UTIL_SIGNALS_HPP

#include <atomic>
#include <memory>
#include <thread>

namespace opentxs
{

class Signals
{
public:
    static void Block();

    Signals(std::atomic<bool>& shutdown);

    virtual ~Signals();

protected:
    void shutdown();

private:
    std::atomic<bool>& shutdown_;
    std::unique_ptr<std::thread> thread_{nullptr};

    void handle();
    virtual void process(const int signal);

    Signals() = delete;
    Signals(const Signals&) = delete;
    Signals(Signals&&) = delete;
    Signals& operator=(const Signals&) = delete;
    Signals& operator=(Signals&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_UTIL_SIGNALS_HPP
