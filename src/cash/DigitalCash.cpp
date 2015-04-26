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

#include <opentxs/core/stdafx.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/cash/DigitalCash.hpp>

#include <fstream>

namespace opentxs
{

#ifdef OT_CASH_USING_MAGIC_MONEY

// Todo:  Someday...

#endif

// Open-Transactions
// NOTE: review this in security audit...
// This is da2ce7's fix for the problems that appeared from removing
// Lucre from the OT source and linking it separately. (Without applink.c
// which causes cross-boundary issues with file handles.)
#ifdef _WIN32
#ifdef _DEBUG

void CleanupDumpFile(const char* filepathexact)
{
    std::fstream f(filepathexact, std::ios::in);

    if (f) {
        f.close();
        f.open(filepathexact, std::ios::out | std::ios::trunc);
        f.close();
        remove(filepathexact);
    }
}

void SetDumper(const char* filepathexact)
{
    // lets clear the last time we used this file.
    CleanupDumpFile(filepathexact);
    BIO* out = BIO_new_file(filepathexact, "w");
    assert(out);
    SetDumper(out);
}

#endif
#endif

#ifdef OT_CASH_USING_LUCRE

// We don't need this for release builds
LucreDumper::LucreDumper()
{
#ifdef _WIN32
#ifdef _DEBUG
    String strOpenSSLDumpFilename("openssl.dumpfile"), strOpenSSLDumpFilePath,
        strDataPath; // todo security. We shouldn't necessarily be dumping this
                     // info to file AT ALL.
    bool bGetDataFolderSuccess = OTDataFolder::Get(strDataPath);
    OT_ASSERT_MSG(bGetDataFolderSuccess,
                  "_OT_LucreDumper(): Failed to Get Data Path");
    bool bRelativeToCanonicalSuccess = OTPaths::RelativeToCanonical(
        strOpenSSLDumpFilePath, strDataPath, strOpenSSLDumpFilename);
    OT_ASSERT_MSG(bRelativeToCanonicalSuccess,
                  "_OT_LucreDumper(): Unable To Build Full Path");

    strOpenSSLDumpFilename.Set("");
    strDataPath.Set("");
    SetDumper(strOpenSSLDumpFilePath.Get()); // We are only dumping this way
                                             // currently as a temporary
                                             // solution to the applink.c
                                             // openssl thing that can cause
                                             // crashes in Lucre when
                                             // withdrawing cash. (Caused by
                                             // da2ce7 removing Lucre from OT
                                             // and moving it into a dylib.)
    m_str_dumpfile = strOpenSSLDumpFilePath.Get();
    strOpenSSLDumpFilePath.Set("");
#endif
#else
    SetDumper(stderr);
#endif
}

LucreDumper::~LucreDumper()
{
#ifdef _WIN32
#ifdef _DEBUG
    CleanupDumpFile(m_str_dumpfile.c_str());
#endif
#endif
}

#else // No digital cash lib is selected? Perhaps error message here?

#endif // Which digital cash library we're using.

} // namespace opentxs
