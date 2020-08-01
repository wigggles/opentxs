// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "ui/BlockchainSelection.hpp"  // IWYU pragma: associated

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
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
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

// #define OT_METHOD "opentxs::ui::implementation::BlockchainSelection::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto BlockchainSelectionModel(
    const api::client::internal::Manager& api,
    const api::client::internal::Blockchain& blockchain
#if OT_QT
    ,
    const bool qt
#endif  // OT_QT
    ) noexcept -> std::unique_ptr<ui::implementation::BlockchainSelection>
{
    using ReturnType = ui::implementation::BlockchainSelection;

    return std::make_unique<ReturnType>(
        api,
        blockchain
#if OT_QT
        ,
        qt
#endif
    );
}

#if OT_QT
auto BlockchainSelectionQtModel(
    ui::implementation::BlockchainSelection& parent) noexcept
    -> std::unique_ptr<ui::BlockchainSelectionQt>
{
    using ReturnType = ui::BlockchainSelectionQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(
    BlockchainSelectionQt,
    implementation::BlockchainSelection)

auto BlockchainSelectionQt::toggleChain(const int chain) const noexcept -> bool
{
    const auto out = parent_.ToggleChain(static_cast<blockchain::Type>(chain));
    emit updated();

    return out;
}
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
BlockchainSelection::BlockchainSelection(
    const api::client::internal::Manager& api,
    const api::client::internal::Blockchain& blockchain
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : BlockchainSelectionList(
          api,
          Identifier::Factory(),
          SimpleCallback {}
#if OT_QT
          ,
          qt,
          Roles{
              {BlockchainSelectionQt::TypeRole, "type"},
          },
          3
#endif
          )
    , blockchain_(blockchain)
{
}

auto BlockchainSelection::construct_row(
    const BlockchainSelectionRowID& id,
    const BlockchainSelectionSortKey& index,
    CustomData& custom) const noexcept -> void*
{
    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id,
        factory::BlockchainSelectionItem(
            *this, api_, blockchain_, id, index, custom));

    return it->second.get();
}

auto BlockchainSelection::init() noexcept -> void
{
    auto custom = CustomData{};

    for (const auto& chain : blockchain::SupportedChains()) {
        add_item(
            chain,
            {blockchain::IsTestnet(chain), blockchain::DisplayString(chain)},
            custom);
    }

    finish_startup();
}

auto BlockchainSelection::ToggleChain(
    const blockchain::Type type) const noexcept -> bool
{
    return blockchain_.ToggleChain(type);
}

BlockchainSelection::~BlockchainSelection() = default;
}  // namespace opentxs::ui::implementation
