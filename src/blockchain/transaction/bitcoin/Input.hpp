// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "Output.hpp"

#include <boost/endian/buffers.hpp>

#include <utility>
#include <array>
#include <cstdint>
#include <cstddef>

namespace opentxs::blockchain::transaction::bitcoin
{

namespace be = boost::endian;

using OutpointHashField = std::array<std::byte, 32>;
using OutpointField = std::pair<OutpointHashField, be::little_uint32_buf_t>;

using Outpoint = std::pair<OTData, std::size_t>;
// using Outpoint = std::pair<transaction::Hash, std::size_t>

// using Index = std::uint32_t;

OTData EncodeOutpoint(const Outpoint& in) noexcept;

bool DecodeOutpointFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    Outpoint& result) noexcept;

class Input
{
    using Sequence = std::uint32_t;
    using SequenceField = be::little_uint32_buf_t;

private:
    Outpoint previous_;
    OTData signature_script_;
    Sequence sequence_;

public:
    Outpoint Previous() const { return previous_; }
    const Data& rawScript() const { return signature_script_; }
    Script getScript() const;
    Sequence getSequence() const;

    OTData Encode() const noexcept;

    Input();

    Input(
        const Outpoint& outpoint,
        const Data& signature_script,
        const Sequence sequence) noexcept;

    static bool DecodeFromPayload(
        const std::byte*& it,
        std::size_t& expectedSize,
        const std::size_t size,
        Input& result) noexcept;
};

}  // namespace opentxs::blockchain::transaction::bitcoin
