// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_OUTPUT_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_OUTPUT_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>
#include <optional>

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Output
{
public:
    OPENTXS_EXPORT virtual auto CalculateSize() const noexcept
        -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto Serialize(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;
    OPENTXS_EXPORT virtual auto Serialize(
        const std::uint32_t index,
        proto::BlockchainTransactionOutput& destination) const noexcept
        -> bool = 0;
    OPENTXS_EXPORT virtual auto Script() const noexcept
        -> const bitcoin::Script& = 0;
    OPENTXS_EXPORT virtual auto Value() const noexcept -> std::int64_t = 0;

    virtual ~Output() = default;

protected:
    Output() noexcept = default;

private:
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
