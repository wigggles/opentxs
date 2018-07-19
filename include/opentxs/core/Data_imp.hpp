// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_DATA_IMPLEMENTATION_HPP
#define OPENTXS_CORE_DATA_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace implementation
{
class Data : virtual public opentxs::Data
{
public:
    EXPORT bool operator==(const opentxs::Data& rhs) const override;
    EXPORT bool operator!=(const opentxs::Data& rhs) const override;
    EXPORT Data& operator+=(const opentxs::Data& rhs) override;

    EXPORT std::string asHex() const override;
    EXPORT Data* clone() const override;
    EXPORT bool empty() const override;
    EXPORT bool IsEmpty() const override;
    EXPORT const void* GetPointer() const override;
    EXPORT std::size_t GetSize() const override;

    EXPORT void Assign(const opentxs::Data& source) override;
    EXPORT void Assign(const void* data, const std::size_t& size) override;
    EXPORT void Concatenate(const void* data, const std::size_t& size) override;
    EXPORT std::size_t OTfread(std::uint8_t* data, const std::size_t& size)
        override;
    EXPORT bool Randomize(const std::size_t& size) override;
    EXPORT void Release() override;
    EXPORT void reset() override;
    EXPORT void SetSize(const std::size_t& size) override;
    EXPORT void swap(opentxs::Data&& rhs) override;
    EXPORT void zeroMemory() override;

    EXPORT virtual ~Data() = default;

protected:
    void Initialize();
    void swap(Data& rhs);

    Data() = default;
    explicit Data(const void* data, std::size_t size);
    explicit Data(const OTASCIIArmor& source);
    explicit Data(const std::vector<unsigned char>& sourceVector);
    Data(const Data& rhs);
    Data(Data&& rhs);
    Data& operator=(const Data& rhs);
    Data& operator=(Data&& rhs);

private:
    friend opentxs::Data;

    typedef std::vector<std::uint8_t> Vector;

    Vector data_{};
    std::size_t position_{};

    void concatenate(const Vector& data);
};
}  // namespace opentxs::implementation
}  // namespace opentxs
#endif  // OPENTXS_CORE_DATA_IMPLEMENTATION_HPP
