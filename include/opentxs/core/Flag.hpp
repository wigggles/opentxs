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

#ifndef OPENTXS_CORE_FLAG_HPP
#define OPENTXS_CORE_FLAG_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{
/** Wrapper for a std::atomic<bool> */
class Flag
{
public:
    static OTFlag Factory(const bool state);

    virtual operator bool() const = 0;

    /** Returns true if new state differs from previous state */
    virtual bool Off() = 0;
    /** Returns true if new state differs from previous state */
    virtual bool On() = 0;
    /** Returns previous state */
    virtual bool Set(const bool value) = 0;
    /** Returns previous state */
    virtual bool Toggle() = 0;

    virtual ~Flag() = default;

protected:
    Flag() = default;

private:
    friend OTFlag;

    virtual Flag* clone() const = 0;

    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag& operator=(Flag&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_FLAG_HPP
