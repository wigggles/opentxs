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
#include "opentxs/OT.hpp"

#include <cstdint>
#include <memory>

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
    virtual const std::string GetCommandPort() const = 0;
    virtual const std::string GetDefaultBindIP() const = 0;
    virtual const std::string GetEEP() const = 0;
    virtual const std::string GetExternalIP() const = 0;
    virtual const std::string GetListenCommand() const = 0;
    virtual const std::string GetListenNotify() const = 0;
    virtual const std::string GetOnion() const = 0;
#if OT_CASH
    virtual std::shared_ptr<Mint> GetPrivateMint(
        const Identifier& unitid,
        std::uint32_t series) const = 0;
    virtual std::shared_ptr<const Mint> GetPublicMint(
        const Identifier& unitID) const = 0;
#endif  // OT_CASH
    virtual const std::string GetUserName() const = 0;
    virtual const std::string GetUserTerms() const = 0;
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
