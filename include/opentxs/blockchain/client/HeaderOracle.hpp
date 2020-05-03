// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_HEADERORACLE_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_HEADERORACLE_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>
#include <set>
#include <tuple>

#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace client
{
class HeaderOracle
{
public:
    /// Throws std::out_of_range for invalid type
    OPENTXS_EXPORT static const block::Hash& GenesisBlockHash(
        const blockchain::Type type);

    OPENTXS_EXPORT virtual block::Position BestChain() const noexcept = 0;
    OPENTXS_EXPORT virtual block::pHash BestHash(
        const block::Height height) const noexcept = 0;
    /** Test block position for membership in the best chain
     *
     *  returns {parent position, best position}
     *
     *  parent position is the input block position if that position is in the
     * best chain, otherwise it is the youngest common ancestor of the input
     * block and best chain
     */
    OPENTXS_EXPORT virtual std::pair<block::Position, block::Position>
    CommonParent(const block::Position& input) const noexcept = 0;
    OPENTXS_EXPORT virtual block::Position GetCheckpoint() const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsInBestChain(const block::Hash& hash) const
        noexcept = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<block::Header> LoadHeader(
        const block::Hash& hash) const noexcept = 0;
    OPENTXS_EXPORT virtual std::vector<block::pHash> RecentHashes() const
        noexcept = 0;
    OPENTXS_EXPORT virtual std::set<block::pHash> Siblings() const noexcept = 0;

    OPENTXS_EXPORT virtual bool AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept = 0;
    OPENTXS_EXPORT virtual bool AddHeader(
        std::unique_ptr<block::Header>) noexcept = 0;
    OPENTXS_EXPORT virtual bool AddHeaders(
        std::vector<std::unique_ptr<block::Header>>&) noexcept = 0;
    OPENTXS_EXPORT virtual bool DeleteCheckpoint() noexcept = 0;

    OPENTXS_EXPORT virtual ~HeaderOracle() = default;

protected:
    HeaderOracle() noexcept = default;

private:
    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) = delete;
    HeaderOracle& operator=(const HeaderOracle&) = delete;
    HeaderOracle& operator=(HeaderOracle&&) = delete;
};
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs
#endif
