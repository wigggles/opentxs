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

#ifndef OPENTXS_CLIENT_OTMESSAGEOUTBUFFER_HPP
#define OPENTXS_CLIENT_OTMESSAGEOUTBUFFER_HPP

#include "opentxs/core/String.hpp"
#include <map>

namespace opentxs
{

class Message;
class Nym;
class OTTransaction;

typedef std::multimap<int64_t, Message*> mapOfMessages;

// OUTOING MESSAGES (from me--client--sent to server.)
//
// The purpose of this class is to cache client requests (being sent to the
// server)
// so that they can later be queried (using the request number) by the developer
// using the OTAPI, so that if transaction numbers need to be clawed back from
// failed
// messages, etc, they are available.
//
// The OT client side also can use this as a mechanism to help separate
// old-and-dealt-with
// messages, by explicitly removing messages from this queue once they are dealt
// with.
// This way the developer can automatically assume that any reply is old if it
// carries
// a request number that cannot be found in this queue.
//
// This class is pretty generic and so may be used in other ways, where "map"
// functionality is required.
class OTMessageOutbuffer
{
public:
    EXPORT OTMessageOutbuffer();
    EXPORT ~OTMessageOutbuffer();

    EXPORT void Clear(const String* notaryID = nullptr,
                      const String* nymId = nullptr, Nym* nym = nullptr,
                      const bool* harvestingForRetry = nullptr);
    // Allocate theMsg on the heap (takes ownership.) Mapped by request num.
    // Note: AddSentMessage, if it finds a message already on the map with the
    // same request number, deletes the old one before adding the new one. In
    // the future may contemplate using multimap here instead (if completeness
    // becomes desired over uniqueness.)
    EXPORT void AddSentMessage(Message& message);
    // null == not found. caller NOT responsible to delete.
    EXPORT Message* GetSentMessage(const int64_t& requestNum,
                                   const String& notaryID, const String& nymId);
    // true == it was removed. false == it wasn't found.
    EXPORT bool RemoveSentMessage(const int64_t& requestNum,
                                  const String& notaryID, const String& nymId);
    // null == not found. caller NOT responsible to delete.
    EXPORT Message* GetSentMessage(const OTTransaction& transaction);
    // true == it was removed. false == it wasn't found.
    EXPORT bool RemoveSentMessage(const OTTransaction& transaction);

private:
    OTMessageOutbuffer(const OTMessageOutbuffer&);
    OTMessageOutbuffer& operator=(const OTMessageOutbuffer&);

private:
    mapOfMessages messagesMap_;
    String dataFolder_;
};

} // namespace opentxs

#endif // OPENTXS_CLIENT_OTMESSAGEOUTBUFFER_HPP
