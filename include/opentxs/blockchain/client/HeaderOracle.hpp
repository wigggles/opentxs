// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_HEADERORACLE_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_HEADERORACLE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/blockchain/Blockchain.hpp"

#include <cstdint>
#include <string>
#include <set>

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
    static const block::Hash& GenesisBlockHash(const blockchain::Type type);

    EXPORT virtual bool AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept = 0;
    EXPORT virtual bool AddHeader(std::unique_ptr<block::Header>) noexcept = 0;
    EXPORT virtual block::Position BestChain() noexcept = 0;
    EXPORT virtual block::pHash BestHash(
        const block::Height height) noexcept = 0;
    EXPORT virtual bool DeleteCheckpoint() noexcept = 0;
    EXPORT virtual block::Position GetCheckpoint() noexcept = 0;
    EXPORT virtual std::unique_ptr<block::Header> LoadHeader(
        const block::Hash& hash) noexcept = 0;
    EXPORT virtual std::set<block::pHash> Siblings() noexcept = 0;

    EXPORT virtual ~HeaderOracle() = default;

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
