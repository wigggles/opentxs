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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_KEY_NULL_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_KEY_NULL_HPP

#include "Internal.hpp"

#include "crypto/library/AsymmetricProviderNull.hpp"

#include <memory>

namespace opentxs::crypto::key::implementation
{
class Null : virtual public key::Asymmetric
{
public:
    bool CalculateID(Identifier&) const override { return false; }
    const opentxs::crypto::AsymmetricProvider& engine() const override
    {
        return *engine_;
    }
    const OTSignatureMetadata* GetMetadata() const override { return nullptr; }
    bool GetPublicKey(String&) const override { return false; }
    bool hasCapability(const NymCapability&) const override { return false; }
    bool IsEmpty() const override { return false; }
    bool IsPrivate() const override { return false; }
    bool IsPublic() const override { return false; }
    proto::AsymmetricKeyType keyType() const override
    {
        return proto::AKEYTYPE_NULL;
    }
    const std::string Path() const override { return {}; }
    bool Path(proto::HDPath&) const override { return false; }
    bool ReEncryptPrivateKey(const OTPassword&, bool bImporting) const override
    {
        return false;
    }
    const proto::KeyRole& Role() const override { return role_; }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override
    {
        return nullptr;
    }
    OTData SerializeKeyToData(const proto::AsymmetricKey&) const override
    {
        return Data::Factory();
    }
    proto::HashType SigHashType() const override
    {
        return proto::HASHTYPE_NONE;
    }
    bool Sign(
        const Data&,
        proto::Signature&,
        const OTPasswordData* = nullptr,
        const OTPassword* = nullptr,
        const String& = String(""),
        const proto::SignatureRole = proto::SIGROLE_ERROR) const override
    {
        return false;
    }
    bool TransportKey(Data&, OTPassword&) const override { return false; }
    bool Verify(const Data&, const proto::Signature&) const override
    {
        return false;
    }

    void Release() override {}
    void ReleaseKey() override {}
    void SetAsPublic() override {}
    void SetAsPrivate() override {}

    operator bool() const override { return false; }
    bool operator==(const proto::AsymmetricKey&) const override
    {
        return false;
    }

    Null()
        : engine_{new crypto::implementation::AsymmetricProviderNull}
    {
    }
    ~Null() = default;

private:
    std::unique_ptr<opentxs::crypto::AsymmetricProvider> engine_{nullptr};
    const proto::KeyRole role_{proto::KEYROLE_ERROR};

    Null* clone() const override { return new Null; }

    Null(const Null&) = delete;
    Null(Null&&) = delete;
    Null& operator=(const Null&) = delete;
    Null& operator=(Null&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_KEY_NULL_HPP
