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

#include <opentxs/core/NymIDSource.hpp>

#include  <opentxs/core/Log.hpp>
#include  <opentxs/core/OTData.hpp>
#include  <opentxs/core/crypto/MasterCredential.hpp>
#include  <opentxs/core/crypto/NymParameters.hpp>
#include  <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include  <opentxs/core/crypto/OTASCIIArmor.hpp>
#include  <opentxs/core/Proto.hpp>

namespace opentxs
{

NymIDSource::NymIDSource(const proto::NymIDSource& serializedSource)
    : version_(serializedSource.version())
    , type_(serializedSource.type())
{
    pubkey_.reset(OTAsymmetricKey::KeyFactory(serializedSource.key()));
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

OTData NymIDSource::asData() const
{
    serializedNymIDSource serializedSource = Serialize();

    return proto::ProtoAsData<proto::NymIDSource>(*serializedSource);
}

Identifier NymIDSource::NymID() const
{
    OTData dataVersion = asData();

    Identifier nymID;

    nymID.CalculateDigest(dataVersion);

    return nymID;
}

serializedNymIDSource NymIDSource::Serialize() const
{
    serializedNymIDSource source = std::make_shared<proto::NymIDSource>();
    source->set_version(version_);
    source->set_type(type_);

    serializedAsymmetricKey key = pubkey_->Serialize();
    key->set_role(proto::KEYROLE_SIGN);
    *(source->mutable_key()) = *key;

    return source;
}

bool NymIDSource::Verify(
    const MasterCredential& credential) const
{
    serializedCredential serializedMaster =
        credential.Serialize(
            Credential::AS_PUBLIC,
            Credential::WITH_SIGNATURES);

    if (!proto::Verify(*serializedMaster, proto::CREDROLE_MASTERKEY, true)) {
        otErr << __FUNCTION__ << ": Invalid master credential syntax.\n";
        return false;
    }

    bool isSelfSigned =
        (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
        serializedMaster->publiccredential().masterdata().sourceproof().type());

    if (isSelfSigned) {
        if (!credential.VerifySignedBySelf()) {
            otErr << __FUNCTION__ << ": Invalid self-signature.\n";
            return false;
        }
    } else {
        //FIXME implement this
        return false;
    }

    bool sameSource = (*(this->pubkey_) ==
            serializedMaster->publiccredential().key(proto::KEYROLE_SIGN - 1));

    if (!sameSource) {
        otErr << __FUNCTION__ << ": Master credential was not derived from this source->\n";
        return false;
    }

    return true;
}

String NymIDSource::asString() const
{
    OTData dataSource = asData();
    OTASCIIArmor armoredSource(dataSource);

    return armoredSource.Get();
}

//static
serializedNymIDSource NymIDSource::ExtractArmoredSource(
    const OTASCIIArmor& armoredSource)
{
    OTData dataSource(armoredSource);

    OT_ASSERT(dataSource.GetSize()>0);

    serializedNymIDSource protoSource = std::make_shared<proto::NymIDSource>();
    protoSource->ParseFromArray(dataSource.GetPointer(), dataSource.GetSize());

    return protoSource;
}

} // namespace opentxs
