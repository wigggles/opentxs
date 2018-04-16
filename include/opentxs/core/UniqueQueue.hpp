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

#ifndef OPENTXS_CORE_UNIQUEQUEUE_HPP
#define OPENTXS_CORE_UNIQUEQUEUE_HPP

#include "opentxs/Version.hpp"

#include <deque>
#include <mutex>
#include <set>

namespace opentxs
{
template <class T>
class UniqueQueue
{
public:
    void CancelByValue(const T& in) const
    {
        Lock lock(lock_);

        if (0 == reverse_map_.count(in)) {

            return;
        }

        const auto& key = reverse_map_.at(in);

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            if (*i == in) {
                queue_.erase(i);
                break;
            }
        }

        map_.erase(key);
        reverse_map_.erase(in);
    }

    void CancelByKey(const OTIdentifier& key) const
    {
        Lock lock(lock_);

        if (0 == map_.count(key)) {

            return;
        }

        const auto& value = map_[key];

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            if (*i == value) {
                queue_.erase(i);
                break;
            }
        }

        reverse_map_.erase(value);
        map_.erase(key);
    }

    std::map<T, OTIdentifier> Copy() const
    {
        Lock lock(lock_);

        return reverse_map_;
    }

    bool Push(const OTIdentifier& key, const T& in) const
    {
        Lock lock(lock_);

        if (0 == reverse_map_.count(in)) {
            queue_.push_front(in);
            map_[key] = in;
            reverse_map_.emplace(in, OTIdentifier(key));

            return true;
        }

        return false;
    }

    bool Pop(OTIdentifier& key, T& out) const
    {
        Lock lock(lock_);

        if (0 == queue_.size()) {

            return false;
        }

        out = queue_.back();
        key->SetString(reverse_map_.at(out)->str());
        queue_.pop_back();
        reverse_map_.erase(out);
        map_.erase(key);

        return true;
    }

private:
    mutable std::mutex lock_;
    mutable std::deque<T> queue_;
    mutable std::map<OTIdentifier, T> map_;
    mutable std::map<T, OTIdentifier> reverse_map_;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_UNIQUEQUEUE_HPP
