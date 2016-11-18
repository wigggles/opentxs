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

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/app/Wallet.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

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

std::unique_ptr<PeerReply> PeerReply::Create(
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& requestID,
    const std::string& terms)
{
    auto peerRequest = LoadRequest(nym, requestID);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentReply(
                nym, Identifier(peerRequest->initiator()), requestID, terms));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentReply(
                nym, Identifier(peerRequest->initiator()), requestID, terms));
        } break;
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const ConstNym& nym,
    const Identifier& requestID,
    const bool& ack)
{
    auto peerRequest = LoadRequest(nym, requestID);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;
    const auto& type = peerRequest->type();

    switch (type) {
        case (proto::PEERREQUEST_PENDINGBAILMENT) :
        case (proto::PEERREQUEST_STORESECRET) : {
            contract.reset(new NoticeAcknowledgement(
                nym,
                Identifier(peerRequest->initiator()),
                requestID,
                type,
                ack));
        } break;
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const ConstNym& nym,
    const Identifier& request,
    const bool& ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key)
{
    auto peerRequest = LoadRequest(nym, request);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;
    const auto& type = peerRequest->type();

    switch (type) {
        case (proto::PEERREQUEST_CONNECTIONINFO) : {
            contract.reset(new ConnectionReply(
                nym,
                Identifier(peerRequest->initiator()),
                request,
                ack,
                url,
                login,
                password,
                key));
        } break;
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Factory(
    const ConstNym& nym,
    const proto::PeerReply& serialized)
{
    if (!proto::Check(serialized, 0, 0xFFFFFFFF)) { return nullptr; }

    std::unique_ptr<PeerReply> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentReply(nym, serialized));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentReply(nym, serialized));
        } break;
        case (proto::PEERREQUEST_PENDINGBAILMENT) : {
            contract.reset(new NoticeAcknowledgement(nym, serialized));
        } break;
        case (proto::PEERREQUEST_CONNECTIONINFO) : {
            contract.reset(new ConnectionReply(nym, serialized));
        } break;
        default : {
            otErr << __FUNCTION__ << ": invalid reply type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate reply." << std::endl;

        return nullptr;
    }

    if (!contract->Validate()) {
        otErr << __FUNCTION__ << ": invalid reply." << std::endl;

        return nullptr;
    }

    const Identifier purportedID(serialized.id());

    if (!contract->CalculateID()) {
        otErr << __FUNCTION__ << ": failed to calculate ID." << std::endl;

        return nullptr;
    }

    const auto& actualID = contract->ID();

    if (purportedID != actualID) {
        otErr << __FUNCTION__ << ": invalid ID." << std::endl;

        return nullptr;
    }

    return contract;
}

bool PeerReply::FinalizeContract(PeerReply& contract)
{
    if (!contract.CalculateID()) { return false; }

    if (!contract.UpdateSignature()) { return false; }

    return contract.Validate();
}

std::unique_ptr<PeerReply> PeerReply::Finish(
    std::unique_ptr<PeerReply>& contract)
{
    std::unique_ptr<PeerReply> output(contract.release());

    if (!output) {
        otErr << __FUNCTION__ << ": failed to instantiate reply."
              << std::endl;

        return nullptr;
    }

    if (FinalizeContract(*output)) {

        return output;
    } else {
        otErr << __FUNCTION__ << ": failed to finalize contract." << std::endl;

        return nullptr;
    }
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

std::shared_ptr<proto::PeerRequest> PeerReply::LoadRequest(
    const ConstNym& nym,
    const Identifier& requestID)
{
    std::shared_ptr<proto::PeerRequest> output;

    output = App::Me().Contract().PeerRequest(
            nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST);

    if (!output) {
        output = App::Me().Contract().PeerRequest(
            nym->ID(), requestID, StorageBox::PROCESSEDPEERREQUEST);

        if (output) {
            otErr << __FUNCTION__ << ": request has already been processed."
                  << std::endl;
        } else {
            otErr << __FUNCTION__ << ": request does not exist."
                << std::endl;
        }
    }

    return output;
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

bool PeerReply::UpdateSignature()
{
    if (!ot_super::UpdateSignature()) { return false; }

    bool success = false;

    signatures_.clear();
    auto serialized = SigVersion();
    auto& signature = *serialized.mutable_signature();
    signature.set_role(proto::SIGROLE_PEERREPLY);

    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        otErr << __FUNCTION__ << ": failed to create signature."
                << std::endl;
    }

    return success;
}

bool PeerReply::Validate() const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        otErr << __FUNCTION__ << ": invalid nym." << std::endl;
    }

    const bool validSyntax = proto::Check(Contract(), version_, version_);

    if (!validSyntax) {
        otErr << __FUNCTION__ << ": invalid syntax." << std::endl;
    }

    if (1 > signatures_.size()) {
        otErr << __FUNCTION__ << ": Missing signature." << std::endl;

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) {
        validSig = VerifySignature(*signature);
    }

    if (!validSig) {
        otErr << __FUNCTION__ << ": invalid signature." << std::endl;
    }

    return (validNym && validSyntax && validSig);
}

bool PeerReply::VerifySignature(const proto::Signature& signature) const
{
    if (!ot_super::VerifySignature(signature)) { return false; }

    auto serialized = SigVersion();
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);;
}
} // namespace opentxs
