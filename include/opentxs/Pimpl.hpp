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

#ifndef OPENTXS_PIMPL_HPP
#define OPENTXS_PIMPL_HPP

#include <cassert>
#include <memory>

namespace opentxs
{
template <class C>
class Pimpl
{
public:
    explicit Pimpl(C* in) noexcept
        : pimpl_(in)
    {
        assert(pimpl_);
    }

    Pimpl(const Pimpl& rhs) noexcept
        : pimpl_(rhs.pimpl_->clone())
    {
        assert(pimpl_);
    }

    Pimpl(const C& rhs) noexcept
        : pimpl_(rhs.clone())
    {
        assert(pimpl_);
    }

    Pimpl(Pimpl&& rhs) noexcept
        : pimpl_(std::move(rhs.pimpl_))
    {
        assert(pimpl_);
    }

    Pimpl& operator=(const Pimpl& rhs) noexcept
    {
        pimpl_.reset(rhs.pimpl_->clone());
        assert(pimpl_);

        return *this;
    }

    Pimpl& operator=(Pimpl&& rhs) noexcept
    {
        pimpl_ = std::move(rhs.pimpl_);
        assert(pimpl_);

        return *this;
    }

    Pimpl& operator=(const C& rhs) noexcept
    {
        pimpl_.reset(rhs.clone());
        assert(pimpl_);

        return *this;
    }

    Pimpl& operator+=(const C& rhs)
    {
        *pimpl_ += rhs;

        return *this;
    }

    bool operator==(const Pimpl& rhs) const { return *pimpl_ == *rhs.pimpl_; }
    bool operator==(const C& rhs) const { return *pimpl_ == rhs; }

    bool operator!=(const C& rhs) const { return *pimpl_ != rhs; }
    bool operator<(const Pimpl& rhs) const { return *pimpl_ < *rhs.pimpl_; }
    bool operator<(const C& rhs) const { return *pimpl_ < rhs; }
    bool operator<=(const C& rhs) const { return *pimpl_ <= rhs; }
    bool operator>(const C& rhs) const { return *pimpl_ > rhs; }
    bool operator>=(const C& rhs) const { return *pimpl_ >= rhs; }

    operator C&() noexcept { return *pimpl_; }
    operator const C&() const noexcept { return *pimpl_; }

    C* operator->() { return pimpl_.get(); }
    const C* operator->() const { return pimpl_.get(); }

    C& get() noexcept { return *pimpl_; }
    const C& get() const noexcept { return *pimpl_; }

    ~Pimpl() = default;

#ifdef SWIG_VERSION
    Pimpl() = default;
#endif

private:
    std::unique_ptr<C> pimpl_{nullptr};

#ifndef SWIG_VERSION
    Pimpl() = delete;
#endif
};  // class Pimpl
}  // namespace opentxs

#endif  // OPENTXS_PIMPL_HPP
