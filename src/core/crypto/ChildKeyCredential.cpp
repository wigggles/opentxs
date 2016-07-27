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

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "opentxs/core/crypto/ChildKeyCredential.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"

#include <memory>
#include <ostream>

namespace opentxs
{

ChildKeyCredential::ChildKeyCredential(
    CredentialSet& other,
    const proto::Credential& serializedCred)
    : ot_super(other, serializedCred)
{
    role_ = proto::CREDROLE_CHILDKEY;

    master_id_ = String(serializedCred.childdata().masterid());
}

ChildKeyCredential::ChildKeyCredential(
    CredentialSet& other,
    const NymParameters& nymParameters)
    : ot_super(other, nymParameters)
{
    role_ = proto::CREDROLE_CHILDKEY;

    nym_id_ = other.GetNymID();
    master_id_ = other.GetMasterCredID();
}

ChildKeyCredential::~ChildKeyCredential() {}

serializedCredential ChildKeyCredential::asSerialized(
    SerializationModeFlag asPrivate,
    SerializationSignatureFlag asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::asSerialized(asPrivate, asSigned);

    if (asSigned) {
        SerializedSignature masterSignature = MasterSignature();

        if (masterSignature) {
            // We do not own this pointer.
            proto::Signature* serializedMasterSignature =
                serializedCredential->add_signature();
            *serializedMasterSignature = *masterSignature;
        } else {
            otErr << __FUNCTION__ << ": Failed to get master signature.\n";
        }
    }

    serializedCredential->set_role(proto::CREDROLE_CHILDKEY);

    return serializedCredential;
}

}  // namespace opentxs
