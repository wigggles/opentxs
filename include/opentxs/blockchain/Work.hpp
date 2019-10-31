// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_WORK_HPP
#define OPENTXS_BLOCKCHAIN_WORK_HPP

#include "opentxs/Forward.hpp"

#include <string>

namespace opentxs
{
bool operator==(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
bool operator!=(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
bool operator<(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
bool operator<=(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
bool operator>(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
bool operator>=(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
OTWork operator+(const OTWork& lhs, const blockchain::Work& rhs) noexcept;

namespace blockchain
{
class Work
{
public:
    OPENTXS_EXPORT virtual bool operator==(const blockchain::Work& rhs) const
        noexcept = 0;
    OPENTXS_EXPORT virtual bool operator!=(const blockchain::Work& rhs) const
        noexcept = 0;
    OPENTXS_EXPORT virtual bool operator<(const blockchain::Work& rhs) const
        noexcept = 0;
    OPENTXS_EXPORT virtual bool operator<=(const blockchain::Work& rhs) const
        noexcept = 0;
    OPENTXS_EXPORT virtual bool operator>(const blockchain::Work& rhs) const
        noexcept = 0;
    OPENTXS_EXPORT virtual bool operator>=(const blockchain::Work& rhs) const
        noexcept = 0;
    OPENTXS_EXPORT virtual OTWork operator+(const blockchain::Work& rhs) const
        noexcept = 0;

    OPENTXS_EXPORT virtual std::string asHex() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Decimal() const noexcept = 0;

    OPENTXS_EXPORT virtual ~Work() = default;

protected:
    Work() noexcept = default;

private:
    friend OTWork;

    virtual Work* clone() const noexcept = 0;

    Work(const Work& rhs) = delete;
    Work(Work&& rhs) = delete;
    Work& operator=(const Work& rhs) = delete;
    Work& operator=(Work&& rhs) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif
