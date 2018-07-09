// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::storage::implementation
{
class StorageMultiplex : virtual public opentxs::api::storage::Multiplex
{
public:
    bool EmptyBucket(const bool bucket) const override;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    bool Load(const std::string& key, const bool checking, std::string& value)
        const override;
    std::string LoadRoot() const override;
    bool Migrate(
        const std::string& key,
        const opentxs::api::storage::Driver& to) const override;
    bool Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const override;
    void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const override;
    bool Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const override;
    bool StoreRoot(const bool commit, const std::string& hash) const override;

    std::string BestRoot(bool& primaryOutOfSync) override;
    void InitBackup() override;
    void InitEncryptedBackup(crypto::key::Symmetric& key) override;
    opentxs::api::storage::Driver& Primary() override;
    void SynchronizePlugins(
        const std::string& hash,
        const storage::Root& root,
        const bool syncPrimary) override;

    ~StorageMultiplex();

private:
    friend Factory;

    const api::storage::Storage& storage_;
    const Flag& primary_bucket_;
    const StorageConfig& config_;
    std::unique_ptr<opentxs::api::storage::Plugin> primary_plugin_;
    std::vector<std::unique_ptr<opentxs::api::storage::Plugin>> backup_plugins_;
    const Digest digest_;
    const Random random_;
    OTSymmetricKey null_;

    StorageMultiplex(
        const api::storage::Storage& storage,
        const Flag& primaryBucket,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
    StorageMultiplex() = delete;
    StorageMultiplex(const StorageMultiplex&) = delete;
    StorageMultiplex(StorageMultiplex&&) = delete;
    StorageMultiplex& operator=(const StorageMultiplex&) = delete;
    StorageMultiplex& operator=(StorageMultiplex&&) = delete;

    void Cleanup();
    void Cleanup_StorageMultiplex();
    void init(
        const std::string& primary,
        std::unique_ptr<opentxs::api::storage::Plugin>& plugin);
    void init_fs(std::unique_ptr<opentxs::api::storage::Plugin>& plugin);
    void init_lmdb(std::unique_ptr<opentxs::api::storage::Plugin>& plugin);
    void init_memdb(std::unique_ptr<opentxs::api::storage::Plugin>& plugin);
    void init_sqlite(std::unique_ptr<opentxs::api::storage::Plugin>& plugin);
    void Init_StorageMultiplex(
        const String& primary,
        const bool migrate,
        const String& previous);
    void migrate_primary(const std::string& from, const std::string& to);
};
}  // namespace opentxs::storage::implementation
