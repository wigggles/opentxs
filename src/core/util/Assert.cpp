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

#include "opentxs/core/util/Assert.hpp"

#include <iostream>
#include <cstring>

Assert* Assert::s_pAssert = new Assert(Assert::doAssert);

Assert::Assert(fpt_Assert_sz_n_sz& fp1)
    : m_fpt_Assert(fp1)
{
}

size_t Assert::m_AssertDefault(const char* filename, size_t linenumber,
                               const char* message)
{
    if (message) {
        if (std::strcmp(message, "") != 0) {
            std::cerr << message << "\n";
            std::cerr.flush();
        }
    }

    const char* file = filename ? filename : "nullptr";

    std::cerr << "OT_ASSERT in " << file << " at line " << linenumber << "\n";
    std::cerr.flush();

    return 0; // since we are not logging.
}

size_t Assert::doAssert(const char* filename, size_t linenumber,
                        const char* message)
{
    if (Assert::s_pAssert == nullptr) std::terminate();
    return Assert::s_pAssert->m_fpt_Assert(filename, linenumber, message);
}
