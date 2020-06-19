// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/database/Blocks.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::database::Blocks::"

namespace opentxs::blockchain::database
{
Blocks::Blocks(
    const api::client::Manager& api,
    const Common& common,
    const blockchain::Type type) noexcept
    : api_(api)
    , common_(common)
    , chain_(type)
{
}

auto Blocks::LoadBitcoin(const block::Hash& block) const noexcept
    -> std::shared_ptr<const block::bitcoin::Block>
{
    const auto bytes = common_.BlockLoad(block);

    if (false == bytes.valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Block ")(block.asHex())(
            " not found ")
            .Flush();

        return {};
    }

    return factory::BitcoinBlock(api_, chain_, bytes.get());
}

auto Blocks::Store(const block::Block& block) const noexcept -> bool
{
    const auto size = block.CalculateSize();
    auto writer = common_.BlockStore(block.ID(), size);

    if (false == writer.get().valid(size)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate storage for block")
            .Flush();

        return false;
    }

    if (false ==
        block.Serialize(preallocated(writer.size(), writer.get().data()))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize block")
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::blockchain::database
