// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Legacy final : virtual public api::Legacy
{
public:
    std::string ClientConfigFilePath(const int instance) const final;
    std::string ClientDataFolder(const int instance) const final;
    std::string CryptoConfigFilePath() const final;
    std::string LogConfigFilePath() const final;
    std::string PIDFilePath() const final;
    std::string ServerConfigFilePath(const int instance) const final;
    std::string ServerDataFolder(const int instance) const final;

    ~Legacy() final = default;

private:
    friend opentxs::Factory;

    const std::string client_data_folder_{""};
    const std::string server_data_folder_{""};
    const std::string client_config_file_{""};
    const std::string crypto_config_file_{""};
    const std::string log_config_file_{""};
    const std::string server_config_file_{""};
    const std::string pid_file_{""};

    std::string get_path(const std::string& fragment, const int instance = 0)
        const;
    std::string get_file(const std::string& fragment, const int instance = 0)
        const;

    Legacy();
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    Legacy& operator=(const Legacy&) = delete;
    Legacy& operator=(Legacy&&) = delete;
};
}  // namespace opentxs::api::implementation
