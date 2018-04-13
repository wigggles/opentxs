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

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIREVENT_LISTENER_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIREVENT_LISTENER_HPP

#include "opentxs/Internal.hpp"

#include "SubscribeSocket.hpp"

namespace opentxs::network::zeromq::implementation
{
class PairEventListener : public SubscribeSocket
{
public:
    ~PairEventListener();

private:
    friend zeromq::implementation::Context;
    typedef SubscribeSocket ot_super;

    PairEventListener* clone() const override;

    PairEventListener(
        const zeromq::Context& context,
        const zeromq::PairEventCallback& callback);
    PairEventListener() = delete;
    PairEventListener(const PairEventListener&) = delete;
    PairEventListener(PairEventListener&&) = delete;
    PairEventListener& operator=(const PairEventListener&) = delete;
    PairEventListener& operator=(PairEventListener&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_PAIREVENT_LISTENER_HPP
