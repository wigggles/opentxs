// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PairEventCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class PairEventCallback final : virtual public zeromq::PairEventCallback
{
public:
    void Process(zeromq::Message& message) const final;

    ~PairEventCallback() final;

private:
    friend zeromq::PairEventCallback;

    const zeromq::PairEventCallback::ReceiveCallback callback_;

    PairEventCallback* clone() const final;

    PairEventCallback(zeromq::PairEventCallback::ReceiveCallback callback);
    PairEventCallback() = delete;
    PairEventCallback(const PairEventCallback&) = delete;
    PairEventCallback(PairEventCallback&&) = delete;
    PairEventCallback& operator=(const PairEventCallback&) = delete;
    PairEventCallback& operator=(PairEventCallback&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
