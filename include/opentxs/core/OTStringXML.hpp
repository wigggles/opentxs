// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_OTSTRINGXML_HPP
#define OPENTXS_CORE_OTSTRINGXML_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/String.hpp"

#include <cstdint>

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

    std::int32_t read(void* buffer, std::uint32_t sizeToRead);
    std::int32_t getSize();

private:
    class OTStringXMLPvt;

    OTStringXMLPvt* const pvt_;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTSTRINGXML_HPP
