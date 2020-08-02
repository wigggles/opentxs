// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/BlockchainSelection.cpp"

#pragma once

#include <map>
#include <memory>
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
#include "opentxs/ui/BlockchainSelection.hpp"
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
struct Blockchain;
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
using BlockchainSelectionList = List<
    BlockchainSelectionExternalInterface,
    BlockchainSelectionInternalInterface,
    BlockchainSelectionRowID,
    BlockchainSelectionRowInterface,
    BlockchainSelectionRowInternal,
    BlockchainSelectionRowBlank,
    BlockchainSelectionSortKey,
    BlockchainSelectionPrimaryID>;

class BlockchainSelection final : public BlockchainSelectionList
{
public:
    auto ToggleChain(const blockchain::Type type) const noexcept -> bool final;

    BlockchainSelection(
        const api::client::internal::Manager& api,
        const api::client::internal::Blockchain& blockchain
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;

    auto init() noexcept -> void;

    ~BlockchainSelection() final;

private:
    const api::client::internal::Blockchain& blockchain_;

    auto construct_row(
        const BlockchainSelectionRowID& id,
        const BlockchainSelectionSortKey& index,
        CustomData& custom) const noexcept -> void* final;

    BlockchainSelection() = delete;
    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    auto operator=(const BlockchainSelection&) -> BlockchainSelection& = delete;
    auto operator=(BlockchainSelection &&) -> BlockchainSelection& = delete;
};
}  // namespace opentxs::ui::implementation
