// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SERVER_MAINFILE_HPP
#define OPENTXS_SERVER_MAINFILE_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs
{
class String;

namespace api
{
class Crypto;
class Server;

namespace client
{
class Wallet;
}  // namespace client
}  // namespace api

namespace server
{

class Server;

class MainFile
{
public:
    explicit MainFile(
        Server& server,
        const opentxs::api::Crypto& crypto_,
        const opentxs::api::Legacy& legacy,
        const opentxs::api::Wallet& wallet_);

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
    const opentxs::api::Legacy& legacy_;
    const opentxs::api::Crypto& crypto_;
    const opentxs::api::Wallet& wallet_;
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
