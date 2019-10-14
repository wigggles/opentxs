// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blind::implementation
{
class Purse final : virtual public blind::Purse
{
public:
    const Token& at(const std::size_t position) const override
    {
        return tokens_.at(position);
    }
    const_iterator begin() const noexcept override { return cbegin(); }
    const_iterator cbegin() const noexcept override
    {
        return const_iterator(this, 0);
    }
    const_iterator cend() const noexcept override
    {
        return const_iterator(this, tokens_.size());
    }
    Time EarliestValidTo() const override { return earliest_valid_to_; }
    const_iterator end() const noexcept override { return cend(); }
    bool IsUnlocked() const override { return unlocked_; }
    Time LatestValidFrom() const override { return latest_valid_from_; }
    const identifier::Server& Notary() const override { return notary_; }
    bool Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) override;
    proto::Purse Serialize() const override;
    std::size_t size() const noexcept override { return tokens_.size(); }
    proto::PurseType State() const override { return state_; }
    proto::CashType Type() const override { return type_; }
    const identifier::UnitDefinition& Unit() const override { return unit_; }
    bool Unlock(const identity::Nym& nym, const PasswordPrompt& reason)
        const override;
    bool Verify(const api::server::internal::Manager& server) const override;
    Amount Value() const override { return total_value_; }

    bool AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
        override;
    Token& at(const std::size_t position) override
    {
        return tokens_.at(position);
    }
    iterator begin() noexcept override { return iterator(this, 0); }
    iterator end() noexcept override { return iterator(this, tokens_.size()); }
    bool GeneratePrototokens(
        const identity::Nym& owner,
        const Mint& mint,
        const Amount amount,
        const PasswordPrompt& reason);
    crypto::key::Symmetric& PrimaryKey() override;
    std::shared_ptr<Token> Pop() override;
    bool Push(std::shared_ptr<Token> token, const PasswordPrompt& reason)
        override;
    crypto::key::Symmetric& SecondaryKey(const identity::Nym& owner) override;

    ~Purse() override = default;

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
    std::vector<proto::SessionKey> primary_passwords_;
    OTPassword secondary_key_password_;
    std::shared_ptr<OTSymmetricKey> secondary_;
    std::shared_ptr<proto::Ciphertext> secondary_password_;

    static std::vector<proto::SessionKey> get_passwords(const proto::Purse& in);

    Purse* clone() const noexcept override { return new Purse(*this); }
    OTSymmetricKey generate_key(OTPassword& password) const;

    void apply_times(const Token& token);
    void recalculate_times();

    Purse(
        const api::internal::Core& api,
        const identifier::Nym& owner,
        const identifier::Server& server,
        const proto::CashType type,
        const Mint& mint);
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
        const std::vector<proto::SessionKey>& primaryPasswords,
        const std::shared_ptr<OTSymmetricKey> secondary,
        const std::shared_ptr<proto::Ciphertext> secondaryPassword);
    Purse(const api::internal::Core& api, const Purse& owner);
    Purse(const api::internal::Core& api, const proto::Purse& serialized);
    Purse() = delete;
    Purse(const Purse&);
    Purse(Purse&&) = delete;
    Purse& operator=(const Purse&) = delete;
    Purse& operator=(Purse&&) = delete;
};
}  // namespace opentxs::blind::implementation
