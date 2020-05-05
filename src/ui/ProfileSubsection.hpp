// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ProfileSubsection.cpp"

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
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
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

namespace ui
{
class ProfileSubsection;
}  // namespace ui

class ContactGroup;
class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ProfileSubsectionList = List<
    ProfileSubsectionExternalInterface,
    ProfileSubsectionInternalInterface,
    ProfileSubsectionRowID,
    ProfileSubsectionRowInterface,
    ProfileSubsectionRowInternal,
    ProfileSubsectionRowBlank,
    ProfileSubsectionSortKey,
    ProfileSubsectionPrimaryID>;
using ProfileSubsectionRow = RowType<
    ProfileSectionRowInternal,
    ProfileSectionInternalInterface,
    ProfileSectionRowID>;

class ProfileSubsection final : public Combined<
                                    ProfileSubsectionList,
                                    ProfileSubsectionRow,
                                    ProfileSectionSortKey>
{
public:
    bool AddItem(
        const std::string& value,
        const bool primary,
        const bool active) const noexcept final;
    bool Delete(const std::string& claimID) const noexcept final;
    std::string Name(const std::string& lang) const noexcept final;
    const identifier::Nym& NymID() const noexcept final { return primary_id_; }
    proto::ContactSectionName Section() const noexcept final
    {
        return row_id_.first;
    }
    bool SetActive(const std::string& claimID, const bool active) const
        noexcept final;
    bool SetPrimary(const std::string& claimID, const bool primary) const
        noexcept final;
    bool SetValue(const std::string& claimID, const std::string& value) const
        noexcept final;
    proto::ContactItemType Type() const noexcept final
    {
        return row_id_.second;
    }

    void reindex(
        const ProfileSectionSortKey& key,
        const CustomData& custom) noexcept final;

    ProfileSubsection(
        const ProfileSectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ProfileSectionRowID& rowID,
        const ProfileSectionSortKey& key,
        const CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    ~ProfileSubsection() = default;

private:
    friend opentxs::Factory;

    static bool check_type(const ProfileSubsectionRowID type);

    void* construct_row(
        const ProfileSubsectionRowID& id,
        const ProfileSubsectionSortKey& index,
        const CustomData& custom) const noexcept final;

    bool last(const ProfileSubsectionRowID& id) const noexcept final
    {
        return ProfileSubsectionList::last(id);
    }
    std::set<ProfileSubsectionRowID> process_group(
        const opentxs::ContactGroup& group) noexcept;
    int sort_key(const ProfileSubsectionRowID type) const noexcept;
    void startup(const CustomData& custom) noexcept;

    ProfileSubsection() = delete;
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    ProfileSubsection& operator=(const ProfileSubsection&) = delete;
    ProfileSubsection& operator=(ProfileSubsection&&) = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ProfileSubsection>;
