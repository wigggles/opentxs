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

#ifndef OPENTXS_EXCLUSIVE_HPP
#define OPENTXS_EXCLUSIVE_HPP

#include "opentxs/Types.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <shared_mutex>

#ifdef SWIG
%ignore opentxs::Exclusive::Exclusive(Exclusive&&);
%rename(move) opentxs::Exclusive::operator=(Exclusive&&);
%rename(valid) opentxs::Exclusive::operator bool();
#endif

namespace opentxs
{
template <class C>
class Exclusive
{
public:
    using Save = std::function<void(C*, eLock&, bool)>;

    operator bool() const;
#ifndef SWIG
    operator const C&() const;
    const C& get() const;

    operator C&();
#endif

    bool Abort();
    C& get();
    bool Release();

    Exclusive(C* in, std::shared_mutex& lock, Save save) noexcept;
    Exclusive() noexcept;
    Exclusive(const Exclusive&) = delete;
    Exclusive(Exclusive&&) noexcept;
    Exclusive& operator=(const Exclusive&) noexcept = delete;
    Exclusive& operator=(Exclusive&&) noexcept;

    ~Exclusive();

private:
    C* p_{nullptr};
    std::unique_ptr<eLock> lock_{nullptr};
    Save save_{[](C*, eLock&, bool) -> void {}};
    std::atomic<bool> success_{true};
};  // class Exclusive
}  // namespace opentxs
#endif  // OPENTXS_EXCLUSIVE_HPP
