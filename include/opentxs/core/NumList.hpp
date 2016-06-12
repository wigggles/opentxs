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

#ifndef OPENTXS_CORE_OTNUMLIST_HPP
#define OPENTXS_CORE_OTNUMLIST_HPP

#include <cstdint>
#include <set>
#include <string>

namespace opentxs
{

class OTAsymmetricKey;
class OTPasswordData;
class String;

// Useful for storing a std::set of longs,
// serializing to/from comma-separated string,
// And easily being able to add/remove/verify the
// individual transaction numbers that are there.
// (Used by OTTransaction::blank and
// OTTransaction::successNotice.)
// Also used in OTMessage, for storing lists of acknowledged
// request numbers.
//
class NumList
{
    std::set<int64_t> m_setData;

    // private for security reasons, used internally only by a function that
    // knows the string length already.
    bool Add(const char* szfNumbers); // if false, means the numbers were
                                      // already there. (At least one of them.)

public:
    EXPORT NumList(const std::set<int64_t>& theNumbers);
    //        OTNumList(const char* szNumbers); // removed for security
    // reasons.
    EXPORT NumList(const String& strNumbers);
    EXPORT NumList(const std::string& strNumbers);
    EXPORT NumList(int64_t lInput);
    EXPORT NumList();
    EXPORT ~NumList();
    EXPORT bool Add(const String& strNumbers); // if false, means the numbers
                                               // were already there. (At
                                               // least one of them.)
    EXPORT bool Add(const std::string& strNumbers); // if false, means the
                                                    // numbers were already
                                                    // there. (At least one of
                                                    // them.)
    EXPORT bool Add(const int64_t& theValue); // if false, means the value was
                                              // already there.
    EXPORT bool Remove(const int64_t& theValue); // if false, means the value
                                                 // was NOT already there.
    EXPORT bool Verify(const int64_t& theValue) const; // returns true/false
                                                       // (whether value is
                                                       // already there.)
    EXPORT bool Add(const NumList& theNumList); // if false, means the numbers
                                                // were already there. (At
                                                // least one of them.)
    EXPORT bool Add(const std::set<int64_t>& theNumbers); // if false, means the
                                                          // numbers were
                                                          // already there. (At
                                                          // least one of them.)
    EXPORT bool Remove(const std::set<int64_t>& theNumbers); // if false, means
                                                             // the numbers were
                                                             // NOT already
                                                             // there. (At least
                                                             // one of them.)
    EXPORT bool Verify(const std::set<int64_t>& theNumbers)
        const; // True/False, based on whether values are already there. (ALL
               // theNumbers must be present.)
    EXPORT bool Verify(const NumList& rhs) const; // True/False, based on
                                                  // whether OTNumLists MATCH
                                                  // in COUNT and CONTENT (NOT
                                                  // ORDER.)
    EXPORT bool VerifyAny(const NumList& rhs) const; // True/False, based on
                                                     // whether ANY of rhs are
                                                     // found in *this.
    EXPORT bool VerifyAny(const std::set<int64_t>& setData) const; // Verify
                                                                   // whether
                                                                   // ANY of the
                                                                   // numbers on
                                                                   // *this are
                                                                   // found in
                                                                   // setData.
    EXPORT int32_t Count() const;
    EXPORT bool Peek(int64_t& lPeek) const;
    EXPORT bool Pop();
    // Outputs the numlist as set of numbers. (To iterate OTNumList, call this,
    // then iterate the output.)
    EXPORT bool Output(std::set<int64_t>& theOutput) const; // returns false if
                                                            // the numlist was
                                                            // empty.

    // Outputs the numlist as a comma-separated string (for serialization,
    // usually.)
    EXPORT bool Output(String& strOutput) const; // returns false if the
                                                 // numlist was empty.
    EXPORT void Release();
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTNUMLIST_HPP
