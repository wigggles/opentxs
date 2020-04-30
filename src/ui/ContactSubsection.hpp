// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/TransferBalanceItem.cpp"

#pragma once

#include <set>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
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
class ContactSubsection;
}  // namespace ui

class ContactGroup;
class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactSubsectionList = List<
    ContactSubsectionExternalInterface,
    ContactSubsectionInternalInterface,
    ContactSubsectionRowID,
    ContactSubsectionRowInterface,
    ContactSubsectionRowInternal,
    ContactSubsectionRowBlank,
    ContactSubsectionSortKey,
    ContactSubsectionPrimaryID>;
using ContactSubsectionRow = RowType<
    ContactSectionRowInternal,
    ContactSectionInternalInterface,
    ContactSectionRowID>;

class ContactSubsection final : public Combined<
                                    ContactSubsectionList,
                                    ContactSubsectionRow,
                                    ContactSectionSortKey>
{
public:
    std::string Name(const std::string& lang) const noexcept final;
    proto::ContactItemType Type() const noexcept final
    {
        return row_id_.second;
    }

    void reindex(
        const ContactSectionSortKey& key,
        const CustomData& custom) noexcept final;

    ContactSubsection(
        const ContactSectionInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ContactSectionRowID& rowID,
        const ContactSectionSortKey& key,
        const CustomData& custom
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    ~ContactSubsection() = default;

private:
    friend opentxs::Factory;

    static bool check_type(const ContactSubsectionRowID type) noexcept;

    void* construct_row(
        const ContactSubsectionRowID& id,
        const ContactSubsectionSortKey& index,
        const CustomData& custom) const noexcept final;

    bool last(const ContactSubsectionRowID& id) const noexcept final
    {
        return ContactSubsectionList::last(id);
    }
    std::set<ContactSubsectionRowID> process_group(
        const opentxs::ContactGroup& group) noexcept;
    int sort_key(const ContactSubsectionRowID type) const noexcept;
    void startup(const CustomData custom) noexcept;

    ContactSubsection() = delete;
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    ContactSubsection& operator=(const ContactSubsection&) = delete;
    ContactSubsection& operator=(ContactSubsection&&) = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactSubsection>;
