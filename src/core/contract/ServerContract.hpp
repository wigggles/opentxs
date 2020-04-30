// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/core/contract/ServerContract.cpp"

#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <string>

#include "core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/ServerContract.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class Factory;
class OTPassword;
class PasswordPrompt;
class String;
}  // namespace opentxs

namespace opentxs::contract::implementation
{
class Server final : public contract::Server,
                     public opentxs::contract::implementation::Signable
{
public:
    bool ConnectInfo(
        std::string& strHostname,
        std::uint32_t& nPort,
        proto::AddressType& actual,
        const proto::AddressType& preferred) const final;
    proto::ServerContract Contract() const final;
    std::string EffectiveName() const final;
    std::string Name() const final { return name_; }
    proto::ServerContract PublicContract() const final;
    bool Statistics(String& strContents) const final;
    OTData Serialize() const final;
    const Data& TransportKey() const final;
    std::unique_ptr<OTPassword> TransportKey(
        Data& pubkey,
        const PasswordPrompt& reason) const final;

    void InitAlias(const std::string& alias) final
    {
        contract::implementation::Signable::SetAlias(alias);
    }
    void SetAlias(const std::string& alias) final;

    Server(
        const api::internal::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const std::string& terms,
        const std::string& name,
        std::list<contract::Server::Endpoint>&& endpoints,
        OTData&& key,
        const std::string& id = {},
        Signatures&& signatures = {});
    Server(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::ServerContract& serialized);

    ~Server() final = default;

private:
    friend opentxs::Factory;

    const std::list<contract::Server::Endpoint> listen_params_;
    const std::string name_;
    const OTData transport_key_;

    static std::list<contract::Server::Endpoint> extract_endpoints(
        const proto::ServerContract& serialized) noexcept;

    Server* clone() const noexcept final { return new Server(*this); }
    proto::ServerContract contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const final;
    proto::ServerContract IDVersion(const Lock& lock) const;
    proto::ServerContract SigVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const final;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const final;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;

    Server() = delete;
    Server(const Server&);
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace opentxs::contract::implementation
