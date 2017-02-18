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

#include "opentxs/core/contract/peer/PeerObject.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
PeerObject::PeerObject(const ConstNym& nym, const proto::PeerObject serialized)
    : type_(serialized.type())
    , version_(serialized.version())
{
    switch (serialized.type()) {
        case (proto::PEEROBJECT_MESSAGE) : {
            message_.reset(new std::string(serialized.otmessage()));

            break;
        }
        case (proto::PEEROBJECT_REQUEST) : {
            if (nym) {
                request_ = PeerRequest::Factory(nym, serialized.otrequest());
            } else {
                auto providedNym = OT::App().Contract().Nym(serialized.nym());
                request_ =
                    PeerRequest::Factory(providedNym, serialized.otrequest());
            }

            break;
        }
        case (proto::PEEROBJECT_RESPONSE) : {
            auto senderNym = OT::App().Contract().Nym(
                Identifier(serialized.otrequest().initiator()));
            request_ = PeerRequest::Factory(senderNym, serialized.otrequest());
            reply_ = PeerReply::Factory(nym, serialized.otreply());

            break;
        }
        default : {
            otErr << __FUNCTION__ << ": Unknown type" << std::endl;
        }
    }
}

PeerObject::PeerObject(const std::string& message)
    : type_(proto::PEEROBJECT_MESSAGE)
    , version_(2)
{
    message_.reset(new std::string(message));
}

PeerObject::PeerObject(
    std::unique_ptr<PeerRequest>& request,
    std::unique_ptr<PeerReply>& reply)
        : type_(proto::PEEROBJECT_RESPONSE)
        , version_(2)
{
    request_.swap(request);
    reply_.swap(reply);
}

PeerObject::PeerObject(std::unique_ptr<PeerRequest>& request)
    : type_(proto::PEEROBJECT_REQUEST)
    , version_(2)
{
    request_.swap(request);
}

std::unique_ptr<PeerObject> PeerObject::Create(const std::string& message)
{
    std::unique_ptr<PeerObject> output(new PeerObject(message));

    if (!output->Validate()) {
        output.reset();
    }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Create(
    std::unique_ptr<PeerRequest>& request,
    std::unique_ptr<PeerReply>& reply)
{
    std::unique_ptr<PeerObject> output(new PeerObject(request, reply));

    if (!output->Validate()) {
        output.reset();
    }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Create(
    std::unique_ptr<PeerRequest>& request)
{
    std::unique_ptr<PeerObject> output(new PeerObject(request));

    if (!output->Validate()) {
        output.reset();
    }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Factory(
    const ConstNym& nym,
    const proto::PeerObject& serialized)
{
    const bool valid = proto::Check(
        serialized, serialized.version(), serialized.version());
    std::unique_ptr<PeerObject> output;

    if (valid) {
        output.reset(new PeerObject(nym, serialized));
    }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Factory(
    const ConstNym& recipientNym,
    const ConstNym& senderNym,
    const OTASCIIArmor& encrypted)
{
    std::unique_ptr<PeerObject> output;
    OTEnvelope input;

    if (!input.SetCiphertext(encrypted)) { return output; }

    String contents;

    if (!input.Open(*recipientNym, contents)) { return output; }

    auto serialized = proto::StringToProto<proto::PeerObject>(contents);

    output = Factory(senderNym, serialized);

    return output;
}

proto::PeerObject PeerObject::Serialize() const
{
    proto::PeerObject output;

    if (2 > version_) {
        output.set_version(2);
    } else {
        output.set_version(version_);
    }

    output.set_type(type_);

    switch (type_) {
        case (proto::PEEROBJECT_MESSAGE) : {
            if (message_) {
                output.set_otmessage(String(*message_).Get());
            }
            break;
        }
        case (proto::PEEROBJECT_REQUEST) : {
            if (request_) {
                *(output.mutable_otrequest()) = request_->Contract();
                auto nym = OT::App().Contract().Nym(request_->Initiator());

                if (nym) {
                    *output.mutable_nym() = nym->asPublicNym();
                }
            }
            break;
        }
        case (proto::PEEROBJECT_RESPONSE) : {
            if (reply_) {
                *(output.mutable_otreply()) = reply_->Contract();
            }
            if (request_) {
                *(output.mutable_otrequest()) = request_->Contract();
            }
            break;
        }
        default : {
            otErr << __FUNCTION__ << ": Unknown type" << std::endl;
        }
    }

    return output;
}

bool PeerObject::Validate() const
{
    bool validChildren = false;

    switch (type_) {
        case (proto::PEEROBJECT_MESSAGE) : {
            validChildren = bool(message_);
            break;
        }
        case (proto::PEEROBJECT_REQUEST) : {
            if (request_) {
                validChildren = request_->Validate();
            }
            break;
        }
        case (proto::PEEROBJECT_RESPONSE) : {
            if (!reply_ || !request_) { break; }

            validChildren = reply_->Validate() && request_->Validate();
            break;
        }
        default : {}
    }

    const bool validProto = proto::Check(Serialize(), version_, version_);

    return (validChildren && validProto);
}
} // namespace opentxs
