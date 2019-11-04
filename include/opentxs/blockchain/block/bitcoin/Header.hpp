// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_HEADER_HPP
#define OPENTXS_BLOCKCHAIN_HEADER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header : virtual public block::Header
{
public:
    OPENTXS_EXPORT virtual OTData Encode() const noexcept = 0;
    OPENTXS_EXPORT virtual const block::Hash& MerkleRoot() const noexcept = 0;
    OPENTXS_EXPORT virtual std::uint32_t Nonce() const noexcept = 0;
    OPENTXS_EXPORT virtual Time Timestamp() const noexcept = 0;
    OPENTXS_EXPORT virtual std::uint32_t Version() const noexcept = 0;

    OPENTXS_EXPORT ~Header() override = default;

protected:
    Header() noexcept = default;

private:
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    Header& operator=(const Header&) = delete;
    Header& operator=(Header&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
