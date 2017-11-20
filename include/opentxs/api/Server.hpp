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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_API_SERVER_HPP
#define OPENTXS_API_SERVER_HPP

#include "opentxs/Version.hpp"

#include <cstdint>
#include <memory>

#define OT_SERVER_OPTION_BACKUP "backup"
#define OT_SERVER_OPTION_BINDIP "bindip"
#define OT_SERVER_OPTION_COMMANDPORT "commandport"
#define OT_SERVER_OPTION_EEP "eep"
#define OT_SERVER_OPTION_GC "gc"
#define OT_SERVER_OPTION_EXTERNALIP "externalip"
#define OT_SERVER_OPTION_LISTENCOMMAND "listencommand"
#define OT_SERVER_OPTION_LISTENNOTIFY "listennotify"
#define OT_SERVER_OPTION_NAME "name"
#define OT_SERVER_OPTION_NOTIFICATIONPORT "notificationport"
#define OT_SERVER_OPTION_ONION "onion"
#define OT_SERVER_OPTION_STORAGE "storage"
#define OT_SERVER_OPTION_TERMS "terms"
#define OT_SERVER_OPTION_VERSION "version"
#define OT_SERVER_OPTION_INIT "only-init"

namespace opentxs
{
class Identifier;
#if OT_CASH
class Mint;
#endif  // OT_CASH

namespace api
{

class Server
{
public:
#if OT_CASH
    virtual std::shared_ptr<Mint> GetPrivateMint(
        const Identifier& unitid,
        std::uint32_t series) const = 0;
    virtual std::shared_ptr<const Mint> GetPublicMint(
        const Identifier& unitID) const = 0;
#endif  // OT_CASH
    virtual const Identifier& ID() const = 0;
    virtual const Identifier& NymID() const = 0;
#if OT_CASH
    virtual void ScanMints() const = 0;
    virtual void UpdateMint(const Identifier& unitID) const = 0;
#endif  // OT_CASH

    virtual ~Server() = default;

protected:
    Server() = default;

private:
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_SERVER_HPP
