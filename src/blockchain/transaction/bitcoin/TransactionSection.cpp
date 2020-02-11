// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "Transaction.hpp"
#include "TransactionSection.hpp"

//#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"

//#define OT_METHOD "
// opentxs::blockchain::transaction::bitcoin::TransactionSection::"

namespace opentxs::blockchain::transaction::bitcoin
{
template <class SectionType>
TransactionSection<SectionType>::TransactionSection(
    const TransactionSection<SectionType>& transactionSection)
    : parent_(transactionSection.parent_)
    , position_(transactionSection.position_.load())
    , size_(transactionSection.size_.load())
{
    OT_ASSERT(nullptr != parent_);
}

template <class SectionType>
TransactionSection<SectionType>::TransactionSection(
    const Transaction* parent,
    std::size_t position,
    std::size_t size)
    : parent_(parent)
    , position_(position)
    , size_(size)
{
    OT_ASSERT(nullptr != parent_);
}

// template <class SectionType>
// const SectionType& TransactionSection<SectionType>::at
//    (const std::size_t index) const
//{
//    OT_ASSERT(size_ > index);
//
//    return parent_->at(position_ + index);
//}
//
// template <class SectionType>
// TransactionSection<SectionType>::iterator
// TransactionSection<SectionType>::begin() const
//{
//    return iterator(parent_, position_);
//}
//
// template <class SectionType>
// iterator TransactionSection::end() const
//{
//    return iterator(parent_, position_ + size_);
//}

template <class SectionType>
std::size_t TransactionSection<SectionType>::size() const
{
    return size_;
}

}  // namespace opentxs::blockchain::transaction::bitcoin
