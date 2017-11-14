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

#ifndef OPENTXS_SERVER_MAINFILE_HPP
#define OPENTXS_SERVER_MAINFILE_HPP

#include "opentxs/Version.hpp"

#include <string>

namespace opentxs
{
class CryptoEngine;
class String;

namespace api
{
class Server;
class Wallet;
}  // namespace api

namespace server
{

class Server;

class MainFile
{
public:
    explicit MainFile(
        Server& server,
        opentxs::CryptoEngine& crypto_,
        opentxs::api::Wallet& wallet_);

    bool CreateMainFile(
        const std::string& strContract,
        const std::string& strNotaryID,
        const std::string& strCert,
        const std::string& strNymID,
        const std::string& strCachedKey);
    bool LoadMainFile(bool readOnly = false);
    bool LoadServerUserAndContract();
    bool SaveMainFile();
    bool SaveMainFileToString(String& filename);

private:
    Server& server_;  // TODO: remove when feasible
    opentxs::CryptoEngine& crypto_;
    opentxs::api::Wallet& wallet_;
    std::string version_;

    MainFile() = delete;
    MainFile(const MainFile&) = delete;
    MainFile(MainFile&&) = delete;
    MainFile& operator=(const MainFile&) = delete;
    MainFile& operator=(MainFile&&) = delete;
};
}  // namespace server
}  // namespace opentxs

#endif  // OPENTXS_SERVER_MAINFILE_HPP
