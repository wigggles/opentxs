// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMPLEMENTATION_OPENTXS_API_LEGACY_HPP
#define IMPLEMENTATION_OPENTXS_API_LEGACY_HPP

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Legacy final : virtual public api::Legacy
{
public:
    std::string ClientConfigFilePath() const override;
    std::string ClientDataFolder() const override;
    std::string ConfigFilePath() const override;
    std::string DataFolderPath() const override;
    std::string ServerConfigFilePath() const override;
    std::string ServerDataFolder() const override;

    ~Legacy() = default;

private:
    friend Factory;

    const bool client_{true};
    const std::string client_data_folder_{""};
    const std::string server_data_folder_{""};
    const std::string client_config_file_{""};
    const std::string server_config_file_{""};

    std::string get_path(const std::string& fragment) const;
    std::string get_file(const std::string& fragment) const;

    Legacy(const std::string& key);
    Legacy() = delete;
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    Legacy& operator=(const Legacy&) = delete;
    Legacy& operator=(Legacy&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // IMPLEMENTATION_OPENTXS_API_LEGACY_HPP
