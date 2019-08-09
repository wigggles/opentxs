// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace bb = opentxs::blockchain::bitcoin;

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Block final : public bitcoin::Block, public block::implementation::Block
{
public:
    auto FindMatches(
        const FilterType type,
        const Patterns& outpoints,
        const Patterns& scripts) const noexcept -> Matches final;

    Block(
        const api::internal::Core& api,
        const blockchain::Type chain,
        std::unique_ptr<const internal::Header> header,
        std::vector<std::pair<bb::EncodedTransaction, Space>>&&
            parsed) noexcept(false);

private:
    const std::unique_ptr<const internal::Header> header_p_;
    std::vector<std::pair<bb::EncodedTransaction, Space>> transactions_;
    const internal::Header& header_;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
