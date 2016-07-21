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

#include "opentxs/core/NymIDSource.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/util/Assert.hpp"

#include <memory>
#include <ostream>

namespace opentxs
{

NymIDSource::NymIDSource(const proto::NymIDSource& serializedSource)
    : version_(serializedSource.version())
    , type_(serializedSource.type())
{
    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            pubkey_.reset(OTAsymmetricKey::KeyFactory(serializedSource.key()));
            break;
        case proto::SOURCETYPE_BIP47:
            payment_code_.reset(
                new PaymentCode(serializedSource.paymentcode()));
            break;
        default:
            break;
    }
}

NymIDSource::NymIDSource(const String& stringSource)
    : NymIDSource(*ExtractArmoredSource(stringSource))
{
}

NymIDSource::NymIDSource(
    const NymParameters& nymParameters,
    proto::AsymmetricKey& pubkey)
    : version_(1)
    , type_(nymParameters.SourceType())
{
    pubkey_.reset(OTAsymmetricKey::KeyFactory(pubkey));
}

NymIDSource::NymIDSource(std::unique_ptr<PaymentCode>& source)
    : version_(1)
    , type_(proto::SOURCETYPE_BIP47)
{
    payment_code_.reset(source.release());
}

OTData NymIDSource::asData() const
{
    serializedNymIDSource serializedSource = Serialize();

    return proto::ProtoAsData<proto::NymIDSource>(*serializedSource);
}

std::unique_ptr<proto::AsymmetricKey> NymIDSource::ExtractKey(
    const proto::Credential& credential,
    const proto::KeyRole role)
{
    std::unique_ptr<proto::AsymmetricKey> output;

    const bool master = (proto::CREDROLE_MASTERKEY == credential.role());
    const bool child = (proto::CREDROLE_CHILDKEY == credential.role());
    const bool keyCredential = master || child;

    if (!keyCredential) { return output; }

    const auto& publicCred = credential.publiccredential();

    for (auto& key : publicCred.key()) {
        if (role == key.role()) {
            output.reset(new proto::AsymmetricKey(key));
        }
    }

    return output;
}

Identifier NymIDSource::NymID() const
{
    Identifier nymID;
    OTData dataVersion;

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            dataVersion = asData();
            nymID.CalculateDigest(dataVersion);

            break;
        case proto::SOURCETYPE_BIP47:
            if (payment_code_) {
                nymID = payment_code_->ID();
            }

            break;
        default:
            break;
    }
    return nymID;
}

serializedNymIDSource NymIDSource::Serialize() const
{
    serializedNymIDSource source = std::make_shared<proto::NymIDSource>();
    source->set_version(version_);
    source->set_type(type_);

    serializedAsymmetricKey key;
    SerializedPaymentCode paycode;

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            key = pubkey_->Serialize();
            key->set_role(proto::KEYROLE_SIGN);
            *(source->mutable_key()) = *key;

            break;
        case proto::SOURCETYPE_BIP47:
            paycode = payment_code_->Serialize();
            *(source->mutable_paymentcode()) = *paycode;

            break;
        default:
            break;
    }

    return source;
}

// This function assumes that all internal verification checks are complete
// except for the source proof
bool NymIDSource::Verify(const MasterCredential& credential) const
{
    serializedCredential serializedMaster;
    bool isSelfSigned, sameSource;
    std::unique_ptr<proto::AsymmetricKey> signingKey;
    serializedAsymmetricKey sourceKey;

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            if (!pubkey_) { return false; }

            serializedMaster = credential.asSerialized(
                Credential::AS_PUBLIC, Credential::WITH_SIGNATURES);

            isSelfSigned =
                (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
                 serializedMaster->masterdata().sourceproof().type());

            if (!isSelfSigned) {
                OT_ASSERT_MSG(false, "Not yet implemented");

                return false;
            }

            signingKey = ExtractKey(*serializedMaster, proto::KEYROLE_SIGN);

            if (!signingKey) {
                otErr << __FUNCTION__ << ": Failed to extract signing key"
                      << std::endl;

                return false;
            }

            sourceKey = pubkey_->Serialize();
            sameSource = (sourceKey->key() == signingKey->key());

            if (!sameSource) {
                otErr << __FUNCTION__ << ": Master credential was not"
                      << " derived from this source." << std::endl;

                return false;
            }

            break;
        case proto::SOURCETYPE_BIP47:
            if (payment_code_) {
                if (!payment_code_->Verify(credential)) {
                    otErr << __FUNCTION__ << ": Invalid source signature."
                          << std::endl;

                    return false;
                }
            }

            break;
        default:
            break;
    }

    return true;
}

bool NymIDSource::Sign(
    const NymParameters& nymParameters,
    const MasterCredential& credential,
    proto::Signature& sig,
    const OTPasswordData* pPWData) const
{
    bool goodsig = false;

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY):
            OT_ASSERT_MSG(false, "This is not implemented yet.");

            break;
        case (proto::SOURCETYPE_BIP47):
            if (payment_code_) {
                goodsig = payment_code_->Sign(
                    nymParameters.Nym(), credential, sig, pPWData);
            }

            break;
        default:
            break;
    }

    return goodsig;
}

String NymIDSource::asString() const
{
    OTData dataSource = asData();
    OTASCIIArmor armoredSource(dataSource);

    return armoredSource.Get();
}

// static
serializedNymIDSource NymIDSource::ExtractArmoredSource(
    const OTASCIIArmor& armoredSource)
{
    OTData dataSource(armoredSource);

    OT_ASSERT(dataSource.GetSize() > 0);

    serializedNymIDSource protoSource = std::make_shared<proto::NymIDSource>();
    protoSource->ParseFromArray(dataSource.GetPointer(), dataSource.GetSize());

    return protoSource;
}

String NymIDSource::Description() const
{
    String description;
    Identifier keyID;

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY):
            if (pubkey_) {
                pubkey_->CalculateID(keyID);
                description = String(keyID);
            }

            break;
        case (proto::SOURCETYPE_BIP47):
            if (payment_code_) {
                description = String(payment_code_->asBase58());
            }

            break;
        default:
            break;
    }

    return description;
}

}  // namespace opentxs
