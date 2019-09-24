// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SHARED_TPP
#define OPENTXS_SHARED_TPP

#include "Internal.hpp"

#include "opentxs/Shared.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
template <class C>
Shared<C>::Shared() noexcept
    : p_(nullptr)
    , lock_(nullptr)
{
}

template <class C>
Shared<C>::Shared(const C* in, std::shared_mutex& lock) noexcept
    : p_(in)
    , lock_(new sLock(lock))
{
    OT_ASSERT(lock_)
}

template <class C>
Shared<C>::Shared(const Shared& rhs) noexcept
    : p_(rhs.p_)
    , lock_(
          (nullptr != rhs.lock_->mutex()) ? new sLock(*rhs.lock_->mutex())
                                          : nullptr)
{
}

template <class C>
Shared<C>::Shared(Shared&& rhs) noexcept
    : p_(rhs.p_)
    , lock_(rhs.lock_.release())
{
    rhs.p_ = nullptr;
}

template <class C>
Shared<C>& Shared<C>::operator=(const Shared& rhs) noexcept
{
    p_ = rhs.p_;

    if (nullptr != rhs.lock_->mutex()) {
        lock_.reset(new sLock(*rhs.lock_->mutex()));
    } else {
        lock_.reset(nullptr);
    }

    return *this;
}

template <class C>
Shared<C>& Shared<C>::operator=(Shared&& rhs) noexcept
{
    p_ = rhs.p_;
    rhs.p_ = nullptr;
    lock_.reset(rhs.lock_.release());

    return *this;
}

template <class C>
Shared<C>::operator bool() const
{
    return nullptr != p_;
}

template <class C>
Shared<C>::operator const C&() const
{
    return get();
}

template <class C>
const C& Shared<C>::get() const
{
    OT_ASSERT(nullptr != p_)

    return *p_;
}

template <class C>
bool Shared<C>::Release()
{
    if (nullptr == p_) { return false; }

    p_ = nullptr;
    lock_.reset(nullptr);

    return true;
}

template <class C>
Shared<C>::~Shared()
{
    Release();
}
}  // namespace opentxs
#endif  // OPENTXS_SHARED_TPP
