// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/key/Symmetric.hpp"

#include "internal/api/storage/Storage.hpp"
#include "storage/StorageConfig.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace opentxs::api::implementation
{
class StorageParent
{
protected:
    const api::Crypto& crypto_;
    const api::Settings& config_;
    const ArgList args_;
    const std::chrono::seconds gc_interval_{0};
    const std::string data_folder_;
#if OT_QT
    const bool enable_qt_;
#endif
    StorageConfig storage_config_;
    bool migrate_storage_{false};
    OTString migrate_from_;
    OTString primary_storage_plugin_;
    OTString archive_directory_;
    OTString encrypted_directory_;
    std::unique_ptr<api::storage::StorageInternal> storage_;
#if OT_CRYPTO_WITH_BIP32
    OTSymmetricKey storage_encryption_key_;
#endif
    void init(
        const api::Factory& factory
#if OT_CRYPTO_WITH_BIP32
        ,
        const api::HDSeed& seeds
#endif  // OT_CRYPTO_WITH_BIP32
    );
    void start();

    StorageParent(
        const Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const api::Legacy& legacy,
        const std::string& dataFolder);

    virtual ~StorageParent() = default;

private:
    static OTString extract_arg(const std::string& name, const ArgList& args);
    static OTString extract_archive_directory(const ArgList& args);
    static OTString extract_encrypted_directory(const ArgList& args);
    static OTString extract_primary_storage_plugin(const ArgList& args);
#if OT_QT
    static bool extract_qt(const ArgList& args);
#endif
    static OTString get_primary_storage_plugin(
        const api::Settings& config,
        const StorageConfig& storageConfig,
        const ArgList args,
        bool& migrate,
        String& previous);

    StorageParent() = delete;
    StorageParent(const StorageParent&) = delete;
    StorageParent(StorageParent&&) = delete;
    StorageParent& operator=(const StorageParent&) = delete;
    StorageParent& operator=(StorageParent&&) = delete;
};
}  // namespace opentxs::api::implementation
