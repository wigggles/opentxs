// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/Proto.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>

#include "Workflow.hpp"

#define OUTGOING_CHEQUE_EVENT_VERSION 1
#define OUTGOING_CHEQUE_SOURCE_VERSION 1
#define OUTGOING_CHEQUE_WORKFLOW_VERSION 2
#define INCOMING_CHEQUE_EVENT_VERSION 1
#define INCOMING_CHEQUE_SOURCE_VERSION 1
#define INCOMING_CHEQUE_WORKFLOW_VERSION 2
#define RPC_ACCOUNT_EVENT_VERSION 1
#define RPC_PUSH_VERSION 1

#define OT_METHOD "opentxs::api::client::implementation::Workflow::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
api::client::Workflow* Factory::Workflow(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact)
{
    return new api::client::implementation::Workflow(api, activity, contact);
}
}  // namespace opentxs

namespace opentxs::api::client
{
bool Workflow::ContainsCheque(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {

            return true;
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        default: {
        }
    }

    return false;
}

std::string Workflow::ExtractCheque(const proto::PaymentWorkflow& workflow)
{
    if (false == ContainsCheque(workflow)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong workflow type"
              << std::endl;

        return {};
    }

    if (1 != workflow.source().size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid workflow" << std::endl;

        return {};
    }

    return workflow.source(0).item();
}

Workflow::Cheque Workflow::InstantiateCheque(
    const api::Core& core,
    const proto::PaymentWorkflow& workflow)
{
    Cheque output{proto::PAYMENTWORKFLOWSTATE_ERROR, nullptr};
    auto& [state, cheque] = output;

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            cheque.reset(core.Factory().Cheque().release());

            OT_ASSERT(cheque)

            const auto serialized = ExtractCheque(workflow);

            if (serialized.empty()) { return output; }

            const auto loaded = cheque->LoadContractFromString(
                String::Factory(serialized.c_str()));

            if (false == loaded) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to instantiate cheque" << std::endl;
                cheque.reset();

                return output;
            }

            state = workflow.state();
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow type"
                  << std::endl;
        }
    }

    return output;
}

bool Workflow::ContainsTransfer(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {

            return true;
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {
        }
    }

    return false;
}

std::string Workflow::ExtractTransfer(const proto::PaymentWorkflow& workflow)
{
    if (false == ContainsTransfer(workflow)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong workflow type"
              << std::endl;

        return {};
    }

    if (1 != workflow.source().size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid workflow" << std::endl;

        return {};
    }

    return workflow.source(0).item();
}

Workflow::Transfer Workflow::InstantiateTransfer(
    const api::Core& core,
    const proto::PaymentWorkflow& workflow)
{
    Transfer output{proto::PAYMENTWORKFLOWSTATE_ERROR, nullptr};
    auto& [state, transfer] = output;

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            transfer.reset(core.Factory().Transfer().release());

            OT_ASSERT(transfer)

            const auto serialized = ExtractTransfer(workflow);

            if (serialized.empty()) { return output; }

            const auto loaded =
                transfer->LoadContractFromString(serialized.c_str());

            if (false == loaded) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to instantiate transfer" << std::endl;
                transfer.reset();

                return output;
            }

            state = workflow.state();
        } break;

        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow type"
                  << std::endl;
        }
    }

    return output;
}

namespace implementation
{
Workflow::Workflow(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact)
    : api_(api)
    , activity_(activity)
    , contact_(contact)
    , account_publisher_(api_.ZeroMQ().PublishSocket())
    , rpc_publisher_(api_.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    // WARNING: do not access api_.Wallet() during construction
    const auto endpoint = api_.Endpoints().WorkflowAccountUpdate();
    otWarn << OT_METHOD << __FUNCTION__ << ": Binding to " << endpoint
           << std::endl;
    auto bound = account_publisher_->Start(endpoint);

    OT_ASSERT(bound)

    bound = rpc_publisher_->Start(
        api_.ZeroMQ().BuildEndpoint("rpc/push/internal", -1, 1));

    OT_ASSERT(bound)
}

bool Workflow::cheque_deposit_success(const Message* message)
{
    if (nullptr == message) { return false; }

    // TODO this might not be sufficient

    return message->m_bSuccess;
}

// Creates outgoing and internal transfer workflows.
OTIdentifier Workflow::CreateTransfer(
    const opentxs::OTItem& transfer,
    const Message& request,
    const Message* reply) const
{
    if (false == isTransfer(transfer)) { return Identifier::Factory(); }

    const auto senderNymID = transfer.GetSenderNymID().str();
    const auto recipientNymID = transfer.GetRecipientNymID().str();
    const bool is_internal = (0 == senderNymID.compare(recipientNymID));

    eLock lock(shared_lock_);
    const auto [workflowID, workflow] = is_internal
      ? create_transfer(
        lock,
        senderNymID,
        transfer,
        proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER,
        proto::PAYMENTWORKFLOWSTATE_INITIATED,
        INTERNAL_TRANSFER_WORKFLOW_VERSION,
        INTERNAL_TRANSFER_SOURCE_VERSION,
        INTERNAL_TRANSFER_EVENT_VERSION,
        recipientNymID,
        transfer.GetSenderAcctID().str())
    : create_transfer(
        lock,
        senderNymID,
        transfer,
        proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER,
        proto::PAYMENTWORKFLOWSTATE_INITIATED,
        OUTGOING_TRANSFER_WORKFLOW_VERSION,
        OUTGOING_TRANSFER_SOURCE_VERSION,
        OUTGOING_TRANSFER_EVENT_VERSION,
        recipientNymID,
        transfer.GetSenderAcctID().str());

    const bool haveWorkflow = (false == workflowID->empty());

    if (haveWorkflow) {
        update_activity(
            transfer.GetSenderNymID(),
            transfer.GetRecipientNymID(),
            Identifier::Factory(transfer),
            workflowID,
            is_internal
            ? StorageBox::INTERNALTRANSFER
            : StorageBox::OUTGOINGTRANSFER,
            std::chrono::system_clock::from_time_t(workflow.event(0).time()));
    }

    return workflowID;
}

// NOTE: Since this is an INCOMING transfer, then we need to CREATE its
// corresponding transfer workflow, since it does not already exist.
//
// (Whereas if this had been an INTERNAL transfer, then it would ALREADY
// have been created, and thus we'd need to GET the existing workflow, and
// then add the new event to it).
//
OTIdentifier Workflow::ConveyIncomingTransfer(
    const Identifier& nymID,
    const opentxs::OTItem& transfer) const
{
    if (false == isTransfer(transfer)) { return Identifier::Factory(); }

    const auto senderNymID = transfer.GetSenderNymID().str();
    const auto recipientNymID = transfer.GetRecipientNymID().str();
    const bool is_internal = (0 == senderNymID.compare(recipientNymID));

    if (is_internal) {
        otErr << OT_METHOD << __FUNCTION__ << ": Transfer is internal, not "
        "incoming. This function was erroneously called." << std::endl;

        return Identifier::Factory();
    }

    if (false == validate_recipient(nymID, transfer)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " can not accept this transfer." << std::endl;

        return Identifier::Factory();
    }
    const auto & party = senderNymID;

    eLock lock(shared_lock_);

    const auto [workflowID, workflow] = create_transfer(
        lock,
        nymID.str(),
        transfer,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        INCOMING_TRANSFER_WORKFLOW_VERSION,
        INCOMING_TRANSFER_SOURCE_VERSION,
        INCOMING_TRANSFER_EVENT_VERSION,
        party,
        "");

    if (false == workflowID->empty()) {
        update_activity(
            nymID,
            transfer.GetSenderNymID(),
            Identifier::Factory(transfer),
            workflowID,
            StorageBox::INCOMINGTRANSFER,
            extract_conveyed_time(workflow));
    }

    return workflowID;
}

// NOTE: Since this is an INTERNAL transfer, then it was already CREATED,
// and thus we need to GET the existing workflow, and then add the new
// event to it.
// Whereas if this is an INCOMING transfer, then we need to CREATE its
// corresponding transfer workflow since it does not already exist.
bool Workflow::ConveyInternalTransfer(
    const opentxs::OTItem& transfer,
    const Message& request,
    const Message* reply) const
{
    if (false == isTransfer(transfer)) { return false; }

    const auto senderNymID = transfer.GetSenderNymID().str();
    const auto recipientNymID = transfer.GetRecipientNymID().str();
    const bool is_internal = (0 == senderNymID.compare(recipientNymID));

    if (false == is_internal) {
        otErr << OT_METHOD << __FUNCTION__ << ": Transfer is incoming, not "
        "internal. This function was erroneously called." << std::endl;

        return false;
    }
    const auto & nymID = senderNymID;

    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER}, nymID, transfer);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this transfer does not exist." << std::endl;

        return false;
    }

    if (false == can_convey_transfer(*workflow)) { return false; }

    return add_transfer_event(
        nymID,
        request.m_strNymID2->Get(),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        proto::PAYMENTEVENTTYPE_CONVEY,
        INTERNAL_TRANSFER_EVENT_VERSION,
        request,
        reply);
}

// Works for Incoming and Internal transfer workflows.
bool Workflow::AcceptTransfer(
    const Identifier& receiver,
    const Identifier& accountID,
    const opentxs::OTItem& transfer,
    const Message& request,
    const Message* reply) const
{
    if (false == isTransfer(transfer)) { return false; }

    const auto senderNymID = transfer.GetSenderNymID().str();
    const auto recipientNymID = transfer.GetRecipientNymID().str();
    const bool is_internal = (0 == senderNymID.compare(recipientNymID));

    const auto nymID = receiver.str();

    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        { is_internal
            ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
            : proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER
        }, nymID, transfer);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this transfer does not exist." << std::endl;

        return false;
    }

    if (false == can_accept_transfer(*workflow)) { return false; }

    return add_transfer_event(
        nymID,
        transfer.GetSenderNymID().str(),
        *workflow,
        is_internal
        ? proto::PAYMENTWORKFLOWSTATE_ACCEPTED
        : proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        is_internal
        ? INTERNAL_TRANSFER_EVENT_VERSION
        : INCOMING_TRANSFER_EVENT_VERSION,
        request,
        reply,
        accountID.str());
}

// Works for outgoing and internal transfer workflows.
bool Workflow::CompleteTransfer(
    const opentxs::OTItem& transfer,
    const Message& request,
    const Message* reply) const
{
    if (false == isTransfer(transfer)) { return false; }

    const auto senderNymID = transfer.GetSenderNymID().str();
    const auto recipientNymID = transfer.GetRecipientNymID().str();
    const bool is_internal = (0 == senderNymID.compare(recipientNymID));

    const auto & nymID = senderNymID;

    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        { is_internal
            ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
            : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER
        }, nymID, transfer);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this transfer does not exist." << std::endl;

        return false;
    }

    if (false == can_complete_transfer(*workflow)) { return false; }

    return add_transfer_event(
        nymID,
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_COMPLETE,
        is_internal
        ? INTERNAL_TRANSFER_EVENT_VERSION
        : OUTGOING_TRANSFER_EVENT_VERSION,
        request,
        reply);
}

Transfer Workflow::LoadTransfer(
    const Identifier& nymID,
    const Identifier& transferID) const
{
    auto workflow = get_workflow_by_source(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        nymID.str(),
        transferID.str());

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this transfer does not exist." << std::endl;

        return {};
    }

    return InstantiateTransfer(api_, *workflow);
}

Transfer Workflow::LoadTransferByWorkflow(
    const Identifier& nymID,
    const Identifier& workflowID) const
{
    auto workflow = get_workflow_by_id(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        nymID.str(),
        workflowID.str());

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this transfer does not exist." << std::endl;

        return {};
    }

    return InstantiateTransfer(api_, *workflow);
}

bool Workflow::add_transfer_event(
    const std::string& nymID,
    const std::string& eventNym,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const Message& request,
    const Message* reply,
    const std::string& account) const
{
    const bool haveReply = (nullptr != reply);
    const bool success = (haveReply) ? reply->m_bSuccess : false;

    if (success) {
        workflow.set_state(newState);

        if ((false == account.empty()) && (0 == workflow.account_size())) {
            workflow.add_account(account);
        }
    }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(request)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(request.m_strNotaryID->Get());

    switch (newEventType) {
        case proto::PAYMENTEVENTTYPE_CANCEL:
        case proto::PAYMENTEVENTTYPE_COMPLETE: {
        } break;
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT: {
            event.set_nym(request.m_strNymID2->Get());
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        case proto::PAYMENTEVENTTYPE_CREATE:
        default: {
            OT_FAIL
        }
    }

    event.set_success(success);

    if (haveReply) {
        event.add_item(String(*reply).Get());
        event.set_time(reply->m_lTime);
    } else {
        event.set_time(request.m_lTime);
    }

    return save_workflow(nymID, account, workflow);
}

bool Workflow::add_cheque_event(
    const std::string& nymID,
    const std::string&,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const Message& request,
    const Message* reply,
    const std::string& account) const
{
    const bool haveReply = (nullptr != reply);
    const bool success = cheque_deposit_success(reply);

    if (success) {
        workflow.set_state(newState);

        if ((false == account.empty()) && (0 == workflow.account_size())) {
            workflow.add_account(account);
        }
    }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(request)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(request.m_strNotaryID->Get());

    switch (newEventType) {
        case proto::PAYMENTEVENTTYPE_CANCEL:
        case proto::PAYMENTEVENTTYPE_COMPLETE: {
        } break;
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT: {
            event.set_nym(request.m_strNymID2->Get());
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        case proto::PAYMENTEVENTTYPE_CREATE:
        default: {
            OT_FAIL
        }
    }

    event.set_success(success);

    if (haveReply) {
        event.add_item(String::Factory(*reply)->Get());
        event.set_time(reply->m_lTime);
    } else {
        event.set_time(request.m_lTime);
    }

    return save_workflow(nymID, account, workflow);
}

// Only used for ClearCheque
bool Workflow::add_cheque_event(
    const std::string& nymID,
    const std::string& accountID,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const Identifier& recipientNymID,
    const OTTransaction& receipt,
    const std::chrono::time_point<std::chrono::system_clock> time) const
{
    auto message = String::Factory();
    receipt.SaveContractRaw(message);
    workflow.set_state(newState);
    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(message->Get());
    event.set_time(std::chrono::system_clock::to_time_t(time));
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(receipt.GetRealNotaryID().str());
    event.set_nym(recipientNymID.str());
    event.set_success(true);

    if (0 == workflow.party_size()) {
        workflow.add_party(recipientNymID.str());
    }

    return save_workflow(nymID, accountID, workflow);
}

bool Workflow::can_accept_cheque(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_accept_transfer(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_cancel_cheque(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_UNSENT:
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_convey_cheque(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_UNSENT != workflow.state()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_convey_transfer(const proto::PaymentWorkflow& workflow)
{
    // NOTE: is a transfer "unsent" before CreateTransfer has been called?
    if (proto::PAYMENTWORKFLOWSTATE_UNSENT != workflow.state()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_deposit_cheque(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_CONVEYED != workflow.state()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_expire_cheque(
    const opentxs::Cheque& cheque,
    const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
                    correctState = true;
                } break;
                default: {
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
                    correctState = true;
                } break;
                default: {
                }
            }
        } break;
        default: {
            OT_FAIL
        }
    }

    if (false == correctState) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    if (now() < cheque.GetValidTo()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Can not expire valid cheque."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_finish_cheque(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_ACCEPTED != workflow.state()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::can_complete_transfer(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_ACCEPTED != workflow.state()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect workflow state."
              << std::endl;

        return false;
    }

    return true;
}

bool Workflow::CancelCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_cancel_cheque(*workflow)) { return false; }

    return add_cheque_event(
        nymID,
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_CANCELLED,
        proto::PAYMENTEVENTTYPE_CANCEL,
        OUTGOING_CHEQUE_EVENT_VERSION,
        request,
        reply);
}

bool Workflow::ClearCheque(
    const Identifier& recipientNymID,
    const OTTransaction& receipt) const
{
    if (recipientNymID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid cheque recipient"
              << std::endl;

        return false;
    }

    auto cheque{api_.Factory().Cheque(receipt)};

    if (false == bool(cheque)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to load cheque from receipt." << std::endl;

        return false;
    }

    if (false == isCheque(*cheque)) { return false; }

    const auto nymID = cheque->GetSenderNymID().str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, *cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_accept_cheque(*workflow)) { return false; }

    OT_ASSERT(1 == workflow->account_size())

    const bool needNym = (0 == workflow->party_size());
    const auto time = std::chrono::system_clock::from_time_t(now());
    const auto output = add_cheque_event(
        nymID,
        workflow->account(0),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_ACCEPTED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        OUTGOING_CHEQUE_EVENT_VERSION,
        recipientNymID,
        receipt,
        time);

    if (needNym) {
        update_activity(
            cheque->GetSenderNymID(),
            recipientNymID,
            Identifier::Factory(*cheque),
            Identifier::Factory(workflow->id()),
            StorageBox::OUTGOINGCHEQUE,
            extract_conveyed_time(*workflow));
    }

    update_rpc(
        nymID,
        cheque->GetRecipientNymID().str(),
        cheque->SourceAccountID().str(),
        proto::ACCOUNTEVENT_OUTGOINGCHEQUE,
        workflow->id(),
        -1 * cheque->GetAmount(),
        0,
        time,
        cheque->GetMemo().Get());

    return output;
}

std::pair<OTIdentifier, proto::PaymentWorkflow> Workflow::create_transfer(
        const eLock& lock,
        const std::string& nymID,
        const opentxs::OTItem& transfer,
        const proto::PaymentWorkflowType workflowType,
        const proto::PaymentWorkflowState workflowState,
        const std::uint32_t workflowVersion,
        const std::uint32_t sourceVersion,
        const std::uint32_t eventVersion,
        const std::string& party,
        const std::string& account,
        const Message* message) const
{
    OT_ASSERT(verify_lock(lock))

    std::pair<OTIdentifier, proto::PaymentWorkflow> output{
        Identifier::Factory(), {}};
    auto& [workflowID, workflow] = output;
    const auto transferID = Identifier::Factory(transfer);
    const std::string serialized = String::Factory(transfer)->Get();
    const auto existing = get_workflow({workflowType}, nymID, transfer);

    if (existing) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this transfer already exists." << std::endl;
        workflowID = Identifier::Factory(existing->id());

        return output;
    }

    workflowID = Identifier::Random();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID->str());
    workflow.set_type(workflowType);
    workflow.set_state(workflowState);
    auto& source = *(workflow.add_source());
    source.set_version(sourceVersion);
    source.set_id(transferID->str());
    source.set_revision(1);
    source.set_item(serialized);
    // Note: is the below correct? Shouldn't it be notaryID, not unit type?
    workflow.set_notary(transfer.GetInstrumentDefinitionID().str());

    // add party if it was passed in and is not already present
    if ((false == party.empty()) && (0 == workflow.party_size())) {
        workflow.add_party(party);
    }

    auto& event = *workflow.add_event();
    event.set_version(eventVersion);

    event.set_time(now());

    if (proto::PAYMENTWORKFLOWSTATE_INITIATED == workflowState) {
        event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
        event.set_method(proto::TRANSPORTMETHOD_OT);
        OT_ASSERT(nullptr != message);
        event.set_transport(String(message->m_strNotaryID).Get());
    } else if (proto::PAYMENTWORKFLOWSTATE_CONVEYED == workflowState) {
        event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
        event.set_method(proto::TRANSPORTMETHOD_OT);
        OT_ASSERT(nullptr != message);
        event.set_transport(String(message->m_strNotaryID).Get());
    } else {
        OT_FAIL
    }


    if (false == party.empty()) { event.set_nym(party); }

    event.set_success(true);
    workflow.add_unit(transfer.GetInstrumentDefinitionID().str());

    // add account if it was passed in and is not already present
    if ((false == account.empty()) && (0 == workflow.account_size())) {
        workflow.add_account(account);
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

std::pair<OTIdentifier, proto::PaymentWorkflow> Workflow::create_cheque(
    const eLock& lock,
    const std::string& nymID,
    const opentxs::Cheque& cheque,
    const proto::PaymentWorkflowType workflowType,
    const proto::PaymentWorkflowState workflowState,
    const std::uint32_t workflowVersion,
    const std::uint32_t sourceVersion,
    const std::uint32_t eventVersion,
    const std::string& party,
    const std::string& account,
    const Message* message) const
{
    OT_ASSERT(verify_lock(lock))

    std::pair<OTIdentifier, proto::PaymentWorkflow> output{
        Identifier::Factory(), {}};
    auto& [workflowID, workflow] = output;
    const auto chequeID = Identifier::Factory(cheque);
    const std::string serialized = String::Factory(cheque)->Get();
    const auto existing = get_workflow({workflowType}, nymID, cheque);

    if (existing) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque already exists." << std::endl;
        workflowID = Identifier::Factory(existing->id());

        return output;
    }

    workflowID = Identifier::Random();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID->str());
    workflow.set_type(workflowType);
    workflow.set_state(workflowState);
    auto& source = *(workflow.add_source());
    source.set_version(sourceVersion);
    source.set_id(chequeID->str());
    source.set_revision(1);
    source.set_item(serialized);
    workflow.set_notary(cheque.GetInstrumentDefinitionID().str());

    // add party if it was passed in and is not already present
    if ((false == party.empty()) && (0 == workflow.party_size())) {
        workflow.add_party(party);
    }

    auto& event = *workflow.add_event();
    event.set_version(eventVersion);

    if (nullptr != message) {
        event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
        event.set_transport(message->m_strNotaryID->Get());
        event.set_time(message->m_lTime);
        event.set_method(proto::TRANSPORTMETHOD_OT);
        event.set_transport(message->m_strNotaryID->Get());
    } else {
        event.set_time(now());

        if (proto::PAYMENTWORKFLOWSTATE_UNSENT == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
            event.set_method(proto::TRANSPORTMETHOD_NONE);
        } else if (proto::PAYMENTWORKFLOWSTATE_CONVEYED == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
            event.set_method(proto::TRANSPORTMETHOD_OOB);
        } else {
            OT_FAIL
        }
    }

    if (false == party.empty()) { event.set_nym(party); }

    event.set_success(true);
    workflow.add_unit(cheque.GetInstrumentDefinitionID().str());

    // add account if it was passed in and is not already present
    if ((false == account.empty()) && (0 == workflow.account_size())) {
        workflow.add_account(account);
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

bool Workflow::DepositCheque(
    const Identifier& receiver,
    const Identifier& accountID,
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = receiver.str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_deposit_cheque(*workflow)) { return false; }

    const auto output = add_cheque_event(
        nymID,
        cheque.GetSenderNymID().str(),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        INCOMING_CHEQUE_EVENT_VERSION,
        request,
        reply,
        accountID.str());

    if (output && cheque_deposit_success(reply)) {
        update_rpc(
            receiver.str(),
            cheque.GetSenderNymID().str(),
            accountID.str(),
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflow->id(),
            cheque.GetAmount(),
            0,
            std::chrono::system_clock::from_time_t(reply->m_lTime),
            cheque.GetMemo().Get());
    }

    return output;
}

bool Workflow::ExpireCheque(
    const Identifier& nym,
    const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = nym.str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID,
        cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_expire_cheque(cheque, *workflow)) { return false; }

    workflow->set_state(proto::PAYMENTWORKFLOWSTATE_EXPIRED);

    return save_workflow(nymID, cheque.GetSenderAcctID().str(), *workflow);
}

bool Workflow::ExportCheque(const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow({}, nymID, cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_convey_cheque(*workflow)) { return false; }

    workflow->set_state(proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    auto& event = *(workflow->add_event());
    event.set_version(OUTGOING_CHEQUE_EVENT_VERSION);
    event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
    event.set_time(now());
    event.set_method(proto::TRANSPORTMETHOD_OOB);
    event.set_success(true);

    return save_workflow(nymID, cheque.GetSenderAcctID().str(), *workflow);
}

std::chrono::time_point<std::chrono::system_clock> Workflow::
    extract_conveyed_time(const proto::PaymentWorkflow& workflow)
{
    for (const auto& event : workflow.event()) {
        if (proto::PAYMENTEVENTTYPE_CONVEY == event.type()) {
            if (event.success()) {

                return std::chrono::system_clock::from_time_t(event.time());
            }
        }
    }

    return {};
}

bool Workflow::FinishCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_finish_cheque(*workflow)) { return false; }

    return add_cheque_event(
        nymID,
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_COMPLETE,
        OUTGOING_CHEQUE_EVENT_VERSION,
        request,
        reply);
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow(
    const std::set<proto::PaymentWorkflowType>& types,
    const std::string& nymID,
    const opentxs::Cheque& source) const
{
    return get_workflow_by_source(
        types, nymID, Identifier::Factory(source)->str());
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow_by_id(
    const std::string& nymID,
    const std::string& workflowID) const
{
    std::shared_ptr<proto::PaymentWorkflow> output{nullptr};
    const auto loaded = api_.Storage().Load(nymID, workflowID, output);

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Workflow " << workflowID
              << " for nym " << nymID << " can not be loaded" << std::endl;

        return output;
    }

    return output;
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow_by_id(
    const std::set<proto::PaymentWorkflowType>& types,
    const std::string& nymID,
    const std::string& workflowID) const
{
    auto output = get_workflow_by_id(nymID, workflowID);

    if (0 == types.count(output->type())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect type on workflow "
              << workflowID << " for nym " << nymID << std::endl;

        return {nullptr};
    }

    return output;
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow_by_source(
    const std::set<proto::PaymentWorkflowType>& types,
    const std::string& nymID,
    const std::string& sourceID) const
{
    const auto workflowID =
        api_.Storage().PaymentWorkflowLookup(nymID, sourceID);

    if (workflowID.empty()) { return {}; }

    return get_workflow_by_id(types, nymID, workflowID);
}

bool Workflow::isCheque(const opentxs::Cheque& cheque)
{
    if (cheque.HasRemitter()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Provided instrument is a voucher" << std::endl;

        return false;
    }

    if (0 > cheque.GetAmount()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Provided instrument is an invoice" << std::endl;

        return false;
    }

    if (0 == cheque.GetAmount()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Provided instrument is a cancellation" << std::endl;

        return false;
    }

    return true;
}

OTIdentifier Workflow::ImportCheque(
    const Identifier& nymID,
    const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    if (false == validate_recipient(nymID, cheque)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " can not deposit this cheque." << std::endl;

        return Identifier::Factory();
    }

    const std::string party = cheque.GetSenderNymID().str();
    eLock lock(shared_lock_);
    const auto [workflowID, workflow] = create_cheque(
        lock,
        nymID.str(),
        cheque,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        INCOMING_CHEQUE_WORKFLOW_VERSION,
        INCOMING_CHEQUE_SOURCE_VERSION,
        INCOMING_CHEQUE_EVENT_VERSION,
        party,
        "");

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            Identifier::Factory(cheque),
            workflowID,
            StorageBox::INCOMINGCHEQUE,
            time);
        update_rpc(
            nymID.str(),
            cheque.GetSenderNymID().str(),
            "",
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflowID->str(),
            0,
            cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}

std::set<OTIdentifier> Workflow::List(
    const Identifier& nymID,
    const proto::PaymentWorkflowType type,
    const proto::PaymentWorkflowState state) const
{
    const auto input =
        api_.Storage().PaymentWorkflowsByState(nymID.str(), type, state);
    std::set<OTIdentifier> output{};
    std::transform(
        input.begin(),
        input.end(),
        std::inserter(output, output.end()),
        [](const std::string& id) -> OTIdentifier {
            return Identifier::Factory(id);
        });

    return output;
}

Workflow::Cheque Workflow::LoadCheque(
    const Identifier& nymID,
    const Identifier& chequeID) const
{
    auto workflow = get_workflow_by_source(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID.str(),
        chequeID.str());

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return {};
    }

    return InstantiateCheque(api_, *workflow);
}

Workflow::Cheque Workflow::LoadChequeByWorkflow(
    const Identifier& nymID,
    const Identifier& workflowID) const
{
    auto workflow = get_workflow_by_id(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID.str(),
        workflowID.str());

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return {};
    }

    return InstantiateCheque(api_, *workflow);
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::LoadWorkflow(
    const Identifier& nymID,
    const Identifier& workflowID) const
{
    return get_workflow_by_id(nymID.str(), workflowID.str());
}

std::int64_t Workflow::now() { return std::time(nullptr); }

OTIdentifier Workflow::ReceiveCheque(
    const Identifier& nymID,
    const opentxs::Cheque& cheque,
    const Message& message) const
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    if (false == validate_recipient(nymID, cheque)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " can not deposit this cheque." << std::endl;

        return Identifier::Factory();
    }

    const std::string party = cheque.GetSenderNymID().str();
    eLock lock(shared_lock_);
    const auto [workflowID, workflow] = create_cheque(
        lock,
        nymID.str(),
        cheque,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        INCOMING_CHEQUE_WORKFLOW_VERSION,
        INCOMING_CHEQUE_SOURCE_VERSION,
        INCOMING_CHEQUE_EVENT_VERSION,
        party,
        "",
        &message);

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            Identifier::Factory(cheque),
            workflowID,
            StorageBox::INCOMINGCHEQUE,
            time);
        update_rpc(
            nymID.str(),
            cheque.GetSenderNymID().str(),
            "",
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflowID->str(),
            0,
            cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}

bool Workflow::save_workflow(
    const std::string& nymID,
    const std::string& accountID,
    const proto::PaymentWorkflow& workflow) const
{
    const bool valid = proto::Validate(workflow, VERBOSE);

    OT_ASSERT(valid)

    const auto saved = api_.Storage().Store(nymID, workflow);

    OT_ASSERT(saved)

    if (false == accountID.empty()) { account_publisher_->Publish(accountID); }

    return valid && saved;
}

OTIdentifier Workflow::save_workflow(
    OTIdentifier&& output,
    const std::string& nymID,
    const std::string& accountID,
    const proto::PaymentWorkflow& workflow) const
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return Identifier::Factory();
}

std::pair<OTIdentifier, proto::PaymentWorkflow> Workflow::save_workflow(
    std::pair<OTIdentifier, proto::PaymentWorkflow>&& output,
    const std::string& nymID,
    const std::string& accountID,
    const proto::PaymentWorkflow& workflow) const
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return {Identifier::Factory(), {}};
}

bool Workflow::SendCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    eLock lock(shared_lock_);
    auto workflow = get_workflow(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Workflow for this cheque does not exist." << std::endl;

        return false;
    }

    if (false == can_convey_cheque(*workflow)) { return false; }

    return add_cheque_event(
        nymID,
        request.m_strNymID2->Get(),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        proto::PAYMENTEVENTTYPE_CONVEY,
        OUTGOING_CHEQUE_EVENT_VERSION,
        request,
        reply);
}

bool Workflow::validate_recipient(
    const Identifier& nymID,
    const opentxs::Cheque& cheque)
{
    if (nymID.empty()) { return true; }

    return (nymID == cheque.GetRecipientNymID());
}


bool Workflow::validate_recipient(
    const Identifier& nymID,
    const opentxs::OTItem& transfer)
{
    if (nymID.empty()) { return true; }

    return (nymID == transfer.GetRecipientNymID());
}

void Workflow::update_activity(
    const Identifier& localNymID,
    const Identifier& remoteNymID,
    const Identifier& sourceID,
    const Identifier& workflowID,
    const StorageBox type,
    std::chrono::time_point<std::chrono::system_clock> time) const
{
    const auto contactID = contact_.ContactID(remoteNymID);

    OT_ASSERT(false == contactID->empty())

    const bool added = activity_.AddPaymentEvent(
        localNymID, contactID, type, sourceID, workflowID, time);

    if (added) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Success adding payment event to thread " << contactID->str()
              << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to add payment event to thread " << contactID->str()
              << std::endl;
    }
}

void Workflow::update_rpc(
    const std::string& localNymID,
    const std::string& remoteNymID,
    const std::string& accountID,
    const proto::AccountEventType type,
    const std::string& workflowID,
    const Amount amount,
    const Amount pending,
    const std::chrono::time_point<std::chrono::system_clock> time,
    const std::string& memo) const
{
    proto::RPCPush push{};
    push.set_version(RPC_PUSH_VERSION);
    push.set_type(proto::RPCPUSH_ACCOUNT);
    push.set_id(localNymID);
    auto& event = *push.mutable_accountevent();
    event.set_version(RPC_ACCOUNT_EVENT_VERSION);
    event.set_id(accountID);
    event.set_type(type);

    if (false == remoteNymID.empty()) {
        event.set_contact(
            contact_.NymToContact(Identifier::Factory(remoteNymID))->str());
    }

    event.set_workflow(workflowID);
    event.set_amount(amount);
    event.set_pendingamount(pending);
    event.set_timestamp(std::chrono::system_clock::to_time_t(time));
    event.set_memo(memo);

    OT_ASSERT(proto::Validate(push, VERBOSE));

    auto message = zmq::Message::Factory();
    message->AddFrame();
    message->AddFrame(localNymID);
    message->AddFrame(proto::ProtoAsData(push));
    rpc_publisher_->Push(message);
}

std::vector<OTIdentifier> Workflow::WorkflowsByAccount(
    const Identifier& nymID,
    const Identifier& accountID) const
{
    std::vector<OTIdentifier> output{};
    const auto workflows =
        api_.Storage().PaymentWorkflowsByAccount(nymID.str(), accountID.str());
    std::transform(
        workflows.begin(),
        workflows.end(),
        std::inserter(output, output.end()),
        [](const std::string& id) -> OTIdentifier {
            return Identifier::Factory(id);
        });

    return output;
}

OTIdentifier Workflow::WriteCheque(const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    eLock lock(shared_lock_);
    const std::string party =
        cheque.HasRecipient() ? cheque.GetRecipientNymID().str() : "";
    const auto [workflowID, workflow] = create_cheque(
        lock,
        cheque.GetSenderNymID().str(),
        cheque,
        proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_UNSENT,
        OUTGOING_CHEQUE_WORKFLOW_VERSION,
        OUTGOING_CHEQUE_SOURCE_VERSION,
        OUTGOING_CHEQUE_EVENT_VERSION,
        party,
        cheque.GetSenderAcctID().str());
    const bool haveWorkflow = (false == workflowID->empty());
    const auto time{
        std::chrono::system_clock::from_time_t(workflow.event(0).time())};

    if (haveWorkflow && cheque.HasRecipient()) {
        update_activity(
            cheque.GetSenderNymID(),
            cheque.GetRecipientNymID(),
            Identifier::Factory(cheque),
            workflowID,
            StorageBox::OUTGOINGCHEQUE,
            time);
    }

    if (false == workflowID->empty()) {
        update_rpc(
            cheque.GetSenderNymID().str(),
            cheque.GetRecipientNymID().str(),
            cheque.SourceAccountID().str(),
            proto::ACCOUNTEVENT_OUTGOINGCHEQUE,
            workflowID->str(),
            0,
            -1 * cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}
}  // namespace implementation
}  // namespace opentxs::api::client
