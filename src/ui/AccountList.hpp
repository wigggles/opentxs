// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/AccountList.cpp"

#pragma once

#include <map>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/ui/AccountList.hpp"
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

class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountListList = List<
    AccountListExternalInterface,
    AccountListInternalInterface,
    AccountListRowID,
    AccountListRowInterface,
    AccountListRowInternal,
    AccountListRowBlank,
    AccountListSortKey,
    AccountListPrimaryID>;

class AccountList final : public AccountListList
{
public:
    AccountList(
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
#if OT_QT
        const bool qt,
#endif
        const SimpleCallback& cb) noexcept;

    ~AccountList() final;

private:
#if OT_BLOCKCHAIN
    OTZMQListenCallback blockchain_balance_cb_;
    OTZMQDealerSocket blockchain_balance_;
#endif  // OT_BLOCKCHAIN
    const ListenerDefinitions listeners_;

    auto construct_row(
        const AccountListRowID& id,
        const AccountListSortKey& index,
        CustomData& custom) const noexcept -> void* final;
#if OT_BLOCKCHAIN
    auto subscribe(const blockchain::Type chain) const noexcept -> void;
#endif  // OT_BLOCKCHAIN

#if OT_BLOCKCHAIN
    auto add_blockchain_account(
        const blockchain::Type chain,
        const Amount balance) noexcept -> void;
#endif  // OT_BLOCKCHAIN
    auto process_account(const Identifier& id) noexcept -> void;
    auto process_account(const Identifier& id, const Amount balance) noexcept
        -> void;
    auto process_account(
        const Identifier& id,
        const Amount balance,
        const std::string& name) noexcept -> void;
    auto process_account(const network::zeromq::Message& message) noexcept
        -> void;
#if OT_BLOCKCHAIN
    auto process_blockchain_account(
        const network::zeromq::Message& message) noexcept -> void;
    auto process_blockchain_balance(
        const network::zeromq::Message& message) noexcept -> void;
    auto setup_listeners(const ListenerDefinitions& definitions) noexcept
        -> void final;
#endif  // OT_BLOCKCHAIN
    auto startup() noexcept -> void;

    AccountList() = delete;
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    auto operator=(const AccountList&) -> AccountList& = delete;
    auto operator=(AccountList &&) -> AccountList& = delete;
};
}  // namespace opentxs::ui::implementation
