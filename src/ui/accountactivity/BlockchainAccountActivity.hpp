// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/BlockchainAccountActivity.cpp"

#pragma once

#include <atomic>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#include "ui/accountactivity/AccountActivity.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

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

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class BlockchainAccountActivity final : public AccountActivity
{
public:
    auto ContractID() const noexcept -> std::string final
    {
        return ui::UnitID(api_, chain_).str();
    }
    using AccountActivity::DepositAddress;
    auto DepositAddress(const blockchain::Type) const noexcept
        -> std::string final;
    auto DepositChains() const noexcept -> std::vector<blockchain::Type> final
    {
        return {chain_};
    }
    auto DisplayBalance() const noexcept -> std::string final;
    auto DisplayUnit() const noexcept -> std::string final
    {
        return blockchain::internal::Ticker(chain_);
    }
    auto Name() const noexcept -> std::string final
    {
        return ui::AccountName(chain_);
    }
    auto NotaryID() const noexcept -> std::string final
    {
        return ui::NotaryID(api_, chain_).str();
    }
    auto NotaryName() const noexcept -> std::string final
    {
        return blockchain::DisplayString(chain_);
    }
    using AccountActivity::Send;
    auto Send(
        const std::string& address,
        const Amount amount,
        const std::string& memo) const noexcept -> bool final;
    auto Send(
        const std::string& address,
        const std::string& amount,
        const std::string& memo) const noexcept -> bool final;
    auto SyncPercentage() const noexcept -> double final
    {
        return progress_.get_percentage();
    }
    auto SyncProgress() const noexcept -> std::pair<int, int> final
    {
        return progress_.get_progress();
    }
    auto Unit() const noexcept -> proto::ContactItemType final
    {
        return Translate(chain_);
    }
    auto ValidateAddress(const std::string& text) const noexcept -> bool final;
    auto ValidateAmount(const std::string& text) const noexcept
        -> std::string final;

    auto SetSyncCallback(const SyncCallback cb) noexcept -> void final;

    BlockchainAccountActivity(
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback& cb) noexcept;

    ~BlockchainAccountActivity() final = default;

private:
    struct SyncCB {
        std::mutex lock_{};
        SyncCallback cb_{};
    };

    struct Progress {
        auto get_percentage() const noexcept -> double
        {
            Lock lock{lock_};

            return percentage_;
        }
        auto get_progress() const noexcept -> std::pair<int, int>
        {
            Lock lock{lock_};

            return ratio_;
        }

        auto set(const int height, const int target) noexcept -> double
        {
            Lock lock{lock_};
            auto& [current, max] = ratio_;
            current = height;
            max = target;
            percentage_ = (double(height) / double(target)) * double{100};

            return percentage_;
        }

    private:
        mutable std::mutex lock_{};
        double percentage_{};
        std::pair<int, int> ratio_{};
    };

    const blockchain::Type chain_;
    Progress progress_;
    SyncCB sync_cb_;

    auto load_thread() noexcept -> void;
    auto process_sync(const network::zeromq::Message& in) noexcept -> void;
    auto process_txid(const network::zeromq::Message& in) noexcept -> void;
    auto startup() noexcept -> void final;

    BlockchainAccountActivity() = delete;
    BlockchainAccountActivity(const BlockchainAccountActivity&) = delete;
    BlockchainAccountActivity(BlockchainAccountActivity&&) = delete;
    auto operator=(const BlockchainAccountActivity&)
        -> BlockchainAccountActivity& = delete;
    auto operator=(BlockchainAccountActivity &&)
        -> BlockchainAccountActivity& = delete;
};
}  // namespace opentxs::ui::implementation
