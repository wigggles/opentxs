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

#ifndef OPENTXS_EXT_HELPERS_HPP
#define OPENTXS_EXT_HELPERS_HPP

#include <opentxs/core/Log.hpp>
#include <string>
#include <iostream>

// Reads from cin until Newline.
inline std::string OT_CLI_ReadLine()
{
    std::string line;
    if (std::getline(std::cin, line)) {
        return line;
    }

    return "";
}

// Reads from cin until EOF. (Or until the ~ character as the first character on
// a line.)
inline std::string OT_CLI_ReadUntilEOF()
{
    std::string result("");

    for (;;) {
        std::string input_line("");
        if (std::getline(std::cin, input_line, '\n')) {
            input_line += "\n";

            if (input_line[0] == '~') // This is our special "break" character
                                      // for multi-line input.
                break;

            result += input_line;
        }
        else {
            opentxs::otErr << "OT_CLI_ReadUntilEOF: getline() was unable to "
                              "read a string from std::cin\n";
            break;
        }
        if (std::cin.eof() || std::cin.fail() || std::cin.bad()) {
            std::cin.clear();
            break;
        }
    }

    return result;
}

#endif // OPENTXS_EXT_HELPERS_HPP
