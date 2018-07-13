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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_KEY_LEGACYSYMMETRICNULL_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_KEY_LEGACYSYMMETRICNULL_HPP

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class LegacySymmetricNull final : virtual public key::LegacySymmetric
{
public:
    OTPassword* CalculateDerivedKeyFromPassphrase(
        const OTPassword&,
        bool = true) const override
    {
        return nullptr;
    }
    void GetIdentifier(Identifier&) const override {}
    void GetIdentifier(String&) const override {}
    bool GetRawKeyFromDerivedKey(const OTPassword&, OTPassword&) const override
    {
        return false;
    }
    bool GetRawKeyFromPassphrase(
        const OTPassword&,
        OTPassword&,
        OTPassword* = nullptr) const override
    {
        return false;
    }
    bool HasHashCheck() const override { return false; }
    bool IsGenerated() const override { return false; }
    bool SerializeTo(Data&) const override { return false; }
    bool SerializeTo(OTASCIIArmor&) const override { return false; }
    bool SerializeTo(String&, bool = false) const override { return false; }

    OTPassword* CalculateNewDerivedKeyFromPassphrase(const OTPassword&) override
    {
        return nullptr;
    }
    bool ChangePassphrase(const OTPassword&, const OTPassword&) override
    {
        return false;
    }
    bool GenerateHashCheck(const OTPassword&) override { return false; }
    bool GenerateKey(const OTPassword&, OTPassword** = nullptr) override
    {
        return false;
    }
    bool SerializeFrom(Data&) override { return false; }
    bool SerializeFrom(const OTASCIIArmor&) override { return false; }
    bool SerializeFrom(const String& strInput, bool = false) override
    {
        return false;
    }

    operator bool() const override { return false; }

    void Release() override {}

    LegacySymmetricNull() = default;

    ~LegacySymmetricNull() = default;

private:
    LegacySymmetricNull* clone() const override { return nullptr; }

    LegacySymmetricNull(const LegacySymmetricNull&) = delete;
    LegacySymmetricNull(LegacySymmetricNull&&) = delete;
    LegacySymmetricNull& operator=(const LegacySymmetricNull&) = delete;
    LegacySymmetricNull& operator=(LegacySymmetricNull&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_KEY_LEGACYSYMMETRICNULL_HPP
