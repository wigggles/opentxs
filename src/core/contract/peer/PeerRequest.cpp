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

#include "opentxs/core/contract/peer/PeerRequest.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"

namespace opentxs
{
PeerRequest::PeerRequest(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
      : ot_super(nym)
      , initiator_(serialized.initiator())
      , recipient_(serialized.recipient())
      , cookie_(serialized.cookie())
      , type_(serialized.type())
{
    id_ = Identifier(serialized.id());
    signatures_.push_front(
        SerializedSignature(
            std::make_shared<proto::Signature>(serialized.signature())));
    version_ = serialized.version();
}

PeerRequest::PeerRequest(
    const ConstNym& nym,
    const Identifier& recipient,
    const proto::PeerRequestType& type)
        : ot_super(nym)
        , initiator_(nym->ID())
        , recipient_(String(recipient).Get())
        , type_(type)
{
    version_ = 1;
    auto random = App::Me().Crypto().AES().InstantiateBinarySecretSP();
    random->randomizeMemory(32);
    cookie_.CalculateDigest(
        OTData(random->getMemory(), random->getMemorySize()));
}

proto::PeerRequest PeerRequest::Contract() const
{
    auto contract = SigVersion();
    *(contract.mutable_signature()) = *(signatures_.front());

    return contract;
}

PeerRequest* PeerRequest::Create(
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& unitID,
    const Identifier& serverID)
{
    auto unit = App::Me().Contract().UnitDefinition(unitID);

    if (!unit) {
        otErr << __FUNCTION__ << ": failed to load unit definition."
              << std::endl;

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(
                new BailmentRequest(nym, unit->Nym()->ID(), unitID, serverID));
            break;
        }
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate request."
              << std::endl;

        return nullptr;
    }

    if (FinalizeContract(*contract)) {
        return contract.release();
    } else {
        otErr << __FUNCTION__ << ": failed to finalize contract." << std::endl;

        return nullptr;
    }
}

PeerRequest* PeerRequest::Create(
    const ConstNym& nym,
    const proto::PeerRequestType& type,
    const Identifier& unitID,
    const Identifier& serverID,
    const std::string& terms)
{
    auto unit = App::Me().Contract().UnitDefinition(unitID);

    if (!unit) {
        otErr << __FUNCTION__ << ": failed to load unit definition."
              << std::endl;

        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (type) {
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(
                new OutBailmentRequest(
                    nym, unit->Nym()->ID(), unitID, serverID, terms));
            break;
        }
        default: {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate request."
              << std::endl;

        return nullptr;
    }

    if (FinalizeContract(*contract)) {
        return contract.release();
    } else {
        otErr << __FUNCTION__ << ": failed to finalize contract." << std::endl;

        return nullptr;
    }
}

PeerRequest* PeerRequest::Factory(
    const ConstNym& nym,
    const proto::PeerRequest& serialized)
{
    if (!proto::Check(serialized, 0, 0xFFFFFFFF)) {
        otErr << __FUNCTION__ << ": invalid protobuf." << std::endl;
        return nullptr;
    }

    std::unique_ptr<PeerRequest> contract;

    switch (serialized.type()) {
        case (proto::PEERREQUEST_BAILMENT) : {
            contract.reset(new BailmentRequest(nym, serialized));
            break;
        }
        case (proto::PEERREQUEST_OUTBAILMENT) : {
            contract.reset(new OutBailmentRequest(nym, serialized));
            break;
        }
        default : {
            otErr << __FUNCTION__ << ": invalid request type." << std::endl;

            return nullptr;
        }
    }

    if (!contract) {
        otErr << __FUNCTION__ << ": failed to instantiate request."
              << std::endl;

        return nullptr;
    }
    if (!contract->Validate()) {
        otErr << __FUNCTION__ << ": invalid request." << std::endl;

        return nullptr;
    }

    return contract.release();
}

bool PeerRequest::FinalizeContract(PeerRequest& contract)
{
    if (!contract.CalculateID()) {
        otErr << __FUNCTION__ << ": failed to calculate ID." << std::endl;

        return false;
    }

    if (contract.nym_) {
        auto serialized = contract.SigVersion();
        std::shared_ptr<proto::Signature> sig =
            std::make_shared<proto::Signature>();

        if (contract.nym_->Sign(serialized, *sig)) {
            contract.signatures_.push_front(sig);
        }
    }

    return contract.Validate();
}

Identifier PeerRequest::GetID() const
{
    return GetID(IDVersion());
}

Identifier PeerRequest::GetID(const proto::PeerRequest& contract)
{
    Identifier id;
    id.CalculateDigest(proto::ProtoAsData(contract));
    return id;
}

proto::PeerRequest PeerRequest::IDVersion() const
{
    proto::PeerRequest contract;

    contract.set_version(version_);
    contract.clear_id();         // reinforcing that this field must be blank.
    contract.set_initiator(String(initiator_).Get());
    contract.set_recipient(String(recipient_).Get());
    contract.set_type(type_);
    contract.set_cookie(String(cookie_).Get());
    contract.clear_signature();  // reinforcing that this field must be blank.

    return contract;
}

std::string PeerRequest::Name() const
{
    return String(id_).Get();
}

OTData PeerRequest::Serialize() const
{

    return proto::ProtoAsData(Contract());
}

proto::PeerRequest PeerRequest::SigVersion() const
{
    auto contract = IDVersion();
    contract.set_id(String(ID()).Get());

    return contract;
}

bool PeerRequest::Validate() const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    } else {
        otErr << __FUNCTION__ << ": invalid nym." << std::endl;
    }

    bool validSyntax = proto::Check(Contract(), 0, 0xFFFFFFFF);

    if (!validSyntax) {
        otErr << __FUNCTION__ << ": invalid syntax." << std::endl;
    }

    bool validSig = false;

    if (nym_) {
        validSig = nym_->Verify(
            proto::ProtoAsData(SigVersion()),
            *(signatures_.front()));
    }

    if (!validSig) {
        otErr << __FUNCTION__ << ": invalid signature." << std::endl;
    }

    return (validNym && validSyntax && validSig);
}
} // namespace opentxs
