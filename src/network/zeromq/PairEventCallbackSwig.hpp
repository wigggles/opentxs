// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PairEventCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class PairEventCallbackSwig : virtual public zeromq::PairEventCallback
{
public:
    void Process(const zeromq::Message& message) const override;

    ~PairEventCallbackSwig();

private:
    friend zeromq::PairEventCallback;

    opentxs::PairEventCallbackSwig* callback_;

    PairEventCallbackSwig* clone() const override;

    PairEventCallbackSwig(opentxs::PairEventCallbackSwig* callback);
    PairEventCallbackSwig() = delete;
    PairEventCallbackSwig(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig(PairEventCallbackSwig&&) = delete;
    PairEventCallbackSwig& operator=(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig& operator=(PairEventCallbackSwig&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_IMPLEMENTATION_HPP
