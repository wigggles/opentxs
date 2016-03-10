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
 *       -- Currency Currencies, Markets, Payment Plans.
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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/Nym.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/contract/CurrencyContract.hpp>

namespace opentxs
{

CurrencyContract::CurrencyContract(
    const ConstNym& nym,
    const proto::UnitDefinition serialized)
        : ot_super(nym, serialized)
        , tla_(serialized.currency().tla())
        , factor_(serialized.currency().factor())
        , power_(serialized.currency().power())
        , fractional_unit_name_(serialized.currency().fraction())
{
}

CurrencyContract::CurrencyContract(
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::string& tla,
    const uint32_t& factor,
    const uint32_t& power,
    const std::string& fraction)
        : ot_super(nym, shortname, name, symbol, terms)
{
    tla_ = tla;
    factor_ = factor;
    power_ = power;
    fractional_unit_name_ = fraction;
}

proto::UnitDefinition CurrencyContract::IDVersion() const
{
    proto::UnitDefinition contract = ot_super::IDVersion();

    contract.set_type(proto::UNITTYPE_CURRENCY);

    auto currency = contract.mutable_currency();
    currency->set_version(1);
    currency->set_tla(tla_);
    currency->set_factor(factor_);
    currency->set_power(power_);
    currency->set_fraction(fractional_unit_name_);

    return contract;
}

// Convert 912545 to "$9,125.45"
//
// (Assuming a Factor of 100, Decimal Power of 2, Currency Symbol of "$",
//  separator of "," and decimal point of ".")
//
bool CurrencyContract::FormatAmountLocale(int64_t amount, std::string& str_output,
                                       const std::string& str_thousand,
                                       const std::string& str_decimal) const
{
    // Lookup separator and decimal point symbols based on locale.

    // Get a moneypunct facet from the global ("C") locale
    //
    // NOTE: Turns out moneypunct kind of sucks.
    // As a result, for internationalization purposes,
    // these values have to be set here before compilation.
    //
    static String strSeparator(str_thousand.empty() ? OT_THOUSANDS_SEP
                                                    : str_thousand);
    static String strDecimalPoint(str_decimal.empty() ? OT_DECIMAL_POINT
                                                      : str_decimal);

    // NOTE: from web searching, I've determined that locale / moneypunct has
    // internationalization problems. Therefore it looks like if you want to
    // build OT for various languages / regions, you're just going to have to
    // edit stdafx.hpp and change the OT_THOUSANDS_SEP and OT_DECIMAL_POINT
    // variables.
    //
    // The best improvement I can think on that is to check locale and then use
    // it to choose from our own list of hardcoded values. Todo.

    str_output = UnitDefinition::formatLongAmount(
        amount, GetCurrencyFactor(), GetCurrencyDecimalPower(),
        primary_unit_symbol_.c_str(), strSeparator.Get(), strDecimalPoint.Get());
    return true; // Note: might want to return false if str_output is empty.
}

// Convert 912545 to "9,125.45"
//
// (Example assumes a Factor of 100, Decimal Power of 2
//  separator of "," and decimal point of ".")
//
bool CurrencyContract::FormatAmountWithoutSymbolLocale(
    int64_t amount, std::string& str_output, const std::string& str_thousand,
    const std::string& str_decimal) const
{
    // --------------------------------------------------------
    // Lookup separator and decimal point symbols based on locale.
    // --------------------------------------------------------
    // Get a moneypunct facet from the global ("C") locale
    //
    // NOTE: Turns out moneypunct kind of sucks.
    // As a result, for internationalization purposes,
    // these values have to be set here before compilation.
    //
    static String strSeparator(str_thousand.empty() ? OT_THOUSANDS_SEP
                                                    : str_thousand);
    static String strDecimalPoint(str_decimal.empty() ? OT_DECIMAL_POINT
                                                      : str_decimal);

    str_output = UnitDefinition::formatLongAmount(
        amount, GetCurrencyFactor(), GetCurrencyDecimalPower(), NULL,
        strSeparator.Get(), strDecimalPoint.Get());
    return true; // Note: might want to return false if str_output is empty.
}

// Convert "$9,125.45" to 912545.
//
// (Assuming a Factor of 100, Decimal Power of 2, separator of "," and decimal
// point of ".")
//
bool CurrencyContract::StringToAmountLocale(int64_t& amount,
                                         const std::string& str_input,
                                         const std::string& str_thousand,
                                         const std::string& str_decimal) const
{
    // Lookup separator and decimal point symbols based on locale.

    // Get a moneypunct facet from the global ("C") locale
    //

    // NOTE: from web searching, I've determined that locale / moneypunct has
    // internationalization problems. Therefore it looks like if you want to
    // build OT for various languages / regions, you're just going to have to
    // edit stdafx.hpp and change the OT_THOUSANDS_SEP and OT_DECIMAL_POINT
    // variables.
    //
    // The best improvement I can think on that is to check locale and then use
    // it to choose from our own list of hardcoded values. Todo.

    static String strSeparator(str_thousand.empty() ? OT_THOUSANDS_SEP
                                                    : str_thousand);
    static String strDecimalPoint(str_decimal.empty() ? OT_DECIMAL_POINT
                                                      : str_decimal);

    bool bSuccess = UnitDefinition::ParseFormatted(
        amount, str_input, GetCurrencyFactor(), GetCurrencyDecimalPower(),
        strSeparator.Get(), strDecimalPoint.Get());

    return bSuccess;
}

} // namespace opentxs
