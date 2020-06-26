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
#include <numeric>
#include <string>
#include <thread>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/storage/Storage.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "util/Container.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainAccountActivity::"

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
          {
              {api.Endpoints().BlockchainTransactions(),
               new MessageProcessor<BlockchainAccountActivity>(
                   &BlockchainAccountActivity::process_txid)},
              {api.Endpoints().BlockchainTransactions(nymID),
               new MessageProcessor<BlockchainAccountActivity>(
                   &BlockchainAccountActivity::process_txid)},
          })
    , chain_(ui::Chain(api_, accountID))
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
#if OT_BLOCKCHAIN
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
        balance_ += tx.NetBalanceChange(primary_id_);
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
#endif  // OT_BLOCKCHAIN
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

auto BlockchainAccountActivity::startup() noexcept -> void
{
    load_thread();
    finish_startup();
}
}  // namespace opentxs::ui::implementation
#endif  // OT_BLOCKCHAIN
