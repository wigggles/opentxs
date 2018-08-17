// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_WORKFLOW_HPP
#define OPENTXS_API_CLIENT_WORKFLOW_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <memory>
#include <vector>

namespace opentxs
{
namespace api
{
namespace client
{
/** Store and retrieve payment workflow events
 *
 *  Sequence for sending a cheque:
 *    1. WriteCheque
 *    2. SendCheque or ExportCheque
 *       a. SendCheque may be repeated as necessary until successful
 *    3. (optional) ExpireCheque
 *       a. May be performed after step 1 or after step 2
 *       b. It's possible that a cheque that's been expired locally was already
 *          accepted by the notary
 *    4. (optional) CancelCheque
 *       a. May be performed after step 1 or after step 2
 *    5. ClearCheque
 *       a. Called after receiving a cheque deposit receipt
 *    6. FinishCheque
 *       a. Called after a process inbox attempt
 *       b. May be repeated as necessary until successful
 *
 *  Sequence for receiving a cheque:
 *    1. ReceiveCheque or ImportCheque
 *    2. (optional) ExpireCheque
 *    3. DepositCheque
 *       a. May be repeated as necessary until successful
 */
class Workflow
{
public:
    using Cheque = std::
        pair<proto::PaymentWorkflowState, std::unique_ptr<opentxs::Cheque>>;

    static bool ContainsCheque(const proto::PaymentWorkflow& workflow);
    static std::string ExtractCheque(const proto::PaymentWorkflow& workflow);
    static Cheque InstantiateCheque(
        const api::Core& core,
        const proto::PaymentWorkflow& workflow);

    /** Record a cheque cancellation or cancellation attempt */
    EXPORT virtual bool CancelCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const = 0;
    /** Record a cheque deposit receipt */
    EXPORT virtual bool ClearCheque(
        const Identifier& recipientNymID,
        const OTTransaction& receipt) const = 0;
    /** Record a cheque deposit or deposit attempt */
    EXPORT virtual bool DepositCheque(
        const Identifier& nymID,
        const Identifier& accountID,
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const = 0;
    /** Mark a cheque workflow as expired */
    EXPORT virtual bool ExpireCheque(
        const Identifier& nymID,
        const opentxs::Cheque& cheque) const = 0;
    /** Record an out of band cheque conveyance */
    EXPORT virtual bool ExportCheque(const opentxs::Cheque& cheque) const = 0;
    /** Record a process inbox that accepts a cheque deposit receipt */
    EXPORT virtual bool FinishCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const = 0;
    /** Create a new incoming cheque workflow from an out of band cheque */
    EXPORT virtual OTIdentifier ImportCheque(
        const Identifier& nymID,
        const opentxs::Cheque& cheque) const = 0;
    EXPORT virtual std::set<OTIdentifier> List(
        const Identifier& nymID,
        const proto::PaymentWorkflowType type,
        const proto::PaymentWorkflowState state) const = 0;
    EXPORT virtual Cheque LoadCheque(
        const Identifier& nymID,
        const Identifier& chequeID) const = 0;
    EXPORT virtual Cheque LoadChequeByWorkflow(
        const Identifier& nymID,
        const Identifier& workflowID) const = 0;
    /** Load a serialized workflow, if it exists*/
    EXPORT virtual std::shared_ptr<proto::PaymentWorkflow> LoadWorkflow(
        const Identifier& nymID,
        const Identifier& workflowID) const = 0;
    /** Create a new incoming cheque workflow from an OT message */
    EXPORT virtual OTIdentifier ReceiveCheque(
        const Identifier& nymID,
        const opentxs::Cheque& cheque,
        const Message& message) const = 0;
    /** Record a send or send attempt via an OT notary */
    EXPORT virtual bool SendCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const = 0;
    /** Get a list of workflow IDs relevant to a specified account */
    EXPORT virtual std::vector<OTIdentifier> WorkflowsByAccount(
        const Identifier& nymID,
        const Identifier& accountID) const = 0;
    /** Create a new outgoing cheque workflow */
    EXPORT virtual OTIdentifier WriteCheque(
        const opentxs::Cheque& cheque) const = 0;

    EXPORT virtual ~Workflow() = default;

protected:
    Workflow() = default;

private:
    Workflow(const Workflow&) = delete;
    Workflow(Workflow&&) = delete;
    Workflow& operator=(const Workflow&) = delete;
    Workflow& operator=(Workflow&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
