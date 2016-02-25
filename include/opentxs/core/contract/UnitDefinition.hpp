/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_OTASSETCONTRACT_HPP
#define OPENTXS_CORE_OTASSETCONTRACT_HPP

#include <opentxs-proto/verify/VerifyContracts.hpp>

#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/contract/Signable.hpp"

namespace opentxs
{

class Account;
class AccountVisitor;
class Basket;
class Identifier;
class String;
class Tag;

class UnitDefinition : public Signable
{
private:
    typedef Signable ot_super;

    String alias_;
    String primary_unit_name_;
    String short_name_;

    EXPORT Identifier GetID() const override;

protected:
    // shares only:
    String m_strIssueDate;
    String primary_unit_symbol_;

    EXPORT static Identifier GetID(const proto::UnitDefinition& contract);

    EXPORT UnitDefinition(const proto::UnitDefinition serialized);
    EXPORT UnitDefinition(
        const Nym& nym,
        const String& shortname,
        const String& name,
        const String& symbol,
        const String& terms);

    EXPORT virtual proto::UnitDefinition IDVersion() const;
    EXPORT virtual proto::UnitDefinition SigVersion() const;

public:
    EXPORT static UnitDefinition* Create(
        const Nym& nym,
        const String& shortname,
        const String& name,
        const String& symbol,
        const String& terms,
        const String& tla,
        const uint32_t& factor,
        const uint32_t& power,
        const String& fraction);
    EXPORT static UnitDefinition* Create(
        const Nym& nym,
        const String& shortname,
        const String& name,
        const String& symbol,
        const String& terms,
        const uint64_t weight);
    EXPORT static UnitDefinition* Factory(
        const proto::UnitDefinition& serialized);

    EXPORT String Alias() { return alias_; }
    EXPORT void SetAlias(String alias) { alias_ = alias;}

    // Some instrument definitions keep a list of "user" accounts (the
    // complete set of
    // that type.)
    // This is called when the user creates a new asset account, in order to add
    // it to that list.
    // (Currently only operational for "shares", not "currencies", since it's
    // used exclusively
    // for the payment of dividends.)

    // adds the account to the list. (When account is created.)
    EXPORT bool AddAccountRecord(const Account& theAccount) const;

    // removes the account from the list. (When account is deleted.)
    EXPORT bool EraseAccountRecord(const Identifier& theAcctID) const;

    EXPORT bool VisitAccountRecords(AccountVisitor& visitor) const;

    EXPORT static std::string formatLongAmount(
        int64_t lValue, int32_t nFactor = 100, int32_t nPower = 2,
        const char* szCurrencySymbol = "",
        const char* szThousandSeparator = ",",
        const char* szDecimalPoint = ".");
    EXPORT static bool ParseFormatted(int64_t& lResult,
                                      const std::string& str_input,
                                      int32_t nFactor = 100, int32_t nPower = 2,
                                      const char* szThousandSeparator = ",",
                                      const char* szDecimalPoint = ".");
    EXPORT const String& GetCurrencyName() const
    {
        return primary_unit_name_;
    }
    EXPORT const String& GetCurrencySymbol() const
    {
        return primary_unit_symbol_;
    }

    EXPORT virtual bool SaveContractWallet(Tag& parent) const;
    EXPORT virtual bool DisplayStatistics(String& strContents) const;
    EXPORT const proto::UnitDefinition Contract() const;
    EXPORT OTData Serialize() const override;
    EXPORT bool Save() const override;
    EXPORT bool Validate() const override;
    EXPORT String Name() const override { return short_name_; }
    EXPORT const proto::UnitDefinition PublicContract() const;

    EXPORT virtual proto::UnitType Type() const = 0;

    EXPORT virtual ~UnitDefinition() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTASSETCONTRACT_HPP
