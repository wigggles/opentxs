// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_EXCLUSIVE_TPP
#define OPENTXS_EXCLUSIVE_TPP

#include "Internal.hpp"

#include "opentxs/Exclusive.hpp"

#include "opentxs/core/Log.hpp"

namespace opentxs
{
template <class C>
Exclusive<C>::Exclusive() noexcept
    : p_{nullptr}
    , lock_{nullptr}
    , save_{[](C*, eLock&, bool) -> void {}}
    , success_{true}
{
}

template <class C>
Exclusive<C>::Exclusive(C* in, std::shared_mutex& lock, Save save) noexcept
    : p_{in}
    , lock_{new eLock(lock)}
    , save_{save}
    , success_{true}
{
    OT_ASSERT(lock_)
}

template <class C>
Exclusive<C>::Exclusive(Exclusive&& rhs) noexcept
    : p_{rhs.p_}
    , lock_{rhs.lock_.release()}
    , save_{rhs.save_}
    , success_{rhs.success_.load()}
{
    rhs.p_ = nullptr;
    rhs.save_ = [](C*, eLock&, bool) -> void {};
    rhs.success_.store(true);
}

template <class C>
Exclusive<C>& Exclusive<C>::operator=(Exclusive&& rhs) noexcept
{
    p_ = rhs.p_;
    rhs.p_ = nullptr;
    lock_.reset(rhs.lock_.release());
    save_ = rhs.save_;
    rhs.save_ = [](C*, eLock&, bool) -> void {};
    success_.store(rhs.success_.load());
    rhs.success_.store(true);

    return *this;
}

template <class C>
Exclusive<C>::operator bool() const
{
    return nullptr != p_;
}

template <class C>
Exclusive<C>::operator const C&() const
{
    return get();
}

template <class C>
Exclusive<C>::operator C&()
{
    return get();
}

template <class C>
bool Exclusive<C>::Abort()
{
    if (nullptr == p_) { return false; }

    success_.store(false);

    return Release();
}

template <class C>
const C& Exclusive<C>::get() const
{
    OT_ASSERT(nullptr != p_)

    return *p_;
}

template <class C>
C& Exclusive<C>::get()
{
    OT_ASSERT(nullptr != p_)

    return *p_;
}

template <class C>
bool Exclusive<C>::Release()
{
    if (nullptr == p_) { return false; }

    OT_ASSERT(lock_)

    save_(p_, *lock_, success_);
    p_ = nullptr;
    save_ = [](C*, eLock&, bool) -> void {};
    success_.store(true);

    return true;
}

template <class C>
Exclusive<C>::~Exclusive()
{
    Release();
}
}  // namespace opentxs
#endif  // OPENTXS_EXCLUSIVE_TPP
