// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/PeerReply.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"

#define OT_METHOD "opentxs::PeerReply::"

namespace opentxs
{
PeerReply::PeerReply(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerReply& serialized)
    : ot_super(nym)
    , initiator_(Identifier::Factory(serialized.initiator()))
    , recipient_(Identifier::Factory(serialized.recipient()))
    , server_(Identifier::Factory(serialized.server()))
    , cookie_(Identifier::Factory(serialized.cookie()))
    , type_(serialized.type())
    , wallet_{wallet}
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
    version_ = serialized.version();
}

PeerReply::PeerReply(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::uint32_t version,
    const Identifier& initiator,
    const Identifier& server,
    const proto::PeerRequestType& type,
    const Identifier& request)
    : ot_super(nym, version)
    , initiator_(Identifier::Factory(initiator))
    , recipient_(Identifier::Factory(nym->ID()))
    , server_(Identifier::Factory(server))
    , cookie_(Identifier::Factory(request))
    , type_(type)
    , wallet_{wallet}
{
}

proto::PeerReply PeerReply::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);
    *(contract.mutable_signature()) = *(signatures_.front());

    return contract;
}

proto::PeerReply PeerReply::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& requestID,
    const Identifier& server,
    const std::string& terms)
{
    auto peerRequest = LoadRequest(wallet, nym, requestID);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT): {
            contract.reset(new BailmentReply(
                wallet,
                nym,
                Identifier::Factory(peerRequest->initiator()),
                requestID,
                server,
                terms));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT): {
            contract.reset(new OutBailmentReply(
                wallet,
                nym,
                Identifier::Factory(peerRequest->initiator()),
                requestID,
                server,
                terms));
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": invalid request type."
                  << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const Identifier& requestID,
    const Identifier& server,
    const bool& ack)
{
    auto peerRequest = LoadRequest(wallet, nym, requestID);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;
    const auto& type = peerRequest->type();

    switch (type) {
        case (proto::PEERREQUEST_PENDINGBAILMENT):
        case (proto::PEERREQUEST_STORESECRET): {
            contract.reset(new NoticeAcknowledgement(
                wallet,
                nym,
                Identifier::Factory(peerRequest->initiator()),
                requestID,
                server,
                type,
                ack));
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": invalid request type."
                  << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Create(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const Identifier& request,
    const Identifier& server,
    const bool& ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key)
{
    auto peerRequest = LoadRequest(wallet, nym, request);

    if (!peerRequest) { return nullptr; }

    std::unique_ptr<PeerReply> contract;
    const auto& type = peerRequest->type();

    switch (type) {
        case (proto::PEERREQUEST_CONNECTIONINFO): {
            contract.reset(new ConnectionReply(
                wallet,
                nym,
                Identifier::Factory(peerRequest->initiator()),
                request,
                server,
                ack,
                url,
                login,
                password,
                key));
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": invalid request type."
                  << std::endl;

            return nullptr;
        }
    }

    return Finish(contract);
}

std::unique_ptr<PeerReply> PeerReply::Factory(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::PeerReply& serialized)
{
    if (!proto::Validate(serialized, VERBOSE)) {
        otErr << OT_METHOD << __FUNCTION__ << ": invalid serialized reply."
              << std::endl;

        return nullptr;
    }

    std::unique_ptr<PeerReply> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT): {
            contract.reset(new BailmentReply(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT): {
            contract.reset(new OutBailmentReply(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_PENDINGBAILMENT):
        case (proto::PEERREQUEST_STORESECRET): {
            contract.reset(new NoticeAcknowledgement(wallet, nym, serialized));
        } break;
        case (proto::PEERREQUEST_CONNECTIONINFO): {
            contract.reset(new ConnectionReply(wallet, nym, serialized));
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": invalid reply type."
                  << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to instantiate reply."
              << std::endl;

        return nullptr;
    }

    Lock lock(contract->lock_);

    if (!contract->validate(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": invalid reply." << std::endl;

        return nullptr;
    }

    const auto purportedID = Identifier::Factory(serialized.id());

    if (!contract->CalculateID(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to calculate ID."
              << std::endl;

        return nullptr;
    }

    const auto& actualID = contract->id_;

    if (purportedID != actualID) {
        otErr << OT_METHOD << __FUNCTION__ << ": invalid ID." << std::endl;

        return nullptr;
    }

    return contract;
}

bool PeerReply::FinalizeContract(PeerReply& contract)
{
    Lock lock(contract.lock_);

    if (!contract.CalculateID(lock)) { return false; }

    if (!contract.update_signature(lock)) { return false; }

    return contract.validate(lock);
}

std::unique_ptr<PeerReply> PeerReply::Finish(
    std::unique_ptr<PeerReply>& contract)
{
    std::unique_ptr<PeerReply> output(contract.release());

    if (!output) {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to instantiate reply."
              << std::endl;

        return nullptr;
    }

    if (FinalizeContract(*output)) {

        return output;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to finalize contract."
              << std::endl;

        return nullptr;
    }
}

OTIdentifier PeerReply::GetID(const Lock& lock) const
{
    return GetID(IDVersion(lock));
}

OTIdentifier PeerReply::GetID(const proto::PeerReply& contract)
{
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData(contract));
    return id;
}

proto::PeerReply PeerReply::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::PeerReply contract;

    if (version_ < 2) {
        contract.set_version(2);
    } else {
        contract.set_version(version_);
    }

    contract.clear_id();  // reinforcing that this field must be blank.
    contract.set_initiator(String::Factory(initiator_)->Get());
    contract.set_recipient(String::Factory(recipient_)->Get());
    contract.set_type(type_);
    contract.set_cookie(String::Factory(cookie_)->Get());
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.set_server(String::Factory(server_)->Get());

    return contract;
}

std::shared_ptr<proto::PeerRequest> PeerReply::LoadRequest(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const Identifier& requestID)
{
    std::shared_ptr<proto::PeerRequest> output;
    std::time_t notUsed = 0;

    output = wallet.PeerRequest(
        nym->ID(), requestID, StorageBox::INCOMINGPEERREQUEST, notUsed);

    if (!output) {
        output = wallet.PeerRequest(
            nym->ID(), requestID, StorageBox::PROCESSEDPEERREQUEST, notUsed);

        if (output) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": request has already been processed." << std::endl;
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": request does not exist."
                  << std::endl;
        }
    }

    return output;
}

std::string PeerReply::Name() const { return String::Factory(id_)->Get(); }

OTData PeerReply::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(contract(lock));
}

proto::PeerReply PeerReply::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(String::Factory(id(lock))->Get());

    return contract;
}

bool PeerReply::update_signature(const Lock& lock)
{
    if (!ot_super::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_role(proto::SIGROLE_PEERREPLY);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to create signature."
              << std::endl;
    }

    return success;
}

bool PeerReply::validate(const Lock& lock) const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": missing nym." << std::endl;

        return false;
    }

    if (false == validNym) {
        otErr << OT_METHOD << __FUNCTION__ << ": invalid nym." << std::endl;

        return false;
    }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE);

    if (!validSyntax) {
        otErr << OT_METHOD << __FUNCTION__ << ": invalid syntax." << std::endl;

        return false;
    }

    if (1 > signatures_.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Missing signature."
              << std::endl;

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (!validSig) {
        otErr << OT_METHOD << __FUNCTION__ << ": invalid signature."
              << std::endl;
    }

    return (validNym && validSyntax && validSig);
}

bool PeerReply::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (!ot_super::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
    ;
}
}  // namespace opentxs
