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

#ifndef OPENTXS_CORE_ASSERT_HPP
#define OPENTXS_CORE_ASSERT_HPP

#include <cstddef>
#include <exception>

#define OT_FAIL                                                                \
    {                                                                          \
        Assert::doAssert(__FILE__, __LINE__, nullptr);                         \
        std::terminate();                                                      \
    };
#define OT_FAIL_MSG(s)                                                         \
    {                                                                          \
        Assert::doAssert(__FILE__, __LINE__, (s));                             \
        std::terminate();                                                      \
    };

#if defined(__clang__)
#define OT_ASSERT(x)                                                           \
    if (false == static_cast<bool>(x)) {                                       \
        Assert::doAssert(__FILE__, __LINE__, nullptr);                         \
        std::terminate();                                                      \
    };
#else
#define OT_ASSERT(x)                                                           \
    if (false == (x)) {                                                        \
        Assert::doAssert(__FILE__, __LINE__, nullptr);                         \
        std::terminate();                                                      \
    };
#endif
#if defined(__clang__)
#define OT_ASSERT_MSG(x, s)                                                    \
    if (false == static_cast<bool>(x)) {                                       \
        Assert::doAssert(__FILE__, __LINE__, (s));                             \
        std::terminate();                                                      \
    };
#else
#define OT_ASSERT_MSG(x, s)                                                    \
    if (false == (x)) {                                                        \
        Assert::doAssert(__FILE__, __LINE__, (s));                             \
        std::terminate();                                                      \
    };
#endif

class Assert
{
public:
    typedef size_t(fpt_Assert_sz_n_sz)(const char*, size_t, const char*);

private:
    fpt_Assert_sz_n_sz* m_fpt_Assert;

    fpt_Assert_sz_n_sz(m_AssertDefault);

public:
    // if not null, must be deleted before changed.
    static Assert* s_pAssert;

    EXPORT Assert(fpt_Assert_sz_n_sz& fp1);

    EXPORT static fpt_Assert_sz_n_sz(doAssert); // assert
};

#endif // OPENTXS_CORE_ASSERT_HPP
