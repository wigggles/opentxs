// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PairEventCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class PairEventCallbackSwig final : virtual public zeromq::PairEventCallback
{
public:
    void Process(zeromq::Message& message) const final;

    ~PairEventCallbackSwig() final;

private:
    friend zeromq::PairEventCallback;

    opentxs::PairEventCallbackSwig* callback_;

    PairEventCallbackSwig* clone() const final;

    PairEventCallbackSwig(opentxs::PairEventCallbackSwig* callback);
    PairEventCallbackSwig() = delete;
    PairEventCallbackSwig(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig(PairEventCallbackSwig&&) = delete;
    PairEventCallbackSwig& operator=(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig& operator=(PairEventCallbackSwig&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
