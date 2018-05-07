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

#ifndef OPENTXS_CORE_IMPLEMENTATION_FLAG_HPP
#define OPENTXS_CORE_IMPLEMENTATION_FLAG_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"

#include <atomic>

namespace opentxs::implementation
{
class Flag : virtual public opentxs::Flag, Lockable
{
public:
    operator bool() const override;

    void Off() override;
    void On() override;
    bool Set(const bool value) override;
    bool Toggle() override;

    Flag(const bool state);

    ~Flag() = default;

private:
    std::atomic<bool> flag_;

    Flag* clone() const override;

    Flag() = delete;
    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag& operator=(Flag&&) = delete;
};
}  // namespace opentxs::implementation
#endif  // OPENTXS_CORE_IMPLEMENTATION_FLAG_HPP
