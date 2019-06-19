// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/Types.hpp"

#include "core/StateMachine.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/otx/client/Client.hpp"

namespace opentxs::otx::client::implementation
{
class PaymentTasks;

class DepositPayment final : public opentxs::internal::StateMachine
{
public:
    using TaskID = api::client::OTX::TaskID;

    DepositPayment(
        client::internal::StateMachine& parent,
        const TaskID taskID,
        const DepositPaymentTask& payment,
        PaymentTasks& paymenttasks);
    ~DepositPayment();

private:
    client::internal::StateMachine& parent_;
    const TaskID task_id_;
    DepositPaymentTask payment_;
    Depositability state_;
    api::client::OTX::Result result_;
    PaymentTasks& payment_tasks_;

    bool deposit();
    OTIdentifier get_account_id(
        const PasswordPrompt& reason,
        const identifier::UnitDefinition& unit);

    DepositPayment() = delete;
};
}  // namespace opentxs::otx::client::implementation
