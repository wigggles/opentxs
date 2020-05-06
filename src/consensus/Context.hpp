// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <set>
#include <string>

#include "core/contract/Signable.hpp"
#include "internal/api/Api.hpp"
#include "internal/consensus/Consensus.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/protobuf/ConsensusEnums.pb.h"

namespace opentxs
{
namespace api
{
class Core;

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

class Factory;
class NymFile;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::implementation
{
class Context : virtual public internal::Context,
                public opentxs::contract::implementation::Signable
{
public:
    std::set<RequestNumber> AcknowledgedNumbers() const final;
    std::size_t AvailableNumbers() const final;
    bool HaveLocalNymboxHash() const final;
    bool HaveRemoteNymboxHash() const final;
    std::set<TransactionNumber> IssuedNumbers() const final;
    std::string Name() const final;
    bool NymboxHashMatch() const final;
    std::string LegacyDataFolder() const final;
    OTIdentifier LocalNymboxHash() const final;
    std::unique_ptr<const opentxs::NymFile> Nymfile(
        const PasswordPrompt& reason) const final;
    const identity::Nym& RemoteNym() const final;
    OTIdentifier RemoteNymboxHash() const final;
    RequestNumber Request() const final;
    OTData Serialize() const final;
    proto::Context Serialized() const final;
    const identifier::Server& Server() const final { return server_id_; }
    bool VerifyAcknowledgedNumber(const RequestNumber& req) const final;
    bool VerifyAvailableNumber(const TransactionNumber& number) const final;
    bool VerifyIssuedNumber(const TransactionNumber& number) const final;

    bool AddAcknowledgedNumber(const RequestNumber req) final;
    bool CloseCronItem(const TransactionNumber) override { return false; }
    bool ConsumeAvailable(const TransactionNumber& number) final;
    bool ConsumeIssued(const TransactionNumber& number) final;
    RequestNumber IncrementRequest() final;
    bool InitializeNymbox(const PasswordPrompt& reason) final;
    Editor<opentxs::NymFile> mutable_Nymfile(
        const PasswordPrompt& reason) final;
    bool OpenCronItem(const TransactionNumber) override { return false; }
    bool RecoverAvailableNumber(const TransactionNumber& number) final;
    proto::Context Refresh(const PasswordPrompt& reason) final;
    bool RemoveAcknowledgedNumber(const std::set<RequestNumber>& req) final;
    void Reset() final;
    void SetLocalNymboxHash(const Identifier& hash) final;
    void SetRemoteNymboxHash(const Identifier& hash) final;
    void SetRequest(const RequestNumber req) final;

    ~Context() override = default;

protected:
    const OTServerID server_id_;
    Nym_p remote_nym_{};
    std::set<TransactionNumber> available_transaction_numbers_{};
    std::set<TransactionNumber> issued_transaction_numbers_{};
    std::atomic<RequestNumber> request_number_{0};
    std::set<RequestNumber> acknowledged_request_numbers_{};
    OTIdentifier local_nymbox_hash_;
    OTIdentifier remote_nymbox_hash_;

    proto::Context contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const final;
    proto::Context serialize(const Lock& lock, const proto::ConsensusType type)
        const;
    virtual proto::Context serialize(const Lock& lock) const = 0;
    virtual std::string type() const = 0;
    bool validate(const Lock& lock) const final;

    bool add_acknowledged_number(const Lock& lock, const RequestNumber req);
    bool consume_available(const Lock& lock, const TransactionNumber& number);
    bool consume_issued(const Lock& lock, const TransactionNumber& number);
    void finish_acknowledgements(
        const Lock& lock,
        const std::set<RequestNumber>& req);
    bool issue_number(const Lock& lock, const TransactionNumber& number);
    bool recover_available_number(
        const Lock& lock,
        const TransactionNumber& number);
    bool remove_acknowledged_number(
        const Lock& lock,
        const std::set<RequestNumber>& req);
    bool save(const Lock& lock, const PasswordPrompt& reason);
    void set_local_nymbox_hash(const Lock& lock, const Identifier& hash);
    void set_remote_nymbox_hash(const Lock& lock, const Identifier& hash);
    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;
    bool verify_available_number(const Lock& lock, const TransactionNumber& req)
        const;
    bool verify_acknowledged_number(const Lock& lock, const RequestNumber& req)
        const;
    bool verify_issued_number(const Lock& lock, const TransactionNumber& number)
        const;

    Context(
        const api::internal::Core& api,
        const VersionNumber targetVersion,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);
    Context(
        const api::internal::Core& api,
        const VersionNumber targetVersion,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Server& server);

private:
    friend opentxs::Factory;

    const VersionNumber target_version_{0};

    static auto calculate_id(
        const api::Core& api,
        const Nym_p& client,
        const Nym_p& server) noexcept(false) -> OTIdentifier;

    virtual const identifier::Nym& client_nym_id(const Lock& lock) const = 0;
    Context* clone() const noexcept final { return nullptr; }
    proto::Context IDVersion(const Lock& lock) const;
    virtual const identifier::Nym& server_nym_id(const Lock& lock) const = 0;
    proto::Context SigVersion(const Lock& lock) const;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const final;

    // Transition method used for converting from Nym class
    bool insert_available_number(const TransactionNumber& number);
    // Transition method used for converting from Nym class
    bool insert_issued_number(const TransactionNumber& number);

    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace opentxs::implementation
