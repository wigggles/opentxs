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

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/contract/Signable.hpp"

#include <string>

namespace opentxs
{

class PeerReply : public Signable
{
private:
    typedef Signable ot_super;

    Identifier initiator_;
    Identifier recipient_;
    Identifier cookie_;
    proto::PeerRequestType type_{proto::PEERREQUEST_ERROR};

    static Identifier GetID(const proto::PeerReply& contract);
    static bool FinalizeContract(PeerReply& contract);
    static std::unique_ptr<PeerReply> Finish(
        std::unique_ptr<PeerReply>& contract);
    static std::shared_ptr<proto::PeerRequest> LoadRequest(
        const ConstNym& nym,
        const Identifier& requestID);

    Identifier GetID() const override;
    proto::PeerReply SigVersion() const;

    PeerReply() = delete;

protected:

    virtual proto::PeerReply IDVersion() const;

    PeerReply(
        const ConstNym& nym,
        const proto::PeerReply& serialized);
    PeerReply(
        const ConstNym& nym,
        const Identifier& initiator,
        const proto::PeerRequestType& type,
        const Identifier& request);

public:
    static std::unique_ptr<PeerReply> Create(
        const ConstNym& nym,
        const proto::PeerRequestType& type,
        const Identifier& request,
        const std::string& terms);
    static std::unique_ptr<PeerReply> Create(
        const ConstNym& nym,
        const Identifier& request,
        const bool& ack);
    static std::unique_ptr<PeerReply> Create(
        const ConstNym& nym,
        const Identifier& request,
        const bool& ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key);
    static std::unique_ptr<PeerReply> Factory(
        const ConstNym& nym,
        const proto::PeerReply& serialized);

    std::string Alias() const override { return Name(); }
    proto::PeerReply Contract() const;
    std::string Name() const override;
    OTData Serialize() const override;
    const proto::PeerRequestType& Type() const { return type_; }
    void SetAlias(__attribute__((unused)) std::string alias) override {}
    bool UpdateSignature() override;
    bool Validate() const override;
    bool VerifySignature(const proto::Signature& signature) const override;

    ~PeerReply() = default;
};
} // namespace opentxs

#endif // OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP
