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

#ifndef OPENTXS_CASH_DIGITALCASH_HPP
#define OPENTXS_CASH_DIGITALCASH_HPP

// WHICH DIGITAL CASH LIBRARY?
//
// Many algorithms may come available. We are currently using Lucre, by Ben
// Laurie,
// which is an implementation of Wagner, which is a variant of Chaum.
//
// We plan to have alternatives such as "Magic Money" by Pr0duct Cypher.
//
// Implementations for Chaum and Brands are circulating online. They could all
// be easily added here as options for Open-Transactions.

#ifdef OT_CASH_USING_LUCRE
// IWYU pragma: begin_exports
#include <lucre/bank.h>
// IWYU pragma: end_exports
#endif

#ifdef OT_CASH_USING_MAGIC_MONEY
#include... // someday
#endif
#include <string>

namespace opentxs
{

#ifdef OT_CASH_USING_LUCRE

class LucreDumper
{
    std::string m_str_dumpfile;

public:
    LucreDumper();
    ~LucreDumper();
};

#endif

#ifdef OT_CASH_USING_MAGIC_MONEY

// Todo:  Someday...

#endif

} // namespace opentxs

#endif // OPENTXS_CASH_DIGITALCASH_HPP
