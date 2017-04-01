/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CONSENSUS_CONTEXT_HPP
#define OPENTXS_CONSENSUS_CONTEXT_HPP

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <set>

namespace opentxs
{

class Nym;
class Wallet;

class Context : public Signable
{
private:
    friend class Wallet;

    typedef Signable ot_super;

    std::shared_ptr<const class Nym> remote_nym_;
    Identifier local_nymbox_hash_;
    Identifier remote_nymbox_hash_;
    std::atomic<RequestNumber> request_number_;
    std::set<RequestNumber> acknowledged_request_numbers_;
    std::set<TransactionNumber> available_transaction_numbers_;
    std::set<TransactionNumber> issued_transaction_numbers_;

    proto::Context contract(const Lock& lock) const;
    proto::Context IDVersion(const Lock& lock) const;
    proto::Context SigVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const override;

    bool update_signature(const Lock& lock) override;

    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

protected:
    typedef std::unique_lock<std::mutex> Lock;

    Identifier GetID(const Lock& lock) const override;

    virtual proto::Context serialize(
        const Lock& lock,
        const proto::ConsensusType type) const;
    virtual proto::Context serialize(const Lock& lock) const = 0;

    void finish_acknowledgements(
        const Lock& lock,
        const std::set<RequestNumber>& req);

    Context(
        const ConstNym& local,
        const ConstNym& remote);
    Context(
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote);

public:
    std::set<RequestNumber> AcknowledgedNumbers() const;
    bool HaveLocalNymboxHash() const;
    bool HaveRemoteNymboxHash() const;
    bool NymboxHashMatch() const;
    Identifier LocalNymboxHash() const;
    Identifier RemoteNymboxHash() const;
    RequestNumber Request() const;
    proto::Context Serialized() const;
    bool VerifyAcknowledgedNumber(const RequestNumber req) const;

    std::string Name() const override;
    OTData Serialize() const override;

    bool AddAcknowledgedNumber(const RequestNumber req);
    virtual bool CloseCronItem(const TransactionNumber) { return false; }
    RequestNumber IncrementRequest();
    virtual bool OpenCronItem(const TransactionNumber) { return false; }
    bool RemoveAcknowledgedNumber(const std::set<RequestNumber>& req);
    void SetLocalNymboxHash(const Identifier& hash);
    void SetRemoteNymboxHash(const Identifier& hash);
    void SetRequest(const RequestNumber req);

    virtual proto::ConsensusType Type() const = 0;

    virtual ~Context() = default;
};
} // namespace opentxs

#endif // OPENTXS_CONSENSUS_CONTEXT_HPP
