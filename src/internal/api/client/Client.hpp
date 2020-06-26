// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/api/Api.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database

namespace internal
{
struct BalanceTree;
}  // namespace internal
}  // namespace blockchain
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace client
{
namespace internal
{
struct IO;
struct ThreadPool;
}  // namespace internal
}  // namespace client
}  // namespace blockchain

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
class OTPayment;
template <class T>
class UniqueQueue;
}  // namespace opentxs

namespace opentxs
{
auto Translate(const blockchain::Type type) noexcept -> proto::ContactItemType;
auto Translate(const proto::ContactItemType type) noexcept -> blockchain::Type;
}  // namespace opentxs

namespace opentxs::api::client::internal
{
struct Activity : virtual public api::client::Activity {
    virtual void MigrateLegacyThreads() const = 0;

    virtual ~Activity() = default;
};
struct Blockchain : virtual public api::client::Blockchain {
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
    virtual auto UpdateElement(
        std::vector<ReadView>& pubkeyHashes) const noexcept -> void = 0;

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
