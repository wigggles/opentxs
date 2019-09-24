// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
class Data : virtual public opentxs::Data
{
public:
    bool operator==(const opentxs::Data& rhs) const final;
    bool operator!=(const opentxs::Data& rhs) const final;
    bool operator<(const opentxs::Data& rhs) const final;
    bool operator>(const opentxs::Data& rhs) const final;
    bool operator<=(const opentxs::Data& rhs) const final;
    bool operator>=(const opentxs::Data& rhs) const final;
    Data& operator+=(const opentxs::Data& rhs) final;
    Data& operator+=(const std::uint8_t rhs) final;
    Data& operator+=(const std::uint16_t rhs) final;
    Data& operator+=(const std::uint32_t rhs) final;
    Data& operator+=(const std::uint64_t rhs) final;

    std::string asHex() const final;
    const std::byte& at(const std::size_t position) const final
    {
        return reinterpret_cast<const std::byte&>(data_.at(position));
    }
    const_iterator begin() const final { return const_iterator(this, 0); }
    const_iterator cbegin() const final { return const_iterator(this, 0); }
    const_iterator cend() const final
    {
        return const_iterator(this, data_.size());
    }
    bool empty() const final { return data_.empty(); }
    const void* data() const final { return data_.data(); }
    const_iterator end() const final
    {
        return const_iterator(this, data_.size());
    }
    bool Extract(
        const std::size_t amount,
        opentxs::Data& output,
        const std::size_t pos) const final;
    bool Extract(std::uint8_t& output, const std::size_t pos) const final;
    bool Extract(std::uint16_t& output, const std::size_t pos) const final;
    bool Extract(std::uint32_t& output, const std::size_t pos) const final;
    bool Extract(std::uint64_t& output, const std::size_t pos) const final;
    bool IsEmpty() const final { return empty(); }
    bool IsNull() const final;
    const void* GetPointer() const final { return data_.data(); }
    std::size_t GetSize() const final { return size(); }
    std::size_t size() const final { return data_.size(); }

    void Assign(const opentxs::Data& source) final;
    void Assign(const void* data, const std::size_t& size) final;
    std::byte& at(const std::size_t position) final
    {
        return reinterpret_cast<std::byte&>(data_.at(position));
    }
    iterator begin() final { return iterator(this, 0); }
    void Concatenate(const void* data, const std::size_t& size) final;
    void* data() final { return data_.data(); }
    bool DecodeHex(const std::string& hex) final;
    iterator end() final { return iterator(this, data_.size()); }
    std::size_t OTfread(std::uint8_t* data, const std::size_t& size) final;
    bool Randomize(const std::size_t& size) final;
    void Release() final;
    void reset() final { position_ = 0; }
    void SetSize(const std::size_t& size) final;
    std::string str() const override;
    void swap(opentxs::Data&& rhs) final;
    void zeroMemory() final;

    ~Data() override = default;

protected:
    using Vector = std::vector<std::uint8_t>;

    Vector data_{};
    std::size_t position_{0};

    void Initialize();

    Data() = default;
    explicit Data(const void* data, std::size_t size);
    explicit Data(const Armored& source);
    explicit Data(const std::vector<unsigned char>& sourceVector);
    explicit Data(const std::vector<std::byte>& sourceVector);
    explicit Data(const Vector& rhs, const std::size_t size);

private:
    friend opentxs::Data;

    Data* clone() const override
    {
        return new Data(this->data_, this->position_);
    }

    bool check_sub(const std::size_t pos, const std::size_t target) const;
    void concatenate(const Vector& data);

    Data(const Data& rhs) = delete;
    Data(Data&& rhs) = delete;
    Data& operator=(const Data& rhs) = delete;
    Data& operator=(Data&& rhs) = delete;
};
}  // namespace opentxs::implementation
