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

#ifndef OPENTXS_CORE_OTSTRINGXML_HPP
#define OPENTXS_CORE_OTSTRINGXML_HPP

#include "String.hpp"

namespace irr
{
namespace io
{
class IFileReadCallBack;
}
}

namespace opentxs
{

class OTStringXML : public String
{
public:
    EXPORT OTStringXML();
    EXPORT OTStringXML(const String& value);
    EXPORT OTStringXML(const OTStringXML& value);

    EXPORT virtual ~OTStringXML();

    EXPORT operator irr::io::IFileReadCallBack*();

    EXPORT OTStringXML& operator=(const String& rhs);
    EXPORT OTStringXML& operator=(const OTStringXML& rhs);

    int32_t read(void* buffer, uint32_t sizeToRead);
    int32_t getSize();

private:
    class OTStringXMLPvt;
    OTStringXMLPvt* const pvt_;
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTSTRINGXML_HPP
