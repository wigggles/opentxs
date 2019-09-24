// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/Editor.hpp"

#include "Node.hpp"

#include <cstdint>

namespace opentxs::storage
{
class Txos final : public Node
{
public:
    using Coin = api::client::blockchain::Coin;
    using SerializedType = proto::StorageTxoIndex;
    using PayloadType = proto::StorageBlockchainTxo;

    bool Load(
        const Coin& id,
        std::shared_ptr<PayloadType>& output,
        const bool checking) const noexcept;
    std::set<Coin> LookupElement(const Data& element) const noexcept;
    std::set<Coin> LookupTxid(const std::string& txid) const noexcept;

    bool Delete(const Coin& id) noexcept;
    bool Store(const PayloadType& data) noexcept;

    ~Txos() = default;

private:
    friend storage::Nym;

    static const VersionNumber DefaultVersion{1};

    std::map<Coin, std::shared_ptr<PayloadType>> txos_;
    std::map<OTData, std::set<Coin>> element_index_;
    std::map<std::string, std::set<Coin>> txid_index_;

    bool save(const Lock& lock) const final;
    SerializedType serialize() const noexcept;

    void index(const Lock& lock, const PayloadType& item) noexcept;
    void init(const std::string& hash) final;

    Txos(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash) noexcept;
    Txos() = delete;
    Txos(const Txos&) = delete;
    Txos(Txos&&) = delete;
    Txos operator=(const Txos&) = delete;
    Txos operator=(Txos&&) = delete;
};
}  // namespace opentxs::storage
