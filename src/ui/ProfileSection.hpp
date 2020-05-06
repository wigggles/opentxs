// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ProfileSection.cpp"

#pragma once

#include <set>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/ProfileSection.hpp"
#include "ui/Combined.hpp"
#include "ui/List.hpp"
#include "ui/RowType.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class ContactSection;
class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ProfileSectionList = List<
    ProfileSectionExternalInterface,
    ProfileSectionInternalInterface,
    ProfileSectionRowID,
    ProfileSectionRowInterface,
    ProfileSectionRowInternal,
    ProfileSectionRowBlank,
    ProfileSectionSortKey,
    ProfileSectionPrimaryID>;
using ProfileSectionRow =
    RowType<ProfileRowInternal, ProfileInternalInterface, ProfileRowID>;

class ProfileSection final
    : public Combined<ProfileSectionList, ProfileSectionRow, ProfileSortKey>
{
public:
    bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const noexcept final;
    bool Delete(const int type, const std::string& claimID) const
        noexcept final;
    ItemTypeList Items(const std::string& lang) const noexcept final;
#if OT_QT
    int FindRow(const ProfileSectionRowID& id, const ProfileSectionSortKey& key)
        const noexcept final
    {
        return find_row(id, key);
    }
#endif
    std::string Name(const std::string& lang) const noexcept final;
    const identifier::Nym& NymID() const noexcept final { return primary_id_; }
    bool SetActive(
        const int type,
        const std::string& claimID,
        const bool active) const noexcept final;
    bool SetPrimary(
        const int type,
        const std::string& claimID,
        const bool primary) const noexcept final;
    bool SetValue(
        const int type,
        const std::string& claimID,
        const std::string& value) const noexcept final;
    proto::ContactSectionName Type() const noexcept final { return row_id_; }

    void reindex(
        const ProfileSortKey& key,
        const CustomData& custom) noexcept final;

    ProfileSection(
        const ProfileInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ProfileRowID& rowID,
        const ProfileSortKey& key,
        const CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    ~ProfileSection() = default;

private:
    friend opentxs::Factory;

    static int sort_key(const ProfileSectionRowID type) noexcept;
    static bool check_type(const ProfileSectionRowID type) noexcept;

    void* construct_row(
        const ProfileSectionRowID& id,
        const ProfileSectionSortKey& index,
        const CustomData& custom) const noexcept final;

    bool last(const ProfileSectionRowID& id) const noexcept final
    {
        return ProfileSectionList::last(id);
    }
    std::set<ProfileSectionRowID> process_section(
        const opentxs::ContactSection& section) noexcept;
    void startup(const CustomData& custom) noexcept;

    ProfileSection() = delete;
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    ProfileSection& operator=(const ProfileSection&) = delete;
    ProfileSection& operator=(ProfileSection&&) = delete;
};
}  // namespace opentxs::ui::implementation
