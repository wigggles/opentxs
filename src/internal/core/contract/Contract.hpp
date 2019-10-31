// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"

#include <string>

namespace opentxs::contract::blank
{
struct Signable : virtual public opentxs::contract::Signable {
    std::string Alias() const final { return {}; }
    OTIdentifier ID() const final { return id_; }
    std::string Name() const final { return {}; }
    Nym_p Nym() const final { return {}; }
    const std::string& Terms() const final { return terms_; }
    OTData Serialize() const final { return OTData{id_}; }
    bool Validate(const PasswordPrompt&) const final { return {}; }
    VersionNumber Version() const final { return 0; }

    void SetAlias(const std::string&) final {}

    Signable(const api::Core& api)
        : id_(api.Factory().Identifier())
        , terms_()
    {
    }

    ~Signable() override = default;

protected:
    const OTIdentifier id_;
    const std::string terms_;

    Signable(const Signable& rhs)
        : id_(rhs.id_)
        , terms_(rhs.terms_)
    {
    }
};

struct Unit final : virtual public opentxs::contract::Unit, public Signable {
    bool AddAccountRecord(const std::string&, const Account&) const final
    {
        return {};
    }
    SerializedType Contract() const final { return {}; }
    std::int32_t DecimalPower() const final { return {}; }
    bool DisplayStatistics(String&) const final { return {}; }
    bool EraseAccountRecord(const std::string&, const Identifier&) const final
    {
        return {};
    }
    bool FormatAmountLocale(
        Amount,
        std::string&,
        const std::string&,
        const std::string&) const final
    {
        return {};
    }
    bool FormatAmountWithoutSymbolLocale(
        Amount amount,
        std::string&,
        const std::string&,
        const std::string&) const final
    {
        return {};
    }
    std::string FractionalUnitName() const final { return {}; }
    const std::string& GetCurrencyName() const final { return terms_; }
    const std::string& GetCurrencySymbol() const final { return terms_; }
    SerializedType PublicContract() const final { return {}; }
    bool StringToAmountLocale(
        Amount&,
        const std::string&,
        const std::string&,
        const std::string&) const final
    {
        return {};
    }
    std::string TLA() const final { return {}; }
    proto::UnitType Type() const final { return {}; }
    proto::ContactItemType UnitOfAccount() const final { return {}; }
    bool VisitAccountRecords(
        const std::string&,
        AccountVisitor&,
        const PasswordPrompt&) const final
    {
        return {};
    }

    void InitAlias(const std::string&) final {}

    Unit(const api::Core& api)
        : Signable(api)
    {
    }

    ~Unit() override = default;

private:
    Unit* clone() const noexcept override { return new Unit(*this); }

    Unit(const Unit& rhs)
        : Signable(rhs)
    {
    }
};

struct Server final : virtual public opentxs::contract::Server,
                      public blank::Signable {
    bool ConnectInfo(
        std::string&,
        std::uint32_t&,
        proto::AddressType&,
        const proto::AddressType&) const final
    {
        return {};
    }
    proto::ServerContract Contract() const final { return {}; }
    std::string EffectiveName(const PasswordPrompt&) const final { return {}; }
    proto::ServerContract PublicContract() const final { return {}; }
    bool Statistics(String&) const final { return {}; }
    const Data& TransportKey() const final { return id_; }
    std::unique_ptr<OTPassword> TransportKey(Data&, const PasswordPrompt&)
        const final
    {
        return {};
    }

    void InitAlias(const std::string&) final {}

    Server(const api::Core& api)
        : Signable(api)
    {
    }

    ~Server() final = default;

private:
    Server* clone() const noexcept final { return new Server(*this); }

    Server(const Server& rhs)
        : Signable(rhs)
    {
    }
};
}  // namespace opentxs::contract::blank

namespace opentxs::contract::peer::blank
{
struct Reply final : virtual public opentxs::contract::peer::Reply,
                     public contract::blank::Signable {
    SerializedType Contract() const final { return {}; }
    proto::PeerRequestType Type() const final
    {
        return proto::PEERREQUEST_ERROR;
    }

    Reply(const api::Core& api)
        : Signable(api)
    {
    }

    ~Reply() final = default;

private:
    Reply* clone() const noexcept final { return new Reply(*this); }

    Reply(const Reply& rhs)
        : Signable(rhs)
    {
    }
};

struct Request final : virtual public opentxs::contract::peer::Request,
                       public contract::blank::Signable {
    SerializedType Contract() const final { return {}; }
    const identifier::Nym& Initiator() const { return nym_; }
    const identifier::Nym& Recipient() const { return nym_; }
    proto::PeerRequestType Type() const final
    {
        return proto::PEERREQUEST_ERROR;
    }

    Request(const api::Core& api)
        : Signable(api)
        , nym_(api.Factory().NymID())
    {
    }

    ~Request() final = default;

private:
    const identifier::Nym& nym_;

    Request* clone() const noexcept final { return new Request(*this); }

    Request(const Request& rhs)
        : Signable(rhs)
        , nym_(rhs.nym_)
    {
    }
};
}  // namespace opentxs::contract::peer::blank
