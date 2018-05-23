/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/Internal.hpp"

#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/MultipartMessage.hpp"

namespace opentxs::network::zeromq
{
FrameIterator::FrameIterator(
    const MultipartMessage* parent,
    std::size_t position)
    : position_(position)
    , parent_(parent)
{
    OT_ASSERT(nullptr != parent_);
}

FrameIterator::FrameIterator(const FrameIterator& frameIterator)
    : position_(frameIterator.position_.load())
    , parent_(frameIterator.parent_)
{
    OT_ASSERT(nullptr != parent_);
}

FrameIterator& FrameIterator::operator=(const FrameIterator& frameIterator)
{
    position_ = frameIterator.position_.load();
    parent_ = frameIterator.parent_;

    return *this;
}

const Message& FrameIterator::operator*() const
{
    OT_ASSERT(nullptr != parent_);

    return parent_->at(position_);
}

Message& FrameIterator::operator*()
{
    OT_ASSERT(nullptr != parent_);

    return const_cast<MultipartMessage*>(parent_)->at(position_);
}

bool FrameIterator::operator==(const FrameIterator& rhs) const
{
    OT_ASSERT(nullptr != parent_);

    return (parent_ == rhs.parent_) && (position_ == rhs.position_);
}

bool FrameIterator::operator!=(const FrameIterator& rhs) const
{
    OT_ASSERT(nullptr != parent_);

    return !(*this == rhs);
}

FrameIterator& FrameIterator::operator++()
{
    OT_ASSERT(nullptr != parent_);

    position_++;

    return *this;
}

FrameIterator FrameIterator::operator++(int)
{
    OT_ASSERT(nullptr != parent_);

    FrameIterator output(*this);
    position_++;

    return output;
}
}  // namespace opentxs::network::zeromq
