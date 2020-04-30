// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "core/StringXML.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>

#include "core/String.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
OTStringXML StringXML::Factory()
{
    return OTStringXML(new implementation::StringXML());
}

OTStringXML StringXML::Factory(const String& value)
{
    return OTStringXML(new implementation::StringXML(value));
}
}  // namespace opentxs

namespace opentxs::implementation
{
class StringXML::StringXMLPvt : public irr::io::IFileReadCallBack
{
public:
    StringXMLPvt(StringXML* ptr)
        : super(ptr)
    {
    }

    StringXML* super;

    int read(void* buffer, unsigned sizeToRead)
    {
        return super->read(buffer, sizeToRead);
    }

    int getSize() { return super->getSize(); }

private:
    StringXMLPvt(const StringXMLPvt&) = delete;
    StringXMLPvt(StringXMLPvt&&) = delete;
    StringXMLPvt& operator=(const StringXMLPvt&) = delete;
    StringXMLPvt& operator=(StringXMLPvt&&) = delete;
};

StringXML::StringXML()
    : opentxs::String()
    , opentxs::StringXML()
    , String()
    , pvt_(new StringXMLPvt(this))
{
}

StringXML::StringXML(const opentxs::String& value)
    : opentxs::String()
    , opentxs::StringXML()
    , String(value.Get())
    , pvt_(new StringXMLPvt(this))
{
}

StringXML::StringXML(const StringXML& value)
    : opentxs::String()
    , opentxs::StringXML()
    , String(value)
    , pvt_(new StringXMLPvt(this))
{
    Set(value.Get());
}

StringXML& StringXML::operator=(const opentxs::String& rhs)
{
    if ((&rhs) != (&(dynamic_cast<const opentxs::String&>(*this)))) {
        String::operator=(dynamic_cast<const String&>(rhs));
    }

    return *this;
}

StringXML& StringXML::operator=(const opentxs::StringXML& rhs)
{
    if ((&rhs) != this) { String::operator=(dynamic_cast<const String&>(rhs)); }

    return *this;
}

StringXML::operator irr::io::IFileReadCallBack*() { return pvt_; }

std::int32_t StringXML::read(void* buffer, std::uint32_t sizeToRead)
{
    if (buffer && sizeToRead && Exists()) {
        char* pBuf = static_cast<char*>(buffer);

        std::int32_t nBytesToCopy =
            (sizeToRead > GetLength() ? GetLength() : sizeToRead);
        std::int32_t i;
        for (i = 0; i < nBytesToCopy; i++) { pBuf[i] = sgetc(); }
        return i;
    } else {
        return 0;
    }
}

std::int32_t StringXML::getSize() { return GetLength(); }

StringXML::~StringXML() { delete pvt_; }
}  // namespace opentxs::implementation
