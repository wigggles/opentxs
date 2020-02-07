// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blind::implementation
{
class Purse final : virtual public blind::Purse
{
public:
    const Token& at(const std::size_t position) const final
    {
        return tokens_.at(position);
    }
    const_iterator begin() const noexcept final { return cbegin(); }
    const_iterator cbegin() const noexcept final
    {
        return const_iterator(this, 0);
    }
    const_iterator cend() const noexcept final
    {
        return const_iterator(this, tokens_.size());
    }
    Time EarliestValidTo() const final { return earliest_valid_to_; }
    const_iterator end() const noexcept final { return cend(); }
    bool IsUnlocked() const final { return unlocked_; }
    Time LatestValidFrom() const final { return latest_valid_from_; }
    const identifier::Server& Notary() const final { return notary_; }
    bool Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) final;
    proto::Purse Serialize() const final;
    std::size_t size() const noexcept final { return tokens_.size(); }
    proto::PurseType State() const final { return state_; }
    proto::CashType Type() const final { return type_; }
    const identifier::UnitDefinition& Unit() const final { return unit_; }
    bool Unlock(const identity::Nym& nym, const PasswordPrompt& reason)
        const final;
    bool Verify(const api::server::internal::Manager& server) const final;
    Amount Value() const final { return total_value_; }

    bool AddNym(const identity::Nym& nym, const PasswordPrompt& reason) final;
    Token& at(const std::size_t position) final { return tokens_.at(position); }
    iterator begin() noexcept final { return iterator(this, 0); }
    iterator end() noexcept final { return iterator(this, tokens_.size()); }
    bool GeneratePrototokens(
        const identity::Nym& owner,
        const Mint& mint,
        const Amount amount,
        const PasswordPrompt& reason);
    crypto::key::Symmetric& PrimaryKey(PasswordPrompt& password) final;
    std::shared_ptr<Token> Pop() final;
    bool Push(std::shared_ptr<Token> token, const PasswordPrompt& reason) final;
    const crypto::key::Symmetric& SecondaryKey(
        const identity::Nym& owner,
        PasswordPrompt& password) final;

    Purse(
        const api::internal::Core& api,
        const identifier::Nym& owner,
        const identifier::Server& server,
        const proto::CashType type,
        const Mint& mint,
        std::unique_ptr<OTPassword> secondaryKeyPassword,
        std::unique_ptr<const OTSymmetricKey> secondaryKey,
        std::unique_ptr<const OTEnvelope> secondaryEncrypted);

    ~Purse() final = default;

private:
    friend opentxs::Factory;

    static const proto::SymmetricMode mode_;

    const api::internal::Core& api_;
    const VersionNumber version_;
    const proto::CashType type_;
    const OTServerID notary_;
    const OTUnitID unit_;
    proto::PurseType state_;
    Amount total_value_;
    Time latest_valid_from_;
    Time earliest_valid_to_;
    std::vector<OTToken> tokens_;
    mutable bool unlocked_;
    mutable OTPassword primary_key_password_;
    std::shared_ptr<OTSymmetricKey> primary_;
    std::vector<proto::Envelope> primary_passwords_;
    OTPassword secondary_key_password_;
    const std::shared_ptr<const OTSymmetricKey> secondary_;
    const std::shared_ptr<const OTEnvelope> secondary_password_;

    static auto deserialize_secondary_key(
        const api::internal::Core& api,
        const proto::Purse& serialized) noexcept(false)
        -> std::unique_ptr<const OTSymmetricKey>;
    static auto deserialize_secondary_password(
        const api::internal::Core& api,
        const proto::Purse& serialized) noexcept(false)
        -> std::unique_ptr<const OTEnvelope>;
    static std::vector<proto::Envelope> get_passwords(const proto::Purse& in);

    Purse* clone() const noexcept final { return new Purse(*this); }
    OTSymmetricKey generate_key(OTPassword& password) const;

    void apply_times(const Token& token);
    void recalculate_times();

    Purse(
        const api::internal::Core& api,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type);
    Purse(
        const api::internal::Core& api,
        const VersionNumber version,
        const proto::CashType type,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const proto::PurseType state,
        const Amount totalValue,
        const Time validFrom,
        const Time validTo,
        const std::vector<OTToken>& tokens,
        const std::shared_ptr<OTSymmetricKey> primary,
        const std::vector<proto::Envelope>& primaryPasswords,
        const std::shared_ptr<const OTSymmetricKey> secondaryKey,
        const std::shared_ptr<const OTEnvelope> secondaryEncrypted,
        const std::shared_ptr<const OTPassword> secondaryKeyPassword);
    Purse(const api::internal::Core& api, const Purse& owner);
    Purse(const api::internal::Core& api, const proto::Purse& serialized);
    Purse() = delete;
    Purse(const Purse&);
    Purse(Purse&&) = delete;
    Purse& operator=(const Purse&) = delete;
    Purse& operator=(Purse&&) = delete;
};
}  // namespace opentxs::blind::implementation
