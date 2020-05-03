// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "storage/tree/BlockchainTransactions.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <tuple>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

#define OT_METHOD "opentxs::storage::BlockchainTransactions::"

namespace opentxs
{
namespace storage
{
BlockchainTransactions::BlockchainTransactions(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , nym_index_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CurrentVersion);
    }
}

bool BlockchainTransactions::Delete(const std::string& id)
{
    return delete_item(id);
}

void BlockchainTransactions::init(const std::string& hash)
{
    auto serialized = std::shared_ptr<SerializedType>{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load blockchain transaction index file.")
            .Flush();

        abort();
    }

    init_version(CurrentVersion, *serialized);

    for (const auto& it : serialized->transaction()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    // Added in version 2
    for (const auto& index : serialized->nymindex()) {
        const auto& txid = index.contact();
        auto& set = nym_index_[txid];

        for (const auto& nym : index.nym()) {
            set.emplace(identifier::Nym::Factory(nym));
        }
    }
}

bool BlockchainTransactions::Load(
    const std::string& id,
    std::shared_ptr<proto::BlockchainTransaction>& output,
    const bool checking) const
{
    auto alias = std::string{};

    return load_proto<proto::BlockchainTransaction>(
        id, output, alias, checking);
}

std::set<OTNymID> BlockchainTransactions::LookupNyms(
    const std::string& txid) const
{
    Lock lock(write_lock_);

    try {

        return nym_index_.at(txid);
    } catch (...) {

        return {};
    }
}

bool BlockchainTransactions::save(const Lock& lock) const
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();

        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

BlockchainTransactions::SerializedType BlockchainTransactions::serialize() const
{
    auto output = SerializedType{};
    output.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *output.add_transaction());
        }
    }

    // Added in version 2
    for (const auto& [txid, nyms] : nym_index_) {
        auto& index = *output.add_nymindex();
        index.set_version(NymIndexVersion);
        index.set_contact(txid);

        for (const auto& nym : nyms) { index.add_nym(nym->str()); }
    }

    return output;
}

bool BlockchainTransactions::Store(
    const identifier::Nym& nym,
    const proto::BlockchainTransaction& data)
{
    auto alias = std::string{};
    auto plaintext = std::string{};

    if (false == proto::Validate(data, VERBOSE)) { return false; }

    Lock lock(write_lock_);
    nym_index_[data.txid()].emplace(nym);

    return store_proto(lock, data, data.txid(), alias, plaintext);
}
}  // namespace storage
}  // namespace opentxs
