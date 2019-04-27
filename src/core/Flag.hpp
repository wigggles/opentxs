// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::implementation
{
class Flag : virtual public opentxs::Flag, Lockable
{
public:
    operator bool() const override;

    bool Off() override;
    bool On() override;
    bool Set(const bool value) override;
    bool Toggle() override;

    Flag(const bool state);

    ~Flag() = default;

private:
    std::atomic<bool> flag_;

    Flag* clone() const override;

    Flag() = delete;
    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag& operator=(Flag&&) = delete;
};
}  // namespace opentxs::implementation
