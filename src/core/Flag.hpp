// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::implementation
{
class Flag final : virtual public opentxs::Flag
{
public:
    operator bool() const final { return flag_.load(); }

    bool Off() final { return Set(false); }
    bool On() final { return !Set(true); }
    bool Set(const bool value) final { return flag_.exchange(value); }
    bool Toggle() final;

    Flag(const bool state);

    ~Flag() final = default;

private:
    std::atomic<bool> flag_;

    Flag* clone() const final { return new Flag(flag_.load()); }

    Flag() = delete;
    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag& operator=(Flag&&) = delete;
};
}  // namespace opentxs::implementation
