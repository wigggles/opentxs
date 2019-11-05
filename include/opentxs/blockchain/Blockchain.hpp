// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCKCHAIN_HPP
#define OPENTXS_BLOCKCHAIN_BLOCKCHAIN_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace blockchain
{
using Hash = Data;
using pHash = OTData;

namespace block
{
using Height = std::int64_t;
using Hash = blockchain::Hash;
using pHash = blockchain::pHash;
using Position = std::pair<Height, pHash>;

pHash BlankHash() noexcept;
}  // namespace block

namespace filter
{
using Hash = blockchain::Hash;
using pHash = blockchain::pHash;
}  // namespace filter

namespace p2p
{
std::string DisplayService(const Service service) noexcept;
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs
#endif
