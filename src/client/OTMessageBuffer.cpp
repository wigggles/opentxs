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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/client/OTMessageBuffer.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Message.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/OTTransaction.hpp>

#include <memory>

namespace opentxs
{

void OTMessageBuffer::Push(std::shared_ptr<Message> theMessage)
{
    messages_.push_back(theMessage);
}

// **YOU** are responsible to delete the OTMessage object.
// once you receive the pointer that comes back from this function.
//
// This would be a good place to return a smart pointer.
//
// Update: added arguments for: NotaryID AND NymID AND request number
// NOTE: Any messages, when popping, which have the CORRECT notaryID
// and the CORRECT NymID, but the wrong Request number, will be discarded.
//
// (Why? Because the client using the OT API will have already treated
// that message as "dropped" by now, if it's already on to the next one,
// and the protocol is designed to move forward properly based specifically
// on this function returning the one EXPECTED... outgoing messages flush
// the incoming buffer anyway, so the client will have assumed the wrong
// reply was flushed by now anyway.)
//
// However, if the Notary ID and the Nym ID are wrong, this just means that
// some other code is still expecting that reply, and hasn't even popped yet!
// Therefore, we do NOT want to discard THOSE replies, but put them back if
// necessary -- only discarding the ones where the IDs match.
//
std::shared_ptr<Message> OTMessageBuffer::Pop(const int64_t& lRequestNum,
                                              const String& strNotaryID,
                                              const String& strNymID)
{
    std::shared_ptr<Message> pReturnValue;

    Messages temp_list;

    while (!messages_.empty()) {

        std::shared_ptr<Message> pMsg = messages_.front();

        messages_.pop_front();

        if (nullptr == pMsg) {
            otErr << "OTMessageBuffer::Pop: Error: List of incoming server "
                     "replies "
                     "is NOT empty, yet when Pop was called, pMsg was nullptr! "
                     "(Skipping.)\n";
            continue;
        }

        // Below this point, pMsg has been popped, and it's NOT nullptr, and it
        // will be lost if not tracked or returned.
        //
        if (!strNotaryID.Compare(pMsg->m_strNotaryID) ||
            !strNymID.Compare(pMsg->m_strNymID)) {
            // Save it, so we can push it back again after this loop.
            temp_list.push_front(pMsg);
            continue;
        }
        // Below this point, we KNOW that pMsg has the CORRECT NotaryID and
        // NymID.
        // (And that all others, though popped, were pushed to temp_list in
        // order.)

        const int64_t lMsgRequest = pMsg->m_strRequestNum.ToLong();

        // Now we only need to see if the request number matches...
        //
        if (lMsgRequest == lRequestNum) {
            pReturnValue = pMsg;
            break;
        }
        else // Server/Nym IDs match, BUT -- Wrong request num! (Discard
               // message and skip.)
        {
            otOut << "OTMessageBuffer::Pop: Warning: While looking for server ("
                  << strNotaryID << ") reply to request number " << lRequestNum
                  << " for Nym (" << strNymID
                  << "), "
                     "discovered (and discarded) an old server reply for "
                     "request number " << lMsgRequest << " "
                                                         "(A "
                  << pMsg->m_strCommand
                  << " command. The client should have flushed it by now "
                     "anyway, so it was probably slow on the network "
                     "and then assumed to have been dropped. It's okay--the "
                     "protocol is designed to handle these occurrences.)\n";

            continue;
        }

    } // while

    // Put the other messages back, in order...
    //
    while (!temp_list.empty()) {
        std::shared_ptr<Message> pMsg = temp_list.front();
        temp_list.pop_front();
        messages_.push_front(pMsg);
    }

    return pReturnValue;
}

OTMessageBuffer::~OTMessageBuffer()
{
    Clear();
}

void OTMessageBuffer::Clear()
{
    messages_.clear();
}

} // namespace opentxs
