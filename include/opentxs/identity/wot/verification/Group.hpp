// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_WOT_VERIFICATION_GROUP_HPP
#define OPENTXS_IDENTITY_WOT_VERIFICATION_GROUP_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace identity
{
namespace wot
{
namespace verification
{
class Group
{
public:
    using value_type = Nym;
    using iterator = opentxs::iterator::Bidirectional<Group, value_type>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Group, const value_type>;
    using SerializedType = proto::VerificationGroup;

    EXPORT static const VersionNumber DefaultVersion;

    EXPORT virtual operator SerializedType() const noexcept = 0;

    /// Throws std::out_of_range for invalid position
    EXPORT virtual const value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    EXPORT virtual const_iterator begin() const noexcept = 0;
    EXPORT virtual const_iterator cbegin() const noexcept = 0;
    EXPORT virtual const_iterator cend() const noexcept = 0;
    EXPORT virtual const_iterator end() const noexcept = 0;
    EXPORT virtual std::size_t size() const noexcept = 0;
    EXPORT virtual VersionNumber Version() const noexcept = 0;

    EXPORT virtual bool AddItem(
        const identifier::Nym& claimOwner,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value = Item::Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = Item::DefaultVersion) noexcept = 0;
    EXPORT virtual bool AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept = 0;
    EXPORT virtual bool DeleteItem(const Identifier& item) noexcept = 0;
    /// Throws std::out_of_range for invalid position
    EXPORT virtual value_type& at(const std::size_t position) noexcept(
        false) = 0;
    EXPORT virtual iterator begin() noexcept = 0;
    EXPORT virtual iterator end() noexcept = 0;

    EXPORT virtual ~Group() = default;

protected:
    Group() = default;

private:
    Group(const Group&) = delete;
    Group(Group&&) = delete;
    Group& operator=(const Group&) = delete;
    Group& operator=(Group&&) = delete;
};
}  // namespace verification
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
#endif
