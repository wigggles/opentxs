// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NUMLIST_HPP
#define OPENTXS_CORE_NUMLIST_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>
#include <set>
#include <string>

namespace opentxs
{
/** Useful for storing a std::set of longs, serializing to/from comma-separated
 * string, And easily being able to add/remove/verify the individual transaction
 * numbers that are there. (Used by OTTransaction::blank and
 * OTTransaction::successNotice.) Also used in OTMessage, for storing lists of
 * acknowledged request numbers. */
class NumList
{
    std::set<std::int64_t> m_setData;

    /** private for security reasons, used internally only by a function that
     * knows the string length already. if false, means the numbers were already
     * there. (At least one of them.) */
    bool Add(const char* szfNumbers);

public:
    explicit EXPORT NumList(const std::set<std::int64_t>& theNumbers);
    explicit EXPORT NumList(std::set<std::int64_t>&& theNumbers);
    explicit EXPORT NumList(const String& strNumbers);
    explicit EXPORT NumList(const std::string& strNumbers);
    explicit EXPORT NumList(std::int64_t lInput);
    EXPORT NumList();
    EXPORT NumList(const NumList&) = default;
    EXPORT NumList(NumList&&) = default;
    EXPORT NumList& operator=(const NumList&) = default;
    EXPORT NumList& operator=(NumList&&) = default;

    EXPORT ~NumList() = default;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    EXPORT bool Add(const String& strNumbers);

    /** if false, means the numbers were already there. (At least one of them.)
     */
    EXPORT bool Add(const std::string& strNumbers);

    /** if false, means the value was already there. */
    EXPORT bool Add(const std::int64_t& theValue);

    /** if false, means the value was NOT already there. */
    EXPORT bool Remove(const std::int64_t& theValue);

    /** returns true/false (whether value is already there.) */
    EXPORT bool Verify(const std::int64_t& theValue) const;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    EXPORT bool Add(const NumList& theNumList);

    /** if false, means the numbers were already there. (At least one of them.)
     */
    EXPORT bool Add(const std::set<std::int64_t>& theNumbers);

    /** if false, means the numbers were NOT already there. (At least one of
     * them.) */
    EXPORT bool Remove(const std::set<std::int64_t>& theNumbers);

    /** True/False, based on whether values are already there. (ALL theNumbers
     * must be present.) */
    EXPORT bool Verify(const std::set<std::int64_t>& theNumbers) const;

    /** True/False, based on whether OTNumLists MATCH in COUNT and CONTENT (NOT
     * ORDER.) */
    EXPORT bool Verify(const NumList& rhs) const;

    /** True/False, based on whether ANY of rhs are found in *this. */
    EXPORT bool VerifyAny(const NumList& rhs) const;

    /** Verify whether ANY of the numbers on *this are found in setData. */
    EXPORT bool VerifyAny(const std::set<std::int64_t>& setData) const;
    EXPORT std::int32_t Count() const;
    EXPORT bool Peek(std::int64_t& lPeek) const;
    EXPORT bool Pop();

    /** Outputs the numlist as set of numbers. (To iterate OTNumList, call this,
     * then iterate the output.) returns false if the numlist was empty.*/
    EXPORT bool Output(std::set<std::int64_t>& theOutput) const;

    /** Outputs the numlist as a comma-separated string (for serialization,
     * usually.) returns false if the numlist was empty. */
    EXPORT bool Output(String& strOutput) const;
    EXPORT void Release();
};
}  // namespace opentxs
#endif
