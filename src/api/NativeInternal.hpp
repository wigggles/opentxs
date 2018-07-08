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

#ifndef OPENTXS_CORE_API_NATIVEINTERNAL_HPP
#define OPENTXS_CORE_API_NATIVEINTERNAL_HPP

#include "Internal.hpp"

namespace
{
/** Callbacks in this form allow OpenSSL to query opentxs to get key encryption
 *  and decryption passwords*/
extern "C" {
typedef std::int32_t INTERNAL_PASSWORD_CALLBACK(
    char* buf,
    std::int32_t size,
    std::int32_t rwflag,
    void* userdata);
}
}  // namespace

namespace opentxs::api
{
class NativeInternal : virtual public Native
{
public:
    virtual INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const = 0;
    virtual OTCaller& GetPasswordCaller() const = 0;
    virtual void Init() = 0;
    virtual void shutdown() = 0;

    virtual ~NativeInternal() = default;

protected:
    NativeInternal() = default;

private:
    NativeInternal(const NativeInternal&) = delete;
    NativeInternal(NativeInternal&&) = delete;
    NativeInternal& operator=(const NativeInternal&) = delete;
    NativeInternal& operator=(NativeInternal&&) = delete;
};
}  // namespace opentxs::api
#endif  // OPENTXS_CORE_API_NATIVEINTERNAL_HPP
