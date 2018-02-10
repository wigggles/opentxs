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

#ifndef OPENTXS_CORE_CRYPTO_OTSYMMETRICKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTSYMMETRICKEY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Lockable.hpp"

#include <atomic>
#include <cstdint>

namespace opentxs
{
// This class stores the iteration count, the salt, and the encrypted key.
// These are all generated or set when you call GenerateKey.

// Note: this calculates its ID based only on encrypted_key_,
// and does NOT include salt, IV, iteration count, etc when
// generating the hash for the ID.
class OTSymmetricKey : Lockable
{
public:
    // The highest-level possible interface (used by the API)

    // Caller must delete.
    EXPORT static OTPassword* GetPassphraseFromUser(
        const String* pstrDisplay = nullptr,
        bool bAskTwice = false);  // returns a text OTPassword, or nullptr.

    // If you already have the passphrase, you can pass it in as an optional
    // arg.
    // That way if you have to use it 100 times in a row, the user doesn't
    // actually have
    // to TYPE it 100 times in a row.
    //
    EXPORT static bool CreateNewKey(
        String& strOutput,
        const String* pstrDisplay = nullptr,
        const OTPassword* pAlreadyHavePW = nullptr);
    EXPORT static bool Encrypt(
        const String& strKey,
        const String& strPlaintext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        bool bBookends = true,
        const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT static bool Decrypt(
        const String& strKey,
        String& strCiphertext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT static bool Encrypt(
        const OTSymmetricKey& theKey,
        const String& strPlaintext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        bool bBookends = true,
        const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT static bool Decrypt(
        const OTSymmetricKey& theKey,
        const String& strCiphertext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT bool SerializeTo(Data& theOutput) const;
    EXPORT bool SerializeFrom(Data& theInput);

    EXPORT bool SerializeTo(OTASCIIArmor& ascOutput) const;
    EXPORT bool SerializeFrom(const OTASCIIArmor& ascInput);

    EXPORT bool SerializeTo(String& strOutput, bool bEscaped = false) const;
    EXPORT bool SerializeFrom(const String& strInput, bool bEscaped = false);
    inline bool IsGenerated() const { return m_bIsGenerated; }
    inline bool HasHashCheck() const { return has_hash_check_.load(); }
    EXPORT void GetIdentifier(Identifier& theIdentifier) const;
    EXPORT void GetIdentifier(String& strIdentifier) const;
    // The derived key is used for decrypting the actual symmetric key.
    // It's called the derived key because it is derived from the passphrase.
    //

    // Must have a hash-check already!
    // CALLER IS RESPONSIBLE TO DELETE.
    EXPORT OTPassword* CalculateDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase,
        bool bCheckForHashCheck = true) const;

    // Must not have a hash-check yet!
    EXPORT OTPassword* CalculateNewDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase);  // not const!

    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via its passphrase being used to derive a key for that
    // purpose.
    //
    EXPORT bool GetRawKeyFromPassphrase(
        const OTPassword& thePassphrase,
        OTPassword& theRawKeyOutput,
        OTPassword* pDerivedKey = nullptr) const;

    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via a derived key.
    //
    // If returns true, theRawKeyOutput will contain the decrypted symmetric
    // key, in
    // an OTPassword object.
    // Otherwise returns false if failure.
    EXPORT bool GetRawKeyFromDerivedKey(
        const OTPassword& theDerivedKey,
        OTPassword& theRawKeyOutput) const;
    // Generates this OTSymmetricKey based on an OTPassword. The generated key
    // is
    // stored in encrypted form, based on a derived key from that password.
    //
    EXPORT bool GenerateKey(
        const OTPassword& thePassphrase,
        OTPassword** ppDerivedKey = nullptr);  // If you
                                               // want, I
    // can pass this
    // back to you.
    // Changes the passphrase on an existing symmetric key.
    //
    EXPORT bool ChangePassphrase(
        const OTPassword& oldPassphrase,
        const OTPassword& newPassphrase);
    // For old SymmetricKey's that do not yet have a hash-check.
    // This will generate a hash check for them.
    //
    EXPORT bool GenerateHashCheck(const OTPassword& thePassphrase);

    EXPORT OTSymmetricKey();
    EXPORT OTSymmetricKey(const OTPassword& thePassword);

    EXPORT virtual ~OTSymmetricKey();
    EXPORT virtual void Release();

    EXPORT void Release_SymmetricKey();

private:
    // GetKey asserts if this is false; GenerateKey asserts if it's true.
    bool m_bIsGenerated{false};
    // If a hash-check fo the Derived Key has been made yet.
    std::atomic<bool> has_hash_check_{false};
    // The size, in bits. For example, 128 bit key, 256 bit key, etc.
    std::uint32_t m_nKeySize{0};
    // Stores the iteration count, which should probably be at least 2000.
    // (Number of iterations used while generating key from passphrase.)
    std::uint32_t m_uIterationCount{0};
    // Stores the SALT (which is used with the password for generating /
    // retrieving the key from encrypted_key_)
    OTData salt_;
    // Stores the IV used internally for encrypting / decrypting the actual key
    // (using the derived key) from encrypted_key_.
    OTData iv_;
    // Stores only encrypted version of symmetric key.
    OTData encrypted_key_;
    OTData hash_check_;

    OTPassword* calculate_derived_key_from_passphrase(
        const Lock& lock,
        const OTPassword& thePassphrase,
        bool bCheckForHashCheck = true) const;
    bool get_raw_key_from_derived_key(
        const Lock& lock,
        const OTPassword& theDerivedKey,
        OTPassword& theRawKeyOutput) const;
    bool get_raw_key_from_passphrase(
        const Lock& lock,
        const OTPassword& thePassphrase,
        OTPassword& theRawKeyOutput,
        OTPassword* pDerivedKey = nullptr) const;
    bool serialize_to(const Lock& lock, OTASCIIArmor& ascOutput) const;
    bool serialize_to(const Lock& lock, Data& theOutput) const;

    OTPassword* calculate_new_derived_key_from_passphrase(
        const Lock& lock,
        const OTPassword& thePassphrase);
    bool serialize_from(const Lock& lock, Data& theInput);
    bool serialize_from(const Lock& lock, const OTASCIIArmor& ascInput);
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_OTSYMMETRICKEY_HPP
