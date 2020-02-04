// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/Bytes.hpp"

#if OT_STORAGE_LMDB
#include "lmdb.h"

#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <tuple>
#include <vector>

#if defined(__x86_64__) || defined(__aarch64__) || defined(_WIN64)
#define OT_LMDB_SIZE 16UL * 1024UL * 1024UL * 1024UL
#else
#define OT_LMDB_SIZE 384UL * 1024UL * 1024UL
#endif

namespace opentxs::storage::lmdb
{
using Callback = std::function<void(const ReadView data)>;
using Flags = unsigned int;
using ReadCallback =
    std::function<bool(const ReadView key, const ReadView value)>;
using Databases = std::vector<MDB_dbi>;
using Result = std::pair<bool, int>;
using Table = int;
using TablesToInit = std::vector<std::pair<Table, std::size_t>>;
using TableNames = std::map<Table, const std::string>;
using UpdateCallback = std::function<Space(const ReadView data)>;

class LMDB
{
public:
    enum class Dir : bool { Forward = false, Backward = true };
    enum class Mode : bool { One = false, Multiple = true };

    struct Transaction {
        bool success_;

        operator MDB_txn*() noexcept { return ptr_; }

        bool Finalize(const std::optional<bool> success = {}) noexcept;

        Transaction(MDB_env* env, const bool rw) noexcept(false);
        ~Transaction();

    private:
        MDB_txn* ptr_;

        Transaction(const Transaction&) = delete;
        Transaction(Transaction&&) noexcept;
        Transaction& operator=(const Transaction&) = delete;
        Transaction& operator=(Transaction&&) = delete;
    };

    bool Commit() const noexcept;
    bool Delete(const Table table, MDB_txn* parent = nullptr) const noexcept;
    bool Delete(
        const Table table,
        const ReadView key,
        MDB_txn* parent = nullptr) const noexcept;
    bool Delete(
        const Table table,
        const std::size_t key,
        const ReadView value,
        MDB_txn* parent = nullptr) const noexcept;
    bool Delete(
        const Table table,
        const ReadView key,
        const ReadView value,
        MDB_txn* parent = nullptr) const noexcept;
    bool Exists(const Table table, const ReadView key) const noexcept;
    bool Load(
        const Table table,
        const ReadView key,
        const Callback cb,
        const Mode mode = Mode::One) const noexcept;
    bool Load(
        const Table table,
        const std::size_t key,
        const Callback cb,
        const Mode mode = Mode::One) const noexcept;
    bool Queue(
        const Table table,
        const ReadView key,
        const ReadView value,
        const Mode mode = Mode::One) const noexcept;
    bool Read(const Table table, const ReadCallback cb, const Dir dir) const
        noexcept;
    Result Store(
        const Table table,
        const ReadView key,
        const ReadView value,
        MDB_txn* parent = nullptr,
        const Flags flags = 0) const noexcept;
    Result Store(
        const Table table,
        const std::size_t key,
        const ReadView value,
        MDB_txn* parent = nullptr,
        const Flags flags = 0) const noexcept;
    Result StoreOrUpdate(
        const Table table,
        const ReadView key,
        const UpdateCallback cb,
        MDB_txn* parent = nullptr,
        const Flags flags = 0) const noexcept;
    Transaction TransactionRO() const noexcept(false);
    Transaction TransactionRW() const noexcept(false);

    LMDB(
        const TableNames& names,
        const std::string& folder,
        const TablesToInit init,
        const Flags flags = 0)
    noexcept;
    ~LMDB();

private:
    using NewKey = std::tuple<Table, Mode, std::string, std::string>;
    using Pending = std::vector<NewKey>;

    const TableNames& names_;
    mutable MDB_env* env_;
    mutable Databases db_;
    mutable Pending pending_;
    mutable std::mutex lock_;

    MDB_dbi get_database(const Table table) const noexcept;
    MDB_dbi init_db(const Table table, const std::size_t flags) noexcept;
    void init_environment(
        const std::string& folder,
        const std::size_t tables,
        const Flags flags) noexcept;
    void init_tables(const TablesToInit init) noexcept;

    LMDB() = delete;
    LMDB(const LMDB&) = delete;
    LMDB(LMDB&&) = delete;
    LMDB& operator=(const LMDB&) = delete;
    LMDB& operator=(LMDB&&) = delete;
};
}  // namespace opentxs::storage::lmdb
#endif  // OT_STORAGE_LMDB
