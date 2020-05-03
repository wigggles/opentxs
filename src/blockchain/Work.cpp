// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "blockchain/Work.hpp"  // IWYU pragma: associated

#include <iterator>
#include <utility>
#include <vector>

#include "Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::implementation::Work::"

namespace opentxs
{
blockchain::Work* Factory::Work(const std::string& hex)
{
    using ReturnType = blockchain::implementation::Work;
    using ValueType = ReturnType::Type;

    const auto bytes = Data::Factory(hex, Data::Mode::Hex);

    if (bytes->empty()) { return new ReturnType(); }

    ValueType value{};
    mp::cpp_int i;

    try {
        // Interpret bytes as big endian
        mp::import_bits(i, bytes->begin(), bytes->end(), 8, true);
        value = ValueType{i};
    } catch (...) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(": Failed to decode work")
            .Flush();

        return new ReturnType();
    }

    return new ReturnType(std::move(value));
}

blockchain::Work* Factory::Work(const blockchain::NumericHash& input)
{
    using ReturnType = blockchain::implementation::Work;
    using TargetType = mp::checked_cpp_int;
    using ValueType = ReturnType::Type;
    ValueType value{};

    try {
        const auto maxTarget =
            Factory::NumericHashNBits(blockchain::NumericHash::MaxTarget);
        const TargetType targetOne{maxTarget->Decimal()};
        const TargetType target{input.Decimal()};
        value = ValueType{targetOne} / ValueType{target};
    } catch (...) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to calculate difficulty")
            .Flush();

        return new ReturnType();
    }

    return new ReturnType(std::move(value));
}

bool operator==(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() == rhs;
}

bool operator!=(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() != rhs;
}

bool operator<(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() < rhs;
}

bool operator<=(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() <= rhs;
}

bool operator>(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() > rhs;
}

bool operator>=(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() >= rhs;
}

OTWork operator+(const OTWork& lhs, const blockchain::Work& rhs) noexcept
{
    return lhs.get() + rhs;
}
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
Work::Work(Type&& data) noexcept
    : blockchain::Work()
    , data_(std::move(data))
{
}

Work::Work() noexcept
    : Work(Type{})
{
}

Work::Work(const Work& rhs) noexcept
    : Work(Type{rhs.data_})
{
}

bool Work::operator==(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ == input.data_;
}

bool Work::operator!=(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ != input.data_;
}

bool Work::operator<(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ < input.data_;
}

bool Work::operator<=(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ <= input.data_;
}

bool Work::operator>(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ > input.data_;
}

bool Work::operator>=(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ >= input.data_;
}

OTWork Work::operator+(const blockchain::Work& rhs) const noexcept
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return OTWork{new Work{data_ + input.data_}};
}

std::string Work::asHex() const noexcept
{
    std::vector<unsigned char> bytes;

    try {
        // Export as big endian
        mp::export_bits(mp::cpp_int(data_), std::back_inserter(bytes), 8, true);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encode number").Flush();

        return {};
    }

    return opentxs::Data::Factory(bytes.data(), bytes.size())->asHex();
}
}  // namespace opentxs::blockchain::implementation
