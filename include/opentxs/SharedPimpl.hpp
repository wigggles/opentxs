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

#ifndef OPENTXS_SHARED_PIMPL_HPP
#define OPENTXS_SHARED_PIMPL_HPP

#include <cassert>
#include <memory>

#ifdef SWIG
%ignore opentxs::SharedPimpl::SharedPimpl(const std::shared_ptr<const C>&);
%ignore opentxs::SharedPimpl::SharedPimpl(SharedPimpl&&);
%ignore opentxs::SharedPimpl::operator const C&();
%rename(assign) opentxs::SharedPimpl::operator=(const SharedPimpl&);
%rename(move) opentxs::SharedPimpl::operator=(SharedPimpl&&);
#endif

namespace opentxs
{
template <class C>
class SharedPimpl
{
public:
    explicit SharedPimpl(const std::shared_ptr<const C>& in) noexcept
        : pimpl_(in)
    {
        assert(pimpl_);
    }
    SharedPimpl(const SharedPimpl& rhs) noexcept = default;
    SharedPimpl(SharedPimpl&& rhs) noexcept = default;
    SharedPimpl& operator=(const SharedPimpl& rhs) noexcept = default;
    SharedPimpl& operator=(SharedPimpl&& rhs) noexcept = default;

    operator const C&() const noexcept { return *pimpl_; }

    const C* operator->() const { return pimpl_.get(); }

    const C& get() const noexcept { return *pimpl_; }

    ~SharedPimpl() = default;

#ifdef SWIG_VERSION
    SharedPimpl() = default;
#endif

private:
    std::shared_ptr<const C> pimpl_{nullptr};

#ifndef SWIG_VERSION
    SharedPimpl() = delete;
#endif
};  // class SharedPimpl
}  // namespace opentxs

#endif  // OPENTXS_SHARED_PIMPL_HPP
