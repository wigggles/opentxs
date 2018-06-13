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

#ifndef OPENTXS_SHARED_HPP
#define OPENTXS_SHARED_HPP

#include "opentxs/Types.hpp"

#include <memory>
#include <shared_mutex>

#ifdef SWIG
%ignore opentxs::Shared::Shared(Shared&&);
%rename(assign) opentxs::Shared::operator=(const Shared&);
%rename(move) opentxs::Shared::operator=(Shared&&);
%rename(valid) opentxs::Shared::operator bool();
#endif

namespace opentxs
{
template <class C>
class Shared
{
public:
    operator bool() const;
#ifndef SWIG
    operator const C&() const;
#endif

    const C& get() const;

    bool Release();

    Shared(const C* in, std::shared_mutex& lock) noexcept;
    Shared() noexcept;
    Shared(const Shared&) noexcept;
    Shared(Shared&&) noexcept;
    Shared& operator=(const Shared&) noexcept;
    Shared& operator=(Shared&&) noexcept;

    ~Shared();

private:
    const C* p_{nullptr};
    std::unique_ptr<sLock> lock_{nullptr};

};  // class Shared
}  // namespace opentxs
#endif  // OPENTXS_SHARED_HPP
