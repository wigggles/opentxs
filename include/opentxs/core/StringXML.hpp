// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_STRINGXML_HPP
#define OPENTXS_CORE_STRINGXML_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/String.hpp"

#include <cstdint>

namespace irr
{
namespace io
{
class IFileReadCallBack;
}
}  // namespace irr

namespace opentxs
{
class StringXML : virtual public String
{
public:
    EXPORT static OTStringXML Factory();
    EXPORT static OTStringXML Factory(const String& value);

    EXPORT virtual operator irr::io::IFileReadCallBack*() = 0;

    EXPORT virtual std::int32_t read(
        void* buffer,
        std::uint32_t sizeToRead) = 0;
    EXPORT virtual std::int32_t getSize() = 0;

    EXPORT virtual StringXML& operator=(const String& rhs) = 0;
    EXPORT virtual StringXML& operator=(const StringXML& rhs) = 0;

    EXPORT virtual ~StringXML() = default;

protected:
    StringXML() = default;

private:
    StringXML(StringXML&&) = delete;
    StringXML& operator=(StringXML&&) = delete;
};
}  // namespace opentxs
#endif
