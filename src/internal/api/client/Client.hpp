// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <string>
#include <tuple>

#include "internal/api/Api.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/consensus/Server.hpp"

namespace opentxs::api::client::internal
{
struct Activity : virtual public api::client::Activity {
    virtual void MigrateLegacyThreads() const = 0;

    virtual ~Activity() = default;
};
struct Blockchain : virtual public api::client::Blockchain {
    struct TxoDB {
        using Status = std::pair<blockchain::Coin, bool>;

        virtual auto AddSpent(
            const identifier::Nym& nym,
            const blockchain::Coin txo,
            const std::string txid) const noexcept -> bool = 0;
        virtual auto AddUnspent(
            const identifier::Nym& nym,
            const blockchain::Coin txo,
            const std::vector<OTData>& elements) const noexcept -> bool = 0;
        virtual auto Claim(
            const identifier::Nym& nym,
            const blockchain::Coin txo) const noexcept -> bool = 0;
        virtual auto Lookup(const identifier::Nym& nym, const Data& element)
            const noexcept -> std::vector<Status> = 0;

        virtual ~TxoDB() = default;
    };

    virtual auto API() const noexcept -> const api::internal::Core& = 0;
    /// Throws std::runtime_error if type is invalid
    OPENTXS_EXPORT virtual auto BalanceTree(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept(false)
        -> const blockchain::internal::BalanceTree& = 0;
#if OT_BLOCKCHAIN
    virtual auto BlockchainDB() const noexcept
        -> const blockchain::database::implementation::Database& = 0;
#endif  // OT_BLOCKCHAIN
    virtual auto CalculateAddress(
        const opentxs::blockchain::Type chain,
        const blockchain::AddressStyle format,
        const Data& pubkey) const noexcept -> std::string = 0;
    virtual auto Contacts() const noexcept -> const api::client::Contacts& = 0;
    virtual auto DB() const noexcept -> const TxoDB& = 0;
#if OT_BLOCKCHAIN
    virtual auto IO() const noexcept
        -> const opentxs::blockchain::client::internal::IO& = 0;
#endif  // OT_BLOCKCHAIN
    virtual auto PubkeyHash(
        const opentxs::blockchain::Type chain,
        const Data& pubkey) const noexcept(false) -> OTData = 0;
#if OT_BLOCKCHAIN
    virtual auto Reorg() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto ThreadPool() const noexcept
        -> const opentxs::blockchain::client::internal::ThreadPool& = 0;
    virtual auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void = 0;
#endif  // OT_BLOCKCHAIN

    virtual ~Blockchain() = default;
};
struct Contacts : virtual public api::client::Contacts {
    virtual void start() = 0;

    virtual ~Contacts() = default;
};
struct Manager : virtual public api::client::Manager,
                 virtual public api::internal::Core {
    virtual void StartActivity() = 0;
    virtual void StartContacts() = 0;

    virtual ~Manager() = default;
};

struct OTX : virtual public api::client::OTX {
    virtual void associate_message_id(
        const Identifier& messageID,
        const TaskID taskID) const = 0;
    virtual auto can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        identifier::UnitDefinition& unitID,
        Identifier& depositAccount) const -> Depositability = 0;
    virtual auto finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const -> bool = 0;
    virtual auto get_nym_fetch(const identifier::Server& serverID) const
        -> UniqueQueue<OTNymID>& = 0;
    virtual auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask = 0;
};
struct Pair : virtual public opentxs::api::client::Pair {
    virtual void init() noexcept = 0;

    virtual ~Pair() = default;
};
}  // namespace opentxs::api::client::internal
