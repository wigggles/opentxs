// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/blockchain/client/Client.hpp"

#include "UpdateTransaction.hpp"

namespace opentxs
{
blockchain::client::internal::UpdateTransaction* Factory::UpdateTransaction()
{
    using ReturnType = blockchain::client::implementation::UpdateTransaction;

    return new ReturnType;
}
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
UpdateTransaction::UpdateTransaction()
    : internal::UpdateTransaction()
    , is_best_(false)
    , have_reorg_(false)
    , have_checkpoint_(false)
    , tip_(make_blank<block::Position>::value())
    , reorg_from_(make_blank<block::Position>::value())
    , checkpoint_(make_blank<block::Position>::value())
    , headers_()
    , reorg_()
    , add_sib_()
    , delete_sib_()
    , connect_()
    , disconnected_()
{
}

void UpdateTransaction::AddToBestChain(
    const block::Height height,
    const block::Hash& hash)
{
    AddToBestChain({height, hash});
}

void UpdateTransaction::AddToBestChain(const block::Position& position)
{
    reorg_.emplace(position);
    add_sib_.erase(position.second);
    delete_sib_.emplace(position.second);
}

void UpdateTransaction::AddSibling(const block::Position& position)
{
    if (false == IsInBestChain(position)) { add_sib_.emplace(position.second); }
}

void UpdateTransaction::ClearCheckpoint()
{
    have_checkpoint_ = true;
    checkpoint_ = make_blank<block::Position>::value();
}

void UpdateTransaction::ConnectBlock(ChainSegment&& segment)
{
    disconnected_.erase(segment);
    connect_.emplace(std::move(segment));
}

void UpdateTransaction::DisconnectBlock(const block::Header& header)
{
    ChainSegment item{header.ParentHash(), header.Hash()};
    connect_.erase(item);
    disconnected_.emplace(std::move(item));
}

const block::Header& UpdateTransaction::Header(const block::Hash& hash) const
{
    auto& output = headers_.at(hash);

    OT_ASSERT(output);

    return *output;
}

bool UpdateTransaction::IsInBestChain(const block::Position& position) const
{
    return IsInBestChain(position.first, position.second);
}

bool UpdateTransaction::IsInBestChain(
    const block::Height height,
    const block::Hash& hash) const
{
    try {
        return reorg_.at(height) == hash;
    } catch (...) {
        return false;
    }
}

block::Header& UpdateTransaction::ModifyExistingBlock(
    std::unique_ptr<block::Header> header)
{
    OT_ASSERT(header);

    auto [it, added] = headers_.emplace(header->Hash(), std::move(header));

    return *it->second;
}

void UpdateTransaction::RemoveSibling(const block::Hash& hash)
{
    delete_sib_.emplace(hash);
    add_sib_.erase(hash);
}

void UpdateTransaction::SetCheckpoint(block::Position&& checkpoint)
{
    have_checkpoint_ = true;
    checkpoint_ = std::move(checkpoint);
}
}  // namespace opentxs::blockchain::client::implementation
