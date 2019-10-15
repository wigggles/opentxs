// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Legacy.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include "Legacy.hpp"

#define CLIENT_CONFIG_KEY "client"
#define CRYPTO_CONFIG_KEY "crypto"
#define LOG_CONFIG_KEY "log"
#define SERVER_CONFIG_KEY "server"
#define DATA_FOLDER_EXT "_data"
#define CONFIG_FILE_EXT ".cfg"
#define PID_FILE "opentxs.pid"

//#define OT_METHOD "opentxs::Legacy::"

namespace opentxs
{
api::Legacy* Factory::Legacy() { return new api::implementation::Legacy(); }
}  // namespace opentxs

namespace opentxs::api::implementation
{
const char* Legacy::account_{"account"};
const char* Legacy::cert_{"cert"};
const char* Legacy::common_{"common"};
const char* Legacy::contract_{"contract"};
const char* Legacy::cron_{"cron"};
const char* Legacy::expired_box_{"expiredbox"};
const char* Legacy::inbox_{"inbox"};
const char* Legacy::market_{"market"};
const char* Legacy::mint_{"mint"};
const char* Legacy::nym_{"nyms"};
const char* Legacy::nymbox_{"nymbox"};
const char* Legacy::outbox_{"outbox"};
const char* Legacy::payment_inbox_{"paymentinbox"};
const char* Legacy::purse_{"purse"};
const char* Legacy::receipt_{"receipt"};
const char* Legacy::record_box_{"recordbox"};
const char* Legacy::spent_{"spent"};
const char* Legacy::useracct_{"useracct"};

Legacy::Legacy() noexcept
    : client_data_folder_{std::string(CLIENT_CONFIG_KEY) + DATA_FOLDER_EXT}
    , server_data_folder_{std::string(SERVER_CONFIG_KEY) + DATA_FOLDER_EXT}
    , client_config_file_{std::string(CLIENT_CONFIG_KEY) + CONFIG_FILE_EXT}
    , crypto_config_file_{std::string(CRYPTO_CONFIG_KEY) + CONFIG_FILE_EXT}
    , log_config_file_{std::string(LOG_CONFIG_KEY) + CONFIG_FILE_EXT}
    , server_config_file_{std::string(SERVER_CONFIG_KEY) + CONFIG_FILE_EXT}
    , pid_file_{PID_FILE}
{
}

std::string Legacy::ClientConfigFilePath(const int instance) const noexcept
{
    return get_file(client_config_file_, instance);
}

std::string Legacy::ClientDataFolder(const int instance) const noexcept
{
    return get_path(client_data_folder_, instance);
}

std::string Legacy::CryptoConfigFilePath() const noexcept
{
    return get_file(crypto_config_file_);
}

std::string Legacy::get_file(const std::string& fragment, const int instance)
    const noexcept
{
    const auto output = get_path(fragment, instance);

    return {output.c_str(), output.size() - 1};
}

std::string Legacy::get_path(const std::string& fragment, const int instance)
    const noexcept
{
    const auto name =
        (0 == instance) ? fragment : fragment + "-" + std::to_string(instance);
    auto output = String::Factory();
    const auto success = OTPaths::AppendFolder(
        output, OTPaths::AppDataFolder(), String::Factory(name.c_str()));

    OT_ASSERT(success)

    return output->Get();
}

std::string Legacy::LogConfigFilePath() const noexcept
{
    return get_file(log_config_file_);
}

std::string Legacy::PIDFilePath() const noexcept { return get_file(pid_file_); }

std::string Legacy::ServerConfigFilePath(const int instance) const noexcept
{
    return get_file(server_config_file_, instance);
}

std::string Legacy::ServerDataFolder(const int instance) const noexcept
{
    return get_path(server_data_folder_, instance);
}
}  // namespace opentxs::api::implementation
