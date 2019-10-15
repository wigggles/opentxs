// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/peer/PeerRequest.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/Proto.tpp"

#include "internal/api/Api.hpp"

#define OT_METHOD "opentxs::PeerRequest::"

namespace opentxs
{
PeerRequest::PeerRequest(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized)
    : ot_super(nym, serialized.version())
    , initiator_(api.Factory().NymID(serialized.initiator()))
    , recipient_(api.Factory().NymID(serialized.recipient()))
    , server_(Identifier::Factory(serialized.server()))
    , cookie_(Identifier::Factory(serialized.cookie()))
    , type_(serialized.type())
    , api_(api)
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

PeerRequest::PeerRequest(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized,
    const std::string& conditions)
    : ot_super(nym, serialized.version(), conditions)
    , initiator_(api.Factory().NymID(serialized.initiator()))
    , recipient_(api.Factory().NymID(serialized.recipient()))
    , server_(Identifier::Factory(serialized.server()))
    , cookie_(Identifier::Factory(serialized.cookie()))
    , type_(serialized.type())
    , api_(api)
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

PeerRequest::PeerRequest(
    const api::internal::Core& api,
    const Nym_p& nym,
    const VersionNumber version,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const proto::PeerRequestType& type)
    : ot_super(nym, version)
    , initiator_(nym->ID())
    , recipient_(recipient)
    , server_(Identifier::Factory(server))
    , cookie_(Identifier::Factory())
    , type_(type)
    , api_(api)
{
    auto random = api_.Factory().BinarySecret();

    OT_ASSERT(random);

    random->randomizeMemory(32);
    cookie_->CalculateDigest(
        Data::Factory(random->getMemory(), random->getMemorySize()));
}

PeerRequest::PeerRequest(
    const api::internal::Core& api,
    const Nym_p& nym,
    const VersionNumber version,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const std::string& conditions,
    const proto::PeerRequestType& type)
    : ot_super(nym, version, conditions)
    , initiator_(nym->ID())
    , recipient_(recipient)
    , server_(Identifier::Factory(server))
    , cookie_(Identifier::Factory())
    , type_(type)
    , api_(api)
{
    auto random = api_.Factory().BinarySecret();

    OT_ASSERT(random);

    random->randomizeMemory(32);
    cookie_->CalculateDigest(
        Data::Factory(random->getMemory(), random->getMemorySize()));
}

proto::PeerRequest PeerRequest::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);
    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

proto::PeerRequest PeerRequest::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::internal::Core& api,
    const Nym_p& sender,
    const proto::PeerRequestType& type,
    const identifier::UnitDefinition& unitID,
    const identifier::Server& serverID,
    const identifier::Nym& recipient,
    const Identifier& requestID,
    const std::string& txid,
    const Amount& amount,
    const PasswordPrompt& reason)
{
    auto unit = api.Wallet().UnitDefinition(unitID, reason);

    if (!unit) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load unit definition.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_PENDINGBAILMENT): {
            contract.reset(new BailmentNotice(
                api,
                sender,
                recipient,
                unitID,
                serverID,
                requestID,
                txid,
                amount));
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract, reason);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequestType& type,
    const identifier::UnitDefinition& unitID,
    const identifier::Server& serverID,
    const PasswordPrompt& reason)
{
    auto unit = api.Wallet().UnitDefinition(unitID, reason);

    if (!unit) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load unit definition.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT): {
            contract.reset(new BailmentRequest(
                api, nym, unit->Nym()->ID(), unitID, serverID));
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract, reason);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequestType& type,
    const identifier::UnitDefinition& unitID,
    const identifier::Server& serverID,
    const std::uint64_t& amount,
    const std::string& terms,
    const PasswordPrompt& reason)
{
    auto unit = api.Wallet().UnitDefinition(unitID, reason);

    if (!unit) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load unit definition.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_OUTBAILMENT): {
            contract.reset(new OutBailmentRequest(
                api, nym, unit->Nym()->ID(), unitID, serverID, amount, terms));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract, reason);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::internal::Core& api,
    const Nym_p& sender,
    const proto::PeerRequestType& type,
    const proto::ConnectionInfoType connectionType,
    const identifier::Nym& recipient,
    const identifier::Server& serverID,
    const PasswordPrompt& reason)
{
    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_CONNECTIONINFO): {
            contract.reset(new ConnectionRequest(
                api, sender, recipient, connectionType, serverID));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract, reason);
}

std::unique_ptr<PeerRequest> PeerRequest::Create(
    const api::internal::Core& api,
    const Nym_p& sender,
    const proto::PeerRequestType& type,
    const proto::SecretType secretType,
    const identifier::Nym& recipient,
    const std::string& primary,
    const std::string& secondary,
    const identifier::Server& serverID,
    const PasswordPrompt& reason)
{
    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_STORESECRET): {
            contract.reset(new StoreSecret(
                api,
                sender,
                recipient,
                secretType,
                primary,
                secondary,
                serverID));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    return Finish(contract, reason);
}

std::unique_ptr<PeerRequest> PeerRequest::Factory(
    const api::internal::Core& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized,
    const PasswordPrompt& reason)
{
    if (!proto::Validate(serialized, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid protobuf.").Flush();

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT): {
            contract.reset(new BailmentRequest(api, nym, serialized));
        } break;
        case (proto::PEERREQUEST_OUTBAILMENT): {
            contract.reset(new OutBailmentRequest(api, nym, serialized));
        } break;
        case (proto::PEERREQUEST_PENDINGBAILMENT): {
            contract.reset(new BailmentNotice(api, nym, serialized));
        } break;
        case (proto::PEERREQUEST_CONNECTIONINFO): {
            contract.reset(new ConnectionRequest(api, nym, serialized));
        } break;
        case (proto::PEERREQUEST_STORESECRET): {
            contract.reset(new StoreSecret(api, nym, serialized));
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request type.")
                .Flush();

            return nullptr;
        }
    }

    if (!contract) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return nullptr;
    }

    Lock lock(contract->lock_);

    if (!contract->validate(lock, reason)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request.").Flush();

        return nullptr;
    }

    const auto purportedID = Identifier::Factory(serialized.id());

    if (!contract->CalculateID(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate ID.").Flush();

        return nullptr;
    }

    const auto& actualID = contract->id_;

    if (purportedID != actualID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid ID.").Flush();

        return nullptr;
    }

    return contract;
}

bool PeerRequest::FinalizeContract(
    PeerRequest& contract,
    const PasswordPrompt& reason)
{
    Lock lock(contract.lock_);

    if (!contract.CalculateID(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate ID.").Flush();

        return false;
    }

    if (!contract.update_signature(lock, reason)) { return false; }

    return contract.validate(lock, reason);
}

std::unique_ptr<PeerRequest> PeerRequest::Finish(
    std::unique_ptr<PeerRequest>& contract,
    const PasswordPrompt& reason)
{
    std::unique_ptr<PeerRequest> output(contract.release());

    if (!output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request.")
            .Flush();

        return nullptr;
    }

    if (FinalizeContract(*output, reason)) {

        return output;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to finalize contract.")
            .Flush();

        return nullptr;
    }
}

OTIdentifier PeerRequest::GetID(const Lock& lock) const
{
    return GetID(api_, IDVersion(lock));
}

OTIdentifier PeerRequest::GetID(
    const api::internal::Core& api,
    const proto::PeerRequest& contract)
{
    auto id = Identifier::Factory();
    id->CalculateDigest(api.Factory().Data(contract));
    return id;
}

proto::PeerRequest PeerRequest::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::PeerRequest contract;

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
    contract.set_server(String::Factory(server_)->Get());
    contract.clear_signature();  // reinforcing that this field must be blank.

    return contract;
}

std::string PeerRequest::Name() const { return String::Factory(id_)->Get(); }

OTData PeerRequest::Serialize() const
{
    Lock lock(lock_);

    return api_.Factory().Data(contract(lock));
}

proto::PeerRequest PeerRequest::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(String::Factory(id(lock))->Get());

    return contract;
}

bool PeerRequest::update_signature(
    const Lock& lock,
    const PasswordPrompt& reason)
{
    if (!ot_super::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    success =
        nym_->Sign(serialized, proto::SIGROLE_PEERREQUEST, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool PeerRequest::validate(const Lock& lock, const PasswordPrompt& reason) const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym(reason);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym.").Flush();
    }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE);

    if (!validSyntax) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid syntax.").Flush();
    }

    if (1 > signatures_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature, reason); }

    if (!validSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature.").Flush();
    }

    return (validNym && validSyntax && validSig);
}

bool PeerRequest::verify_signature(
    const Lock& lock,
    const proto::Signature& signature,
    const PasswordPrompt& reason) const
{
    if (!ot_super::verify_signature(lock, signature, reason)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Verify(serialized, sigProto, reason);
}
}  // namespace opentxs
