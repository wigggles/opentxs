// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_LEGACY_HPP
#define OPENTXS_API_LEGACY_HPP

#include "opentxs/Forward.hpp"

#include <string>

namespace opentxs
{
namespace api
{
class Legacy
{
public:
    EXPORT virtual std::string ClientConfigFilePath() const = 0;
    EXPORT virtual std::string ClientDataFolder() const = 0;
    EXPORT virtual std::string ConfigFilePath() const = 0;
    EXPORT virtual std::string DataFolderPath() const = 0;
    EXPORT virtual std::string ServerConfigFilePath() const = 0;
    EXPORT virtual std::string ServerDataFolder() const = 0;

    virtual ~Legacy() = default;

protected:
    Legacy() = default;

private:
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    Legacy& operator=(const Legacy&) = delete;
    Legacy& operator=(Legacy&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
