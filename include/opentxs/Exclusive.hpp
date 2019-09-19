// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
template <typename C>
class Exclusive
{
public:
    using Callback = std::function<void(const C&)>;
    using Container = std::unique_ptr<C>;
    using Save = std::function<void(Container&, eLock&, bool)>;

    operator bool() const;
#ifndef SWIG
    operator const C&() const;
    const C& get() const;

    operator C&();
#endif

    bool Abort();
    C& get();
    bool Release();

    Exclusive(
        Container* in,
        std::shared_mutex& lock,
        Save save,
        const Callback callback = nullptr) noexcept;
    Exclusive() noexcept;
    Exclusive(const Exclusive&) = delete;
    Exclusive(Exclusive&&) noexcept;
    Exclusive& operator=(const Exclusive&) noexcept = delete;
    Exclusive& operator=(Exclusive&&) noexcept;

    ~Exclusive();

private:
    Container* p_{nullptr};
    std::unique_ptr<eLock> lock_{nullptr};
    Save save_{[](Container&, eLock&, bool) -> void {}};
    std::atomic<bool> success_{true};
    Callback callback_{nullptr};
};  // class Exclusive
}  // namespace opentxs
#endif
