// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/ActivityThread.cpp"

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/StateMachine.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/ActivityThread.hpp"
#include "ui/List.hpp"
#include "ui/Widget.hpp"

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

class Core;
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

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class StorageThread;
class StorageThreadItem;
}  // namespace proto

class Contact;
}  // namespace opentxs

namespace std
{
using STORAGEID = std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

template <>
struct less<STORAGEID> {
    auto operator()(const STORAGEID& lhs, const STORAGEID& rhs) const -> bool
    {
        /* TODO: these lines will cause a segfault in the clang-5 ast parser.
                const auto & [ lID, lBox, lAccount ] = lhs;
                const auto & [ rID, rBox, rAccount ] = rhs;
        */
        const auto& lID = std::get<0>(lhs);
        const auto& lBox = std::get<1>(lhs);
        const auto& lAccount = std::get<2>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rBox = std::get<1>(rhs);
        const auto& rAccount = std::get<2>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lBox < rBox) { return true; }

        if (rBox < lBox) { return false; }

        if (lAccount->str() < rAccount->str()) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs
{
using DraftTask = std::pair<
    ui::implementation::ActivityThreadRowID,
    api::client::OTX::BackgroundTask>;

template <>
struct make_blank<DraftTask> {
    static auto value(const api::Core& api) -> DraftTask
    {
        return {make_blank<ui::implementation::ActivityThreadRowID>::value(api),
                make_blank<api::client::OTX::BackgroundTask>::value(api)};
    }
};
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ActivityThreadList = List<
    ActivityThreadExternalInterface,
    ActivityThreadInternalInterface,
    ActivityThreadRowID,
    ActivityThreadRowInterface,
    ActivityThreadRowInternal,
    ActivityThreadRowBlank,
    ActivityThreadSortKey,
    ActivityThreadPrimaryID>;

class ActivityThread final : public ActivityThreadList,
                             public opentxs::internal::StateMachine
{
public:
    auto DisplayName() const noexcept -> std::string final;
    auto GetDraft() const noexcept -> std::string final;
    auto Participants() const noexcept -> std::string final;
    auto Pay(
        const std::string& amount,
        const Identifier& sourceAccount,
        const std::string& memo,
        const PaymentType type) const noexcept -> bool final;
    auto Pay(
        const Amount amount,
        const Identifier& sourceAccount,
        const std::string& memo,
        const PaymentType type) const noexcept -> bool final;
    auto PaymentCode(const proto::ContactItemType currency) const noexcept
        -> std::string final;
    auto same(const ActivityThreadRowID& lhs, const ActivityThreadRowID& rhs)
        const noexcept -> bool final;
    auto SendDraft() const noexcept -> bool final;
    auto SetDraft(const std::string& draft) const noexcept -> bool final;
    auto ThreadID() const noexcept -> std::string final;

    ActivityThread(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const Identifier& threadID
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;

    ~ActivityThread() final;

private:
    const ListenerDefinitions listeners_;
    const OTIdentifier threadID_;
    std::set<OTIdentifier> participants_;
    mutable std::promise<void> participants_promise_;
    mutable std::shared_future<void> participants_future_;
    mutable std::mutex contact_lock_;
    mutable std::string draft_;
    mutable std::vector<DraftTask> draft_tasks_;
    std::shared_ptr<const opentxs::Contact> contact_;
    std::unique_ptr<std::thread> contact_thread_;

    auto comma(const std::set<std::string>& list) const noexcept -> std::string;
    void can_message() const noexcept;
    auto construct_row(
        const ActivityThreadRowID& id,
        const ActivityThreadSortKey& index,
        const CustomData& custom) const noexcept -> void* final;
    auto send_cheque(
        const Amount amount,
        const Identifier& sourceAccount,
        const std::string& memo) const noexcept -> bool;
    auto validate_account(const Identifier& sourceAccount) const noexcept
        -> bool;

    void init_contact() noexcept;
    void load_thread(const proto::StorageThread& thread) noexcept;
    void new_thread() noexcept;
    auto process_item(const proto::StorageThreadItem& item) noexcept
        -> ActivityThreadRowID;
    auto process_drafts() noexcept -> bool;
    void process_thread(const network::zeromq::Message& message) noexcept;
    void startup() noexcept;

    ActivityThread() = delete;
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    auto operator=(const ActivityThread&) -> ActivityThread& = delete;
    auto operator=(ActivityThread &&) -> ActivityThread& = delete;
};
}  // namespace opentxs::ui::implementation
