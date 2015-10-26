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
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>

namespace opentxs

{

class Nym;
class OTData;
class OTPasswordData;

typedef std::map<String, OTEnvelope> mapOfSessionKeys;

// A letter is a contract that contains the contents of an OTEnvelope
// along with some necessary metadata.
// Currently only used for the secp256k1 version of Seal() and Open()
class Letter : public Contract
{
private: // Private prevents erroneous use by other classes.
    typedef Contract ot_super;
    String ephemeralKey_;
    String macType_;
    String iv_;
    OTASCIIArmor ciphertext_;
    mapOfSessionKeys sessionKeys_;
    Letter() = delete;

protected:
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);

public:
    Letter(
        const String& ephemeralKey,
        const String& macType,
        const String& iv,
        const OTASCIIArmor& ciphertext,
        const mapOfSessionKeys& sessionKeys);
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
    const String& MACType() const;
    const mapOfSessionKeys& SessionKeys() const;
    const OTASCIIArmor& Ciphertext() const;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_LETTER_HPP
