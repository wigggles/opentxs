// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "OpenSSL_BIO.hpp"

#include "opentxs/core/Log.hpp"

namespace opentxs::crypto::implementation
{
// OpenSSL_BIO
BIO* OpenSSL_BIO::assertBioNotNull(BIO* pBIO)
{
    if (nullptr == pBIO) OT_FAIL;
    return pBIO;
}

OpenSSL_BIO::OpenSSL_BIO(BIO* pBIO)
    : m_refBIO(*assertBioNotNull(pBIO))
    , bCleanup(true)
    , bFreeOnly(false)
{
}

OpenSSL_BIO::~OpenSSL_BIO()
{
    if (bCleanup) {
        if (bFreeOnly) {
            BIO_free(&m_refBIO);
        } else {
            BIO_free_all(&m_refBIO);
        }
    }
}

OpenSSL_BIO::operator BIO*() const { return (&m_refBIO); }

void OpenSSL_BIO::release() { bCleanup = false; }

void OpenSSL_BIO::setFreeOnly() { bFreeOnly = true; }
}  // namespace opentxs::crypto::implementation
