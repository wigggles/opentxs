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

#ifndef OPENTXS_CORE_CURRENCYCONTRACT_HPP
#define OPENTXS_CORE_CURRENCYCONTRACT_HPP

#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{

class Nym;

class CurrencyContract : public UnitDefinition
{
private:
    typedef UnitDefinition ot_super;
    friend ot_super;

    // ISO-4217. E.g., USD, AUG, PSE. Take as hint, not as contract.
    String tla_;
    // A dollar is 100 cents. Therefore factor == 100.
    uint32_t factor_;
    // If value is 103, decimal power of 0 displays 103 (actual value.) Whereas
    // decimal power of 2 displays 1.03 and 4 displays .0103
    uint32_t power_;
    // "cents"
    String fractional_unit_name_;

    EXPORT CurrencyContract(
        const ConstNym& nym,
        const proto::UnitDefinition serialized);
    EXPORT CurrencyContract(
        const ConstNym& nym,
        const String& shortname,
        const String& name,
        const String& symbol,
        const String& terms,
        const String& tla,
        const uint32_t& factor,
        const uint32_t& power,
        const String& fraction);

    EXPORT proto::UnitDefinition IDVersion() const override;

public:
    EXPORT proto::UnitType Type() const override
    {
        return proto::UNITTYPE_CURRENCY;
    }

    EXPORT int32_t GetCurrencyDecimalPower() const
    {
        return power_;
    }
    EXPORT int32_t GetCurrencyFactor() const
    {
        return factor_;
    }
    EXPORT const String& GetCurrencyFraction() const
    {
        return fractional_unit_name_;
    } // "cents"    (for example)
    EXPORT const String& GetCurrencyTLA() const
    {
        return tla_;
    } // "USD""     (for example)

    EXPORT bool FormatAmountLocale(int64_t amount, std::string& str_output,
                                   const std::string& str_thousand,
                                   const std::string& str_decimal) const;
    EXPORT bool FormatAmountWithoutSymbolLocale(
        int64_t amount, std::string& str_output,
        const std::string& str_thousand, const std::string& str_decimal) const;
    EXPORT bool StringToAmountLocale(int64_t& amount,
                                     const std::string& str_input,
                                     const std::string& str_thousand,
                                     const std::string& str_decimal) const;

    virtual ~CurrencyContract() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CURRENCYCONTRACT_HPP
