// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UTIL_PIDFILE_HPP
#define OPENTXS_UTIL_PIDFILE_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

namespace opentxs
{
class PIDFile
{
public:
    virtual bool isOpen() const = 0;

    virtual void Close() = 0;
    virtual void Open() = 0;

    virtual ~PIDFile() = default;

protected:
    PIDFile() = default;

private:
    PIDFile(const PIDFile&) = delete;
    PIDFile(PIDFile&&) = delete;
    PIDFile& operator=(const PIDFile&) = delete;
    PIDFile& operator=(PIDFile&&) = delete;
};
}  // namespace opentxs
#endif
