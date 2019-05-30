// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_UNITDEFINITION_HPP
#define OPENTXS_CORE_CONTRACT_UNITDEFINITION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
class UnitDefinition : public Signable
{
private:
    typedef Signable ot_super;

    std::string primary_unit_name_;
    std::string short_name_;

    proto::UnitDefinition contract(const Lock& lock) const;
    OTIdentifier GetID(const Lock& lock) const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature,
        const PasswordPrompt& reason) const override;

protected:
    const api::Wallet& wallet_;
    std::string primary_unit_symbol_;

    static OTIdentifier GetID(const proto::UnitDefinition& contract);

    virtual proto::UnitDefinition IDVersion(const Lock& lock) const;
    virtual proto::UnitDefinition SigVersion(const Lock& lock) const;
    bool validate(const Lock& lock, const PasswordPrompt& reason)
        const override;

    bool update_signature(const Lock& lock, const PasswordPrompt& reason)
        override;

    UnitDefinition(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);
    UnitDefinition(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms);

public:
    EXPORT static UnitDefinition* Create(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const PasswordPrompt& reason);
    EXPORT static UnitDefinition* Create(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight);
    EXPORT static UnitDefinition* Create(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const PasswordPrompt& reason);
    EXPORT static UnitDefinition* Factory(
        const api::Wallet& wallet,
        const Nym_p& nym,
        const proto::UnitDefinition& serialized,
        const PasswordPrompt& reason);

    // Some instrument definitions keep a list of "user" accounts (the
    // complete set of
    // that type.)
    // This is called when the user creates a new asset account, in order to add
    // it to that list.
    // (Currently only operational for "shares", not "currencies", since it's
    // used exclusively
    // for the payment of dividends.)

    // adds the account to the list. (When account is created.)
    EXPORT bool AddAccountRecord(
        const std::string& dataFolder,
        const Account& theAccount) const;

    // removes the account from the list. (When account is deleted.)
    EXPORT bool EraseAccountRecord(
        const std::string& dataFolder,
        const Identifier& theAcctID) const;

    EXPORT bool VisitAccountRecords(
        const std::string& dataFolder,
        AccountVisitor& visitor,
        const PasswordPrompt& reason) const;

    EXPORT static std::string formatLongAmount(
        std::int64_t lValue,
        std::int32_t nFactor = 100,
        std::int32_t nPower = 2,
        const char* szCurrencySymbol = "",
        const char* szThousandSeparator = ",",
        const char* szDecimalPoint = ".");
    EXPORT static bool ParseFormatted(
        std::int64_t& lResult,
        const std::string& str_input,
        std::int32_t nFactor = 100,
        std::int32_t nPower = 2,
        const char* szThousandSeparator = ",",
        const char* szDecimalPoint = ".");
    EXPORT const std::string& GetCurrencyName() const
    {
        return primary_unit_name_;
    }
    EXPORT const std::string& GetCurrencySymbol() const
    {
        return primary_unit_symbol_;
    }

    EXPORT virtual bool DisplayStatistics(String& strContents) const;
    EXPORT proto::UnitDefinition Contract() const;
    EXPORT bool FormatAmountLocale(
        std::int64_t amount,
        std::string& str_output,
        const std::string& str_thousand,
        const std::string& str_decimal) const;
    EXPORT bool FormatAmountWithoutSymbolLocale(
        std::int64_t amount,
        std::string& str_output,
        const std::string& str_thousand,
        const std::string& str_decimal) const;
    EXPORT bool StringToAmountLocale(
        std::int64_t& amount,
        const std::string& str_input,
        const std::string& str_thousand,
        const std::string& str_decimal) const;
    EXPORT OTData Serialize() const override;
    EXPORT std::string Name() const override { return short_name_; }
    EXPORT proto::UnitDefinition PublicContract() const;
    EXPORT virtual std::int32_t DecimalPower() const { return 0; }
    EXPORT virtual std::string FractionalUnitName() const { return ""; }
    EXPORT virtual std::string TLA() const { return short_name_; }

    EXPORT virtual proto::UnitType Type() const = 0;

    EXPORT void SetAlias(const std::string& alias) override;

    EXPORT virtual ~UnitDefinition() = default;
};

}  // namespace opentxs

#endif
