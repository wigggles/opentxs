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

#ifndef OPENTXS_UI_PROFILE_ITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_ITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileItemType =
    Row<opentxs::ui::ProfileItem, ProfileSubsectionParent, OTIdentifier>;

class ProfileItem : public ProfileItemType
{
public:
    std::string ClaimID() const override { return id_->str(); }
    bool Delete() const override;
    bool IsActive() const override { return active_; }
    bool IsPrimary() const override { return primary_; }
    bool SetActive(const bool& active) const override;
    bool SetPrimary(const bool& primary) const override;
    bool SetValue(const std::string& value) const override;
    std::string Value() const override { return value_; }

    ~ProfileItem() = default;

private:
    friend Factory;

    const api::client::Wallet& wallet_;
    const bool active_{false};
    const bool primary_{false};
    const std::string value_{""};
    const std::time_t start_{0};
    const std::time_t end_{0};

    bool add_claim(const Claim& claim) const;
    Claim as_claim() const;

    ProfileItem(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const ProfileSubsectionParent& parent,
        const opentxs::ContactItem& item);
    ProfileItem() = delete;
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    ProfileItem& operator=(const ProfileItem&) = delete;
    ProfileItem& operator=(ProfileItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_ITEM_IMPLEMENTATION_HPP
