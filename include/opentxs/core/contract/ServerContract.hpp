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

#ifndef OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP

#include <list>
#include <memory>
#include <tuple>

#include <czmq.h>
#include <opentxs-proto/verify/VerifyContracts.hpp>

#include "opentxs/core/Nym.hpp"
#include "opentxs/core/contract/Signable.hpp"

namespace opentxs
{

class OTData;
class String;

class ServerContract : public Signable
{
private:
    typedef std::pair<String, uint32_t> ListenParam;

    std::list<ListenParam> listen_params_;
    std::unique_ptr<Nym> nym_;
    OTData transport_key_;

    Identifier GetID() const override;
    proto::ServerContract IDVersion() const;
    proto::ServerContract SigVersion() const;

    ServerContract() = default;
    ServerContract(const proto::ServerContract& serialized);

public:
    static ServerContract* Create(Nym* nym,  // takes ownership
                                    const String& url,
                                    const uint32_t port,
                                    const String& terms,
                                    const String& name);
    static ServerContract* Factory(const proto::ServerContract& serialized);

    bool ConnectInfo(String& strHostname, uint32_t& nPort) const;
    const proto::ServerContract Contract() const;
    const String Name() const {
        if (nullptr != nym_) { return nym_->GetNymName(); } else { return "";}}
    const proto::ServerContract PublicContract() const;
    const Nym* PublicNym() const;
    bool Statistics(String& strContents) const;
    const unsigned char* PublicTransportKey() const;
    zcert_t* PrivateTransportKey() const;

    bool Save() const override;
    OTData Serialize() const override;
    bool Validate() const override;

    bool SetName(const String& name);

    EXPORT ~ServerContract() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP
