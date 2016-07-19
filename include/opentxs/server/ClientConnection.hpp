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

#ifndef OPENTXS_SERVER_CLIENTCONNECTION_HPP
#define OPENTXS_SERVER_CLIENTCONNECTION_HPP

#include <memory>

namespace opentxs
{

class Message;
class OTAsymmetricKey;
class OTEnvelope;

class ClientConnection
{
public:
    ClientConnection() = default;
    ~ClientConnection() = default;

    void SetPublicKey(const OTAsymmetricKey& publicKey);

    bool SealMessageForRecipient(Message& msg, OTEnvelope& envelope);

private:
    std::unique_ptr<OTAsymmetricKey> publicKey_;
};

} // namespace opentxs

#endif // OPENTXS_SERVER_CLIENTCONNECTION_HPP
