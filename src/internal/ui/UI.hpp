// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/AccountSummaryItem.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/ContactSection.hpp"
#include "opentxs/ui/ContactSubsection.hpp"
#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/PayableListItem.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/ProfileSection.hpp"
#include "opentxs/ui/ProfileSubsection.hpp"

namespace opentxs::ui::internal
{
namespace blank
{
struct AccountListItem;
struct AccountSummaryItem;
struct ActivitySummaryItem;
struct ActivityThreadItem;
struct BalanceItem;
struct ContactItem;
struct ContactListItem;
struct ContactSection;
struct ContactSubsection;
struct IssuerItem;
struct PayableListItem;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
}  // namespace blank

struct AccountActivity;
struct AccountList;
struct AccountListItem;
struct AccountSummary;
struct AccountSummaryItem;
struct ActivitySummary;
struct ActivitySummaryItem;
struct ActivityThread;
struct ActivityThreadItem;
struct BalanceItem;
struct Contact;
struct ContactItem;
struct ContactList;
struct ContactListItem;
struct ContactSection;
struct ContactSubsection;
struct IssuerItem;
struct MessagableList;
struct PayableList;
struct PayableListItem;
struct Profile;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
}  // namespace opentxs::ui::internal

namespace opentxs::ui::implementation
{
using CustomData = std::vector<const void*>;

// Account activity
using AccountActivityPrimaryID = OTNymID;
using AccountActivityExternalInterface = ui::AccountActivity;
using AccountActivityInternalInterface = ui::internal::AccountActivity;
/** WorkflowID, state */
using AccountActivityRowID = std::pair<OTIdentifier, proto::PaymentEventType>;
using AccountActivityRowInterface = ui::BalanceItem;
using AccountActivityRowInternal = ui::internal::BalanceItem;
using AccountActivityRowBlank = ui::internal::blank::BalanceItem;
using AccountActivitySortKey = std::chrono::system_clock::time_point;

// Account list
using AccountListPrimaryID = OTNymID;
using AccountListExternalInterface = ui::AccountList;
using AccountListInternalInterface = ui::internal::AccountList;
using AccountListRowID = OTIdentifier;
using AccountListRowInterface = ui::AccountListItem;
using AccountListRowInternal = ui::internal::AccountListItem;
using AccountListRowBlank = ui::internal::blank::AccountListItem;
// type, notary ID
using AccountListSortKey = std::pair<proto::ContactItemType, std::string>;

// Account summary
using AccountSummaryPrimaryID = OTNymID;
using AccountSummaryExternalInterface = ui::AccountSummary;
using AccountSummaryInternalInterface = ui::internal::AccountSummary;
using AccountSummaryRowID = OTNymID;
using AccountSummaryRowInterface = ui::IssuerItem;
using AccountSummaryRowInternal = ui::internal::IssuerItem;
using AccountSummaryRowBlank = ui::internal::blank::IssuerItem;
using AccountSummarySortKey = std::pair<bool, std::string>;

using IssuerItemPrimaryID = OTNymID;
using IssuerItemExternalInterface = AccountSummaryRowInterface;
using IssuerItemInternalInterface = ui::internal::IssuerItem;
using IssuerItemRowID = std::pair<OTIdentifier, proto::ContactItemType>;
using IssuerItemRowInterface = ui::AccountSummaryItem;
using IssuerItemRowInternal = ui::internal::AccountSummaryItem;
using IssuerItemRowBlank = ui::internal::blank::AccountSummaryItem;
using IssuerItemSortKey = std::string;

// Activity summary
using ActivitySummaryPrimaryID = OTNymID;
using ActivitySummaryExternalInterface = ui::ActivitySummary;
using ActivitySummaryInternalInterface = ui::internal::ActivitySummary;
using ActivitySummaryRowID = OTIdentifier;
using ActivitySummaryRowInterface = ui::ActivitySummaryItem;
using ActivitySummaryRowInternal = ui::internal::ActivitySummaryItem;
using ActivitySummaryRowBlank = ui::internal::blank::ActivitySummaryItem;
using ActivitySummarySortKey =
    std::pair<std::chrono::system_clock::time_point, std::string>;

// Activity thread
using ActivityThreadPrimaryID = OTNymID;
using ActivityThreadExternalInterface = ui::ActivityThread;
using ActivityThreadInternalInterface = ui::internal::ActivityThread;
/** item id, box, accountID, taskID */
using ActivityThreadRowID = std::tuple<OTIdentifier, StorageBox, OTIdentifier>;
using ActivityThreadRowInterface = ui::ActivityThreadItem;
using ActivityThreadRowInternal = ui::internal::ActivityThreadItem;
using ActivityThreadRowBlank = ui::internal::blank::ActivityThreadItem;
/** timestamp, index */
using ActivityThreadSortKey =
    std::pair<std::chrono::system_clock::time_point, std::uint64_t>;

// Contact
using ContactPrimaryID = OTIdentifier;
using ContactExternalInterface = ui::Contact;
using ContactInternalInterface = ui::internal::Contact;
using ContactRowID = proto::ContactSectionName;
using ContactRowInterface = ui::ContactSection;
using ContactRowInternal = ui::internal::ContactSection;
using ContactRowBlank = ui::internal::blank::ContactSection;
using ContactSortKey = int;

using ContactSectionPrimaryID = ContactPrimaryID;
using ContactSectionExternalInterface = ContactRowInterface;
using ContactSectionInternalInterface = ui::internal::ContactSection;
using ContactSectionRowID =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ContactSectionRowInterface = ui::ContactSubsection;
using ContactSectionRowInternal = ui::internal::ContactSubsection;
using ContactSectionRowBlank = ui::internal::blank::ContactSubsection;
using ContactSectionSortKey = int;

using ContactSubsectionPrimaryID = ContactSectionPrimaryID;
using ContactSubsectionExternalInterface = ContactSectionRowInterface;
using ContactSubsectionInternalInterface = ui::internal::ContactSubsection;
using ContactSubsectionRowID = OTIdentifier;
using ContactSubsectionRowInterface = ui::ContactItem;
using ContactSubsectionRowInternal = ui::internal::ContactItem;
using ContactSubsectionRowBlank = ui::internal::blank::ContactItem;
using ContactSubsectionSortKey = int;

// Contact list
using ContactListPrimaryID = OTNymID;
using ContactListExternalInterface = ui::ContactList;
using ContactListInternalInterface = ui::internal::ContactList;
using ContactListRowID = OTIdentifier;
using ContactListRowInterface = ui::ContactListItem;
using ContactListRowInternal = ui::internal::ContactListItem;
using ContactListRowBlank = ui::internal::blank::ContactListItem;
using ContactListSortKey = std::string;

// Messagable list
using MessagableListPrimaryID = OTNymID;
using MessagableExternalInterface = ui::MessagableList;
using MessagableInternalInterface = ui::internal::MessagableList;
using MessagableListRowID = ContactListRowID;
using MessagableListRowInterface = ContactListRowInterface;
using MessagableListRowInternal = ContactListRowInternal;
using MessagableListRowBlank = ContactListRowBlank;
using MessagableListSortKey = std::string;

// Payable list
using PayablePrimaryID = OTNymID;
using PayableExternalInterface = ui::PayableList;
using PayableInternalInterface = ui::internal::PayableList;
using PayableListRowID = ContactListRowID;
using PayableListRowInterface = ui::PayableListItem;
using PayableListRowInternal = ui::internal::PayableListItem;
using PayableListRowBlank = ui::internal::blank::PayableListItem;
using PayableListSortKey = std::string;

// Profile
using ProfilePrimaryID = OTNymID;
using ProfileExternalInterface = ui::Profile;
using ProfileInternalInterface = ui::internal::Profile;
using ProfileRowID = proto::ContactSectionName;
using ProfileRowInterface = ui::ProfileSection;
using ProfileRowInternal = ui::internal::ProfileSection;
using ProfileRowBlank = ui::internal::blank::ProfileSection;
using ProfileSortKey = int;

using ProfileSectionPrimaryID = ProfilePrimaryID;
using ProfileSectionExternalInterface = ProfileRowInterface;
using ProfileSectionInternalInterface = ui::internal::ProfileSection;
using ProfileSectionRowID =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ProfileSectionRowInterface = ui::ProfileSubsection;
using ProfileSectionRowInternal = ui::internal::ProfileSubsection;
using ProfileSectionRowBlank = ui::internal::blank::ProfileSubsection;
using ProfileSectionSortKey = int;

using ProfileSubsectionPrimaryID = ProfileSectionPrimaryID;
using ProfileSubsectionExternalInterface = ProfileSectionRowInterface;
using ProfileSubsectionInternalInterface = ui::internal::ProfileSubsection;
using ProfileSubsectionRowID = OTIdentifier;
using ProfileSubsectionRowInterface = ui::ProfileItem;
using ProfileSubsectionRowInternal = ui::internal::ProfileItem;
using ProfileSubsectionRowBlank = ui::internal::blank::ProfileItem;
using ProfileSubsectionSortKey = int;
}  // namespace opentxs::ui::implementation

namespace opentxs
{
template <>
struct make_blank<ui::implementation::AccountActivityRowID> {
    static ui::implementation::AccountActivityRowID value(const api::Core& api)
    {
        return {api.Factory().Identifier(), proto::PAYMENTEVENTTYPE_ERROR};
    }
};

template <>
struct make_blank<ui::implementation::IssuerItemRowID> {
    static ui::implementation::IssuerItemRowID value(const api::Core& api)
    {
        return {api.Factory().Identifier(), proto::CITEMTYPE_ERROR};
    }
};

template <>
struct make_blank<ui::implementation::ActivityThreadRowID> {
    static ui::implementation::ActivityThreadRowID value(const api::Core& api)
    {
        return {api.Factory().Identifier(), {}, api.Factory().Identifier()};
    }
};
}  // namespace opentxs

namespace opentxs::ui::internal
{
struct List : virtual public ui::List {
#if OT_QT
    virtual void emit_begin_insert_rows(
        const QModelIndex& parent,
        int first,
        int last) const noexcept = 0;
    virtual void emit_begin_remove_rows(
        const QModelIndex& parent,
        int first,
        int last) const noexcept = 0;
    virtual void emit_end_insert_rows() const noexcept = 0;
    virtual void emit_end_remove_rows() const noexcept = 0;
    virtual QModelIndex me() const noexcept = 0;
    virtual void register_child(const void* child) const noexcept = 0;
    virtual void unregister_child(const void* child) const noexcept = 0;
#endif

    ~List() override = default;
};

struct Row : virtual public ui::ListRow {
#if OT_QT
    virtual int qt_column_count() const noexcept { return 0; }
    virtual QVariant qt_data(
        [[maybe_unused]] const int column,
        [[maybe_unused]] const int role) const noexcept
    {
        return {};
    }
    virtual QModelIndex qt_index(
        [[maybe_unused]] const int row,
        [[maybe_unused]] const int column) const noexcept
    {
        return {};
    }
    virtual QModelIndex qt_parent() const noexcept = 0;
    virtual int qt_row_count() const noexcept { return 0; }
#endif  // OT_QT

    ~Row() override = default;
};
struct AccountActivity : virtual public List,
                         virtual public ui::AccountActivity {
    virtual const Identifier& AccountID() const = 0;
    virtual bool last(const implementation::AccountActivityRowID& id) const
        noexcept = 0;

    ~AccountActivity() override = default;
};
struct AccountList : virtual public List, virtual public ui::AccountList {
    virtual bool last(const implementation::AccountListRowID& id) const
        noexcept = 0;

    ~AccountList() override = default;
};
struct AccountListItem : virtual public Row,
                         virtual public ui::AccountListItem {
    virtual void reindex(
        const implementation::AccountListSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~AccountListItem() override = default;
};
struct AccountSummary : virtual public List, virtual public ui::AccountSummary {
    virtual proto::ContactItemType Currency() const = 0;
#if OT_QT
    virtual int FindRow(
        const implementation::AccountSummaryRowID& id,
        const implementation::AccountSummarySortKey& key) const noexcept = 0;
#endif
    virtual bool last(const implementation::AccountSummaryRowID& id) const
        noexcept = 0;
    virtual const identifier::Nym& NymID() const = 0;

    ~AccountSummary() override = default;
};
struct AccountSummaryItem : virtual public Row,
                            virtual public ui::AccountSummaryItem {
    virtual void reindex(
        const implementation::IssuerItemSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~AccountSummaryItem() override = default;
};
struct ActivitySummary : virtual public List,
                         virtual public ui::ActivitySummary {
    virtual bool last(const implementation::ActivitySummaryRowID& id) const
        noexcept = 0;

    ~ActivitySummary() override = default;
};
struct ActivitySummaryItem : virtual public Row,
                             virtual public ui::ActivitySummaryItem {
    virtual void reindex(
        const implementation::ActivitySummarySortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ActivitySummaryItem() override = default;
};
struct ActivityThread : virtual public List {
    virtual bool last(const implementation::ActivityThreadRowID& id) const
        noexcept = 0;
    // custom
    virtual std::string ThreadID() const = 0;

    ~ActivityThread() override = default;
};
struct ActivityThreadItem : virtual public Row,
                            virtual public ui::ActivityThreadItem {
    virtual void reindex(
        const implementation::ActivityThreadSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ActivityThreadItem() override = default;
};
struct BalanceItem : virtual public Row, virtual public ui::BalanceItem {
    virtual void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~BalanceItem() override = default;
};
struct Contact : virtual public List, virtual public ui::Contact {
#if OT_QT
    virtual int FindRow(
        const implementation::ContactRowID& id,
        const implementation::ContactSortKey& key) const noexcept = 0;
#endif
    virtual bool last(const implementation::ContactRowID& id) const
        noexcept = 0;

    ~Contact() override = default;
};
struct ContactItem : virtual public Row, virtual public ui::ContactItem {
    virtual void reindex(
        const implementation::ContactSubsectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ContactItem() override = default;
};
struct ContactList : virtual public List {
    virtual bool last(const implementation::ContactListRowID& id) const
        noexcept = 0;
    // custom
    virtual const Identifier& ID() const = 0;

    ~ContactList() override = default;
};
struct ContactListItem : virtual public Row,
                         virtual public ui::ContactListItem {
    virtual void reindex(
        const implementation::ContactListSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ContactListItem() override = default;
};
struct ContactSection : virtual public List,
                        virtual public Row,
                        virtual public ui::ContactSection {
    virtual std::string ContactID() const noexcept = 0;
#if OT_QT
    virtual int FindRow(
        const implementation::ContactSectionRowID& id,
        const implementation::ContactSectionSortKey& key) const noexcept = 0;
#endif
    virtual bool last(const implementation::ContactSectionRowID& id) const
        noexcept = 0;

    virtual void reindex(
        const implementation::ContactSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ContactSection() override = default;
};
struct ContactSubsection : virtual public List,
                           virtual public Row,
                           virtual public ui::ContactSubsection {
    virtual void reindex(
        const implementation::ContactSectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::ContactSubsectionRowID& id) const
        noexcept = 0;

    ~ContactSubsection() override = default;
};
struct IssuerItem : virtual public List,
                    virtual public Row,
                    virtual public ui::IssuerItem {
    // Row
    virtual void reindex(
        const implementation::AccountSummarySortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::IssuerItemRowID& id) const
        noexcept = 0;

    ~IssuerItem() override = default;
};
struct MessagableList : virtual public ContactList {
    ~MessagableList() override = default;
};
struct PayableList : virtual public ContactList {
    ~PayableList() override = default;
};
struct PayableListItem : virtual public Row,
                         virtual public ui::PayableListItem {
    virtual void reindex(
        const implementation::PayableListSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~PayableListItem() override = default;
};
struct Profile : virtual public List, virtual public ui::Profile {
#if OT_QT
    virtual int FindRow(
        const implementation::ProfileRowID& id,
        const implementation::ProfileSortKey& key) const noexcept = 0;
#endif
    virtual bool last(const implementation::ProfileRowID& id) const
        noexcept = 0;
    virtual const identifier::Nym& NymID() const = 0;

    ~Profile() override = default;
};
struct ProfileSection : virtual public List,
                        virtual public Row,
                        virtual public ui::ProfileSection {
#if OT_QT
    virtual int FindRow(
        const implementation::ProfileSectionRowID& id,
        const implementation::ProfileSectionSortKey& key) const noexcept = 0;
#endif
    virtual bool last(const implementation::ProfileSectionRowID& id) const
        noexcept = 0;
    virtual const identifier::Nym& NymID() const noexcept = 0;

    virtual void reindex(
        const implementation::ProfileSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ProfileSection() override = default;
};
struct ProfileSubsection : virtual public List,
                           virtual public Row,
                           virtual public ui::ProfileSubsection {
    // Row
    virtual void reindex(
        const implementation::ProfileSectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::ProfileSubsectionRowID& id) const
        noexcept = 0;
    // custom
    virtual const identifier::Nym& NymID() const noexcept = 0;
    virtual proto::ContactSectionName Section() const noexcept = 0;

    ~ProfileSubsection() override = default;
};
struct ProfileItem : virtual public Row, virtual public ui::ProfileItem {
    virtual void reindex(
        const implementation::ProfileSubsectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    ~ProfileItem() override = default;
};

#if OT_QT
#define QT_PROXY_MODEL_WRAPPER(WrapperType, InterfaceType)                     \
    WrapperType::WrapperType(InterfaceType& parent) noexcept                   \
        : parent_(parent)                                                      \
    {                                                                          \
        QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);        \
        parent_.SetCallback([this]() -> void { notify(); });                   \
        setSourceModel(&parent_);                                              \
    }                                                                          \
                                                                               \
    void WrapperType::notify() const noexcept { emit updated(); }
#endif

namespace blank
{
struct Widget : virtual public ui::Widget {
    void SetCallback(Callback cb) const noexcept final {}
    OTIdentifier WidgetID() const noexcept override
    {
        return Identifier::Factory();
    }
};
struct Row : virtual public ui::internal::Row, public Widget {
    bool Last() const noexcept final { return true; }
#if OT_QT
    QModelIndex qt_parent() const noexcept final { return {}; }
#endif  // OT_QT
    bool Valid() const noexcept final { return false; }
};
template <typename ListType, typename RowType, typename RowIDType>
struct List : virtual public ListType, public Row {
    RowType First() const noexcept final { return RowType{nullptr}; }
    bool last(const RowIDType&) const noexcept final { return false; }
#if OT_QT
    void emit_begin_insert_rows(const QModelIndex& parent, int first, int last)
        const noexcept
    {
    }
    void emit_begin_remove_rows(const QModelIndex& parent, int first, int last)
        const noexcept
    {
    }
    void emit_end_insert_rows() const noexcept {}
    void emit_end_remove_rows() const noexcept {}
    QModelIndex me() const noexcept final { return {}; }
    void register_child(const void* child) const noexcept final {}
    void unregister_child(const void* child) const noexcept final {}
#endif
    RowType Next() const noexcept final { return RowType{nullptr}; }
    OTIdentifier WidgetID() const noexcept final
    {
        return blank::Widget::WidgetID();
    }
};
struct AccountListItem final : virtual public Row,
                               virtual public internal::AccountListItem {
    std::string AccountID() const noexcept final { return {}; }
    Amount Balance() const noexcept final { return {}; }
    std::string ContractID() const noexcept final { return {}; }
    std::string DisplayBalance() const noexcept final { return {}; }
    std::string DisplayUnit() const noexcept final { return {}; }
    std::string Name() const noexcept final { return {}; }
    std::string NotaryID() const noexcept final { return {}; }
    std::string NotaryName() const noexcept final { return {}; }
    AccountType Type() const noexcept final { return {}; }
    proto::ContactItemType Unit() const noexcept final { return {}; }

    void reindex(
        const implementation::AccountListSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct AccountSummaryItem final : public Row,
                                  public internal::AccountSummaryItem {
    std::string AccountID() const noexcept final { return {}; }
    Amount Balance() const noexcept final { return {}; }
    std::string DisplayBalance() const noexcept final { return {}; }
    std::string Name() const noexcept final { return {}; }

    void reindex(
        const implementation::IssuerItemSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ActivitySummaryItem final
    : virtual public Row,
      virtual public internal::ActivitySummaryItem {
    std::string DisplayName() const noexcept final { return {}; }
    std::string ImageURI() const noexcept final { return {}; }
    std::string Text() const noexcept final { return {}; }
    std::string ThreadID() const noexcept final { return {}; }
    std::chrono::system_clock::time_point Timestamp() const noexcept final
    {
        return {};
    }
    StorageBox Type() const noexcept final { return StorageBox::UNKNOWN; }

    void reindex(
        const implementation::ActivitySummarySortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ActivityThreadItem final : public Row,
                                  public internal::ActivityThreadItem {
    opentxs::Amount Amount() const noexcept final { return 0; }
    bool Deposit() const noexcept final { return false; }
    std::string DisplayAmount() const noexcept final { return {}; }
    bool Loading() const noexcept final { return false; }
    bool MarkRead() const noexcept final { return false; }
    std::string Memo() const noexcept final { return {}; }
    bool Pending() const noexcept final { return false; }
    std::string Text() const noexcept final { return {}; }
    std::chrono::system_clock::time_point Timestamp() const noexcept final
    {
        return {};
    }
    StorageBox Type() const noexcept final { return StorageBox::UNKNOWN; }

    void reindex(
        const implementation::ActivityThreadSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct BalanceItem final : public Row, public internal::BalanceItem {
    opentxs::Amount Amount() const noexcept final { return {}; }
    std::vector<std::string> Contacts() const noexcept final { return {}; }
    std::string DisplayAmount() const noexcept final { return {}; }
    std::string Memo() const noexcept final { return {}; }
    std::string Workflow() const noexcept final { return {}; }
    std::string Text() const noexcept final { return {}; }
    std::chrono::system_clock::time_point Timestamp() const noexcept final
    {
        return {};
    }
    StorageBox Type() const noexcept final { return StorageBox::UNKNOWN; }
    std::string UUID() const noexcept final { return {}; }

    void reindex(
        const implementation::AccountActivitySortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ContactItem final : public Row, public internal::ContactItem {
    std::string ClaimID() const noexcept final { return {}; }
    bool IsActive() const noexcept final { return false; }
    bool IsPrimary() const noexcept final { return false; }
    std::string Value() const noexcept final { return {}; }

    void reindex(
        const implementation::ContactSectionSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ContactListItem : virtual public Row,
                         virtual public internal::ContactListItem {
    std::string ContactID() const noexcept final { return {}; }
    std::string DisplayName() const noexcept final { return {}; }
    std::string ImageURI() const noexcept final { return {}; }
    std::string Section() const noexcept final { return {}; }

    void reindex(
        const implementation::ContactListSortKey&,
        const implementation::CustomData&) noexcept override
    {
    }
};
struct ContactSection final : public List<
                                  internal::ContactSection,
                                  OTUIContactSubsection,
                                  implementation::ContactSectionRowID> {
    std::string ContactID() const noexcept final { return {}; }
#if OT_QT
    int FindRow(
        const implementation::ContactSectionRowID& id,
        const implementation::ContactSectionSortKey& key) const noexcept final
    {
        return -1;
    }
#endif
    std::string Name(const std::string& lang) const noexcept final
    {
        return {};
    }
    proto::ContactSectionName Type() const noexcept final { return {}; }

    void reindex(
        const implementation::ContactSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ContactSubsection final : public List<
                                     internal::ContactSubsection,
                                     OTUIContactItem,
                                     implementation::ContactSubsectionRowID> {
    std::string Name(const std::string& lang) const noexcept final
    {
        return {};
    }
    proto::ContactItemType Type() const noexcept final { return {}; }

    void reindex(
        const implementation::ContactSectionSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct IssuerItem final : public List<
                              internal::IssuerItem,
                              OTUIAccountSummaryItem,
                              implementation::IssuerItemRowID> {
    bool ConnectionState() const noexcept final { return {}; }
    std::string Debug() const noexcept final { return {}; }
    std::string Name() const noexcept final { return {}; }
    bool Trusted() const noexcept final { return {}; }

    void reindex(
        const implementation::AccountSummarySortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct PayableListItem final : virtual public ContactListItem,
                               virtual public internal::PayableListItem {
    std::string PaymentCode() const noexcept final { return {}; }

    void reindex(
        const implementation::PayableListSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ProfileItem : virtual public Row, virtual public internal::ProfileItem {
    std::string ClaimID() const noexcept final { return {}; }
    bool Delete() const noexcept final { return false; }
    bool IsActive() const noexcept final { return false; }
    bool IsPrimary() const noexcept final { return false; }
    std::string Value() const noexcept final { return {}; }
    bool SetActive(const bool& active) const noexcept final { return false; }
    bool SetPrimary(const bool& primary) const noexcept final { return false; }
    bool SetValue(const std::string& value) const noexcept final
    {
        return false;
    }

    void reindex(
        const implementation::ProfileSubsectionSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }
};
struct ProfileSection : public List<
                            internal::ProfileSection,
                            OTUIProfileSubsection,
                            implementation::ProfileSectionRowID> {
    bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const noexcept final
    {
        return false;
    }
    bool Delete(const int, const std::string&) const noexcept final
    {
        return false;
    }
#if OT_QT
    int FindRow(
        const implementation::ProfileSectionRowID& id,
        const implementation::ProfileSectionSortKey& key) const noexcept final
    {
        return -1;
    }
#endif
    ItemTypeList Items(const std::string&) const noexcept final { return {}; }
    std::string Name(const std::string& lang) const noexcept final
    {
        return {};
    }
    const identifier::Nym& NymID() const noexcept final { return nym_id_; }
    bool SetActive(const int, const std::string&, const bool) const
        noexcept final
    {
        return false;
    }
    bool SetPrimary(const int, const std::string&, const bool) const
        noexcept final
    {
        return false;
    }
    bool SetValue(const int, const std::string&, const std::string&) const
        noexcept final
    {
        return false;
    }
    proto::ContactSectionName Type() const noexcept final { return {}; }

    void reindex(
        const implementation::ProfileSortKey& key,
        const implementation::CustomData& custom) noexcept final
    {
    }

private:
    const OTNymID nym_id_{identifier::Nym::Factory()};
};
struct ProfileSubsection : public List<
                               internal::ProfileSubsection,
                               OTUIProfileItem,
                               implementation::ProfileSubsectionRowID> {
    bool AddItem(const std::string&, const bool, const bool) const
        noexcept final
    {
        return false;
    }
    bool Delete(const std::string&) const noexcept final { return false; }
    std::string Name(const std::string&) const noexcept final { return {}; }
    const identifier::Nym& NymID() const noexcept final { return nym_id_; }
    proto::ContactItemType Type() const noexcept final { return {}; }
    proto::ContactSectionName Section() const noexcept final { return {}; }
    bool SetActive(const std::string&, const bool) const noexcept final
    {
        return false;
    }
    bool SetPrimary(const std::string&, const bool) const noexcept final
    {
        return false;
    }
    bool SetValue(const std::string&, const std::string&) const noexcept final
    {
        return false;
    }

    void reindex(
        const implementation::ProfileSortKey&,
        const implementation::CustomData&) noexcept final
    {
    }

private:
    const OTNymID nym_id_{identifier::Nym::Factory()};
};
}  // namespace blank
}  // namespace opentxs::ui::internal

namespace opentxs::factory
{
auto AccountActivityModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const opentxs::Identifier& accountID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::AccountActivity>;
#if OT_QT
auto AccountActivityQtModel(
    ui::implementation::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>;
#endif
auto AccountListItem(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>;
auto AccountListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::AccountList>;
#if OT_QT
auto AccountListQtModel(ui::implementation::AccountList& parent) noexcept
    -> std::unique_ptr<ui::AccountListQt>;
#endif
auto AccountSummaryItem(
    const ui::implementation::IssuerItemInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::IssuerItemRowID& rowID,
    const ui::implementation::IssuerItemSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::IssuerItemRowInternal>;
auto AccountSummaryModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const proto::ContactItemType currency
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::AccountSummary>;
#if OT_QT
auto AccountSummaryQtModel(ui::implementation::AccountSummary& parent) noexcept
    -> std::unique_ptr<ui::AccountSummaryQt>;
#endif
auto ActivitySummaryItem(
    const ui::implementation::ActivitySummaryInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivitySummaryRowID& rowID,
    const ui::implementation::ActivitySummarySortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const Flag& running) noexcept
    -> std::shared_ptr<ui::implementation::ActivitySummaryRowInternal>;
auto ActivitySummaryModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const Flag& running,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::ActivitySummary>;
#if OT_QT
auto ActivitySummaryQtModel(
    ui::implementation::ActivitySummary& parent) noexcept
    -> std::unique_ptr<ui::ActivitySummaryQt>;
#endif
auto ActivityThreadModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const opentxs::Identifier& threadID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::ActivityThread>;
#if OT_QT
auto ActivityThreadQtModel(ui::implementation::ActivityThread& parent) noexcept
    -> std::unique_ptr<ui::ActivityThreadQt>;
#endif
auto BalanceItem(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const opentxs::Identifier& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>;
auto ContactItemWidget(
    const ui::implementation::ContactSubsectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ContactSubsectionRowID& rowID,
    const ui::implementation::ContactSubsectionSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactSubsectionRowInternal>;
auto ContactListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key) noexcept
    -> std::shared_ptr<ui::implementation::ContactListRowInternal>;
auto ContactListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::ContactList>;
#if OT_QT
auto ContactListQtModel(ui::implementation::ContactList& parent) noexcept
    -> std::unique_ptr<ui::ContactListQt>;
#endif
auto ContactModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const opentxs::Identifier& contactID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::Contact>;
#if OT_QT
auto ContactQtModel(ui::implementation::Contact& parent) noexcept
    -> std::unique_ptr<ui::ContactQt>;
#endif
auto ContactSectionWidget(
    const ui::implementation::ContactInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ContactRowID& rowID,
    const ui::implementation::ContactSortKey& key,
    const ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::shared_ptr<ui::implementation::ContactRowInternal>;
auto ContactSubsectionWidget(
    const ui::implementation::ContactSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ContactSectionRowID& rowID,
    const ui::implementation::ContactSectionSortKey& key,
    const ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    -> std::shared_ptr<ui::implementation::ContactSectionRowInternal>;
auto IssuerItem(
    const ui::implementation::AccountSummaryInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::AccountSummaryRowID& rowID,
    const ui::implementation::AccountSummarySortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const proto::ContactItemType currency
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    -> std::shared_ptr<ui::implementation::AccountSummaryRowInternal>;
auto MailItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const bool loading,
    const bool pending) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto MailItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto MessagableListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::MessagableList>;
#if OT_QT
auto MessagableListQtModel(ui::implementation::MessagableList& parent) noexcept
    -> std::unique_ptr<ui::MessagableListQt>;
#endif
auto PayableListItem(
    const ui::implementation::PayableInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::PayableListRowID& rowID,
    const ui::implementation::PayableListSortKey& key,
    const std::string& paymentcode,
    const proto::ContactItemType& currency) noexcept
    -> std::shared_ptr<ui::implementation::PayableListRowInternal>;
auto PaymentItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto PayableListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const proto::ContactItemType& currency
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::PayableList>;
#if OT_QT
auto PayableListQtModel(ui::implementation::PayableList& parent) noexcept
    -> std::unique_ptr<ui::PayableListQt>;
#endif
auto PendingSend(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto ProfileModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::Profile>;
#if OT_QT
auto ProfileQtModel(ui::implementation::Profile& parent) noexcept
    -> std::unique_ptr<ui::ProfileQt>;
#endif
auto ProfileItemWidget(
    const ui::implementation::ProfileSubsectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ProfileSubsectionRowID& rowID,
    const ui::implementation::ProfileSubsectionSortKey& sortKey,
    const ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileSubsectionRowInternal>;
auto ProfileSectionWidget(
    const ui::implementation::ProfileInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ProfileRowID& rowID,
    const ui::implementation::ProfileSortKey& key,
    const ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::shared_ptr<ui::implementation::ProfileRowInternal>;
auto ProfileSubsectionWidget(
    const ui::implementation::ProfileSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ProfileSectionRowID& rowID,
    const ui::implementation::ProfileSectionSortKey& key,
    const ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    -> std::shared_ptr<ui::implementation::ProfileSectionRowInternal>;
}  // namespace opentxs::factory
