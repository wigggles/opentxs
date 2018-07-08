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

#ifndef OPENTXS_CORE_CRYPTO_NULLCALLBACK_HPP
#define OPENTXS_CORE_CRYPTO_NULLCALLBACK_HPP

#include "Internal.hpp"

namespace opentxs::implementation
{
class NullCallback final : virtual public OTCallback
{
public:
    void runOne(const char* display, OTPassword& output) const override;
    void runTwo(const char* display, OTPassword& output) const override;

    NullCallback() = default;

    ~NullCallback() = default;

private:
    friend Factory;

    static const std::string password_;

    NullCallback(const NullCallback&) = delete;
    NullCallback(NullCallback&&) = delete;
    NullCallback& operator=(const NullCallback&) = delete;
    NullCallback& operator=(NullCallback&&) = delete;
};
}  // namespace opentxs::implementation
#endif  // OPENTXS_CORE_CRYPTO_NULLCALLBACK_HPP
