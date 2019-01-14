// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/Data.hpp"

#include "blind/Token.hpp"

#include <memory>
#include <stdexcept>

namespace opentxs::blind::token::implementation
{
class Lucre final : public Token
{
public:
    bool GetSpendable(String& output) const;
    std::string ID() const override;
    bool IsSpent() const override;
    proto::Token Serialize() const override;

    bool AddSignature(const String& signature);
    bool ChangeOwner(crypto::key::Symmetric& key) override;
    bool GenerateTokenRequest(
        const Nym& owner,
        const OTPassword& primaryPassword,
        const OTPassword& secondaryPassword,
        const Mint& mint) override;
    bool GetPublicPrototoken(String& output);
    bool MarkSpent() override;
    bool Process(const Nym& owner, const Mint& mint) override;

    Lucre(const api::Core& api, Purse& purse, const proto::Token& serialized);
    Lucre(
        const api::Core& api,
        const Nym& owner,
        const Mint& mint,
        const Denomination value,
        Purse& purse,
        const OTPassword& primaryPassword,
        const OTPassword& secondaryPassword);

    ~Lucre() override = default;

private:
    friend opentxs::Factory;

    const std::uint32_t lucre_version_;
    OTString signature_;
    std::shared_ptr<proto::Ciphertext> private_;
    std::shared_ptr<proto::Ciphertext> public_;
    std::shared_ptr<proto::Ciphertext> spend_;

    void serialize_private(proto::LucreTokenData& lucre) const;
    void serialize_public(proto::LucreTokenData& lucre) const;
    void serialize_signature(proto::LucreTokenData& lucre) const;
    void serialize_spendable(proto::LucreTokenData& lucre) const;

    Lucre* clone() const noexcept override { return new Lucre(*this); }

    Lucre(
        const api::Core& api,
        Purse& purse,
        const std::uint32_t version,
        const proto::TokenState state,
        const std::uint64_t series,
        const Denomination value,
        const Time validFrom,
        const Time validTo,
        const String& signature,
        const std::shared_ptr<proto::Ciphertext> publicKey,
        const std::shared_ptr<proto::Ciphertext> privateKey,
        const std::shared_ptr<proto::Ciphertext> spendable);
    Lucre() = delete;
    Lucre(const Lucre&);
    Lucre(Lucre&&) = delete;
    Lucre& operator=(const Lucre&) = delete;
    Lucre& operator=(Lucre&&) = delete;
};
}  // namespace opentxs::blind::token::implementation
