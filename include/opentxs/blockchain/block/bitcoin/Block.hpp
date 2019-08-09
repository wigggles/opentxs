// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_BLOCK_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_BLOCK_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block : virtual public block::Block
{
public:
    struct Outpoint {
        block::pHash txid_;
        std::uint32_t output_;
    };

    ~Block() override = default;

protected:
    Block() noexcept = default;

private:
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
