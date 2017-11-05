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

#ifndef OPENTXS_CLIENT_NYMDATA_HPP
#define OPENTXS_CLIENT_NYMDATA_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{

class ContactData;
class Nym;

namespace api
{

class Wallet;
}

class NymData
{
public:
    NymData(const NymData&) = default;
    NymData(NymData&&) = default;

    std::uint32_t GetType() const;
    std::string Name() const;
    std::string PaymentCode(const std::uint32_t currency) const;
    std::string PaymentCode(const proto::ContactItemType currency) const;
    std::string PreferredOTServer() const;
    std::string PrintContactData() const;
    proto::ContactItemType Type() const;
    bool Valid() const;

    bool AddPaymentCode(
        const std::string& code,
        const std::uint32_t currency,
        const bool primary,
        const bool active);
    bool AddPaymentCode(
        const std::string& code,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active);
    bool AddPreferredOTServer(const std::string& id, const bool primary);
    bool SetType(const proto::ContactItemType type, const std::string& name);
    bool SetType(const std::uint32_t type, const std::string& name);

    ~NymData() = default;

private:
    friend class api::Wallet;

    std::shared_ptr<Nym> nym_;

    const ContactData& data() const;

    Nym& nym();

    NymData(const std::shared_ptr<Nym>& nym);
    NymData() = delete;
    NymData& operator=(const NymData&) = delete;
    NymData& operator=(NymData&&) = delete;
};
}  // namespace opentxs

#ifdef SWIG
// clang-format off
%ignore NymData::AddPaymentCode(const std::string&, const std::ContactItemType, const bool, const bool);
%ignore NymData::PaymentCode(const proto::ContactItemType) const;
%ignore NymData::SetType(const proto::ContactItemType);
%ignore NymData::Type() const;
// clang-format on
#endif  // SWIG
#endif  // OPENTXS_CLIENT_NYMDATA_HPP
