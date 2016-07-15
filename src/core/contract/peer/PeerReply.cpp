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

#include "opentxs/core/contract/peer/PeerReply.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"

namespace opentxs
{
PeerReply::PeerReply(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
      : ot_super(nym)
      , initiator_(serialized.initiator())
      , recipient_(serialized.recipient())
      , cookie_(serialized.cookie())
      , type_(serialized.type())
{
    id_ = Identifier(serialized.id());
    signatures_.push_front(
        SerializedSignature(
            std::make_shared<proto::Signature>(serialized.signature())));
    version_ = serialized.version();
}

PeerReply::PeerReply(
    const ConstNym& nym,
    const Identifier& initiator,
    const proto::PeerRequestType& type,
    const Identifier& request)
        : ot_super(nym)
        , initiator_(initiator)
        , recipient_(nym->ID())
        , cookie_(request)
        , type_(type)
{
    version_ = 1;
}

proto::PeerReply PeerReply::Contract() const
{
    auto contract = SigVersion();
    *(contract.mutable_signature()) = *(signatures_.front());

    return contract;
}

PeerReply* PeerReply::Create(
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& requestID,
    const std::string& terms)
{
    auto peerRequest = App::Me().Contract().PeerRequest(
            nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST);

    if (!peerRequest) {
        otErr << __FUNCTION__ << ": failed to load request."
              << std::endl;

        return nullptr;
    }

    std::unique_ptr<PeerReply> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentReply(
                nym, Identifier(peerRequest->initiator()), requestID, terms));
            break;
        }
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentReply(
                nym, Identifier(peerRequest->initiator()), requestID, terms));
            break;
        }
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate reply."
              << std::endl;

        return nullptr;
    }

    if (FinalizeContract(*contract)) {
        return contract.release();
    } else {
        otErr << __FUNCTION__ << ": failed to finalize contract." << std::endl;

        return nullptr;
    }
}

PeerReply* PeerReply::Factory(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
{
    if (!proto::Check(serialized, 0, 0xFFFFFFFF)) { return nullptr; }

    std::unique_ptr<PeerReply> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentReply(nym, serialized));
            break;
        }
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentReply(nym, serialized));
            break;
        }
        default : {
            otErr << __FUNCTION__ << ": invalid reply type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate reply."
              << std::endl;

        return nullptr;
    }
    if (!contract->Validate()) {
        otErr << __FUNCTION__ << ": invalid reply." << std::endl;

        return nullptr;
    }

    return contract.release();
}

bool PeerReply::FinalizeContract(PeerReply& contract)
{
    if (!contract.CalculateID()) {
        otErr << __FUNCTION__ << ": failed to calculate ID." << std::endl;

        return false;
    }

    if (contract.nym_) {
        auto serialized = contract.SigVersion();
        std::shared_ptr<proto::Signature> sig =
            std::make_shared<proto::Signature>();

        if (contract.nym_->Sign(serialized, *sig)) {
            contract.signatures_.push_front(sig);
        }
    }

    return contract.Validate();
}

Identifier PeerReply::GetID() const
{
    return GetID(IDVersion());
}

Identifier PeerReply::GetID(const proto::PeerReply& contract)
{
    Identifier id;
    id.CalculateDigest(proto::ProtoAsData(contract));
    return id;
}

proto::PeerReply PeerReply::IDVersion() const
{
    proto::PeerReply contract;

    contract.set_version(version_);
    contract.clear_id();         // reinforcing that this field must be blank.
    contract.set_initiator(String(initiator_).Get());
    contract.set_recipient(String(recipient_).Get());
    contract.set_type(type_);
    contract.set_cookie(String(cookie_).Get());
    contract.clear_signature();  // reinforcing that this field must be blank.

    return contract;
}

std::string PeerReply::Name() const
{
    return String(id_).Get();
}

OTData PeerReply::Serialize() const
{

    return proto::ProtoAsData(Contract());
}

proto::PeerReply PeerReply::SigVersion() const
{
    auto contract = IDVersion();
    contract.set_id(String(ID()).Get());

    return contract;
}

bool PeerReply::Validate() const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        otErr << __FUNCTION__ << ": invalid nym." << std::endl;
    }

    bool validSyntax = proto::Check(Contract(), 0, 0xFFFFFFFF);

    if (!validSyntax) {
        otErr << __FUNCTION__ << ": invalid syntax." << std::endl;
    }

    bool validSig = false;

    if (nym_) {
        validSig = nym_->Verify(
            proto::ProtoAsData(SigVersion()),
            *(signatures_.front()));
    }

    if (!validSig) {
        otErr << __FUNCTION__ << ": invalid signature." << std::endl;
    }

    return (validNym && validSyntax && validSig);
}
} // namespace opentxs
