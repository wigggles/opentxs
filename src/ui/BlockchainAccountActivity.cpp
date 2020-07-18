// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "ui/BlockchainAccountActivity.hpp"  // IWYU pragma: associated

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/PaymentEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "ui/Widget.hpp"
#include "util/Container.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainAccountActivity::"

namespace opentxs::factory
{
auto BlockchainAccountActivityModel(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
#if OT_QT
    const bool qt,
#endif
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::implementation::AccountActivity>
{
    using ReturnType = ui::implementation::BlockchainAccountActivity;

    return std::make_unique<ReturnType>(
        api,
        nymID,
        accountID,
#if OT_QT
        qt,
#endif
        cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountActivity::BlockchainAccountActivity(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
#if OT_QT
    const bool qt,
#endif
    const SimpleCallback& cb) noexcept
    : AccountActivity(
          api,
          nymID,
          accountID,
          AccountType::Blockchain,
          cb,
#if OT_QT
          qt,
#endif
          {
              {api.Endpoints().BlockchainTransactions(),
               new MessageProcessor<BlockchainAccountActivity>(
                   &BlockchainAccountActivity::process_txid)},
              {api.Endpoints().BlockchainTransactions(nymID),
               new MessageProcessor<BlockchainAccountActivity>(
                   &BlockchainAccountActivity::process_txid)},
              {api.Endpoints().BlockchainSyncProgress(),
               new MessageProcessor<BlockchainAccountActivity>(
                   &BlockchainAccountActivity::process_sync)},
          })
    , chain_(ui::Chain(api_, accountID))
    , sync_(0)
    , sync_cb_()
{
    startup_.reset(new std::thread(&BlockchainAccountActivity::startup, this));

    OT_ASSERT(startup_)
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

auto BlockchainAccountActivity::load_thread() noexcept -> void
{
    const auto transactions =
        api_.Storage().BlockchainTransactionList(primary_id_);

    for (const auto& txid : transactions) {
        const auto rowID = AccountActivityRowID{
            blockchain_thread_item_id(api_.Crypto(), chain_, txid),
            proto::PAYMENTEVENTTYPE_COMPLETE};

        {
            Lock lock(lock_);
            auto& existing = find_by_id(lock, rowID);

            if (existing.Valid()) { continue; }
        }

        auto pTX = api_.Blockchain().LoadTransactionBitcoin(txid);

        if (false == bool(pTX)) { continue; }

        const auto& tx = *pTX;

        if (false == contains(tx.Chains(), chain_)) { continue; }

        const auto sortKey{tx.Timestamp()};
        balance_ += tx.NetBalanceChange(api_.Blockchain(), primary_id_);
        auto custom = CustomData{
            new proto::PaymentWorkflow(),
            new proto::PaymentEvent(),
            const_cast<void*>(static_cast<const void*>(pTX.release())),
            new blockchain::Type{chain_},
            new std::string{
                api_.Blockchain().ActivityDescription(primary_id_, chain_, tx)},
            new OTData{tx.ID()}};
        add_item(rowID, sortKey, custom);
    }
}

auto BlockchainAccountActivity::process_sync(
    const network::zeromq::Message& in) noexcept -> void
{
    const auto& body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto height = body.at(2).as<blockchain::block::Height>();
    const auto target = body.at(3).as<blockchain::block::Height>();
    const auto progress = (double(height) / double(target)) * double{100};
    sync_.store(progress);
    Lock lock(sync_cb_.lock_);
    const auto& cb = sync_cb_.cb_;

    if (cb) { cb(); }
}

auto BlockchainAccountActivity::process_txid(
    const network::zeromq::Message& in) noexcept -> void
{
    static std::mutex zmq_lock_{};

    wait_for_startup();
    const auto& body = in.Body();

    OT_ASSERT(2 <= body.size());

    const auto txid = api_.Factory().Data(body.at(0));
    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    Lock zmq(zmq_lock_);
    const auto rowID = AccountActivityRowID{
        blockchain_thread_item_id(api_.Crypto(), chain_, txid),
        proto::PAYMENTEVENTTYPE_COMPLETE};
    {
        Lock lock(lock_);
        auto& existing = find_by_id(lock, rowID);

        if (existing.Valid()) {
            balance_ -= existing.Amount();
            delete_item(lock, rowID);
        }
    }

    load_thread();
}

auto BlockchainAccountActivity::SetSyncCallback(
    const SimpleCallback cb) noexcept -> void
{
    Lock lock(sync_cb_.lock_);
    sync_cb_.cb_ = cb;
}

auto BlockchainAccountActivity::startup() noexcept -> void
{
    load_thread();
    finish_startup();
}
}  // namespace opentxs::ui::implementation
