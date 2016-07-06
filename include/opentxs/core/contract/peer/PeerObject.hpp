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

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP

#include "opentxs/core/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{

class OTASCIIArmor;

class PeerObject
{
private:
    std::unique_ptr<std::string> message_;
    proto::PeerObjectType type_;
    std::uint32_t version_;

    PeerObject(const proto::PeerObject serialized);
    PeerObject(const std::string& message);

public:
    static std::unique_ptr<PeerObject> Create(const std::string& message);
    static std::unique_ptr<PeerObject> Factory(
        const proto::PeerObject& serialized);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& recipientNym,
        const ConstNym& senderNym,
        const OTASCIIArmor& encrypted);

    std::unique_ptr<std::string>& Message() { return message_; }
    proto::PeerObject Serialize() const;
    proto::PeerObjectType Type() const { return type_; }
    bool Validate() const;

    ~PeerObject() = default;
};
} // namespace opentxs

#endif // OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
