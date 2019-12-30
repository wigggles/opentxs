// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include <string>

namespace opentxs
{
namespace server
{
class MainFile
{
public:
    explicit MainFile(Server& server, const PasswordPrompt& reason);

    bool CreateMainFile(
        const std::string& strContract,
        const std::string& strNotaryID,
        const std::string& strNymID);
    bool LoadMainFile(bool readOnly = false);
    bool LoadServerUserAndContract();
    bool SaveMainFile();
    bool SaveMainFileToString(String& filename);

private:
    Server& server_;
    std::string version_;

    MainFile() = delete;
    MainFile(const MainFile&) = delete;
    MainFile(MainFile&&) = delete;
    MainFile& operator=(const MainFile&) = delete;
    MainFile& operator=(MainFile&&) = delete;
};
}  // namespace server
}  // namespace opentxs
