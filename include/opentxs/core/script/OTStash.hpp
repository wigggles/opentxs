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

#ifndef OPENTXS_CORE_SCRIPT_OTSTASH_HPP
#define OPENTXS_CORE_SCRIPT_OTSTASH_HPP

#include <map>
#include <string>
#include <cstdint>

namespace irr
{
namespace io
{
template <class char_type, class super_class>
class IIrrXMLReader;
class IXMLBase;
typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader;
} // namespace io
} // namespace irr

namespace opentxs
{

class Identifier;
class OTStashItem;
class String;
class Tag;

typedef std::map<std::string, OTStashItem*> mapOfStashItems;

class OTStash
{
    std::string m_str_stash_name;

    mapOfStashItems m_mapStashItems; // map of stash items by instrument
                                     // definition ID.
                                     // owned.
public:
    const std::string GetName() const
    {
        return m_str_stash_name;
    }
    OTStashItem* GetStash(const std::string& str_instrument_definition_id);

    int64_t GetAmount(std::string str_instrument_definition_id);
    bool CreditStash(std::string str_instrument_definition_id,
                     const int64_t& lAmount);
    bool DebitStash(std::string str_instrument_definition_id,
                    const int64_t& lAmount);

    void Serialize(Tag& parent) const;
    int32_t ReadFromXMLNode(irr::io::IrrXMLReader*& xml,
                            const String& strStashName,
                            const String& strItemCount);

    OTStash();
    OTStash(std::string str_stash_name)
        : m_str_stash_name(str_stash_name)
    {
    }
    OTStash(const String& strInstrumentDefinitionID, int64_t lAmount = 0);
    OTStash(const Identifier& theInstrumentDefinitionID, int64_t lAmount = 0);
    virtual ~OTStash();
};

} // namespace opentxs

#endif // OPENTXS_CORE_SCRIPT_OTSTASH_HPP
