// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/ServerContract.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/Proto.tpp"

#define OT_METHOD "opentxs::ServerContract::"

namespace opentxs
{
const VersionNumber ServerContract::DefaultVersion{2};

ServerContract::ServerContract(const api::Core& api, const Nym_p& nym)
    : ot_super(nym)
    , api_{api}
    , listen_params_()
    , name_()
    , transport_key_(Data::Factory())
{
}

ServerContract::ServerContract(
    const api::Core& api,
    const Nym_p& nym,
    const proto::ServerContract& serialized)
    : ServerContract(api, nym)
{
    id_ = Identifier::Factory(serialized.id());
    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
    version_ = serialized.version();
    conditions_ = serialized.terms();

    for (auto& listen : serialized.address()) {
        ServerContract::Endpoint endpoint{listen.type(),
                                          listen.protocol(),
                                          listen.host(),
                                          listen.port(),
                                          listen.version()};
        // WARNING: preserve the order of this list, or signature verfication
        // will fail!
        listen_params_.push_back(endpoint);
    }

    name_ = serialized.name();
    transport_key_->Assign(
        serialized.transportkey().c_str(), serialized.transportkey().size());
}

ServerContract* ServerContract::Create(
    const api::Core& api,
    const Nym_p& nym,
    const std::list<ServerContract::Endpoint>& endpoints,
    const std::string& terms,
    const std::string& name,
    const VersionNumber version,
    const PasswordPrompt& reason)
{
    OT_ASSERT(nym);
    OT_ASSERT(nym->HasCapability(NymCapability::AUTHENTICATE_CONNECTION));

    ServerContract* contract = new ServerContract(api, nym);

    if (nullptr != contract) {
        contract->version_ = version;
        contract->listen_params_ = endpoints;
        contract->conditions_ = terms;
        nym->TransportKey(contract->transport_key_, reason);
        contract->name_ = name;

        Lock lock(contract->lock_);

        if (!contract->CalculateID(lock)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error calculating contract id")
                .Flush();

            return nullptr;
        }

        if (false == contract->update_signature(lock, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign contract")
                .Flush();

            return nullptr;
        }

        if (!contract->validate(lock, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid contract").Flush();

            return nullptr;
        }

        contract->alias_ = contract->name_;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to create server contract.")
            .Flush();
    }

    return contract;
}

std::string ServerContract::EffectiveName(const PasswordPrompt& reason) const
{
    OT_ASSERT(nym_)

    // TODO The version stored in nym_ might be out of date so load it from the
    // wallet. This can be fixed correctly by implementing in-place updates of
    // Nym credentials
    const auto nym = api_.Wallet().Nym(nym_->ID(), reason);
    const auto output = nym->Name();

    if (output.empty()) { return name_; }

    return output;
}

ServerContract* ServerContract::Factory(
    const api::Core& api,
    const Nym_p& nym,
    const proto::ServerContract& serialized,
    const PasswordPrompt& reason)
{
    if (!proto::Validate<proto::ServerContract>(serialized, VERBOSE)) {
        return nullptr;
    }

    std::unique_ptr<ServerContract> contract(
        new ServerContract(api, nym, serialized));

    if (!contract) { return nullptr; }

    Lock lock(contract->lock_);

    if (!contract->validate(lock, reason)) { return nullptr; }

    contract->alias_ = contract->name_;

    return contract.release();
}

OTIdentifier ServerContract::GetID(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    auto id = Identifier::Factory();
    id->CalculateDigest(api_.Factory().Data(contract));
    return id;
}

bool ServerContract::ConnectInfo(
    std::string& strHostname,
    std::uint32_t& nPort,
    proto::AddressType& actual,
    const proto::AddressType& preferred) const
{
    if (0 < listen_params_.size()) {
        for (auto& endpoint : listen_params_) {
            const auto& type = std::get<0>(endpoint);
            const auto& url = std::get<2>(endpoint);
            const auto& port = std::get<3>(endpoint);

            if (preferred == type) {
                strHostname = url;
                nPort = port;
                actual = type;

                return true;
            }
        }

        // If we didn't find the preferred type, return the first result
        const auto& endpoint = listen_params_.front();
        const auto& type = std::get<0>(endpoint);
        const auto& url = std::get<2>(endpoint);
        const auto& port = std::get<3>(endpoint);
        strHostname = url;
        nPort = port;
        actual = type;

        return true;
    }

    return false;
}

proto::ServerContract ServerContract::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);
    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

proto::ServerContract ServerContract::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

proto::ServerContract ServerContract::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::ServerContract contract;
    contract.set_version(version_);
    contract.clear_id();         // reinforcing that this field must be blank.
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.clear_publicnym();  // reinforcing that this field must be blank.

    if (nym_) {
        auto nymID = String::Factory();
        nym_->GetIdentifier(nymID);
        contract.set_nymid(nymID->Get());
    }

    contract.set_name(name_);

    for (const auto& endpoint : listen_params_) {
        auto& addr = *contract.add_address();
        const auto& version = std::get<4>(endpoint);
        const auto& type = std::get<0>(endpoint);
        const auto& protocol = std::get<1>(endpoint);
        const auto& url = std::get<2>(endpoint);
        const auto& port = std::get<3>(endpoint);
        addr.set_version(version);
        addr.set_type(type);
        addr.set_protocol(protocol);
        addr.set_host(url);
        addr.set_port(port);
    }

    contract.set_terms(conditions_);
    contract.set_transportkey(transport_key_->data(), transport_key_->size());

    return contract;
}

void ServerContract::SetAlias(const std::string& alias)
{
    ot_super::SetAlias(alias);

    api_.Wallet().SetServerAlias(
        identifier::Server::Factory(id_->str()), alias);  // TODO conversion
}

proto::ServerContract ServerContract::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(String::Factory(id(lock))->Get());

    return contract;
}

proto::ServerContract ServerContract::PublicContract() const
{
    Lock lock(lock_);

    auto serialized = contract(lock);

    if (nym_) {
        auto publicNym = nym_->asPublicNym();
        *(serialized.mutable_publicnym()) = publicNym;
    }

    return serialized;
}

bool ServerContract::Statistics(String& strContents) const
{
    const auto strID = String::Factory(id_);

    strContents.Concatenate(
        " Notary Provider: %s\n"
        " NotaryID: %s\n"
        "\n",
        nym_->Alias().c_str(),
        strID->Get());

    return true;
}

OTData ServerContract::Serialize() const
{
    Lock lock(lock_);

    return api_.Factory().Data(contract(lock));
}

const Data& ServerContract::TransportKey() const
{
    return transport_key_.get();
}

std::unique_ptr<OTPassword> ServerContract::TransportKey(
    Data& pubkey,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(nym_);

    return nym_->TransportKey(pubkey, reason);
}

bool ServerContract::update_signature(
    const Lock& lock,
    const PasswordPrompt& reason)
{
    if (!ot_super::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Sign(
        serialized, proto::SIGROLE_SERVERCONTRACT, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": failed to create signature.")
            .Flush();
    }

    return success;
}

bool ServerContract::validate(const Lock& lock, const PasswordPrompt& reason)
    const
{
    bool validNym = false;

    if (nym_) { validNym = nym_->VerifyPseudonym(reason); }

    if (!validNym) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE);

    if (!validSyntax) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid syntax.").Flush();

        return false;
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

        return false;
    }

    return true;
}

bool ServerContract::verify_signature(
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
