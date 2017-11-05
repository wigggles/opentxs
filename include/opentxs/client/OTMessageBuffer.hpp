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

#ifndef OPENTXS_CLIENT_OTMESSAGEBUFFER_HPP
#define OPENTXS_CLIENT_OTMESSAGEBUFFER_HPP

#include "opentxs/Version.hpp"

#include <stdint.h>
#include <list>
#include <memory>

namespace opentxs
{

class Message;
class String;

class OTMessageBuffer
{
public:
    OTMessageBuffer() {}

    EXPORT ~OTMessageBuffer();

    EXPORT void Clear();
    // message must be heap-allocated. Takes ownership.
    EXPORT void Push(std::shared_ptr<Message> message);
    // Caller IS responsible to delete.
    EXPORT std::shared_ptr<Message> Pop(
        const int64_t& requestNum,
        const String& notaryID,
        const String& nymId);

private:
    OTMessageBuffer(const OTMessageBuffer&);
    OTMessageBuffer& operator=(const OTMessageBuffer&);

private:
    typedef std::list<std::shared_ptr<Message>> Messages;

private:
    Messages messages_;
};

}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OTMESSAGEBUFFER_HPP
