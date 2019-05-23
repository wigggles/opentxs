// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class LegacySymmetric final : virtual public key::LegacySymmetric, Lockable
{
public:
    // Must have a hash-check already!
    // CALLER IS RESPONSIBLE TO DELETE.
    OTPassword* CalculateDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase,
        bool bCheckForHashCheck = true) const override;
    void GetIdentifier(Identifier& theIdentifier) const override;
    void GetIdentifier(String& strIdentifier) const override;
    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via a derived key.
    //
    // If returns true, theRawKeyOutput will contain the decrypted symmetric
    // key, in an OTPassword object.
    // Otherwise returns false if failure.
    bool GetRawKeyFromDerivedKey(
        const OTPassword& theDerivedKey,
        OTPassword& theRawKeyOutput) const override;
    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via its passphrase being used to derive a key for that
    // purpose.
    bool GetRawKeyFromPassphrase(
        const OTPassword& thePassphrase,
        OTPassword& theRawKeyOutput,
        OTPassword* pDerivedKey = nullptr) const override;
    bool HasHashCheck() const override { return has_hash_check_.get(); }
    bool IsGenerated() const override { return m_bIsGenerated; }
    bool SerializeTo(Data& theOutput) const override;
    bool SerializeTo(Armored& ascOutput) const override;
    bool SerializeTo(String& strOutput, bool bEscaped = false) const override;

    // Must not have a hash-check yet!
    OTPassword* CalculateNewDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase) override;  // not const!
    // Changes the passphrase on an existing symmetric key.
    bool ChangePassphrase(
        const OTPassword& oldPassphrase,
        const OTPassword& newPassphrase) override;
    // For old symmetric keys that do not yet have a hash-check.
    // This will generate a hash check for them.
    bool GenerateHashCheck(const OTPassword& thePassphrase) override;
    // Generates this LegacySymmetric based on an OTPassword. The generated key
    // is stored in encrypted form, based on a derived key from that password.
    bool GenerateKey(
        const OTPassword& thePassphrase,
        OTPassword** ppDerivedKey = nullptr) override;  // If you want, I can
                                                        // pass this back to
                                                        // you.
    bool SerializeFrom(Data& theInput) override;
    bool SerializeFrom(const Armored& ascInput) override;
    bool SerializeFrom(const String& strInput, bool bEscaped = false) override;

    operator bool() const override { return true; }

    void Release() override;
    void Release_SymmetricKey();

    LegacySymmetric(const api::Core& api);
    LegacySymmetric(const api::Core& api, const OTPassword& thePassword);

    virtual ~LegacySymmetric();

private:
    const api::Core& api_;
    // GetKey asserts if this is false; GenerateKey asserts if it's true.
    bool m_bIsGenerated{false};
    // If a hash-check fo the Derived Key has been made yet.
    OTFlag has_hash_check_;
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
    LegacySymmetric* clone() const override;
    bool get_raw_key_from_derived_key(
        const Lock& lock,
        const OTPassword& theDerivedKey,
        OTPassword& theRawKeyOutput) const;
    bool get_raw_key_from_passphrase(
        const Lock& lock,
        const OTPassword& thePassphrase,
        OTPassword& theRawKeyOutput,
        OTPassword* pDerivedKey = nullptr) const;
    bool serialize_to(const Lock& lock, Armored& ascOutput) const;
    bool serialize_to(const Lock& lock, Data& theOutput) const;

    OTPassword* calculate_new_derived_key_from_passphrase(
        const Lock& lock,
        const OTPassword& thePassphrase);
    bool serialize_from(const Lock& lock, Data& theInput);
    bool serialize_from(const Lock& lock, const Armored& ascInput);

    LegacySymmetric() = delete;
    LegacySymmetric(const LegacySymmetric&);
    LegacySymmetric(LegacySymmetric&&) = delete;
    LegacySymmetric& operator=(const LegacySymmetric&) = delete;
    LegacySymmetric& operator=(LegacySymmetric&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
