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

#include "opentxs/core/crypto/NymParameters.hpp"

#include "opentxs/core/crypto/OTAsymmetricKey.hpp"

#include <stdint.h>
#include <memory>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{

NymParameters::NymParameters(
    NymParameters::NymParameterType theKeytype,
    proto::CredentialType theCredentialtype)
{
        setNymParameterType(theKeytype);
        setCredentialType(theCredentialtype);
}

NymParameters::NymParameterType NymParameters::nymParameterType() {
    return nymType_;
}

proto::AsymmetricKeyType NymParameters::AsymmetricKeyType() const
{
    proto::AsymmetricKeyType newKeyType;

    switch (nymType_) {
        case NymParameters::LEGACY :
            newKeyType = proto::AKEYTYPE_LEGACY;
            break;
        case NymParameters::SECP256K1 :
            newKeyType = proto::AKEYTYPE_SECP256K1;
            break;
        case NymParameters::ED25519 :
            newKeyType = proto::AKEYTYPE_ED25519;
            break;
        default :
            newKeyType = proto::AKEYTYPE_ERROR;
    }
    return newKeyType;
}

void NymParameters::setNymParameterType(NymParameters::NymParameterType theKeytype) {
    nymType_ = theKeytype;
}

proto::CredentialType NymParameters::credentialType() const {
    return credentialType_;
}

void NymParameters::setCredentialType(
    proto::CredentialType theCredentialtype)
{
    credentialType_ = theCredentialtype;

    switch (theCredentialtype) {
        case (proto::CREDTYPE_LEGACY) :
            SetSourceType(proto::SOURCETYPE_PUBKEY);
            SetSourceProofType(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

            break;
        case (proto::CREDTYPE_HD) :
            SetSourceType(proto::SOURCETYPE_BIP47);
            SetSourceProofType(proto::SOURCEPROOFTYPE_SIGNATURE);

            break;
        default :

            break;
    }
}

void NymParameters::SetContactData(const proto::ContactData& contactData)
{
    contact_data_.reset(new proto::ContactData(contactData));
}

void NymParameters::SetVerificationSet(
    const proto::VerificationSet& verificationSet)
{
    verification_set_.reset(new proto::VerificationSet(verificationSet));
}

#if defined(OT_CRYPTO_SUPPORTED_KEY_RSA)
NymParameters::NymParameters(const int32_t keySize)
    : nymType_(NymParameters::LEGACY),
    credentialType_(proto::CREDTYPE_LEGACY),
    nBits_(keySize)
{
}

int32_t NymParameters::keySize() {
    return nBits_;
}

void NymParameters::setKeySize(int32_t keySize) {
    nBits_ = keySize;
}
#endif

} // namespace opentxs
