// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "ui/AccountList.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/api/client/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/api/storage/Storage.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "ui/List.hpp"
#include "util/Blank.hpp"

#define OT_METHOD "opentxs::ui::implementation::AccountList::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto AccountListModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::AccountList>
{
    using ReturnType = ui::implementation::AccountList;

    return std::make_unique<ReturnType>(
        api,
        publisher,
        nymID
#if OT_QT
        ,
        qt
#endif
    );
}

#if OT_QT
auto AccountListQtModel(ui::implementation::AccountList& parent) noexcept
    -> std::unique_ptr<ui::AccountListQt>
{
    using ReturnType = ui::AccountListQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(AccountListQt, implementation::AccountList)
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
AccountList::AccountList(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : AccountListList(
          api,
          publisher,
          nymID
#if OT_QT
          ,
          qt,
          Roles{
              {AccountListQt::NotaryIDRole, "notary"},
              {AccountListQt::UnitRole, "unit"},
              {AccountListQt::AccountIDRole, "account"},
              {AccountListQt::BalanceRole, "balance"},
              {AccountListQt::PolarityRole, "polarity"},
              {AccountListQt::AccountTypeRole, "accounttype"},
              {AccountListQt::ContractIdRole, "contractid"},
          },
          4
#endif
          )
#if OT_BLOCKCHAIN
    , blockchain_balance_cb_(zmq::ListenCallback::Factory(
          [this](const auto& in) { process_blockchain_balance(in); }

          ))
    , blockchain_balance_(api_.ZeroMQ().DealerSocket(
          blockchain_balance_cb_,
          zmq::socket::Socket::Direction::Connect))
#endif  // OT_BLOCKCHAIN
    , listeners_{
          {api_.Endpoints().AccountUpdate(),
           new MessageProcessor<AccountList>(&AccountList::process_account)}}
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&AccountList::startup, this));

    OT_ASSERT(startup_)
}

auto AccountList::construct_row(
    const AccountListRowID& id,
    const AccountListSortKey& index,
    CustomData& custom) const noexcept -> void*
{
    names_.emplace(id, index);
#if OT_BLOCKCHAIN
    const auto blockchain{extract_custom<bool>(custom, 0)};
#endif  // OT_BLOCKCHAIN
    const auto [it, added] = items_[index].emplace(
        id,
#if OT_BLOCKCHAIN
        (blockchain ? factory::BlockchainAccountListItem
                    : factory::AccountListItem)
#else
        (factory::AccountListItem)
#endif  // OT_BLOCKCHAIN
            (*this, api_, publisher_, id, index, custom));

    return it->second.get();
}

auto AccountList::process_account(const Identifier& id) noexcept -> void
{
    auto account = api_.Wallet().Account(id);
    process_account(id, account.get().GetBalance(), account.get().Alias());
}

auto AccountList::process_account(
    const Identifier& id,
    const Amount balance) noexcept -> void
{
    auto account = api_.Wallet().Account(id);
    process_account(id, balance, account.get().Alias());
}

auto AccountList::process_account(
    const Identifier& id,
    const Amount balance,
    const std::string& name) noexcept -> void
{
    auto index = make_blank<AccountListSortKey>::value(api_);
    auto& [type, notary] = index;
    type = api_.Storage().AccountUnit(id);
    notary = api_.Storage().AccountServer(id)->str();
    const auto contract = api_.Storage().AccountContract(id);
    auto custom = CustomData{};
    custom.emplace_back(new bool{false});
    custom.emplace_back(new Amount{balance});
    custom.emplace_back(new OTUnitID{contract});
    custom.emplace_back(new std::string{name});
    add_item(id, index, custom);
}

auto AccountList::process_account(
    const network::zeromq::Message& message) noexcept -> void
{
    wait_for_startup();

    if (2 > message.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    const std::string id(message.Body_at(0));
    const auto& balanceFrame = message.Body_at(1);
    const auto accountID = Identifier::Factory(id);
    auto balance = Amount{};

    if (accountID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account id").Flush();

        return;
    }

    try {
        balance = balanceFrame.as<Amount>();
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid balance for account ")(
            accountID->str())
            .Flush();
        return;
    }

    {
        Lock lock(lock_);

        if (0 < names_.count(accountID)) { delete_item(lock, accountID); }

        OT_ASSERT(0 == names_.count(accountID));
    }

    process_account(accountID, balance);
}

#if OT_BLOCKCHAIN
auto AccountList::process_blockchain_balance(
    const network::zeromq::Message& message) noexcept -> void
{
    wait_for_startup();

    if (3 > message.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    const auto& chainFrame = message.Body_at(0);
    // NOTE confirmed balance is not used here
    const auto& balanceFrame = message.Body_at(2);

    try {
        const auto chain = chainFrame.as<blockchain::Type>();
        const auto& accountID = AccountID(api_, chain);

        {
            Lock lock(lock_);

            if (0 < names_.count(accountID)) { delete_item(lock, accountID); }

            OT_ASSERT(0 == names_.count(accountID));
        }

        auto index = make_blank<AccountListSortKey>::value(api_);
        auto& [type, notary] = index;
        type = Translate(chain);
        notary = NotaryID(api_, chain).str();
        auto custom = CustomData{};
        custom.emplace_back(new bool{true});
        custom.emplace_back(new Amount{balanceFrame.as<Amount>()});
        custom.emplace_back(new blockchain::Type{chain});
        custom.emplace_back(new std::string{AccountName(chain)});
        add_item(accountID, index, custom);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid chain or balance")
            .Flush();

        return;
    }
}

auto AccountList::setup_listeners(
    const ListenerDefinitions& definitions) noexcept -> void
{
    Widget::setup_listeners(definitions);
    const auto connected =
        blockchain_balance_->Start(api_.Endpoints().BlockchainBalance());

    OT_ASSERT(connected);
}
#endif  // OT_BLOCKCHAIN

auto AccountList::startup() noexcept -> void
{
    const auto accounts = api_.Storage().AccountsByOwner(primary_id_);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(accounts.size())(
        " accounts.")
        .Flush();

    for (const auto& id : accounts) { process_account(id); }

#if OT_BLOCKCHAIN
    for (const auto& chain : blockchain::SupportedChains()) {
        if (0 < api_.Blockchain().AccountList(primary_id_, chain).size()) {
            auto out = api_.ZeroMQ().Message();
            out->AddFrame();
            out->AddFrame(chain);
            blockchain_balance_->Send(out);
        }
    }
#endif  // OT_BLOCKCHAIN

    finish_startup();
}

AccountList::~AccountList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
