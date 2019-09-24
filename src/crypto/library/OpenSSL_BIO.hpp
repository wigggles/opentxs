// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

extern "C" {
#include <openssl/bio.h>
}

namespace opentxs::crypto::implementation
{
class OpenSSL_BIO
{
private:
    BIO& m_refBIO;
    bool bCleanup;
    bool bFreeOnly;

    EXPORT static BIO* assertBioNotNull(BIO* pBIO);

    void read_bio(
        const std::size_t amount,
        std::size_t& read,
        std::size_t& total,
        std::vector<std::byte>& output);

public:
    EXPORT OpenSSL_BIO(BIO* pBIO);

    EXPORT ~OpenSSL_BIO();

    EXPORT operator BIO*() const;

    EXPORT void release();
    EXPORT void setFreeOnly();

    std::vector<std::byte> ToBytes();
    OTString ToString();
};
}  // namespace opentxs::crypto::implementation
