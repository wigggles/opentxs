// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include "internal/api/client/Client.hpp"

#include "StateMachine.tpp"

#define CONTRACT_DOWNLOAD_MILLISECONDS 10000
#define NYM_REGISTRATION_MILLISECONDS 10000
#define STATE_MACHINE_READY_MILLISECONDS 100

#define DO_OPERATION(a, ...)                                                   \
    auto started = op_.a(__VA_ARGS__);                                         \
                                                                               \
    while (false == started) {                                                 \
        LogDebug(OT_METHOD)(__FUNCTION__)(": State machine is not ready")      \
            .Flush();                                                          \
        Log::Sleep(                                                            \
            std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));      \
                                                                               \
        if (running_) {                                                        \
            started = op_.a(__VA_ARGS__);                                      \
        } else {                                                               \
            op_.Shutdown();                                                    \
            return false;                                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    if (!running_) {                                                           \
        op_.Shutdown();                                                        \
        return false;                                                          \
    }                                                                          \
                                                                               \
    Result result = op_.GetFuture().get();                                     \
    const auto success =                                                       \
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

#define DO_OPERATION_TASK_DONE(a, ...)                                         \
    auto started = op_.a(__VA_ARGS__);                                         \
                                                                               \
    while (false == started) {                                                 \
        LogDebug(OT_METHOD)(__FUNCTION__)(": State machine is not ready")      \
            .Flush();                                                          \
        Log::Sleep(                                                            \
            std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));      \
                                                                               \
        if (running_) {                                                        \
            started = op_.a(__VA_ARGS__);                                      \
        } else {                                                               \
            op_.Shutdown();                                                    \
            return task_done(false);                                           \
        }                                                                      \
    }                                                                          \
                                                                               \
    if (!running_) {                                                           \
        op_.Shutdown();                                                        \
        return task_done(false);                                               \
    }                                                                          \
                                                                               \
    Result result = op_.GetFuture().get();                                     \
    const auto success =                                                       \
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

#define OT_METHOD "opentxs::api::client::implementation::StateMachine::"

namespace opentxs::api::client::implementation
{
StateMachine::StateMachine(
    const api::client::Manager& client,
    const api::client::internal::OTX& parent,
    const Flag& running,
    const api::client::Manager& api,
    const ContextID& id,
    std::atomic<TaskID>& nextTaskID,
    const UniqueQueue<CheckNymTask>& missingnyms,
    const UniqueQueue<CheckNymTask>& outdatednyms,
    const UniqueQueue<OTServerID>& missingservers,
    const UniqueQueue<OTUnitID>& missingUnitDefinitions)
    : client_(client)
    , parent_(parent)
    , next_task_id_(nextTaskID)
    , missing_nyms_(missingnyms)
    , outdated_nyms_(outdatednyms)
    , missing_servers_(missingservers)
    , missing_unit_definitions_(missingUnitDefinitions)
    , pOp_(opentxs::Factory::Operation(api, id.first, id.second))
    , op_(*pOp_)
    , check_nym_()
    , deposit_payment_()
    , download_contract_()
    , download_mint_()
    , download_nymbox_()
    , get_transaction_numbers_()
    , issue_unit_definition_()
    , register_account_()
    , register_nym_()
    , send_message_()
    , send_payment_()
#if OT_CASH
    , send_cash_()
    , withdraw_cash_()
#endif  // OT_CASH
    , send_transfer_()
    , publish_server_contract_()
    , process_inbox_()
    , send_cheque_()
    , peer_reply_()
    , peer_request_()
    , running_(running)
    , param_()
    , task_id_()
    , counter_(0)
    , lock_()
    , thread_(false)
    , continue_(false)
    , tasks_()
{
    OT_ASSERT(pOp_);
}

std::future<void> StateMachine::add_task(const Lock& lock)
{
    OT_ASSERT(lock.owns_lock());

    tasks_.emplace_back(counter_.load() + 2, std::promise<void>{});
    auto& task = *tasks_.rbegin();
    auto& promise = task.second;

    return promise.get_future();
}

std::future<void> StateMachine::check_future(
    Thread task,
    std::unique_ptr<std::thread>& thread)
{
    Lock lock(lock_);
    check_thread(lock, task, thread);

    return add_task(lock);
}

void StateMachine::check_nym_revision(const ServerContext& context) const
{
    if (context.StaleNym()) {
        const auto& nymID = context.Nym()->ID();
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has is newer than version last registered version on server ")(
            context.Server())(".")
            .Flush();
        get_task<RegisterNymTask>().Push(next_task_id(), true);
    }
}

bool StateMachine::check_registration(
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    std::shared_ptr<const ServerContext>& context) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    context = client_.Wallet().ServerContext(nymID, serverID);
    RequestNumber request{0};

    if (context) {
        request = context->Request();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has never registered on ")(serverID)
            .Flush();
    }

    if (0 != request) {
        OT_ASSERT(context)

        return true;
    }

    const auto output = register_nym(next_task_id(), false);

    if (output) {
        context = client_.Wallet().ServerContext(nymID, serverID);

        OT_ASSERT(context)
    }

    return output;
}

bool StateMachine::check_server_contract(
    const identifier::Server& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    const auto serverContract = client_.Wallet().Server(serverID);

    if (serverContract) { return true; }

    LogDetail(OT_METHOD)(__FUNCTION__)(": Server contract for ")(serverID)(
        " is not in the wallet.")
        .Flush();
    missing_servers_.Push(next_task_id(), serverID);

    return false;
}

bool StateMachine::check_server_name(const ServerContext& context) const
{
    const auto server = client_.Wallet().Server(op_.ServerID());

    OT_ASSERT(server)

    const auto myName = server->Alias();
    const auto hisName = server->EffectiveName();

    if (myName == hisName) { return true; }

    DO_OPERATION(
        AddClaim,
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        String::Factory(myName),
        true);

    if (success) {
        get_task<CheckNymTask>().Push(next_task_id(), context.RemoteNym().ID());
    }

    return success;
}

void StateMachine::check_thread(
    const Lock& lock,
    Thread task,
    std::unique_ptr<std::thread>& thread)
{
    OT_ASSERT(lock.owns_lock());

    if (thread_) {
        continue_ = true;
    } else {
        if (thread && thread->joinable()) { thread->join(); }

        thread.reset(new std::thread(task));
        thread_ = bool(thread);
    }
}

void StateMachine::check_thread(
    Thread task,
    std::unique_ptr<std::thread>& thread)
{
    Lock lock(lock_);
    check_thread(lock, task, thread);
}

void StateMachine::check_transaction_numbers(const ServerContext& context) const
{
    if (0 == context.Accounts().size()) { return; }

    if (0 < context.AvailableNumbers()) { return; }

    get_task<GetTransactionNumbersTask>().Push(next_task_id(), {});
}

void StateMachine::cleanup(Lock& lock)
{
    // Ignore "already locked" errors
    try {
        lock.lock();
    } catch (...) {
    }

    if (lock.owns_lock()) {
        thread_ = false;
    } else {
        OT_FAIL;
    }

    lock.unlock();
}

bool StateMachine::deposit_cheque(
    const TaskID taskID,
    const DepositPaymentTask& task) const
{
    const auto& [accountID, payment] = task;

    OT_ASSERT(false == accountID->empty());
    OT_ASSERT(payment);

    if ((false == payment->IsCheque()) && (false == payment->IsVoucher())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled payment type.").Flush();

        return finish_task(taskID, false, error_result());
    }

    std::shared_ptr<Cheque> cheque{client_.Factory().Cheque()};

    OT_ASSERT(cheque);

    const auto loaded = cheque->LoadContractFromString(payment->Payment());

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid cheque.").Flush();

        return finish_task(taskID, false, error_result());
    }

    DO_OPERATION(DepositCheque, accountID, cheque);

    if (success) { return finish_task(taskID, success, std::move(result)); }

    return false;
}

bool StateMachine::download_contract(
    const TaskID taskID,
    const Identifier& contractID) const
{
    OT_ASSERT(false == contractID.empty())

    DO_OPERATION(DownloadContract, contractID);

    return finish_task(taskID, success, std::move(result));
}

#if OT_CASH
bool StateMachine::download_mint(
    const TaskID taskID,
    const DownloadMintTask& task) const
{
    DO_OPERATION(Start, internal::Operation::Type::DownloadMint, task, {});

    return finish_task(taskID, success, std::move(result));
}
#endif

bool StateMachine::download_nym(
    const TaskID taskID,
    const identifier::Nym& targetNymID) const
{
    OT_ASSERT(false == targetNymID.empty())

    ServerContext::ExtraArgs args{};

    DO_OPERATION(Start, internal::Operation::Type::CheckNym, targetNymID, args);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::download_nymbox(const TaskID taskID) const
{
    op_.join();
    auto contextE =
        client_.Wallet().mutable_ServerContext(op_.NymID(), op_.ServerID());
    auto& context = contextE.It();
    context.Join();
    context.ResetThread();
    auto future = context.RefreshNymbox(client_);

    if (false == bool(future)) {

        return finish_task(taskID, false, error_result());
    }

    Result result{future->get()};
    const auto success =
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

    return finish_task(taskID, success, std::move(result));
}

StateMachine::Result StateMachine::error_result()
{
    Result output{proto::LASTREPLYSTATUS_NOTSENT, nullptr};

    return output;
}

bool StateMachine::find_nym(
    const identifier::Nym& targetNymID,
    const bool skipExisting) const
{
    OT_ASSERT(false == targetNymID.empty())

    const auto nym = client_.Wallet().Nym(targetNymID);

    if (nym && skipExisting) {
        missing_nyms_.CancelByValue(targetNymID);

        return true;
    }

    if (download_nym(next_task_id(), targetNymID)) {
        missing_nyms_.CancelByValue(targetNymID);
        outdated_nyms_.CancelByValue(targetNymID);

        return true;
    }

    return false;
}

bool StateMachine::find_server(const identifier::Server& notary) const
{
    OT_ASSERT(false == notary.empty())

    const auto serverContract = client_.Wallet().Server(notary);

    if (serverContract) {
        missing_servers_.CancelByValue(notary);

        return true;
    }

    if (download_contract(next_task_id(), notary)) {
        missing_servers_.CancelByValue(notary);

        return true;
    }

    return false;
}

bool StateMachine::get_admin(const TaskID taskID, const OTPassword& password)
    const
{
    DO_OPERATION(RequestAdmin, String::Factory(password.getPassword()));

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::get_transaction_numbers(const TaskID taskID) const
{
    ServerContext::ExtraArgs args{};

    DO_OPERATION(Start, internal::Operation::Type::GetTransactionNumbers, args);

    return finish_task(taskID, success, std::move(result));
}

void StateMachine::increment_counter(const bool missing, Lock& lock, bool& run)
{
    lock.lock();
    ++counter_;

    if (missing) {
        run = true;
    } else if (continue_) {
        run = true;
        continue_ = false;
    } else {
        run = false;
    }

    for (auto i = tasks_.begin(); i < tasks_.end();) {
        // auto& [limit, future] = *i;
        auto& limit = std::get<0>(*i);
        auto& future = std::get<1>(*i);
        const bool erase = (false == run) || (counter_ >= limit);

        if (erase) {
            future.set_value();
            i = tasks_.erase(i);
        } else {
            ++i;
        }
    }

    if (run) { lock.unlock(); }
}

bool StateMachine::initiate_peer_reply(
    const TaskID taskID,
    const PeerReplyTask& task) const
{
    const auto [targetNymID, peerReply, peerRequest] = task;

    DO_OPERATION(SendPeerReply, targetNymID, peerReply, peerRequest);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::initiate_peer_request(
    const TaskID taskID,
    const PeerRequestTask& task) const
{
    const auto& [targetNymID, peerRequest] = task;

    DO_OPERATION(SendPeerRequest, targetNymID, peerRequest);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::issue_unit_definition(
    const TaskID taskID,
    const IssueUnitDefinitionTask& task) const
{
    const auto& [unitID, label] = task;
    auto unitDefinition = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(unitDefinition)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unit definition not found.")
            .Flush();

        return false;
    }

    auto serialized = std::make_shared<proto::UnitDefinition>();

    OT_ASSERT(serialized);

    *serialized = unitDefinition->PublicContract();
    ServerContext::ExtraArgs args{label, false};

    DO_OPERATION(IssueUnitDefinition, serialized, args);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::message_nym(const TaskID taskID, const MessageTask& task)
    const
{
    const auto& [recipient, text, setID] = task;
    auto messageID = Identifier::Factory();
    auto updateID = [&](const Identifier& in) -> void {
        messageID = in;
        auto& pID = std::get<2>(task);

        if (pID) {
            auto& id = *pID;

            if (id) { id(in); }
        }
    };

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(SendMessage, recipient, String::Factory(text), updateID);

    if (success) {
        if (false == messageID->empty()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent message: ")(messageID)
                .Flush();
            associate_message_id(messageID, taskID);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message ID").Flush();
        }
    }

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::pay_nym(const TaskID taskID, const PaymentTask& task) const
{
    const auto& [recipient, payment] = task;

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(ConveyPayment, recipient, payment);

    return finish_task(taskID, success, std::move(result));
}

#if OT_CASH
bool StateMachine::pay_nym_cash(const TaskID taskID, const PayCashTask& task)
    const
{
    const auto& [recipient, workflowID] = task;

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(SendCash, recipient, workflowID);

    return finish_task(taskID, success, std::move(result));
}
#endif  // OT_CASH

void StateMachine::Process()
{
    struct Cleanup {
        Lock& lock_;
        StateMachine& queue_;

        Cleanup(Lock& lock, StateMachine& queue)
            : lock_(lock)
            , queue_(queue)
        {
        }

        ~Cleanup() { queue_.cleanup(lock_); }
    };

    Lock lock{lock_, std::defer_lock};
    Cleanup cleanup(lock, *this);
    const auto& nymID = op_.NymID();
    const auto& serverID = op_.ServerID();

    // Make sure the server contract is available
    while (running_) {
        if (check_server_contract(serverID)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Server contract ")(serverID)(
                " exists.")
                .Flush();

            break;
        }

        YIELD(CONTRACT_DOWNLOAD_MILLISECONDS);
    }

    SHUTDOWN()

    std::shared_ptr<const ServerContext> context{nullptr};

    // Make sure the nym has registered for the first time on the server
    while (running_) {
        if (check_registration(nymID, serverID, context)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
                " has registered on server ")(serverID)(" at least once.")
                .Flush();

            break;
        }

        YIELD(NYM_REGISTRATION_MILLISECONDS);
    }

    SHUTDOWN()
    OT_ASSERT(context)

    bool run{true};
    bool needAdmin{false};
    OTPassword serverPassword;

    // Primary loop
    while (run) {
        SHUTDOWN()

        // If the local nym has updated since the last registernym operation,
        // schedule a registernym
        check_nym_revision(*context);

        SHUTDOWN()

        // Ensure transactions numbers are requested if necessary
        check_transaction_numbers(*context);

        SHUTDOWN()

        // Register the nym, if scheduled. Keep trying until success
        run_task<RegisterNymTask>(
            [this](const TaskID task, const RegisterNymTask& param) -> bool {
                const auto output = register_nym(task, param);

                if (false == output) {
                    get_task<RegisterNymTask>().Push(next_task_id(), param);
                }

                return output;
            });

        SHUTDOWN()

        // If this server was added by a pairing operation that included
        // a server password then request admin permissions on the server
        const auto haveAdmin = context->isAdmin();
        needAdmin = context->HaveAdminPassword() && (false == haveAdmin);

        if (needAdmin) {
            serverPassword.setPassword(context->AdminPassword());
            get_admin(next_task_id(), serverPassword);
        }

        SHUTDOWN()

        if (haveAdmin) { check_server_name(*context); }

        SHUTDOWN()

        if (0 == counter() % 100) {
            // download server nym in case it has been renamed
            get_task<CheckNymTask>().Push(
                next_task_id(), context->RemoteNym().ID());
        }

        SHUTDOWN()

        // This is a list of servers for which we do not have a contract.
        // We ask all known servers on which we are registered to try to find
        // the contracts.
        const auto servers = missing_servers_.Copy();

        for (const auto& [targetID, taskID] : servers) {
            SHUTDOWN()

            if (targetID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty serverID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Searching for server contract for ")(targetID)
                    .Flush();
            }

            const auto& notUsed [[maybe_unused]] = taskID;
            find_server(targetID);
        }

        // This is a list of unit definitions for which we do not have a
        // contract. We ask all known servers on which we are registered to try
        // to find the contracts.
        const auto units = missing_unit_definitions_.Copy();

        for (const auto& [targetID, taskID] : units) {
            SHUTDOWN()

            download_contract(taskID, targetID);
        }

        // This is a list of contracts (server and unit definition) which a
        // user of this class has requested we download from this server.
        run_task<DownloadContractTask>(
            get_task<DownloadContractTask>(),
            [this](const TaskID task, const DownloadContractTask& param)
                -> bool { return download_contract(task, param); });

        SHUTDOWN();

        // This is a list of nyms for which we do not have credentials..
        // We ask all known servers on which we are registered to try to find
        // their credentials.
        auto nyms = missing_nyms_.Copy();

        for (const auto& [targetID, taskID] : nyms) {
            SHUTDOWN()

            if (targetID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty nymID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Searching for nym ")(
                    targetID)
                    .Flush();
            }

            [[maybe_unused]] const auto& notUsed = taskID;
            find_nym(targetID, true);
        }

        SHUTDOWN()

        nyms = outdated_nyms_.Copy();

        for (const auto& [targetID, taskID] : nyms) {
            SHUTDOWN()

            if (targetID->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": How did an empty nymID get in here?")
                    .Flush();

                continue;
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(": Updating nym ")(targetID)
                    .Flush();
            }

            [[maybe_unused]] const auto& notUsed = taskID;
            find_nym(targetID, false);
        }

        SHUTDOWN()

        run_task<CheckNymTask>(
            get_nym_fetch(serverID),
            [this](const TaskID task, const CheckNymTask& param) -> bool {
                return download_nym(task, param);
            });

        SHUTDOWN()

        run_task<CheckNymTask>(
            [this](const TaskID task, const CheckNymTask& param) -> bool {
                return download_nym(task, param);
            });

        SHUTDOWN()

        run_task<MessageTask>(
            [this](const TaskID task, const MessageTask& param) -> bool {
                return message_nym(task, param);
            });

        SHUTDOWN()

        run_task<PeerReplyTask>(
            [this](const TaskID task, const PeerReplyTask& param) -> bool {
                return initiate_peer_reply(task, param);
            });

        SHUTDOWN()

        run_task<PeerRequestTask>(
            [this](const TaskID task, const PeerRequestTask& param) -> bool {
                return initiate_peer_request(task, param);
            });

        SHUTDOWN();

        run_task<DownloadNymboxTask>(
            [this](const TaskID task, const DownloadNymboxTask&) -> bool {
                return download_nymbox(task);
            });

        SHUTDOWN()

        run_task<GetTransactionNumbersTask>(
            [this](const TaskID task, const GetTransactionNumbersTask&)
                -> bool { return get_transaction_numbers(task); });

        SHUTDOWN()

        run_task<SendChequeTask>(
            [this](const TaskID task, const SendChequeTask& param) -> bool {
                const auto done = write_and_send_cheque(task, param);

                if (TaskDone::retry == done) {
                    const auto numbersTaskID{next_task_id()};
                    start_task(
                        numbersTaskID,
                        get_task<GetTransactionNumbersTask>().Push(
                            numbersTaskID, {}));

                    get_task<SendChequeTask>().Push(task, param);
                }

                return TaskDone::yes == done;
            });

        SHUTDOWN()

        run_task<PaymentTask>(
            [this](const TaskID task, const PaymentTask& param) -> bool {
                return pay_nym(task, param);
            });

        SHUTDOWN()

#if OT_CASH
        run_task<DownloadMintTask>(
            [this](const TaskID task, const DownloadMintTask& param) -> bool {
                return download_mint(task, param);
            });

        SHUTDOWN()

        run_task<WithdrawCashTask>(
            [this](const TaskID task, const WithdrawCashTask& param) -> bool {
                return withdraw_cash(task, param);
            });

        SHUTDOWN()

        run_task<PayCashTask>(
            [this](const TaskID task, const PayCashTask& param) -> bool {
                return pay_nym_cash(task, param);
            });

        SHUTDOWN()
#endif

        run_task<RegisterAccountTask>(
            [this](
                const TaskID task, const RegisterAccountTask& param) -> bool {
                const auto done = register_account(task, param);

                if (false == done) {
                    get_task<RegisterNymTask>().Push(next_task_id(), false);
                }

                return done;
            });

        SHUTDOWN()

        run_task<IssueUnitDefinitionTask>(
            [this](const TaskID task, const IssueUnitDefinitionTask& param)
                -> bool {
                const auto output = issue_unit_definition(task, param);

                if (false == output) {
                    get_task<RegisterNymTask>().Push(next_task_id(), false);
                }

                return output;
            });

        SHUTDOWN()

        run_task<DepositPaymentTask>(
            [this](const TaskID task, const DepositPaymentTask& param) -> bool {
                bool output{false};
                const auto& [accountIDHint, payment] = param;

                OT_ASSERT(payment);

                auto depositServer = identifier::Server::Factory();
                auto depositAccount = Identifier::Factory();

                const auto status = can_deposit(
                    *payment,
                    op_.NymID(),
                    accountIDHint,
                    depositServer,
                    depositAccount);

                switch (status) {
                    case Depositability::READY: {
                        auto revised{param};
                        revised.first = depositAccount;
                        output = deposit_cheque(task, revised);

                        if (false == output) {
                            get_task<RegisterNymTask>().Push(
                                next_task_id(), false);
                            get_task<DepositPaymentTask>().Push(task, revised);
                        }
                    } break;
                    case Depositability::NOT_REGISTERED:
                    case Depositability::NO_ACCOUNT: {
                        LogDetail(OT_METHOD)(__FUNCTION__)(
                            ": Temporary failure trying to deposit payment")
                            .Flush();
                        get_task<DepositPaymentTask>().Push(task, param);
                    } break;
                    default: {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Permanent failure trying to deposit payment.")
                            .Flush();
                    }
                }

                return output;
            });

        SHUTDOWN()

        run_task<SendTransferTask>(
            [this](const TaskID task, const SendTransferTask& param) -> bool {
                return send_transfer(task, param);
            });

        SHUTDOWN()

        run_task<PublishServerContractTask>(
            [this](const TaskID task, const PublishServerContractTask& param)
                -> bool { return publish_server_contract(task, param); });

        SHUTDOWN()

        run_task<ProcessInboxTask>(
            get_task<ProcessInboxTask>(),
            [this](const TaskID task, const ProcessInboxTask& param) -> bool {
                return process_inbox(task, param);
            });

        const bool missing =
            !(missing_nyms_.empty() && missing_servers_.empty());
        increment_counter(missing, lock, run);

        // Ensure transactions numbers are requested if necessary
        check_transaction_numbers(*context);

        SHUTDOWN()
    }
}

bool StateMachine::process_inbox(
    const TaskID taskID,
    const Identifier& accountID) const
{
    OT_ASSERT(false == accountID.empty())

    DO_OPERATION(UpdateAccount, accountID);

    return finish_task(taskID, success, std::move(result));
}

template <typename T>
const UniqueQueue<T>& StateMachine::Queue() const
{
    return get_task<T>();
}

bool StateMachine::publish_server_contract(
    const TaskID taskID,
    const identifier::Server& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    DO_OPERATION(PublishContract, serverID);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::register_account(
    const TaskID taskID,
    const RegisterAccountTask& task) const
{
    const auto& [label, unitID] = task;

    OT_ASSERT(false == unitID->empty())

    auto contract = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(contract)) {
        DO_OPERATION(DownloadContract, unitID, ContractType::UNIT);

        if (false == success) {
            return finish_task(taskID, success, std::move(result));
        }
    }

    ServerContext::ExtraArgs args{label, false};

    DO_OPERATION(
        Start,
        internal::Operation::Type::RegisterAccount,
        unitID,
        {label, false});

    finish_task(taskID, success, std::move(result));

    if (success) { client_.Pair().Update(); }

    return success;
}

bool StateMachine::register_nym(const TaskID taskID, const bool resync) const
{
    ServerContext::ExtraArgs args{};

    if (resync) { std::get<1>(args) = true; }

    DO_OPERATION(Start, internal::Operation::Type::RegisterNym, args);

    return finish_task(taskID, success, std::move(result));
}

template <typename T>
void StateMachine::run_task(std::function<bool(const TaskID, const T&)> func)
{
    return run_task<T>(get_task<T>(), func);
}

template <typename T>
void StateMachine::run_task(
    const UniqueQueue<T>& queue,
    std::function<bool(const TaskID, const T&)> func)
{
    auto& param = get_param<T>();
    new (&param) T(make_blank<T>::value());

    while (queue.Pop(task_id_, param)) {
        SHUTDOWN()

        func(task_id_, param);
    }

    param.~T();
}

bool StateMachine::send_transfer(
    const TaskID taskID,
    const SendTransferTask& task) const
{
    const auto& [sourceAccountID, targetAccountID, value, memo] = task;

    DO_OPERATION(
        SendTransfer,
        sourceAccountID,
        targetAccountID,
        value,
        String::Factory(memo));

    return finish_task(taskID, success, std::move(result));
}

#if OT_CASH
bool StateMachine::withdraw_cash(
    const TaskID taskID,
    const WithdrawCashTask& task) const
{
    const auto& [accountID, amount] = task;

    DO_OPERATION(WithdrawCash, accountID, amount);

    return finish_task(taskID, success, std::move(result));
}
#endif  // OT_CASH

StateMachine::TaskDone StateMachine::write_and_send_cheque(
    const TaskID taskID,
    const SendChequeTask& task) const
{
    const auto& [accountID, recipient, value, memo, validFrom, validTo] = task;

    OT_ASSERT(false == accountID->empty())
    OT_ASSERT(false == recipient->empty())

    if (0 >= value) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    auto context = client_.Wallet().ServerContext(op_.NymID(), op_.ServerID());

    OT_ASSERT(context);

    const auto available = context->AvailableNumbers();

    if (0 == available) { return TaskDone::retry; }

    std::unique_ptr<Cheque> cheque(client_.OTAPI().WriteCheque(
        op_.ServerID(),
        value,
        Clock::to_time_t(validFrom),
        Clock::to_time_t(validTo),
        accountID,
        op_.NymID(),
        String::Factory(memo.c_str()),
        recipient));

    if (false == bool(cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to write cheque.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    std::shared_ptr<OTPayment> payment{
        client_.Factory().Payment(String::Factory(*cheque))};

    if (false == bool(payment)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate payment.")
            .Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    if (false == payment->SetTempValues()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid payment.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    DO_OPERATION_TASK_DONE(ConveyPayment, recipient, payment);

    return task_done(finish_task(taskID, success, std::move(result)));
}
}  // namespace opentxs::api::client::implementation
