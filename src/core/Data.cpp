// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <boost/endian/buffers.hpp>

#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "Data.hpp"

template class opentxs::Pimpl<opentxs::Data>;

namespace std
{
bool less<opentxs::Pimpl<opentxs::Data>>::operator()(
    const opentxs::OTData& lhs,
    const opentxs::OTData& rhs) const
{
    return lhs.get() < rhs.get();
}
}  // namespace std

namespace opentxs
{
bool operator==(const OTData& lhs, const Data& rhs) { return lhs.get() == rhs; }

bool operator!=(const OTData& lhs, const Data& rhs) { return lhs.get() != rhs; }

bool operator<(const OTData& lhs, const Data& rhs) { return lhs.get() < rhs; }

bool operator>(const OTData& lhs, const Data& rhs) { return lhs.get() > rhs; }

bool operator<=(const OTData& lhs, const Data& rhs) { return lhs.get() <= rhs; }

bool operator>=(const OTData& lhs, const Data& rhs) { return lhs.get() >= rhs; }

OTData& operator+=(OTData& lhs, const OTData& rhs)
{
    lhs.get() += rhs.get();

    return lhs;
}

OTData& operator+=(OTData& lhs, const std::uint8_t rhs)
{
    lhs.get() += rhs;

    return lhs;
}

OTData& operator+=(OTData& lhs, const std::uint16_t rhs)
{
    lhs.get() += rhs;

    return lhs;
}

OTData& operator+=(OTData& lhs, const std::uint32_t rhs)
{
    lhs.get() += rhs;

    return lhs;
}

OTData& operator+=(OTData& lhs, const std::uint64_t rhs)
{
    lhs.get() += rhs;

    return lhs;
}

OTData Data::Factory() { return OTData(new implementation::Data()); }

OTData Data::Factory(const Data& rhs)
{
    return OTData(new implementation::Data(rhs.data(), rhs.size()));
}

OTData Data::Factory(const void* data, std::size_t size)
{
    return OTData(new implementation::Data(data, size));
}

OTData Data::Factory(const Armored& source)
{
    return OTData(new implementation::Data(source));
}

OTData Data::Factory(const std::vector<unsigned char>& source)
{
    return OTData(new implementation::Data(source));
}

OTData Data::Factory(const std::vector<std::byte>& source)
{
    return OTData(new implementation::Data(source));
}

OTData Data::Factory(const network::zeromq::Frame& message)
{
    return OTData(new implementation::Data(message.data(), message.size()));
}

OTData Data::Factory(const std::uint8_t in)
{
    return OTData(new implementation::Data(&in, sizeof(in)));
}

OTData Data::Factory(const std::uint16_t in)
{
    const auto input = boost::endian::big_uint16_buf_t(in);

    return OTData(new implementation::Data(&input, sizeof(input)));
}

OTData Data::Factory(const std::uint32_t in)
{
    const auto input = boost::endian::big_uint32_buf_t(in);

    return OTData(new implementation::Data(&input, sizeof(input)));
}

OTData Data::Factory(const std::uint64_t in)
{
    const auto input = boost::endian::big_uint64_buf_t(in);

    return OTData(new implementation::Data(&input, sizeof(input)));
}

OTData Data::Factory(const std::string in, const Mode mode)
{
    if (Mode::Hex == mode) {
        auto output = OTData(new implementation::Data(in.data(), in.size()));

        if (output->DecodeHex(in)) { return output; }

        return OTData(new implementation::Data());
    } else {
        return OTData(new implementation::Data(in.data(), in.size()));
    }
}

namespace implementation
{
Data::Data(const Armored& source)
{
    if (source.Exists()) { source.GetData(*this); }
}

Data::Data(const void* data, std::size_t size)
    : data_(
          static_cast<const std::uint8_t*>(data),
          static_cast<const std::uint8_t*>(data) + size)
{
}

Data::Data(const std::vector<unsigned char>& sourceVector)
{
    Assign(sourceVector.data(), sourceVector.size());
}

Data::Data(const std::vector<std::byte>& sourceVector)
{
    Assign(sourceVector.data(), sourceVector.size());
}

Data::Data(const Vector& rhs, const std::size_t size)
    : data_{rhs}
    , position_{size}
{
}

bool Data::operator==(const opentxs::Data& in) const
{
    if (data_.size() != in.size()) { return false; }

    std::size_t x{0};

    for (auto i = data_.begin(); i != data_.end(); ++x, ++i) {
        const auto& lhs = *i;
        const auto rhs = std::to_integer<std::uint8_t>(in.at(x));

        if (lhs != rhs) { return false; }
    }

    return true;
}

bool Data::operator!=(const opentxs::Data& in) const
{
    if (data_.size() != in.size()) { return true; }

    std::size_t x{0};

    for (auto i = data_.begin(); i != data_.end(); ++x, ++i) {
        const auto& lhs = *i;
        const auto rhs = std::to_integer<std::uint8_t>(in.at(x));

        if (lhs != rhs) { return true; }
    }

    return false;
}

bool Data::operator<(const opentxs::Data& in) const
{
    if (data_.size() < in.size()) { return true; }

    if (data_.size() > in.size()) { return false; }

    std::size_t x{0};

    for (auto i = data_.begin(); i != data_.end(); ++x, ++i) {
        const auto& lhs = *i;
        const auto rhs = std::to_integer<std::uint8_t>(in.at(x));

        if (lhs < rhs) { return true; }
        if (lhs > rhs) { return false; }
    }

    --x;

    return at(x) < in.at(x);
}

bool Data::operator>(const opentxs::Data& in) const
{
    if (data_.size() < in.size()) { return false; }

    if (data_.size() > in.size()) { return true; }

    std::size_t x{0};

    for (auto i = data_.begin(); i != data_.end(); ++x, ++i) {
        const auto& lhs = *i;
        const auto rhs = std::to_integer<std::uint8_t>(in.at(x));

        if (lhs > rhs) { return true; }
        if (lhs < rhs) { return false; }
    }

    --x;

    return at(x) > in.at(x);
}

bool Data::operator<=(const opentxs::Data& in) const
{
    if (data_.size() < in.size()) { return true; }

    if (data_.size() > in.size()) { return false; }

    std::size_t x{0};

    for (auto i = data_.begin(); i != data_.end(); ++x, ++i) {
        const auto& lhs = *i;
        const auto rhs = std::to_integer<std::uint8_t>(in.at(x));

        if (lhs < rhs) { return true; }
        if (lhs > rhs) { return false; }
    }

    return true;
}

bool Data::operator>=(const opentxs::Data& in) const
{
    if (data_.size() < in.size()) { return false; }

    if (data_.size() > in.size()) { return true; }

    std::size_t x{0};

    for (auto i = data_.begin(); i != data_.end(); ++x, ++i) {
        const auto& lhs = *i;
        const auto rhs = std::to_integer<std::uint8_t>(in.at(x));

        if (lhs > rhs) { return true; }
        if (lhs < rhs) { return false; }
    }

    return true;
}

Data& Data::operator+=(const opentxs::Data& rhs)
{
    concatenate(dynamic_cast<const Data&>(rhs).data_);

    return *this;
}

Data& Data::operator+=(const std::uint8_t rhs)
{
    data_.emplace_back(rhs);

    return *this;
}

Data& Data::operator+=(const std::uint16_t rhs)
{
    const auto input = boost::endian::big_uint16_buf_t(rhs);
    Data temp(&input, sizeof(input));
    concatenate(temp.data_);

    return *this;
}

Data& Data::operator+=(const std::uint32_t rhs)
{
    const auto input = boost::endian::big_uint32_buf_t(rhs);
    Data temp(&input, sizeof(input));
    concatenate(temp.data_);

    return *this;
}

Data& Data::operator+=(const std::uint64_t rhs)
{
    const auto input = boost::endian::big_uint64_buf_t(rhs);
    Data temp(&input, sizeof(input));
    concatenate(temp.data_);

    return *this;
}

std::string Data::asHex() const
{
    std::stringstream out{};

    // TODO: std::to_integer<int>(byte)

    for (const auto byte : data_) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<const int&>(byte);
    }

    return out.str();
}

void Data::Assign(const opentxs::Data& rhs)
{
    // can't assign to self.
    if (&dynamic_cast<const Data&>(rhs) == this) { return; }

    data_ = dynamic_cast<const Data&>(rhs).data_;
    position_ = dynamic_cast<const Data&>(rhs).position_;
}

void Data::Assign(const void* data, const std::size_t& size)
{
    Release();

    if (data != nullptr && size > 0) {
        auto start = static_cast<const std::uint8_t*>(data);
        const std::uint8_t* end = start + size;
        data_.assign(start, end);
    }
}

bool Data::check_sub(const std::size_t pos, const std::size_t target) const
{
    const auto size = data_.size();

    if (pos > size) { return false; }

    if ((std::numeric_limits<std::size_t>::max() - pos) < target) {
        return false;
    }

    if ((pos + target) > size) { return false; }

    return true;
}

void Data::concatenate(const Vector& data)
{
    for (const auto& byte : data) { data_.emplace_back(byte); }
}

void Data::Concatenate(const void* data, const std::size_t& size)
{
    OT_ASSERT(data != nullptr);
    OT_ASSERT(size > 0);

    if (size == 0) { return; }

    Data temp(data, size);
    concatenate(temp.data_);
}

bool Data::DecodeHex(const std::string& hex)
{
    data_.clear();

    if (hex.empty()) { return true; }

    if (2 > hex.size()) { return false; }

    const auto prefix = hex.substr(0, 2);
    const auto stripped = (prefix == "0x" || prefix == "0X")
                              ? hex.substr(2, hex.size() - 2)
                              : hex;
    const auto padded =
        (0 == stripped.size() % 2) ? stripped : std::string("0") + stripped;

    for (std::size_t i = 0; i < padded.length(); i += 2) {
        data_.emplace_back(strtol(padded.substr(i, 2).c_str(), nullptr, 16));
    }

    return true;
}

bool Data::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const
{
    if (false == check_sub(pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

bool Data::Extract(std::uint8_t& output, const std::size_t pos) const
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    output = data_.at(pos);

    return true;
}

bool Data::Extract(std::uint16_t& output, const std::size_t pos) const
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint16_buf_t();
    std::memcpy(&temp, &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

bool Data::Extract(std::uint32_t& output, const std::size_t pos) const
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint32_buf_t();
    std::memcpy(&temp, &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

bool Data::Extract(std::uint64_t& output, const std::size_t pos) const
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint64_buf_t();
    std::memcpy(&temp, &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

void Data::Initialize()
{
    data_.clear();
    reset();
}

bool Data::IsNull() const
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        if (0 != byte) { return false; }
    }

    return true;
}

// First use reset() to set the internal position to 0. Then you pass in the
// buffer where the results go. You pass in the length of that buffer. It
// returns how much was actually read. If you start at position 0, and read 100
// bytes, then you are now on position 100, and the next OTfread will proceed
// from that position. (Unless you reset().)
std::size_t Data::OTfread(std::uint8_t* data, const std::size_t& readSize)
{
    OT_ASSERT(data != nullptr && readSize > 0);

    std::size_t sizeToRead = 0;

    if (position_ < size()) {
        // If the size is 20, and position is 5 (I've already read the first 5
        // bytes) then the size remaining to read is 15. That is, GetSize()
        // minus position_.
        sizeToRead = size() - position_;

        if (readSize < sizeToRead) { sizeToRead = readSize; }

        OTPassword::safe_memcpy(data, readSize, &data_[position_], sizeToRead);
        position_ += sizeToRead;
    }

    return sizeToRead;
}

bool Data::Randomize(const std::size_t& size)
{
    SetSize(size);

    if (size == 0) { return false; }

    return OTPassword::randomizeMemory_uint8(data_.data(), size);
}

void Data::Release()
{
    zeroMemory();
    Initialize();
}

void Data::SetSize(const std::size_t& size)
{
    Release();

    if (size > 0) { data_.assign(size, 0); }
}

std::string Data::str() const
{
    if (data_.empty()) { return {}; }

    return std::string{reinterpret_cast<const char*>(&data_[0]), data_.size()};
}

void Data::swap(opentxs::Data&& rhs)
{
    auto& in = dynamic_cast<Data&>(rhs);
    std::swap(data_, in.data_);
    std::swap(position_, in.position_);
}

void Data::zeroMemory()
{
    if (0 < data_.size()) { data_.assign(data_.size(), 0); }
}
}  // namespace implementation
}  // namespace opentxs
