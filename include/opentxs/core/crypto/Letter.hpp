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

#ifndef OPENTXS_CORE_CRYPTO_LETTER_HPP
#define OPENTXS_CORE_CRYPTO_LETTER_HPP

#include <opentxs/core/Contract.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/crypto/CryptoSymmetric.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>

#include <list>
#include <tuple>

namespace opentxs

{

class Nym;
class OTData;
class OTPasswordData;

typedef std::list<symmetricEnvelope> listOfSessionKeys;

// A letter is a contract that contains the contents of an OTEnvelope
// along with some necessary metadata.
// Currently only used for the secp256k1 version of Seal() and Open()
class Letter : public Contract
{
private: // Private prevents erroneous use by other classes.
    typedef Contract ot_super;
    static const CryptoSymmetric::Mode defaultPlaintextMode_ = CryptoSymmetric::AES_256_GCM;
    static const CryptoSymmetric::Mode defaultSessionKeyMode_ = CryptoSymmetric::AES_256_GCM;
    static const CryptoHash::HashType defaultHMAC_ = CryptoHash::SHA256;
    String ephemeralKey_;
    String iv_;
    String tag_;
    OTASCIIArmor ciphertext_;
    listOfSessionKeys sessionKeys_;
    Letter() = delete;

protected:
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);

public:
    Letter(
        const String& ephemeralKey,
        const String& iv,
        const String& tag,
        const OTASCIIArmor& ciphertext,
        const listOfSessionKeys& sessionKeys);
    Letter(const String& input);
    virtual ~Letter();
    void Release_Letter();
    virtual void Release();
    virtual void UpdateContents();

    static bool Seal(
        const mapOfAsymmetricKeys& RecipPubKeys,
        const String& theInput,
        OTData& dataOutput);
    static bool Open(
        const OTData& dataInput,
        const Nym& theRecipient,
        String& theOutput,
        const OTPasswordData* pPWData = nullptr);

    const String& EphemeralKey() const;
    const String& IV() const;
    const String& AEADTag() const;
    const listOfSessionKeys& SessionKeys() const;
    const OTASCIIArmor& Ciphertext() const;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_LETTER_HPP
