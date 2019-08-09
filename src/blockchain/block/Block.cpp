// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Block.hpp"

//  #define OT_METHOD "opentxs::blockchain::block::implementation::Block::"

namespace opentxs::blockchain::block::implementation
{
Block::Block(
    const api::internal::Core& api,
    const block::Header& header) noexcept
    : api_(api)
    , base_header_(header)
{
}
}  // namespace opentxs::blockchain::block::implementation
