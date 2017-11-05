/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_STORAGE_TREE_ROOT_HPP
#define OPENTXS_STORAGE_TREE_ROOT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/storage/tree/Node.hpp"

#include <atomic>
#include <cstdint>
#include <limits>
#include <string>
#include <thread>

namespace opentxs
{
namespace api
{

class Storage;

}  // namespace api

namespace storage
{

class Tree;

class Root : public Node
{
private:
    typedef Node ot_super;
    friend class api::Storage;

    const std::uint64_t gc_interval_{std::numeric_limits<int64_t>::max()};

    mutable std::string gc_root_;
    std::atomic<bool>& current_bucket_;
    mutable std::atomic<bool> gc_running_;
    mutable std::atomic<bool> gc_resume_;
    mutable std::atomic<std::uint64_t> last_gc_;
    mutable std::atomic<std::uint64_t> sequence_;
    mutable std::mutex gc_lock_;
    mutable std::unique_ptr<std::thread> gc_thread_;

    std::string tree_root_;
    mutable std::mutex tree_lock_;
    mutable std::unique_ptr<class Tree> tree_;

    proto::StorageRoot serialize() const;
    class Tree* tree() const;

    void cleanup() const;
    void collect_garbage(const StorageDriver* to) const;
    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    void save(class Tree* tree, const Lock& lock);

    Root(
        const StorageDriver& storage,
        const std::string& hash,
        const std::int64_t interval,
        std::atomic<bool>& bucket);
    Root() = delete;
    Root(const Root&) = delete;
    Root(Root&&) = delete;
    Root operator=(const Root&) = delete;
    Root operator=(Root&&) = delete;

public:
    const class Tree& Tree() const;

    Editor<class Tree> mutable_Tree();

    bool Migrate(const StorageDriver& to) const override;
    std::uint64_t Sequence() const;

    ~Root() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_ROOT_HPP
