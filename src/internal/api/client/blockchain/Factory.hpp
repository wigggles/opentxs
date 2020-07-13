// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <set>

#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace internal
{
struct BalanceList;
struct BalanceTree;
struct HD;
}  // namespace internal
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class HDAccount;
class HDPath;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::factory
{
auto BlockchainBalanceList(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& parent,
    const blockchain::Type chain) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::BalanceList>;
auto BlockchainBalanceTree(
    const api::internal::Core& api,
    const api::client::blockchain::internal::BalanceList& parent,
    const identifier::Nym& id,
    const std::set<OTIdentifier>& hdAccounts,
    const std::set<OTIdentifier>& importedAccounts,
    const std::set<OTIdentifier>& paymentCodeAccounts) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::BalanceTree>;
auto BlockchainHDBalanceNode(
    const api::internal::Core& api,
    const api::client::blockchain::internal::BalanceTree& parent,
    const proto::HDPath& path,
    Identifier& id) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::HD>;
auto BlockchainHDBalanceNode(
    const api::internal::Core& api,
    const api::client::blockchain::internal::BalanceTree& parent,
    const proto::HDAccount& serialized,
    Identifier& id) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::HD>;
}  // namespace opentxs::factory
