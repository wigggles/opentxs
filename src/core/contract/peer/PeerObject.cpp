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

#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"

namespace opentxs
{
PeerObject::PeerObject(
    const proto::PeerObject serialized)
      : type_(serialized.type())
      , version_(serialized.version())
{
    switch (serialized.type()) {
        case (proto::PEEROBJECT_MESSAGE) : {
            message_.reset(new std::string(serialized.otmessage()));

            OT_ASSERT(message_);

        }
        default : {
            otErr << __FUNCTION__ << ": Unknown type" << std::endl;
        }
    }
}

PeerObject::PeerObject(const std::string& message)
    : type_(proto::PEEROBJECT_MESSAGE)
    , version_(1)
{
    message_.reset(new std::string(message));

    OT_ASSERT(message_);
}

std::unique_ptr<PeerObject> PeerObject::Create(const std::string& message)
{
    std::unique_ptr<PeerObject> output(new PeerObject(message));

    OT_ASSERT(output);

    if (!output->Validate()) {
        output.reset();
    }

    return output;
}

std::unique_ptr<PeerObject> PeerObject::Factory(
    const proto::PeerObject& serialized)
{
    const bool valid = proto::Check(
        serialized, serialized.version(), serialized.version());
    std::unique_ptr<PeerObject> output;

    if (valid) {
        output.reset(new PeerObject(serialized));
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

    if (!input.SetAsciiArmoredData(encrypted)) { return output; }

    String contents;

    if (!input.Open(*recipientNym, contents)) { return output; }

    auto serialized = proto::StringToProto<proto::PeerObject>(contents);

    output = Factory(senderNym, serialized);

    return output;
}

proto::PeerObject PeerObject::Serialize() const
{
    proto::PeerObject output;
    output.set_version(version_);
    output.set_type(type_);

    switch (type_) {
        case (proto::PEEROBJECT_MESSAGE) : {
            output.set_otmessage(String(*message_).Get());
        }
        default : {
            otErr << __FUNCTION__ << ": Unknown type" << std::endl;
        }
    }

    return output;
}

bool PeerObject::Validate() const
{
    const auto serialized = Serialize();

    return proto::Check(serialized, version_, version_);
}
} // namespace opentxs
