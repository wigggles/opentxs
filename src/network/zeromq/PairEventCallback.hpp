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

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACK_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACK_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PairEventCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class PairEventCallback : virtual public zeromq::PairEventCallback
{
public:
    void Process(const zeromq::MultipartMessage& message) const override;

    ~PairEventCallback();

private:
    friend zeromq::PairEventCallback;

    const zeromq::PairEventCallback::ReceiveCallback callback_;

    PairEventCallback* clone() const override;

    PairEventCallback(zeromq::PairEventCallback::ReceiveCallback callback);
    PairEventCallback() = delete;
    PairEventCallback(const PairEventCallback&) = delete;
    PairEventCallback(PairEventCallback&&) = delete;
    PairEventCallback& operator=(const PairEventCallback&) = delete;
    PairEventCallback& operator=(PairEventCallback&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACK_IMPLEMENTATION_HPP
