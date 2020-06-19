// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Proto.tpp"  // IWYU pragma: associated

namespace opentxs
{
auto operator==(const ProtobufType& lhs, const ProtobufType& rhs) noexcept
    -> bool
{
    auto sLeft = std::string{};
    auto sRight = std::string{};
    lhs.SerializeToString(&sLeft);
    rhs.SerializeToString(&sRight);

    return sLeft == sRight;
}
}  // namespace opentxs

namespace opentxs::proto
{
auto ToString(const ProtobufType& input) -> std::string
{
    std::string output{};
    output.resize(static_cast<std::size_t>(input.ByteSize()));
    input.SerializeToArray(output.data(), static_cast<int>(output.size()));

    return output;
}
}  // namespace opentxs::proto
