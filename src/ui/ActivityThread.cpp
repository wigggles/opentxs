// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/Types.hpp"

#include "ActivityThreadItemBlank.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ActivityThread.hpp"

template class std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

#define OT_METHOD "opentxs::ui::implementation::ActivityThread::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
ui::ActivityThread* Factory::ActivityThread(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const Identifier& threadID
#if OT_QT
    ,
    const bool qt
#endif
)
{
    return new ui::implementation::ActivityThread(
        api,
        publisher,
        nymID,
        threadID
#if OT_QT
        ,
        qt
#endif
    );
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ActivityThread::ActivityThread(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const Identifier& threadID
#if OT_QT
        ,
        const bool qt
#endif
    )
    : ActivityThreadList(
        api,
        publisher,
        nymID
#if OT_QT
        ,
        qt,
        Roles{{AmountRole, "amount"},
              {MemoRole, "memo"},
              {TextRole, "text"},
              {TimestampRole, "timestamp"},
              {TypeRole, "type"},
              {DepositRole, "deposit"},
              {LoadingRole, "loading"},
              {PendingRole, "pending"}},
        1
#endif
    )
    , listeners_{{api_.Activity().ThreadPublisher(nymID),
        new MessageProcessor<ActivityThread>(&ActivityThread::process_thread)},}
    , threadID_(Identifier::Factory(threadID))
    , participants_()
    , contact_lock_()
    , draft_lock_()
    , draft_()
    , draft_tasks_()
    , contact_(nullptr)
    , contact_thread_(nullptr)
    , queue_callback_(zmq::ListenCallback::Factory(
          [=](const zmq::Message& in) -> void { this->process_draft(in); }))
    , queue_pull_(api.ZeroMQ().PullSocket(
          queue_callback_,
          zmq::Socket::Direction::Bind))
    , queue_push_(api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    init();
    init_sockets();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&ActivityThread::startup, this));

    OT_ASSERT(startup_)

    contact_thread_.reset(new std::thread(&ActivityThread::init_contact, this));

    OT_ASSERT(contact_thread_)
}

void ActivityThread::can_message() const
{
    for (const auto& id : participants_) {
        api_.OTX().CanMessage(primary_id_, id, true);
    }
}

std::string ActivityThread::comma(const std::set<std::string>& list) const
{
    std::ostringstream stream;

    for (const auto& item : list) {
        stream << item;
        stream << ", ";
    }

    std::string output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 2, 2); }

    return output;
}

void ActivityThread::construct_row(
    const ActivityThreadRowID& id,
    const ActivityThreadSortKey& index,
    const CustomData& custom) const
{
    names_.emplace(id, index);
    const auto& box = std::get<1>(id);

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            items_[index].emplace(
                id,
                Factory::MailItem(
                    *this, api_, publisher_, primary_id_, id, index, custom));
        } break;
        case StorageBox::DRAFT: {
            items_[index].emplace(
                id,
                Factory::MailItem(
                    *this,
                    api_,
                    publisher_,
                    primary_id_,
                    id,
                    index,
                    custom,
                    false,
                    true));
        } break;
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            items_[index].emplace(
                id,
                Factory::PaymentItem(
                    *this, api_, publisher_, primary_id_, id, index, custom));
        } break;
        case StorageBox::PENDING_SEND: {
            items_[index].emplace(
                id,
                Factory::PendingSend(
                    *this, api_, publisher_, primary_id_, id, index, custom));
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }
}

#if OT_QT
QVariant ActivityThread::data(const QModelIndex& index, int role) const
{
    const auto [valid, pRow] = check_index(index);

    if (false == valid) { return {}; }

    const auto& row = *pRow;

    switch (role) {
        case AmountRole: {
            return row.DisplayAmount().c_str();
        }
        case MemoRole: {
            return row.Memo().c_str();
        }
        case TextRole: {
            return row.Text().c_str();
        }
        case TimestampRole: {
            QDateTime qdatetime;
            qdatetime.setSecsSinceEpoch(
                std::chrono::system_clock::to_time_t(row.Timestamp()));
            return qdatetime;
        }
        case TypeRole: {
            return storage_box_name(row.Type()).c_str();
        }
        case DepositRole: {
            return row.Deposit();
        }
        case LoadingRole: {
            return row.Loading();
        }
        case PendingRole: {
            return row.Pending();
        }
        default: {
            return {};
        }
    }

    return {};
}
#endif

std::string ActivityThread::DisplayName() const
{
    Lock lock(lock_);

    std::set<std::string> names{};

    for (const auto& contactID : participants_) {
        auto name = api_.Contacts().ContactName(contactID);

        if (name.empty()) {
            names.emplace(contactID->str());
        } else {
            names.emplace(name);
        }
    }

    return comma(names);
}

std::string ActivityThread::GetDraft() const
{
    sLock lock(draft_lock_);

    return draft_;
}

void ActivityThread::init_contact()
{
    wait_for_startup();

    if (1 != participants_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong number of participants (")(
            participants_.size())(").")
            .Flush();

        return;
    }

    auto contact = api_.Contacts().Contact(*participants_.cbegin());
    Lock lock(contact_lock_);
    contact_ = contact;
    lock.unlock();
    UpdateNotify();
}

void ActivityThread::init_sockets()
{
    const auto endpoint =
        std::string("inproc://") + Identifier::Random()->str();
    auto started = queue_pull_->Start(endpoint);

    if (false == started) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start pull socket ")
            .Flush();

        OT_FAIL;
    }

    started = queue_push_->Start(endpoint);

    if (false == started) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start push socket ")
            .Flush();

        OT_FAIL;
    }
}

void ActivityThread::load_thread(const proto::StorageThread& thread)
{
    for (const auto& id : thread.participant()) {
        participants_.emplace(Identifier::Factory(id));
    }

    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(thread.item().size())(
        " items.")
        .Flush();

    for (const auto& item : thread.item()) { process_item(item); }

    startup_complete_->On();
}

void ActivityThread::new_thread()
{
    participants_.emplace(threadID_);
    UpdateNotify();
    startup_complete_->On();
}

std::string ActivityThread::Participants() const
{
    Lock lock(lock_);
    std::set<std::string> ids{};

    for (const auto& id : participants_) { ids.emplace(id->str()); }

    return comma(ids);
}

bool ActivityThread::Pay(
    const Amount amount,
    const Identifier& sourceAccount,
    const std::string& memo,
    const PaymentType type) const
{
    if (0 >= amount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount: (")(amount)(")")
            .Flush();

        return false;
    }

    switch (type) {
        case PaymentType::Cheque: {
            return send_cheque(amount, sourceAccount, memo);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported payment type: (")(
                static_cast<int>(type))(")")
                .Flush();

            return false;
        }
    }
}

std::string ActivityThread::PaymentCode(
    const proto::ContactItemType currency) const
{
    Lock lock(contact_lock_);

    if (contact_) { return contact_->PaymentCode(currency); }

    return {};
}

void ActivityThread::process_draft(const network::zeromq::Message&)
{
    eLock lock(draft_lock_);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Checking ")(draft_tasks_.size())(
        " pending sends.")
        .Flush();
    std::vector<ActivityThreadRowID> deleted{};

    for (auto i = draft_tasks_.begin(); i != draft_tasks_.end();) {
        auto& [rowID, backgroundTask] = *i;
        auto& future = std::get<1>(backgroundTask);
        const auto status = future.wait_for(std::chrono::milliseconds(1));

        if (std::future_status::ready == status) {
            // TODO maybe keep failed sends
            deleted.emplace_back(rowID);
            i = draft_tasks_.erase(i);
        } else {
            ++i;
        }
    }

    Lock widgetLock(lock_);

    for (const auto& id : deleted) { delete_item(widgetLock, id); }

    if (0 < deleted.size()) { UpdateNotify(); }

    if (0 < draft_tasks_.size()) { queue_push_->Push(""); }
}

ActivityThreadRowID ActivityThread::process_item(
    const proto::StorageThreadItem& item)
{
    const ActivityThreadRowID id{Identifier::Factory(item.id()),
                                 static_cast<StorageBox>(item.box()),
                                 Identifier::Factory(item.account())};
    const ActivityThreadSortKey key{std::chrono::seconds(item.time()),
                                    item.index()};
    const CustomData custom{new std::string};
    add_item(id, key, custom);

    return id;
}

void ActivityThread::process_thread(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto threadID = Identifier::Factory(id);

    OT_ASSERT(false == threadID->empty())

    if (threadID_ != threadID) { return; }

    const auto thread = api_.Activity().Thread(primary_id_, threadID_);

    OT_ASSERT(thread)

    std::set<ActivityThreadRowID> active{};

    for (const auto& item : thread->item()) {
        const auto id = process_item(item);
        active.emplace(id);
    }

    delete_inactive(active);
}

bool ActivityThread::same(
    const ActivityThreadRowID& lhs,
    const ActivityThreadRowID& rhs) const
{
    const auto& [lID, lBox, lAccount] = lhs;
    const auto& [rID, rBox, rAccount] = rhs;
    const bool sameID = (lID->str() == rID->str());
    const bool sameBox = (lBox == rBox);
    const bool sameAccount = (lAccount->str() == rAccount->str());

    return sameID && sameBox && sameAccount;
}

bool ActivityThread::send_cheque(
    const Amount amount,
    const Identifier& sourceAccount,
    const std::string& memo) const
{
    if (false == validate_account(sourceAccount)) { return false; }

    if (participants_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No recipients.").Flush();

        return false;
    }

    if (1 < participants_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Sending to multiple recipient not yet supported.")
            .Flush();

        return false;
    }

    const auto contract = api_.Wallet().UnitDefinition(
        api_.Storage().AccountContract(sourceAccount));

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load unit definition contract")
            .Flush();

        return false;
    }

    std::string displayAmount{};
    contract->FormatAmountLocale(amount, displayAmount, ",", ".");
    auto task = make_blank<DraftTask>::value();
    auto& [id, otx] = task;
    otx = api_.OTX().SendCheque(
        primary_id_, sourceAccount, *participants_.begin(), amount, memo);
    const auto taskID = std::get<0>(otx);

    if (0 == taskID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to queue payment for sending.")
            .Flush();

        return false;
    }

    id = ActivityThreadRowID{
        Identifier::Random(), StorageBox::PENDING_SEND, Identifier::Factory()};
    const ActivityThreadSortKey key{std::chrono::system_clock::now(), 0};
    const CustomData custom{new std::string{"Sending cheque"},
                            new Amount{amount},
                            new std::string{displayAmount},
                            new std::string{memo}};
    const_cast<ActivityThread&>(*this).add_item(id, key, custom);

    OT_ASSERT(1 == items_.count(key));
    OT_ASSERT(1 == names_.count(id));

    draft_tasks_.emplace_back(std::move(task));
    queue_push_->Push("");

    return true;
}

bool ActivityThread::SendDraft() const
{
    eLock draftLock(draft_lock_);

    if (draft_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No draft message to send.")
            .Flush();

        return false;
    }

    if (participants_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No recipients.").Flush();

        return false;
    }

    if (1 < participants_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Sending to multiple recipient not yet supported.")
            .Flush();

        return false;
    }

    auto task = make_blank<DraftTask>::value();
    auto& [id, otx] = task;
    otx =
        api_.OTX().MessageContact(primary_id_, *participants_.begin(), draft_);
    const auto taskID = std::get<0>(otx);

    if (0 == taskID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to queue message for sending.")
            .Flush();

        return false;
    }

    id = ActivityThreadRowID{
        Identifier::Random(), StorageBox::DRAFT, Identifier::Factory()};
    const ActivityThreadSortKey key{std::chrono::system_clock::now(), 0};
    const CustomData custom{new std::string(draft_)};
    const_cast<ActivityThread&>(*this).add_item(id, key, custom);

    OT_ASSERT(1 == items_.count(key));
    OT_ASSERT(1 == names_.count(id));

    draft_tasks_.emplace_back(std::move(task));
    draft_.clear();
    queue_push_->Push("");

    return true;
}

bool ActivityThread::SetDraft(const std::string& draft) const
{
    can_message();

    if (draft.empty()) { return false; }

    eLock lock(draft_lock_);
    draft_ = draft;

    return true;
}

void ActivityThread::startup()
{
    const auto thread = api_.Activity().Thread(primary_id_, threadID_);

    if (thread) {
        load_thread(*thread);
    } else {
        new_thread();
    }
}

std::string ActivityThread::ThreadID() const
{
    Lock lock(lock_);

    return threadID_->str();
}

bool ActivityThread::validate_account(const Identifier& sourceAccount) const
{
    const auto owner = api_.Storage().AccountOwner(sourceAccount);

    if (owner->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account id: (")(
            sourceAccount)(")")
            .Flush();

        return false;
    }

    if (primary_id_ != owner) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Account ")(sourceAccount)(
            " is not owned by nym ")(primary_id_)
            .Flush();

        return false;
    }

    return true;
}

ActivityThread::~ActivityThread()
{
    for (auto& it : listeners_) { delete it.second; }

    if (contact_thread_ && contact_thread_->joinable()) {
        contact_thread_->join();
        contact_thread_.reset();
    }
}
}  // namespace opentxs::ui::implementation
