// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/AccountSummaryItem.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/ContactSection.hpp"
#include "opentxs/ui/ContactSubsection.hpp"
#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/ui/PayableListItem.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/ProfileSection.hpp"
#include "opentxs/ui/ProfileSubsection.hpp"

namespace opentxs
{
template <>
struct make_blank<ui::implementation::AccountActivityRowID> {
    static ui::implementation::AccountActivityRowID value()
    {
        return {Identifier::Factory(), proto::PAYMENTEVENTTYPE_ERROR};
    }
};

template <>
struct make_blank<ui::implementation::IssuerItemRowID> {
    static ui::implementation::IssuerItemRowID value()
    {
        return {Identifier::Factory(), proto::CITEMTYPE_ERROR};
    }
};

template <>
struct make_blank<ui::implementation::ActivityThreadRowID> {
    static ui::implementation::ActivityThreadRowID value()
    {
        return {Identifier::Factory(), {}, Identifier::Factory()};
    }
};
}  // namespace opentxs

namespace opentxs::ui::internal
{
struct AccountActivity {
    virtual const Identifier& AccountID() const = 0;
    virtual bool last(const implementation::AccountActivityRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;

    virtual ~AccountActivity() = default;
};
struct AccountList {
    virtual bool last(const implementation::AccountListRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;

    virtual ~AccountList() = default;
};
struct AccountListItem : virtual public ui::AccountListItem {
    virtual void reindex(
        const implementation::AccountListSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~AccountListItem() = default;
};
struct AccountSummary {
    virtual bool last(const implementation::AccountSummaryRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual proto::ContactItemType Currency() const = 0;
    virtual const identifier::Nym& NymID() const = 0;

    virtual ~AccountSummary() = default;
};
struct AccountSummaryItem : virtual public ui::AccountSummaryItem {
    virtual void reindex(
        const implementation::IssuerItemSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~AccountSummaryItem() = default;
};
struct ActivitySummary {
    virtual bool last(const implementation::ActivitySummaryRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;

    virtual ~ActivitySummary() = default;
};
struct ActivitySummaryItem : virtual public ui::ActivitySummaryItem {
    virtual void reindex(
        const implementation::ActivitySummarySortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~ActivitySummaryItem() = default;
};
struct ActivityThread {
    virtual bool last(const implementation::ActivityThreadRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual std::string ThreadID() const = 0;

    virtual ~ActivityThread() = default;
};
struct ActivityThreadItem : virtual public ui::ActivityThreadItem {
    virtual void reindex(
        const implementation::ActivityThreadSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~ActivityThreadItem() = default;
};
struct BalanceItem : virtual public ui::BalanceItem {
    virtual void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~BalanceItem() = default;
};
struct Contact {
    virtual bool last(const implementation::ContactRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual std::string ContactID() const = 0;

    virtual ~Contact() = default;
};
struct ContactItem : virtual public ui::ContactItem {
    virtual void reindex(
        const implementation::ContactSubsectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~ContactItem() = default;
};
struct ContactList {
    virtual bool last(const implementation::ContactListRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual const Identifier& ID() const = 0;

    virtual ~ContactList() = default;
};
struct ContactListItem : virtual public ui::ContactListItem {
    virtual void reindex(
        const implementation::ContactListSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~ContactListItem() = default;
};
struct ContactSection : virtual public ui::ContactSection {
    // Row
    virtual void reindex(
        const implementation::ContactSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::ContactSectionRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual std::string ContactID() const noexcept = 0;

    virtual ~ContactSection() = default;
};
struct ContactSubsection : virtual public ui::ContactSubsection {
    virtual void reindex(
        const implementation::ContactSectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::ContactSubsectionRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;

    virtual ~ContactSubsection() = default;
};
struct IssuerItem : virtual public ui::IssuerItem {
    // Row
    virtual void reindex(
        const implementation::AccountSummarySortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::IssuerItemRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;

    virtual ~IssuerItem() = default;
};
struct PayableListItem : virtual public ui::PayableListItem {
    virtual void reindex(
        const implementation::PayableListSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~PayableListItem() = default;
};
struct Profile : virtual public ui::Profile {
    virtual bool last(const implementation::ProfileRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual const identifier::Nym& NymID() const = 0;

    virtual ~Profile() = default;
};
struct ProfileSection : virtual public ui::ProfileSection {
    // Row
    virtual void reindex(
        const implementation::ProfileSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::ProfileSectionRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual const identifier::Nym& NymID() const noexcept = 0;

    virtual ~ProfileSection() = default;
};
struct ProfileSubsection : virtual public ui::ProfileSubsection {
    // Row
    virtual void reindex(
        const implementation::ProfileSectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;
    // List
    virtual bool last(const implementation::ProfileSubsectionRowID& id) const
        noexcept = 0;
    virtual OTIdentifier WidgetID() const noexcept = 0;
    // custom
    virtual const identifier::Nym& NymID() const noexcept = 0;
    virtual proto::ContactSectionName Section() const noexcept = 0;

    virtual ~ProfileSubsection() = default;
};
struct ProfileItem : virtual public ui::ProfileItem {
    virtual void reindex(
        const implementation::ProfileSubsectionSortKey& key,
        const implementation::CustomData& custom) noexcept = 0;

    virtual ~ProfileItem() = default;
};

#if OT_QT
#define QT_MODEL_WRAPPER(WrapperType, InterfaceType)                           \
    WrapperType::WrapperType(ConstructorCallback cb) noexcept(false)           \
        : parent_(                                                             \
              (cb) ? cb({std::bind(                                            \
                             &WrapperType::start_row_add,                      \
                             this,                                             \
                             std::placeholders::_1,                            \
                             std::placeholders::_2,                            \
                             std::placeholders::_3),                           \
                         std::bind(&WrapperType::finish_row_add, this)},       \
                        {std::bind(                                            \
                             &WrapperType::start_row_delete,                   \
                             this,                                             \
                             std::placeholders::_1,                            \
                             std::placeholders::_2,                            \
                             std::placeholders::_3),                           \
                         std::bind(&WrapperType::finish_row_delete, this)})    \
                   : nullptr)                                                  \
    {                                                                          \
        QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);        \
                                                                               \
        if (false == bool(parent_)) {                                          \
            throw std::runtime_error("Invalid model");                         \
        }                                                                      \
                                                                               \
        parent_->SetCallback([this]() -> void { notify(); });                  \
    }                                                                          \
                                                                               \
    const InterfaceType& WrapperType::operator*() const noexcept               \
    {                                                                          \
        return *parent_;                                                       \
    }                                                                          \
                                                                               \
    int WrapperType::columnCount(const QModelIndex& parent) const noexcept     \
    {                                                                          \
        return parent_->columnCount(parent);                                   \
    }                                                                          \
    QVariant WrapperType::data(const QModelIndex& index, int role)             \
        const noexcept                                                         \
    {                                                                          \
        return parent_->data(index, role);                                     \
    }                                                                          \
    QModelIndex WrapperType::index(                                            \
        int row, int column, const QModelIndex& parent) const noexcept         \
    {                                                                          \
        return parent_->index(row, column, parent);                            \
    }                                                                          \
    void WrapperType::notify() const noexcept { emit updated(); }              \
    QModelIndex WrapperType::parent(const QModelIndex& index) const noexcept   \
    {                                                                          \
        return parent_->parent(index);                                         \
    }                                                                          \
    QHash<int, QByteArray> WrapperType::roleNames() const noexcept             \
    {                                                                          \
        return parent_->roleNames();                                           \
    }                                                                          \
    int WrapperType::rowCount(const QModelIndex& parent) const noexcept        \
    {                                                                          \
        return parent_->rowCount(parent);                                      \
    }                                                                          \
                                                                               \
    void WrapperType::finish_row_add() noexcept { endInsertRows(); }           \
    void WrapperType::finish_row_delete() noexcept { endRemoveRows(); }        \
    void WrapperType::start_row_add(                                           \
        const QModelIndex& parent, int first, int last) noexcept               \
    {                                                                          \
        beginInsertRows(parent, first, last);                                  \
    }                                                                          \
    void WrapperType::start_row_delete(                                        \
        const QModelIndex& parent, int first, int last) noexcept               \
    {                                                                          \
        beginRemoveRows(parent, first, last);                                  \
    }                                                                          \
                                                                               \
    WrapperType::~WrapperType() = default;
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
struct Row : virtual public ui::ListRow, public Widget {
    bool Last() const noexcept final { return true; }
    bool Valid() const noexcept final { return false; }
};
template <typename ListType, typename RowType, typename RowIDType>
struct List : virtual public ListType, public Row {
    RowType First() const noexcept final { return RowType{nullptr}; }
    bool last(const RowIDType&) const noexcept final { return false; }
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
