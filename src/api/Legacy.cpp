// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Legacy.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <boost/filesystem.hpp>

#ifndef _WIN32
extern "C" {
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
}
#endif

#include <cstdlib>

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

namespace opentxs::api
{
std::string Legacy::AppDataFolder() noexcept
{
    return implementation::Legacy::app_data_folder_.string();
}
}  // namespace opentxs::api

namespace opentxs::api::implementation
{
const char* Legacy::account_{"account"};
const char* Legacy::common_{"storage"};
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
const char* Legacy::receipt_{"receipt"};
const char* Legacy::record_box_{"recordbox"};
const fs::path Legacy::app_data_folder_{get_app_data_folder()};

Legacy::Legacy() noexcept
    : client_data_folder_(std::string(CLIENT_CONFIG_KEY) + DATA_FOLDER_EXT)
    , server_data_folder_(std::string(SERVER_CONFIG_KEY) + DATA_FOLDER_EXT)
    , client_config_file_(std::string(CLIENT_CONFIG_KEY) + CONFIG_FILE_EXT)
    , crypto_config_file_(std::string(CRYPTO_CONFIG_KEY) + CONFIG_FILE_EXT)
    , log_config_file_(std::string(LOG_CONFIG_KEY) + CONFIG_FILE_EXT)
    , server_config_file_(std::string(SERVER_CONFIG_KEY) + CONFIG_FILE_EXT)
    , pid_file_(PID_FILE)
{
}

bool Legacy::AppendFile(String& out, const String& base, const String& file)
    const noexcept
{
    try {
        const auto path = fs::path{base.Get()}.remove_trailing_separator() /
                          fs::path{file.Get()}.remove_trailing_separator();
        out.Set(path.string().c_str());

        return true;
    } catch (...) {

        return false;
    }
}

bool Legacy::AppendFolder(String& out, const String& base, const String& file)
    const noexcept
{
    try {
        const auto path = fs::path{base.Get()}.remove_trailing_separator() /
                          fs::path{file.Get()}.remove_trailing_separator() /
                          fs::path{"/"};
        out.Set(path.string().c_str());

        return true;
    } catch (...) {

        return false;
    }
}

bool Legacy::BuildFolderPath(const String& path) const noexcept
{
    return ConfirmCreateFolder(path);
}

bool Legacy::BuildFilePath(const String& path) const noexcept
{
    try {
        const auto incoming = fs::path{path.Get()};

        if (false == incoming.has_parent_path()) { return false; }

        const auto parent = incoming.parent_path();
        fs::create_directories(parent);

        return fs::exists(parent);
    } catch (...) {

        return false;
    }
}

std::string Legacy::ClientConfigFilePath(const int instance) const noexcept
{
    return get_file(client_config_file_, instance);
}

std::string Legacy::ClientDataFolder(const int instance) const noexcept
{
    return get_path(client_data_folder_, instance);
}

bool Legacy::ConfirmCreateFolder(const String& path) const noexcept
{
    try {
        const auto folder = fs::path{path.Get()};
        fs::create_directories(folder);

        return fs::exists(folder);
    } catch (...) {

        return false;
    }
}

std::string Legacy::CryptoConfigFilePath() const noexcept
{
    return get_file(crypto_config_file_);
}

bool Legacy::FileExists(const String& path, std::size_t& size) const noexcept
{
    size = 0;

    try {
        const auto file = fs::path{path.Get()};

        if (fs::exists(file)) {
            size = fs::file_size(file);

            return true;
        } else {

            return false;
        }
    } catch (...) {

        return false;
    }
}

fs::path Legacy::get_app_data_folder() noexcept
{
    return get_home_directory() / get_suffix();
}

fs::path Legacy::get_home_directory() noexcept
{
    auto home = std::string{getenv("HOME")};

    if (false == home.empty()) { return home; }

    // Windows
    home = getenv("USERPROFILE");

    if (false == home.empty()) { return home; }

    const auto drive = std::string{getenv("HOMEDRIVE")};
    const auto path = std::string{getenv("HOMEPATH")};

    if ((false == drive.empty()) && (false == drive.empty())) {

        return drive + path;
    }

#ifndef _WIN32
    const auto* pwd = getpwuid(getuid());

    if (nullptr != pwd) {
        if (nullptr != pwd->pw_dir) { return pwd->pw_dir; }
    }
#endif

    LogNormal("Unable to determine home directory.").Flush();

    OT_FAIL;
}

fs::path Legacy::get_suffix() noexcept
{
    return
#if defined(_WIN32)
        "OpenTransactions/"
#elif defined(TARGET_OS_MAC)
#if TARGET_OS_IPHONE  // iOS
        "Documents/"
#else                 // OSX
        "OpenTransactions/"
#endif
#elif defined(ANDROID)
        "ot/"
#else  // unix
        ".ot/"
#endif
        ;
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
    const auto success = AppendFolder(
        output,
        String::Factory(app_data_folder_.string()),
        String::Factory(name.c_str()));

    OT_ASSERT(success)

    return output->Get();
}

std::string Legacy::LogConfigFilePath() const noexcept
{
    return get_file(log_config_file_);
}

bool Legacy::PathExists(const String& path) const noexcept
{
    try {

        return fs::exists(fs::path{path.Get()});
    } catch (...) {

        return false;
    }
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
