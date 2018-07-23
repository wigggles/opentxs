/// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Legacy.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/String.hpp"

#include "Legacy.hpp"

#define CLIENT_CONFIG_KEY "client"
#define SERVER_CONFIG_KEY "server"
#define DATA_FOLDER_EXT "_data"
#define CONFIG_FILE_EXT ".cfg"

//#define OT_METHOD "opentxs::Legacy::"

namespace opentxs
{
api::Legacy* Factory::Legacy(const std::string& key)
{
    return new api::implementation::Legacy(key);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Legacy::Legacy(const std::string& key)
    : client_{key == CLIENT_CONFIG_KEY}
    , client_data_folder_{std::string(CLIENT_CONFIG_KEY) + DATA_FOLDER_EXT}
    , server_data_folder_{std::string(SERVER_CONFIG_KEY) + DATA_FOLDER_EXT}
    , client_config_file_{std::string(CLIENT_CONFIG_KEY) + CONFIG_FILE_EXT}
    , server_config_file_{std::string(SERVER_CONFIG_KEY) + CONFIG_FILE_EXT}
{
}

std::string Legacy::ClientConfigFilePath() const
{
    return get_file(client_config_file_);
}

std::string Legacy::ClientDataFolder() const
{
    return get_path(client_data_folder_);
}

std::string Legacy::ConfigFilePath() const
{
    return client_ ? ClientConfigFilePath() : ServerConfigFilePath();
}

std::string Legacy::DataFolderPath() const
{
    return client_ ? ClientDataFolder() : ServerDataFolder();
}

std::string Legacy::get_file(const std::string& fragment) const
{
    const auto output = get_path(fragment);

    return {output.c_str(), output.size() - 1};
}

std::string Legacy::get_path(const std::string& fragment) const
{
    String output{""};
    const auto success = OTPaths::AppendFolder(
        output, OTPaths::AppDataFolder(), fragment.c_str());

    OT_ASSERT(success)

    return output.Get();
}

std::string Legacy::ServerConfigFilePath() const
{
    return get_file(server_config_file_);
}

std::string Legacy::ServerDataFolder() const
{
    return get_path(server_data_folder_);
}
}  // namespace opentxs::api::implementation
