// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "opentxs/util/PIDFile.hpp"

namespace opentxs
{
class Factory;
}  // namespace opentxs

namespace opentxs::implementation
{
class PIDFile final : virtual public opentxs::PIDFile
{
public:
    auto isOpen() const -> bool final { return open_.load(); }

    void Close() final;
    void Open() final;

    ~PIDFile() final;

private:
    friend opentxs::Factory;

    const std::string path_{};
    std::atomic<bool> open_{false};

    static auto can_recover(std::uint32_t pid) -> bool;

    PIDFile(const std::string& path);
    PIDFile() = delete;
    PIDFile(const PIDFile&) = delete;
    PIDFile(PIDFile&&) = delete;
    auto operator=(const PIDFile&) -> PIDFile& = delete;
    auto operator=(PIDFile &&) -> PIDFile& = delete;
};
}  // namespace opentxs::implementation
