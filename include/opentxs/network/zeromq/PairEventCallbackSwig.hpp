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

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_HPP

#include "opentxs/Forward.hpp"

#ifdef SWIG
// clang-format off
%feature("director") PairEventCallbackSwig;
// clang-format on
#endif  // SWIG

namespace opentxs
{
class PairEventCallbackSwig
{
public:
    virtual void ProcessRename(const std::string& issuer) const = 0;
    virtual void ProcessStoreSecret(const std::string& issuer) const = 0;

    virtual ~PairEventCallbackSwig() = default;

protected:
    PairEventCallbackSwig() = default;

private:
    PairEventCallbackSwig(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig(PairEventCallbackSwig&&) = default;
    PairEventCallbackSwig& operator=(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig& operator=(PairEventCallbackSwig&&) = default;
};
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_HPP
