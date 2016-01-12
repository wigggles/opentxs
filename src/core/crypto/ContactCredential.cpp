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

#include <opentxs/core/crypto/ContactCredential.hpp>

#include <opentxs/core/Proto.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/util/OTFolders.hpp>

namespace opentxs
{

ContactCredential::ContactCredential(
    CredentialSet& parent,
    const proto::Credential& credential)
        : ot_super(parent, credential)
{
    m_strContractType = "CONTACT CREDENTIAL";
    master_id_ = credential.childdata().masterid();
    data_.reset(new proto::ContactData(credential.contactdata()));
}

ContactCredential::ContactCredential(
    CredentialSet& parent,
    const NymParameters& nymParameters)
        : ot_super(parent, nymParameters)
{
    role_ = proto::CREDROLE_CONTACT;
    nym_id_ = parent.GetNymID();
    master_id_ = parent.GetMasterCredID();
    data_.reset(new proto::ContactData(nymParameters.ContactData()));

    Identifier childID;
    CalculateAndSetContractID(childID);

    AddMasterSignature();

    SaveContract();
}

bool ContactCredential::GetContactData(proto::ContactData& contactData) const
{
    if (!data_) {
        return false;
    }

    contactData = *data_;

    return true;
}

serializedCredential ContactCredential::asSerialized(
    SerializationModeFlag asPrivate,
    SerializationSignatureFlag asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::asSerialized(asPrivate, asSigned);

    serializedCredential->clear_signature(); //this fixes a bug, but shouldn't
    if (asSigned) {
        serializedSignature masterSignature = MasterSignature();

        if (masterSignature) {
            // We do not own this pointer.
            proto::Signature* serializedMasterSignature = serializedCredential->add_signature();
            *serializedMasterSignature = *masterSignature;
        } else {
            otErr << __FUNCTION__ << ": Failed to get master signature.\n";
        }
    }

    *(serializedCredential->mutable_contactdata()) = *data_;

    return serializedCredential;
}

} // namespace opentxs
