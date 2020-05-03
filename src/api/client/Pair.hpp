// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/Pair.cpp"

#pragma once

#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "core/StateMachine.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Issuer;
class Pair;
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PeerReply;
class PeerRequest;
}  // namespace proto

class ContactData;
class ContactSection;
class Factory;
class Flag;
class PasswordPrompt;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::implementation
{
class Pair final : virtual public internal::Pair,
                   Lockable,
                   opentxs::internal::StateMachine
{
public:
    bool AddIssuer(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const std::string& pairingCode) const noexcept final;
    bool CheckIssuer(
        const identifier::Nym& localNymID,
        const identifier::UnitDefinition& unitDefinitionID) const
        noexcept final;
    std::string IssuerDetails(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const noexcept final;
    std::set<OTNymID> IssuerList(
        const identifier::Nym& localNymID,
        const bool onlyTrusted) const noexcept final
    {
        return state_.IssuerList(localNymID, onlyTrusted);
    }
    std::shared_future<void> Stop() const noexcept final { return cleanup(); }
    std::shared_future<void> Wait() const noexcept final
    {
        return StateMachine::Wait();
    }

    void init() noexcept final;

    ~Pair() { cleanup().get(); }

    friend class opentxs::api::client::Pair;

private:
    friend opentxs::Factory;
    /// local nym id, issuer nym id
    using IssuerID = std::pair<OTNymID, OTNymID>;
    enum class Status : std::uint8_t {
        Error = 0,
        Started = 1,
        Registered = 2,
    };

    struct State {
        using OfferedCurrencies = std::size_t;
        using RegisteredAccounts = std::size_t;
        using UnusedBailments = std::size_t;
        using NeedRename = bool;
        using AccountDetails =
            std::tuple<OTUnitID, OTIdentifier, UnusedBailments>;
        using Trusted = bool;
        using Details = std::tuple<
            std::unique_ptr<std::mutex>,
            OTServerID,
            OTNymID,
            Status,
            Trusted,
            OfferedCurrencies,
            RegisteredAccounts,
            std::vector<AccountDetails>,
            std::vector<OTX::BackgroundTask>,
            NeedRename>;
        using StateMap = std::map<IssuerID, Details>;

        static std::size_t count_currencies(
            const std::vector<AccountDetails>& in) noexcept;
        static std::size_t count_currencies(const ContactSection& in) noexcept;
        static AccountDetails& get_account(
            const identifier::UnitDefinition& unit,
            const Identifier& account,
            std::vector<AccountDetails>& details) noexcept;

        bool CheckIssuer(const identifier::Nym& id) const noexcept;
        bool check_state() const noexcept;

        void Add(
            const identifier::Nym& localNymID,
            const identifier::Nym& issuerNymID,
            const bool trusted) noexcept;
        void Add(
            const Lock& lock,
            const identifier::Nym& localNymID,
            const identifier::Nym& issuerNymID,
            const bool trusted) noexcept;
        StateMap::iterator begin() noexcept { return state_.begin(); }
        StateMap::iterator end() noexcept { return state_.end(); }
        StateMap::iterator GetDetails(
            const identifier::Nym& localNymID,
            const identifier::Nym& issuerNymID) noexcept;
        bool run(const std::function<void(const IssuerID&)> fn) noexcept;

        std::set<OTNymID> IssuerList(
            const identifier::Nym& localNymID,
            const bool onlyTrusted) const noexcept;

        State(
            std::mutex& lock,
            const api::client::internal::Manager& client) noexcept;

    private:
        std::mutex& lock_;
        const api::client::internal::Manager& client_;
        mutable StateMap state_;
        std::set<OTNymID> issuers_;
    };

    const Flag& running_;
    const api::client::internal::Manager& client_;
    mutable State state_;
    std::promise<void> startup_promise_;
    std::shared_future<void> startup_;
    OTZMQListenCallback nym_callback_;
    OTZMQListenCallback peer_reply_callback_;
    OTZMQListenCallback peer_request_callback_;
    OTZMQPublishSocket pair_event_;
    OTZMQPublishSocket pending_bailment_;
    OTZMQSubscribeSocket nym_subscriber_;
    OTZMQSubscribeSocket peer_reply_subscriber_;
    OTZMQSubscribeSocket peer_request_subscriber_;

    void check_accounts(
        const ContactData& issuerClaims,
        Issuer& issuer,
        const identifier::Server& serverID,
        std::size_t& offered,
        std::size_t& registeredAccounts,
        std::vector<State::AccountDetails>& accountDetails) const noexcept;
    void check_connection_info(
        Issuer& issuer,
        const identifier::Server& serverID) const noexcept;
    void check_rename(
        const Issuer& issuer,
        const identifier::Server& serverID,
        const PasswordPrompt& reason,
        bool& needRename) const noexcept;
    void check_store_secret(Issuer& issuer, const identifier::Server& serverID)
        const noexcept;
    std::shared_future<void> cleanup() const noexcept;
    std::pair<bool, OTIdentifier> get_connection(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const identifier::Server& serverID,
        const proto::ConnectionInfoType type) const;
    std::pair<bool, OTIdentifier> initiate_bailment(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const identifier::Nym& issuerID,
        const identifier::UnitDefinition& unitID) const;
    bool process_connection_info(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    void process_peer_replies(const Lock& lock, const identifier::Nym& nymID)
        const;
    void process_peer_requests(const Lock& lock, const identifier::Nym& nymID)
        const;
    bool process_pending_bailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerRequest& request) const;
    bool process_request_bailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    bool process_request_outbailment(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    bool process_store_secret(
        const Lock& lock,
        const identifier::Nym& nymID,
        const proto::PeerReply& reply) const;
    OTX::BackgroundTask queue_nym_download(
        const identifier::Nym& localNymID,
        const identifier::Nym& targetNymID) const;
    OTX::BackgroundTask queue_nym_registration(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const bool setData) const;
    OTX::BackgroundTask queue_server_contract(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const;
    void queue_unit_definition(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID) const;
    std::pair<bool, OTIdentifier> register_account(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID) const;
    bool need_registration(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const;
    void state_machine(const IssuerID& id) const;
#if OT_CRYPTO_WITH_BIP32
    std::pair<bool, OTIdentifier> store_secret(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const identifier::Server& serverID) const;
#endif  // OT_CRYPTO_WITH_BIP32

    void callback_nym(const zmq::Message& in) noexcept;
    void callback_peer_reply(const zmq::Message& in) noexcept;
    void callback_peer_request(const zmq::Message& in) noexcept;

    Pair(const Flag& running, const api::client::internal::Manager& client);
    Pair() = delete;
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace opentxs::api::client::implementation
