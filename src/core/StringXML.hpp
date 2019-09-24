// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/StringXML.hpp"

namespace opentxs::implementation
{
class StringXML final : virtual public opentxs::StringXML, public String
{
public:
    operator irr::io::IFileReadCallBack*() final;

    std::int32_t read(void* buffer, std::uint32_t sizeToRead) final;
    std::int32_t getSize() final;

    opentxs::StringXML& operator=(const opentxs::String& rhs) final;
    opentxs::StringXML& operator=(const opentxs::StringXML& rhs) final;

    ~StringXML() final;

private:
    friend opentxs::StringXML;

    class StringXMLPvt;

    StringXMLPvt* const pvt_;

    StringXML(const opentxs::String& value);
    StringXML();
    StringXML(const StringXML& value);
};
}  // namespace opentxs::implementation
