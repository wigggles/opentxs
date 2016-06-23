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

#include "opentxs/server/ClientConnection.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"

namespace opentxs
{

// At certain times, when the server has verified that a Nym REALLY is who
// he says he is, he sets the public key onto the connection object for
// that nym.  That way, if the connection object ever needs to encrypt something
// being sent to the client, he has access to the public key.
void ClientConnection::SetPublicKey(const String& publicKey, OTAsymmetricKey::KeyType keyType)
{
    if (nullptr != publicKey_)
    {
        delete publicKey_;
        publicKey_ = nullptr;
    }
    // ----------------------
    OT_ASSERT(nullptr != publicKey_);
    publicKey_ = OTAsymmetricKey::KeyFactory(keyType, publicKey);
}

void ClientConnection::SetPublicKey(const OTAsymmetricKey& publicKey)
{
    if (nullptr != publicKey_)
    {
        delete publicKey_;
        publicKey_ = nullptr;
    }
    // ----------------------
    OT_ASSERT(nullptr == publicKey_);

    String strNymsPublicKey;
    publicKey.GetPublicKey(strNymsPublicKey);

    publicKey_ = OTAsymmetricKey::KeyFactory(publicKey.keyType(), strNymsPublicKey);
}

// This function, you pass in a message and it returns true or false to let
// you know whether the message was successfully sealed into envelope.
// (Based on the public key into cached in the ClientConnection...)
// This is for XmlRpc / HTTP mode.
bool ClientConnection::SealMessageForRecipient(Message& msg,
                                               OTEnvelope& envelope)
{
    OT_ASSERT(nullptr != publicKey_);

    if (!(publicKey_->IsEmpty()) && publicKey_->IsPublic()) {
        // Save the ready-to-go message into a string.
        String strEnvelopeContents(msg);

        // Seal the string up into an encrypted Envelope.
        if (strEnvelopeContents.Exists())
            return envelope.Seal(*publicKey_, strEnvelopeContents);
    }
    else
        Log::Error("ClientConnection::SealMessageForRecipient: "
                   "Unable to seal message, possibly a missing public key. \n");
    return false;
}

ClientConnection::ClientConnection()
{
}

ClientConnection::~ClientConnection()
{
    if (nullptr != publicKey_) {
        delete publicKey_;
        publicKey_ = nullptr;
    }
}

} // namespace opentxs
