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

#ifndef OPENTXS_CORE_LOCKABLE_HPP
#define OPENTXS_CORE_LOCKABLE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <mutex>

namespace opentxs
{
class Lockable
{
public:
    Lockable() = default;

    virtual ~Lockable() = default;

protected:
    mutable std::mutex lock_;

    bool verify_lock(const Lock& lock) const
    {
        return verify_lock(lock, lock_);
    }

    bool verify_lock(const Lock& lock, const std::mutex& mutex) const
    {
        if (lock.mutex() != &mutex) {

            return false;
        }

        if (false == lock.owns_lock()) {

            return false;
        }

        return true;
    }

private:
    Lockable(const Lockable&) = delete;
    Lockable(Lockable&&) = delete;
    Lockable& operator=(const Lockable&) = delete;
    Lockable& operator=(Lockable&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_LOCKABLE_HPP
