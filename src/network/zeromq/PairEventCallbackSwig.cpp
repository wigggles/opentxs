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

#include "PairEventCallbackSwig.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/MultipartMessage.hpp"
#include "opentxs/network/zeromq/PairEventCallbackSwig.hpp"

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::PairEventCallbackSwig::"

namespace opentxs::network::zeromq
{
OTZMQPairEventCallback PairEventCallback::Factory(
    opentxs::PairEventCallbackSwig* callback)
{
    return OTZMQPairEventCallback(
        new implementation::PairEventCallbackSwig(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PairEventCallbackSwig::PairEventCallbackSwig(
    opentxs::PairEventCallbackSwig* callback)
    : callback_(callback)
{
    if (nullptr == callback_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid callback pointer"
              << std::endl;

        OT_FAIL;
    }
}

PairEventCallbackSwig* PairEventCallbackSwig::clone() const
{
    return new PairEventCallbackSwig(callback_);
}

void PairEventCallbackSwig::Process(
    const zeromq::MultipartMessage& message) const
{
    OT_ASSERT(nullptr != callback_)
    OT_ASSERT(1 == message.Body().size());

    const auto event = proto::TextToProto<proto::PairEvent>(message.Body_at(0));

    switch (event.type()) {
        case proto::PAIREVENT_RENAME: {
            callback_->ProcessRename(event.issuer());
        } break;
        case proto::PAIREVENT_STORESECRET: {
            callback_->ProcessStoreSecret(event.issuer());
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Unknown event type"
                  << std::endl;
        }
    }
}

PairEventCallbackSwig::~PairEventCallbackSwig() {}
}  // namespace opentxs::network::zeromq::implementation
