// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "StateMachine.hpp"

namespace opentxs::api::client::implementation
{
template <>
CheckNymTask& StateMachine::get_param()
{
    return param_.check_nym_;
}
template <>
DepositPaymentTask& StateMachine::get_param()
{
    return param_.deposit_payment_;
}
template <>
DownloadContractTask& StateMachine::get_param()
{
    return param_.download_contract_;
}
template <>
DownloadMintTask& StateMachine::get_param()
{
    return param_.download_mint_;
}
template <>
DownloadNymboxTask& StateMachine::get_param()
{
    return param_.download_nymbox_;
}
template <>
GetTransactionNumbersTask& StateMachine::get_param()
{
    return param_.get_transaction_numbers_;
}
template <>
IssueUnitDefinitionTask& StateMachine::get_param()
{
    return param_.issue_unit_definition_;
}
template <>
RegisterAccountTask& StateMachine::get_param()
{
    return param_.register_account_;
}
template <>
RegisterNymTask& StateMachine::get_param()
{
    return param_.register_nym_;
}
template <>
MessageTask& StateMachine::get_param()
{
    return param_.send_message_;
}
template <>
PaymentTask& StateMachine::get_param()
{
    return param_.send_payment_;
}
#if OT_CASH
template <>
PayCashTask& StateMachine::get_param()
{
    return param_.send_cash_;
}
template <>
WithdrawCashTask& StateMachine::get_param()
{
    return param_.withdraw_cash_;
}
#endif
template <>
SendTransferTask& StateMachine::get_param()
{
    return param_.send_transfer_;
}
template <>
PublishServerContractTask& StateMachine::get_param()
{
    return param_.publish_server_contract_;
}
template <>
SendChequeTask& StateMachine::get_param()
{
    return param_.send_cheque_;
}
template <>
PeerReplyTask& StateMachine::get_param()
{
    return param_.peer_reply_;
}
template <>
PeerRequestTask& StateMachine::get_param()
{
    return param_.peer_request_;
}
template <>
const UniqueQueue<CheckNymTask>& StateMachine::get_task() const
{
    return check_nym_;
}
template <>
const UniqueQueue<DepositPaymentTask>& StateMachine::get_task() const
{
    return deposit_payment_;
}
template <>
const UniqueQueue<DownloadContractTask>& StateMachine::get_task() const
{
    return download_contract_;
}
template <>
const UniqueQueue<DownloadMintTask>& StateMachine::get_task() const
{
    return download_mint_;
}
template <>
const UniqueQueue<DownloadNymboxTask>& StateMachine::get_task() const
{
    return download_nymbox_;
}
template <>
const UniqueQueue<GetTransactionNumbersTask>& StateMachine::get_task() const
{
    return get_transaction_numbers_;
}
template <>
const UniqueQueue<IssueUnitDefinitionTask>& StateMachine::get_task() const
{
    return issue_unit_definition_;
}
template <>
const UniqueQueue<RegisterAccountTask>& StateMachine::get_task() const
{
    return register_account_;
}
template <>
const UniqueQueue<RegisterNymTask>& StateMachine::get_task() const
{
    return register_nym_;
}
template <>
const UniqueQueue<MessageTask>& StateMachine::get_task() const
{
    return send_message_;
}
template <>
const UniqueQueue<PaymentTask>& StateMachine::get_task() const
{
    return send_payment_;
}
#if OT_CASH
template <>
const UniqueQueue<PayCashTask>& StateMachine::get_task() const
{
    return send_cash_;
}
template <>
const UniqueQueue<WithdrawCashTask>& StateMachine::get_task() const
{
    return withdraw_cash_;
}
#endif
template <>
const UniqueQueue<SendTransferTask>& StateMachine::get_task() const
{
    return send_transfer_;
}
template <>
const UniqueQueue<PublishServerContractTask>& StateMachine::get_task() const
{
    return publish_server_contract_;
}
template <>
const UniqueQueue<SendChequeTask>& StateMachine::get_task() const
{
    return send_cheque_;
}
template <>
const UniqueQueue<PeerReplyTask>& StateMachine::get_task() const
{
    return peer_reply_;
}
template <>
const UniqueQueue<PeerRequestTask>& StateMachine::get_task() const
{
    return peer_request_;
}

template const UniqueQueue<CheckNymTask>& StateMachine::Queue() const;
template const UniqueQueue<DepositPaymentTask>& StateMachine::Queue() const;
template const UniqueQueue<DownloadContractTask>& StateMachine::Queue() const;
template const UniqueQueue<DownloadMintTask>& StateMachine::Queue() const;
template const UniqueQueue<DownloadNymboxTask>& StateMachine::Queue() const;
template const UniqueQueue<GetTransactionNumbersTask>& StateMachine::Queue()
    const;
template const UniqueQueue<IssueUnitDefinitionTask>& StateMachine::Queue()
    const;
template const UniqueQueue<RegisterAccountTask>& StateMachine::Queue() const;
template const UniqueQueue<RegisterNymTask>& StateMachine::Queue() const;
template const UniqueQueue<MessageTask>& StateMachine::Queue() const;
template const UniqueQueue<PaymentTask>& StateMachine::Queue() const;
#if OT_CASH
template const UniqueQueue<PayCashTask>& StateMachine::Queue() const;
template const UniqueQueue<WithdrawCashTask>& StateMachine::Queue() const;
#endif  // OT_CASH
template const UniqueQueue<SendTransferTask>& StateMachine::Queue() const;
template const UniqueQueue<PublishServerContractTask>& StateMachine::Queue()
    const;
template const UniqueQueue<ProcessInboxTask>& StateMachine::Queue() const;
template const UniqueQueue<SendChequeTask>& StateMachine::Queue() const;
template const UniqueQueue<PeerReplyTask>& StateMachine::Queue() const;
template const UniqueQueue<PeerRequestTask>& StateMachine::Queue() const;
}  // namespace opentxs::api::client::implementation
