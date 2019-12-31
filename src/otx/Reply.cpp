// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/Proto.tpp"

#include "core/contract/Signable.hpp"
#include "internal/api/Api.hpp"

#include "Reply.hpp"

template class opentxs::Pimpl<opentxs::otx::Reply>;

#define OT_METHOD "opentxs::otx::implementation::Reply::"

namespace opentxs::otx
{
const VersionNumber Reply::DefaultVersion{1};
const VersionNumber Reply::MaxVersion{1};

OTXReply Reply::Factory(
    const api::internal::Core& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const proto::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    const PasswordPrompt& reason,
    std::shared_ptr<const proto::OTXPush>&& push)
{
    OT_ASSERT(signer);

    std::unique_ptr<implementation::Reply> output{new implementation::Reply(
        api,
        signer,
        recipient,
        server,
        type,
        number,
        success,
        std::move(push))};

    OT_ASSERT(output);

    Lock lock(output->lock_);
    output->update_signature(lock, reason);

    OT_ASSERT(false == output->id(lock)->empty());

    return OTXReply{output.release()};
}

OTXReply Reply::Factory(
    const api::internal::Core& api,
    const proto::ServerReply serialized)
{
    return OTXReply{new implementation::Reply(api, serialized)};
}
}  // namespace opentxs::otx

namespace opentxs::otx::implementation
{
Reply::Reply(
    const api::internal::Core& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Server& server,
    const proto::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    std::shared_ptr<const proto::OTXPush>&& push)
    : Signable(api, signer, DefaultVersion, "", "")
    , recipient_(recipient)
    , server_(server)
    , type_(type)
    , success_(success)
    , number_(number)
    , payload_(std::move(push))
{
    Lock lock(lock_);
    first_time_init(lock);
}

Reply::Reply(
    const api::internal::Core& api,
    const proto::ServerReply serialized)
    : Signable(
          api,
          extract_nym(api, serialized),
          serialized.version(),
          "",
          "",
          api.Factory().Identifier(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , recipient_(identifier::Nym::Factory(serialized.nym()))
    , server_(identifier::Server::Factory(serialized.server()))
    , type_(serialized.type())
    , success_(serialized.success())
    , number_(serialized.request())
    , payload_(
          serialized.has_push()
              ? std::make_shared<proto::OTXPush>(serialized.push())
              : std::shared_ptr<proto::OTXPush>{})
{
    Lock lock(lock_);
    init_serialized(lock);
}

Reply::Reply(const Reply& rhs)
    : Signable(rhs)
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , type_(rhs.type_)
    , success_(rhs.success_)
    , number_(rhs.number_)
    , payload_(rhs.payload_)
{
}

proto::ServerReply Reply::Contract() const
{
    Lock lock(lock_);
    auto output = full_version(lock);

    return output;
}

Nym_p Reply::extract_nym(
    const api::internal::Core& api,
    const proto::ServerReply serialized)
{
    const auto serverID = identifier::Server::Factory(serialized.server());

    try {
        return api.Wallet().Server(serverID)->Nym();
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid server id.").Flush();

        return nullptr;
    }
}

proto::ServerReply Reply::full_version(const Lock& lock) const
{
    auto contract = signature_version(lock);

    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

OTIdentifier Reply::GetID(const Lock& lock) const
{
    return api_.Factory().Identifier(id_version(lock));
}

proto::ServerReply Reply::id_version(const Lock& lock) const
{
    proto::ServerReply output{};
    output.set_version(version_);
    output.clear_id();  // Must be blank
    output.set_type(type_);
    output.set_nym(recipient_->str());
    output.set_server(server_->str());
    output.set_request(number_);
    output.set_success(success_);

    if (payload_) { *output.mutable_push() = *payload_; }

    output.clear_signature();  // Must be blank

    return output;
}

OTData Reply::Serialize() const
{
    Lock lock(lock_);

    return api_.Factory().Data(full_version(lock));
}

proto::ServerReply Reply::signature_version(const Lock& lock) const
{
    auto contract = id_version(lock);
    contract.set_id(id_->str());

    return contract;
}

bool Reply::update_signature(const Lock& lock, const PasswordPrompt& reason)
{
    if (false == Signable::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = signature_version(lock);
    auto& signature = *serialized.mutable_signature();
    success =
        nym_->Sign(serialized, proto::SIGROLE_SERVERREPLY, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create signature.")
            .Flush();
    }

    return success;
}

bool Reply::validate(const Lock& lock) const
{
    bool validNym{false};

    if (nym_) { validNym = nym_->VerifyPseudonym(); }

    if (false == validNym) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(full_version(lock), VERBOSE);

    if (false == validSyntax) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid syntax.").Flush();

        return false;
    }

    if (1 != signatures_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong number signatures.")
            .Flush();

        return false;
    }

    bool validSig{false};
    auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (false == validSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature.").Flush();

        return false;
    }

    return true;
}

bool Reply::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (false == Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = signature_version(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Verify(serialized, sigProto);
}
}  // namespace opentxs::otx::implementation
