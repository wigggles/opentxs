// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/iterator/Bidirectional.hpp"

#include <atomic>

//#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"

namespace opentxs::blockchain::transaction::bitcoin
{

class Transaction;

template <typename SectionType>
class TransactionSection
{
public:
    using iterator = opentxs::iterator::
        Bidirectional<const TransactionSection<SectionType>, const SectionType>;
    using difference_type = std::size_t;
    using value_type = SectionType;
    using pointer = SectionType*;
    using const_pointer = const SectionType*;
    using reference = SectionType&;
    using const_reference = const SectionType&;
    using iterator_category = std::forward_iterator_tag;

    TransactionSection(const TransactionSection& transactionSection);
    TransactionSection(
        const Transaction* parent,
        std::size_t position,
        std::size_t size);

    //    const_reference at(const std::size_t index) const;
    //    iterator begin() const;
    //    iterator end() const;
    std::size_t size() const;

    virtual ~TransactionSection() = default;

    // protected:
public:
    TransactionSection() = default;

private:
    const Transaction* parent_{nullptr};
    std::atomic<std::size_t> position_{0};
    std::atomic<std::size_t> size_{0};

    TransactionSection(TransactionSection&&) = delete;
    TransactionSection& operator=(const TransactionSection&) = delete;
    TransactionSection& operator=(TransactionSection&&) = delete;
};

}  // namespace opentxs::blockchain::transaction::bitcoin
