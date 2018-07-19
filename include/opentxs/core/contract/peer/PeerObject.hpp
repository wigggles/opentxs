// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
class PeerObject
{
public:
    static std::unique_ptr<PeerObject> Create(
        const ConstNym& senderNym,
        const std::string& message);
    static std::unique_ptr<PeerObject> Create(
        const ConstNym& senderNym,
        const std::string& payment,
        const bool isPayment);
    static std::unique_ptr<PeerObject> Create(
        const std::shared_ptr<PeerRequest>& request,
        const std::shared_ptr<PeerReply>& reply,
        const std::uint32_t& version);
    static std::unique_ptr<PeerObject> Create(
        const std::shared_ptr<PeerRequest>& request,
        const std::uint32_t& version);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& signerNym,
        const proto::PeerObject& serialized);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& recipientNym,
        const OTASCIIArmor& encrypted);

    const ConstNym& Nym() const { return nym_; }
    const std::shared_ptr<PeerRequest>& Request() const { return request_; }
    const std::shared_ptr<PeerReply>& Reply() const { return reply_; }
    proto::PeerObject Serialize() const;
    proto::PeerObjectType Type() const { return type_; }
    bool Validate() const;

    std::unique_ptr<std::string>& Message() { return message_; }
    const std::unique_ptr<std::string>& Message() const { return message_; }
    std::unique_ptr<std::string>& Payment() { return payment_; }
    const std::unique_ptr<std::string>& Payment() const { return payment_; }

    ~PeerObject() = default;

private:
    ConstNym nym_{nullptr};
    std::unique_ptr<std::string> message_{nullptr};
    std::unique_ptr<std::string> payment_{nullptr};
    std::shared_ptr<PeerReply> reply_{nullptr};
    std::shared_ptr<PeerRequest> request_{nullptr};
    proto::PeerObjectType type_{proto::PEEROBJECT_ERROR};
    std::uint32_t version_{0};

    PeerObject(const ConstNym& signerNym, const proto::PeerObject serialized);
    PeerObject(const ConstNym& senderNym, const std::string& message);
    PeerObject(const std::string& payment, const ConstNym& senderNym);
    PeerObject(
        const std::shared_ptr<PeerRequest>& request,
        const std::shared_ptr<PeerReply>& reply,
        const std::uint32_t& version);
    PeerObject(
        const std::shared_ptr<PeerRequest>& request,
        const std::uint32_t& version);
    PeerObject() = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
