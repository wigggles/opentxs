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

#include "opentxs/stdafx.hpp"

#include "opentxs/network/zeromq/Message.hpp"

#include "PairEventCallback.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PairEventCallback>;

//#define OT_METHOD
//"opentxs::network::zeromq::implementation::PairEventCallback::"

namespace opentxs::network::zeromq
{
OTZMQPairEventCallback PairEventCallback::Factory(
    zeromq::PairEventCallback::ReceiveCallback callback)
{
    return OTZMQPairEventCallback(
        new implementation::PairEventCallback(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PairEventCallback::PairEventCallback(
    zeromq::PairEventCallback::ReceiveCallback callback)
    : callback_(callback)
{
}

PairEventCallback* PairEventCallback::clone() const
{
    return new PairEventCallback(callback_);
}

void PairEventCallback::Process(const zeromq::Message& message) const
{
    const auto event = proto::TextToProto<proto::PairEvent>(message);
    callback_(event);
}

PairEventCallback::~PairEventCallback() {}
}  // namespace opentxs::network::zeromq::implementation
