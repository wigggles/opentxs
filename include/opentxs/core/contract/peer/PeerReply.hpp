// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREPLY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/Proto.hpp"

#include <string>

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class PeerReply : public Signable
{
private:
    typedef Signable ot_super;

    OTNymID initiator_;
    OTNymID recipient_;
    OTServerID server_;
    OTIdentifier cookie_;
    proto::PeerRequestType type_{proto::PEERREQUEST_ERROR};

    static OTIdentifier GetID(
        const api::internal::Core& api,
        const proto::PeerReply& contract);
    static bool FinalizeContract(
        PeerReply& contract,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerReply> Finish(
        std::unique_ptr<PeerReply>& contract,
        const PasswordPrompt& reason);
    static std::shared_ptr<proto::PeerRequest> LoadRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const Identifier& requestID);

    proto::PeerReply contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const final;
    proto::PeerReply SigVersion(const Lock& lock) const;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;

    PeerReply() = delete;

protected:
    const api::internal::Core& api_;

    virtual proto::PeerReply IDVersion(const Lock& lock) const;
    bool validate(const Lock& lock, const PasswordPrompt& reason) const final;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature,
        const PasswordPrompt& reason) const final;

    PeerReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized);
    PeerReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const identifier::Nym& initiator,
        const identifier::Server& server,
        const proto::PeerRequestType& type,
        const Identifier& request);

public:
    static std::unique_ptr<PeerReply> Create(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequestType& type,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerReply> Create(
        const api::internal::Core& api,
        const Nym_p& nym,
        const Identifier& request,
        const identifier::Server& server,
        const bool& ack,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerReply> Create(
        const api::internal::Core& api,
        const Nym_p& nym,
        const Identifier& request,
        const identifier::Server& server,
        const bool& ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const PasswordPrompt& reason);
    static std::unique_ptr<PeerReply> Factory(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized,
        const PasswordPrompt& reason);

    std::string Alias() const final { return Name(); }
    proto::PeerReply Contract() const;
    std::string Name() const final;
    OTData Serialize() const final;
    const proto::PeerRequestType& Type() const { return type_; }
    void SetAlias(const std::string&) final {}

    ~PeerReply() override = default;
};
}  // namespace opentxs
#endif
