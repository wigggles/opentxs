// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "ui/CustodialAccountActivity.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"  // IWYU pragma: keep
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/PaymentEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "ui/Widget.hpp"

#define OT_METHOD "opentxs::ui::implementation::CustodialAccountActivity::"

namespace opentxs::factory
{
auto AccountActivityModel(
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
    using ReturnType = ui::implementation::CustodialAccountActivity;

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
CustodialAccountActivity::CustodialAccountActivity(
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
          AccountType::Custodial,
#if OT_QT
          qt,
#endif
          {
              {api.Endpoints().WorkflowAccountUpdate(),
               new MessageProcessor<CustodialAccountActivity>(
                   &CustodialAccountActivity::process_workflow)},
              {api.Endpoints().AccountUpdate(),
               new MessageProcessor<CustodialAccountActivity>(
                   &CustodialAccountActivity::process_balance)},
          })
    , contract_(load_unit(api_, accountID))
    , notary_(load_server(api_, accountID))
    , alias_()
{
    startup_.reset(new std::thread(&CustodialAccountActivity::startup, this));

    OT_ASSERT(startup_)
}

auto CustodialAccountActivity::DisplayBalance() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    if (0 < contract_->Version()) {
        const auto amount = balance_.load();
        std::string output{};
        const auto formatted =
            contract_->FormatAmountLocale(amount, output, ",", ".");

        if (formatted) { return output; }

        return std::to_string(amount);
    }

    return {};
}

auto CustodialAccountActivity::extract_event(
    const proto::PaymentEventType eventType,
    const proto::PaymentWorkflow& workflow) noexcept -> EventRow
{
    bool success{false};
    bool found{false};
    EventRow output{};
    auto& [time, event_p] = output;

    for (const auto& event : workflow.event()) {
        const auto eventTime = Clock::from_time_t(event.time());

        if (eventType != event.type()) { continue; }

        if (eventTime > time) {
            if (success) {
                if (event.success()) {
                    time = eventTime;
                    event_p = &event;
                    found = true;
                }
            } else {
                time = eventTime;
                event_p = &event;
                success = event.success();
                found = true;
            }
        } else {
            if (false == success) {
                if (event.success()) {
                    // This is a weird case. It probably shouldn't happen
                    time = eventTime;
                    event_p = &event;
                    success = true;
                    found = true;
                }
            }
        }
    }

    if (false == found) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Workflow ")(workflow.id())(
            ", type ")(workflow.type())(", state ")(workflow.state())(
            " does not contain an event of type ")(eventType)
            .Flush();

        OT_FAIL;
    }

    return output;
}

auto CustodialAccountActivity::extract_rows(
    const proto::PaymentWorkflow& workflow) noexcept -> std::vector<RowKey>
{
    auto output = std::vector<RowKey>{};

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CANCEL,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CANCEL, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED:
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACCEPT,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACCEPT, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                case proto::PAYMENTWORKFLOWSTATE_INITIATED:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED:
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED:
                case proto::PAYMENTWORKFLOWSTATE_INITIATED:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_COMPLETE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_COMPLETE, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_INITIATED:
                case proto::PAYMENTWORKFLOWSTATE_ABORTED: {
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED:
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACCEPT,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACCEPT, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED:
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
                case proto::PAYMENTWORKFLOWSTATE_INITIATED:
                case proto::PAYMENTWORKFLOWSTATE_ABORTED:
                case proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED:
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_COMPLETE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_COMPLETE, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_INITIATED:
                case proto::PAYMENTWORKFLOWSTATE_ABORTED: {
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported workflow type (")(
                workflow.type())(")")
                .Flush();
        }
    }

    return output;
}

auto CustodialAccountActivity::load_server(
    const api::Core& api,
    const Identifier& id) -> OTServerContract
{
    return api.Wallet().Server(api.Storage().AccountServer(id));
}

auto CustodialAccountActivity::load_unit(
    const api::Core& api,
    const Identifier& id) -> OTUnitDefinition
{
    return api.Wallet().UnitDefinition(api.Storage().AccountContract(id));
}

auto CustodialAccountActivity::Name() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    return alias_;
}

auto CustodialAccountActivity::process_balance(
    const opentxs::network::zeromq::Message& message) noexcept -> void
{
    wait_for_startup();

    OT_ASSERT(2 == message.Body().size())

    const auto accountID = Identifier::Factory(message.Body().at(0));

    if (account_id_ != accountID) { return; }

    const auto& balance = message.Body().at(1);

    OT_ASSERT(balance.size() == sizeof(Amount))

    balance_.store(*static_cast<const Amount*>(balance.data()));

    {
        auto account = api_.Wallet().Account(account_id_);

        OT_ASSERT(account);

        eLock lock(shared_lock_);
        alias_ = account.get().Alias();
    }

    UpdateNotify();
}

auto CustodialAccountActivity::process_workflow(
    const Identifier& workflowID,
    std::set<AccountActivityRowID>& active) noexcept -> void
{
    const auto workflow = api_.Workflow().LoadWorkflow(primary_id_, workflowID);

    OT_ASSERT(workflow)

    const auto rows = extract_rows(*workflow);

    for (const auto& [type, row] : rows) {
        const auto& [time, event_p] = row;
        AccountActivityRowID key{Identifier::Factory(workflowID), type};
        CustomData custom{
            new proto::PaymentWorkflow(*workflow),
            new proto::PaymentEvent(*event_p)};
        add_item(key, time, custom);
        active.emplace(std::move(key));
    }
}

auto CustodialAccountActivity::process_workflow(
    const network::zeromq::Message& message) noexcept -> void
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto accountID = Identifier::Factory(id);

    OT_ASSERT(false == accountID->empty())

    if (account_id_ == accountID) { startup(); }
}

auto CustodialAccountActivity::startup() noexcept -> void
{
    auto account = api_.Wallet().Account(account_id_);

    if (account) {
        balance_.store(account.get().GetBalance());

        {
            eLock lock(shared_lock_);
            alias_ = account.get().Alias();
        }

        UpdateNotify();
    }

    account.Release();
    const auto workflows =
        api_.Workflow().WorkflowsByAccount(primary_id_, account_id_);
    std::set<AccountActivityRowID> active{};

    for (const auto& id : workflows) { process_workflow(id, active); }

    delete_inactive(active);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
