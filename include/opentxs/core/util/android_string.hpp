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

#ifndef OPENTXS_CORE_UTIL_ANDROID_STRING_HPP
#define OPENTXS_CORE_UTIL_ANDROID_STRING_HPP

#include "opentxs/Forward.hpp"

#ifdef ANDROID

#include <cstdlib>
#include <sstream>

namespace std
{
inline long int stoll(const string& s) { return atoll(s.c_str()); }

inline long int stol(const string& s) { return atol(s.c_str()); }

inline int stoi(const string& s) { return atoi(s.c_str()); }

template <typename T>
string to_string(T value)
{
    ostringstream os;
    os << value;
    return os.str();
}
}

#endif  // ANDROID
#endif  // OPENTXS_CORE_COMMON_HPP
