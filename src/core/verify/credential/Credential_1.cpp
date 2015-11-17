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

#include <opentxs/core/verify/Verify.hpp>

#include <opentxs/core/Log.hpp>

namespace opentxs { namespace proto
{

bool Credential_1(
    const Credential serializedCred,
    const proto::CredentialRole role,
    const bool withSigs)
{
    bool isPrivate = false;
    bool isPublic = false;
    bool validPublicData = false;
    bool validPrivateData = false;
    bool expectMasterSignature = false;
    int32_t expectedSigCount = 2;

    if (CREDROLE_CHILDKEY == role) {
        expectedSigCount = 3;
        expectMasterSignature = true;
    }

    if (!serializedCred.has_id()) {
        otErr << "Verify serialized credential failed: missing identifier.\n";
        return false;
    }

    if (MIN_PLAUSIBLE_IDENTIFIER > serializedCred.id().size()) {
        otErr << "Verify serialized credential failed: invalid identifier ("
                << serializedCred.id() << ").\n";
        return false;
    }

    if (!serializedCred.has_type()) {
        otErr << "Verify serialized credential failed: missing type.\n";
        return false;
    }

    if (CREDTYPE_LEGACY != serializedCred.type()) {
        otErr << "Verify serialized credential failed: invalid type ("
                << serializedCred.type() << ").\n";
        return false;
    }

    if (!serializedCred.has_role()) {
        otErr << "Verify serialized credential failed: missing role.\n";
        return false;
    }

    if ((CREDROLE_MASTERKEY != role) && (CREDROLE_CHILDKEY != role)) {
        otErr << "Verify serialized credential failed: invalid role ("
                << serializedCred.role() << ").\n";
        return false;
    }

    if (!serializedCred.has_mode()) {
        otErr << "Verify serialized credential failed: missing mode.\n";
        return false;
    }

    if (KEYMODE_PUBLIC == serializedCred.mode()) {
        isPublic = true;
    }

    if (KEYMODE_PRIVATE == serializedCred.mode()) {
        isPrivate = true;
    }

    if (!(isPrivate || isPublic)) {
        otErr << "Verify serialized credential failed: invalid mode ("
                << serializedCred.mode() << ").\n";
        return false;
    }

    if (!serializedCred.has_nymid()) {
        otErr << "Verify serialized credential failed: missing NymID.\n";
        return false;
    }

    if (MIN_PLAUSIBLE_IDENTIFIER > serializedCred.nymid().size()) {
        otErr << "Verify serialized credential failed: invalid NymID ("
                << serializedCred.nymid() << ").\n";
        return false;
    }

    if (isPublic && serializedCred.has_privatecredential()) {
        otErr << "Verify serialized credential failed: public credential contains private data.\n";
        return false;
    }

    if (!serializedCred.has_publiccredential()) {
        otErr << "Verify serialized credential failed: missing public data.\n";
        return false;
    }

    if (isPrivate && (!serializedCred.has_privatecredential())) {
        otErr << "Verify serialized credential failed: missing private data.\n";
        return false;
    }

    validPublicData = Verify
        (serializedCred.publiccredential(),
            CredentialAllowedKeyCredentials.at(serializedCred.version()).first,
            CredentialAllowedKeyCredentials.at(serializedCred.version()).second,
            role,
            CREDTYPE_LEGACY,
            KEYMODE_PUBLIC,
            SOURCETYPE_SELF);

    if (!validPublicData) {
        otErr << "Verify serialized credential failed: invalid public data.\n";
        return false;
    }

    if (isPrivate) {
        validPrivateData = Verify(
            serializedCred.privatecredential(),
            CredentialAllowedKeyCredentials.at(serializedCred.version()).first,
            CredentialAllowedKeyCredentials.at(serializedCred.version()).second,
            role,
            CREDTYPE_LEGACY,
            KEYMODE_PRIVATE,
            SOURCETYPE_ERROR);

        if (!validPrivateData) {
            otErr << "Verify serialized credential failed: invalid private data.\n";
            return false;
        }
    }

    if (withSigs) {
        std::string masterID = serializedCred.publiccredential().childdata().masterid();

        if (expectedSigCount != serializedCred.signature_size()) {
            otErr << "Verify serialized credential failed: incorrect number of signature(s) ("
            << serializedCred.signature_size() << " of " << expectedSigCount << " found).\n";
            return false;
        }

        uint32_t selfPublicCount = 0;
        uint32_t selfPrivateCount = 0;
        uint32_t masterPublicCount = 0;

        for (auto& it: serializedCred.signature()) {
            bool validSig = Verify(
                it,
                CredentialAllowedSignatures.at(serializedCred.version()).first,
                CredentialAllowedSignatures.at(serializedCred.version()).second,
                serializedCred.id(),
                masterID,
                selfPublicCount,
                selfPrivateCount,
                masterPublicCount);

            if (!validSig) {
                otErr << "Verify serialized credential failed: invalid signature.\n";
                return false;
            }
        }

        if (1 != selfPrivateCount) {
            otErr << "Verify serialized credential failed: incorrect number of private self-signatures ("
            << selfPrivateCount << " of 1 found).\n";
            return false;
        }

        if (1 != selfPublicCount) {
            otErr << "Verify serialized credential failed: incorrect number of public self-signatures ("
            << selfPublicCount << " of 1 found).\n";
            return false;
        }

        if ((1 != masterPublicCount) && (expectMasterSignature)) {
            otErr << "Verify serialized credential failed: incorrect number of public master signatures ("
            << selfPublicCount << " of 1 found).\n";
            return false;
        }
    }

    return true;
}

} // namespace proto
} // namespace opentxs
