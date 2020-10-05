// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UTIL_WORKTYPE_HPP
#define OPENTXS_UTIL_WORKTYPE_HPP

#include <cstdint>

namespace opentxs
{
using OTZMQWorkType = std::uint16_t;

// NOTE 16384 - 32767 are reserved for client applications
enum class WorkType : OTZMQWorkType {
    Shutdown = 0,
    NymCreated = 1,
    BlockchainSyncProgress = 2,
    BlockchainAccountCreated = 3,
};

constexpr auto value(const WorkType in) noexcept
{
    return static_cast<OTZMQWorkType>(in);
}

/*** Tagged Status Message Format
 *
 *   Messages using this taxonomy will always have the first body frame set to a
 *   WorkType value.
 *
 *   Depending on the message type additional body frames may be present as
 *   described below.
 *
 *   Shutdown: reports the pending shutdown of a client session or server
 * session
 *
 *   NymCreated: reports the id of newly generated nyms.
 *       * Additional frames:
 *          1: id as identifier::Nym (encoded as byte sequence)
 *
 *   BlockchainSyncProgress: reports the wallet sync progress for a particular
 *   blockchain.
 *       * Additional frames:
 *          1: chain type as opentxs::blockchain::Type
 *          2: current progress as opentxs::blockchain::block::Height
 *          3: target height as opentxs::blockchain::block::Height
 *
 *   BlockchainAccountCreated: reports the creation of a new blockchain account.
 *       * Additional frames:
 *          1: chain type as opentxs::blockchain::Type
 *          2: account owner as identifier::Nym (encoded as byte sequence)
 *          3: account type as api::client::Blockchain::AccountType
 *          4: account id as Identifier (encoded as byte sequence)
 */
}  // namespace opentxs
#endif
