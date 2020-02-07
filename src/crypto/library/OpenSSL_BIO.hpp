// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/String.hpp"

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

    static BIO* assertBioNotNull(BIO* pBIO);

    void read_bio(
        const std::size_t amount,
        std::size_t& read,
        std::size_t& total,
        std::vector<std::byte>& output);

public:
    OpenSSL_BIO(BIO* pBIO);

    ~OpenSSL_BIO();

    operator BIO*() const;

    void release();
    void setFreeOnly();

    std::vector<std::byte> ToBytes();
    OTString ToString();
};
}  // namespace opentxs::crypto::implementation
