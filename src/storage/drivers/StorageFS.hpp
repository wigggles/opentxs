// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#if OT_STORAGE_FS
#include "opentxs/core/Flag.hpp"

#include "storage/Plugin.hpp"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <atomic>

namespace opentxs
{

class StorageConfig;

// Simple filesystem implementation of opentxs::storage
class StorageFS : public Plugin
{
private:
    typedef Plugin ot_super;

public:
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    std::string LoadRoot() const override;
    bool StoreRoot(const bool commit, const std::string& hash) const override;

    void Cleanup() override;

    ~StorageFS() override;

protected:
    const std::string folder_;
    const std::string path_seperator_{};
    OTFlag ready_;

    bool sync(const std::string& path) const;

    StorageFS(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const std::string& folder,
        const Flag& bucket);

private:
    typedef boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
        File;

    virtual std::string calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const = 0;
    virtual std::string prepare_read(const std::string& input) const;
    virtual std::string prepare_write(const std::string& input) const;
    std::string read_file(const std::string& filename) const;
    virtual std::string root_filename() const = 0;
    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const override;
    bool sync(File& file) const;
    bool sync(int fd) const;
    bool write_file(
        const std::string& directory,
        const std::string& filename,
        const std::string& contents) const;

    void Cleanup_StorageFS();
    void Init_StorageFS();

    StorageFS() = delete;
    StorageFS(const StorageFS&) = delete;
    StorageFS(StorageFS&&) = delete;
    StorageFS& operator=(const StorageFS&) = delete;
    StorageFS& operator=(StorageFS&&) = delete;
};
}  // namespace opentxs

#endif  // OT_STORAGE_FS
