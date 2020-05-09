// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#if OT_BLOCKCHAIN
#include "ui/BlockchainAccountActivity.hpp"  // IWYU pragma: associated

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

#define OT_METHOD "opentxs::ui::implementation::BlockchainAccountActivity::"

namespace opentxs::factory
{
auto BlockchainAccountActivityModel(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const Identifier& accountID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<ui::implementation::AccountActivity>
{
    using ReturnType = ui::implementation::BlockchainAccountActivity;

    return std::make_unique<ReturnType>(
        api,
        publisher,
        nymID,
        accountID
#if OT_QT
        ,
        qt
#endif
    );
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountActivity::BlockchainAccountActivity(
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const Identifier& accountID
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : AccountActivity(
          api,
          publisher,
          nymID,
          accountID,
          AccountType::Blockchain,
#if OT_QT
          qt,
#endif
          {})
    , chain_(ui::Chain(api_, accountID))
    , balance_cb_(zmq::ListenCallback::Factory(
          [this](const auto& in) { process_balance(in); }

          ))
    , balance_listener_(api_.ZeroMQ().DealerSocket(
          balance_cb_,
          zmq::socket::Socket::Direction::Connect))
{
    startup_.reset(new std::thread(&BlockchainAccountActivity::startup, this));

    OT_ASSERT(startup_)
}

auto BlockchainAccountActivity::construct_row(
    const AccountActivityRowID& id,
    const AccountActivitySortKey& index,
    const CustomData& custom) const noexcept -> void*
{
    // TODO

    return nullptr;
}

auto BlockchainAccountActivity::DepositAddress(
    const blockchain::Type chain) const noexcept -> std::string
{
    if ((blockchain::Type::Unknown != chain) && (chain_ != chain)) {
        return {};
    }

    const auto& wallet = api_.Blockchain().Account(primary_id_, chain_);

    return wallet.GetDepositAddress();
}

auto BlockchainAccountActivity::DisplayBalance() const noexcept -> std::string
{
    return blockchain::internal::Format(chain_, balance_.load());
}

auto BlockchainAccountActivity::setup_listeners(
    const ListenerDefinitions& definitions) noexcept -> void
{
    Widget::setup_listeners(definitions);
    const auto connected =
        balance_listener_->Start(api_.Endpoints().BlockchainBalance());

    OT_ASSERT(connected);
}

auto BlockchainAccountActivity::process_balance(
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

        if (chain_ != chain) { return; }

        balance_.store(balanceFrame.as<Amount>());
        UpdateNotify();
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid chain or balance")
            .Flush();

        return;
    }
}

auto BlockchainAccountActivity::startup() noexcept -> void
{
    {  // register for balance updates
        auto out = api_.ZeroMQ().Message();
        out->AddFrame();
        out->AddFrame(chain_);
        balance_listener_->Send(out);
    }

    {  // load transaction history
       // TODO
    }

    finish_startup();
}
}  // namespace opentxs::ui::implementation
#endif  // OT_BLOCKCHAIN
