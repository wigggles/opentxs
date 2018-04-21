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

#ifndef OPENTXS_NETWORK_SERVERCONNECTION_HPP
#define OPENTXS_NETWORK_SERVERCONNECTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <string>

namespace opentxs
{
namespace network
{
class ServerConnection
{
public:
    EXPORT static OTServerConnection Factory(
        const api::network::ZMQ& zmq,
        const std::string& serverID);

    EXPORT virtual bool ChangeAddressType(const proto::AddressType type) = 0;
    EXPORT virtual bool ClearProxy() = 0;
    EXPORT virtual bool EnableProxy() = 0;
    EXPORT virtual NetworkReplyRaw Send(const std::string& message) = 0;
    EXPORT virtual NetworkReplyString Send(const String& message) = 0;
    EXPORT virtual NetworkReplyMessage Send(const Message& message) = 0;
    EXPORT virtual bool Status() const = 0;

    virtual ~ServerConnection() = default;

protected:
    ServerConnection() = default;

private:
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection& operator=(ServerConnection&&) = delete;
};
}  // namespace opentxs::network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_SERVERCONNECTION_HPP
