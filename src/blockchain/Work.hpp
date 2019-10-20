// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace mp = boost::multiprecision;

namespace opentxs::blockchain::implementation
{
class Work : virtual public blockchain::Work
{
public:
    using Type = mp::cpp_bin_float_double;

    bool operator==(const blockchain::Work& rhs) const noexcept final;
    bool operator!=(const blockchain::Work& rhs) const noexcept final;
    bool operator<(const blockchain::Work& rhs) const noexcept final;
    bool operator<=(const blockchain::Work& rhs) const noexcept final;
    bool operator>(const blockchain::Work& rhs) const noexcept final;
    bool operator>=(const blockchain::Work& rhs) const noexcept final;
    OTWork operator+(const blockchain::Work& rhs) const noexcept final;

    std::string asHex() const noexcept final;
    std::string Decimal() const noexcept final { return data_.str(); }

    ~Work() final = default;

private:
    friend opentxs::Factory;

    Type data_;

    Work* clone() const noexcept final { return new Work(*this); }

    Work(Type&& data) noexcept;
    Work() noexcept;
    Work(const Work& rhs) noexcept;
    Work(Work&& rhs) = delete;
    Work& operator=(const Work& rhs) = delete;
    Work& operator=(Work&& rhs) = delete;
};
}  // namespace opentxs::blockchain::implementation
