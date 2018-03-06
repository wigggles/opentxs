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

#include "opentxs/core/Log.hpp"
#include "opentxs/Types.hpp"

#include <mutex>
#include <shared_mutex>

namespace opentxs
{
class Lockable
{
public:
    Lockable() = default;

    virtual ~Lockable() = default;

protected:
    mutable std::mutex lock_;
    mutable std::shared_mutex shared_lock_;

    bool verify_lock(const Lock& lock) const
    {
        return verify_lock(lock, lock_);
    }

    bool verify_lock(const sLock& lock) const
    {
        return verify_lock(lock, shared_lock_);
    }

    bool verify_lock(const eLock& lock) const
    {
        return verify_lock(lock, shared_lock_);
    }

    template <typename L, typename M>
    bool verify_lock(const L& lock, const M& mutex) const
    {
        if (lock.mutex() != &mutex) {
            otErr << "opentxs::Lockable::" << __FUNCTION__
                  << ": lock is on incorrect mutex" << std::endl;

            return false;
        }

        if (false == lock.owns_lock()) {
            otErr << "opentxs::Lockable::" << __FUNCTION__
                  << ": lock is unlocked" << std::endl;

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
