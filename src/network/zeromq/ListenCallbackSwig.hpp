// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/ListenCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class ListenCallbackSwig final : virtual public zeromq::ListenCallback
{
public:
    void Process(zeromq::Message& message) const final;

    ~ListenCallbackSwig() final;

private:
    friend zeromq::ListenCallback;

    opentxs::ListenCallbackSwig* callback_;

    ListenCallbackSwig* clone() const final;

    ListenCallbackSwig(opentxs::ListenCallbackSwig* callback);
    ListenCallbackSwig() = delete;
    ListenCallbackSwig(const ListenCallbackSwig&) = delete;
    ListenCallbackSwig(ListenCallbackSwig&&) = delete;
    ListenCallbackSwig& operator=(const ListenCallbackSwig&) = delete;
    ListenCallbackSwig& operator=(ListenCallbackSwig&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
