// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "internal/ui/UI.hpp"  // IWYU pragma: associated

#include <map>
#include <mutex>

#if OT_BLOCKCHAIN
#include "internal/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs::ui
{
#if OT_BLOCKCHAIN
auto AccountID(const api::Core& api, const blockchain::Type chain) noexcept
    -> const Identifier&
{
    static std::mutex mutex_;
    static auto map = std::map<blockchain::Type, OTIdentifier>{};

    Lock lock(mutex_);

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().Identifier());
    auto& output = it->second;
    const auto preimage = std::string{"blockchain-account-"} +
                          std::to_string(static_cast<std::uint32_t>(chain));
    output->CalculateDigest(preimage);

    return output;
}

auto AccountName([[maybe_unused]] const blockchain::Type chain) noexcept
    -> std::string
{
    return "This device";
}

auto NotaryID(const api::Core& api, const blockchain::Type chain) noexcept
    -> const identifier::Server&
{
    static std::mutex mutex_;
    static auto map = std::map<blockchain::Type, OTServerID>{};

    Lock lock(mutex_);

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().ServerID());
    auto& output = it->second;
    const auto preimage = std::string{"blockchain-"} +
                          std::to_string(static_cast<std::uint32_t>(chain));
    output->CalculateDigest(preimage);

    return output;
}

auto UnitID(const api::Core& api, const blockchain::Type chain) noexcept
    -> const identifier::UnitDefinition&
{
    static std::mutex mutex_;
    static auto map = std::map<blockchain::Type, OTUnitID>{};

    Lock lock(mutex_);

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().UnitID());
    auto& output = it->second;
    const auto preimage = blockchain::internal::Ticker(chain);
    output->CalculateDigest(preimage);

    return output;
}
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::ui
