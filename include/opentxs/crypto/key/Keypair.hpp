// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_KEYPAIR_HPP
#define OPENTXS_CRYPTO_KEY_KEYPAIR_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <list>
#include <memory>

namespace opentxs
{
namespace crypto
{
namespace key
{
class Keypair
{
public:
    using Keys = std::list<const Asymmetric*>;

    EXPORT virtual bool CalculateID(Identifier& theOutput) const = 0;
    EXPORT virtual const Asymmetric& GetPrivateKey() const = 0;
    EXPORT virtual const Asymmetric& GetPublicKey() const = 0;
    EXPORT virtual bool GetPublicKey(String& strKey) const = 0;
    // inclusive means, return keys when theSignature has no metadata.
    EXPORT virtual std::int32_t GetPublicKeyBySignature(
        Keys& listOutput,
        const Signature& theSignature,
        bool bInclusive = false) const = 0;
    EXPORT virtual bool hasCapability(
        const NymCapability& capability) const = 0;
    EXPORT virtual bool HasPrivateKey() const = 0;
    EXPORT virtual bool HasPublicKey() const = 0;
    EXPORT virtual bool ReEncrypt(
        const OTPassword& theExportPassword,
        bool bImporting) = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> Serialize(
        bool privateKey = false) const = 0;
    EXPORT virtual bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const Identifier& credential,
        proto::KeyRole key = proto::KEYROLE_SIGN,
        const OTPasswordData* pPWData = nullptr,
        const proto::HashType hash = proto::HASHTYPE_BLAKE2B256) const = 0;
    EXPORT virtual bool TransportKey(Data& publicKey, OTPassword& privateKey)
        const = 0;
    EXPORT virtual bool Verify(
        const Data& plaintext,
        const proto::Signature& sig) const = 0;

    EXPORT virtual ~Keypair() = default;

protected:
    Keypair() = default;

private:
    friend OTKeypair;

    virtual Keypair* clone() const = 0;

    Keypair(const Keypair&) = delete;
    Keypair(Keypair&&) = delete;
    Keypair& operator=(const Keypair&) = delete;
    Keypair& operator=(Keypair&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
