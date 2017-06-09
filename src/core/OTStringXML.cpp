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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/OTStringXML.hpp"

#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>

namespace opentxs
{

class OTStringXML::OTStringXMLPvt : public irr::io::IFileReadCallBack
{

public:
    OTStringXMLPvt(OTStringXML* ptr)
        : super(ptr)
    {
    }

    OTStringXML* super;

    int read(void* buffer, unsigned sizeToRead)
    {
        return super->read(buffer, sizeToRead);
    }

    int getSize()
    {
        return super->getSize();
    }
};

OTStringXML::OTStringXML()
    : String()
    , pvt_(new OTStringXMLPvt(this))
{
}

OTStringXML::OTStringXML(const String& value)
    : String(value)
    , pvt_(new OTStringXMLPvt(this))
{
}

OTStringXML::OTStringXML(const OTStringXML& value)
    : String(value)
    , pvt_(new OTStringXMLPvt(this))
{
}

OTStringXML& OTStringXML::operator=(const String& rhs)
{
    if ((&rhs) != (&(dynamic_cast<const String&>(*this)))) {
        String::operator=(rhs);
    }
    return *this;
}

OTStringXML& OTStringXML::operator=(const OTStringXML& rhs)
{
    if ((&rhs) != this) {
        String::operator=(dynamic_cast<const String&>(rhs));
    }
    return *this;
}

OTStringXML::~OTStringXML()
{
    delete pvt_;
}

OTStringXML::operator irr::io::IFileReadCallBack*()
{
    return pvt_;
}

int32_t OTStringXML::read(void* buffer, uint32_t sizeToRead)
{
    if (buffer && sizeToRead && Exists()) {
        char* pBuf = static_cast<char*>(buffer);

        int32_t nBytesToCopy =
            (sizeToRead > GetLength() ? GetLength() : sizeToRead);
        int32_t i;
        for (i = 0; i < nBytesToCopy; i++) {
            pBuf[i] = sgetc();
        }
        return i;
    }
    else {
        return 0;
    }
}

int32_t OTStringXML::getSize()
{
    return GetLength();
}

} // namespace opentxs
