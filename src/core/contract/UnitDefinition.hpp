// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/contract/UnitDefinition.hpp"

#include "core/contract/Signable.hpp"

#include <map>

namespace opentxs::contract::implementation
{
class Unit : virtual public contract::Unit,
             public opentxs::contract::implementation::Signable
{
public:
    static const std::map<VersionNumber, VersionNumber>
        unit_of_account_version_map_;

    static OTIdentifier GetID(
        const api::internal::Core& api,
        const SerializedType& contract);

    bool AddAccountRecord(
        const std::string& dataFolder,
        const Account& theAccount) const override;
    SerializedType Contract() const override;
    std::int32_t DecimalPower() const override { return 0; }
    bool DisplayStatistics(String& strContents) const override;
    bool EraseAccountRecord(
        const std::string& dataFolder,
        const Identifier& theAcctID) const override;
    bool FormatAmountLocale(
        std::int64_t amount,
        std::string& str_output,
        const std::string& str_thousand,
        const std::string& str_decimal) const override;
    bool FormatAmountWithoutSymbolLocale(
        std::int64_t amount,
        std::string& str_output,
        const std::string& str_thousand,
        const std::string& str_decimal) const override;
    std::string FractionalUnitName() const override { return ""; }
    const std::string& GetCurrencyName() const override
    {
        return primary_unit_name_;
    }
    const std::string& GetCurrencySymbol() const override
    {
        return primary_unit_symbol_;
    }
    std::string Name() const override { return short_name_; }
    SerializedType PublicContract() const override;
    OTData Serialize() const override;
    bool StringToAmountLocale(
        std::int64_t& amount,
        const std::string& str_input,
        const std::string& str_thousand,
        const std::string& str_decimal) const override;
    std::string TLA() const override { return short_name_; }
    proto::UnitType Type() const override = 0;
    proto::ContactItemType UnitOfAccount() const override
    {
        return unit_of_account_;
    }
    bool VisitAccountRecords(
        const std::string& dataFolder,
        AccountVisitor& visitor,
        const PasswordPrompt& reason) const override;

    void InitAlias(const std::string& alias) final
    {
        contract::implementation::Signable::SetAlias(alias);
    }
    void SetAlias(const std::string& alias) override;

    ~Unit() override = default;

protected:
    const std::string primary_unit_symbol_;
    const proto::ContactItemType unit_of_account_;

    virtual SerializedType IDVersion(const Lock& lock) const;
    virtual SerializedType SigVersion(const Lock& lock) const;
    bool validate(const Lock& lock, const PasswordPrompt& reason)
        const override;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason)
        override;

    Unit(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version);
    Unit(
        const api::internal::Core& api,
        const Nym_p& nym,
        const SerializedType serialized);
    Unit(const Unit&);

private:
    friend opentxs::Factory;

    const std::string primary_unit_name_;
    const std::string short_name_;

    SerializedType contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature,
        const PasswordPrompt& reason) const override;

    Unit(Unit&&) = delete;
    Unit& operator=(const Unit&) = delete;
    Unit& operator=(Unit&&) = delete;
};
}  // namespace opentxs::contract::implementation
