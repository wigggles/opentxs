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

#ifndef OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP
#define OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP

#include <opentxs/core/crypto/Credential.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>

namespace opentxs
{

class NymParameters
{
public:
    enum NymParameterType: int32_t {
      ERROR,
      LEGACY,
      SECP256K1
    };

    NymParameterType nymParameterType();

    OTAsymmetricKey::KeyType AsymmetricKeyType();

    void setNymParameterType(NymParameterType theKeytype);

    Credential::CredentialType credentialType() const;

    void setCredentialType(Credential::CredentialType theCredentialtype);

#if defined(OT_CRYPTO_SUPPORTED_KEY_RSA)
    int32_t keySize();

    void setKeySize(int32_t keySize);
    NymParameters(const int32_t keySize);
#endif

    NymParameters(
        NymParameterType theKeytype,
        Credential::CredentialType theCredentialtype);
    NymParameters() = default;
    ~NymParameters() = default;

private:
    NymParameters(const NymParameters&) = delete;
    NymParameters& operator=(const NymParameters&) = delete;

#if defined(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    NymParameterType nymType_ = NymParameterType::SECP256K1;
    Credential::CredentialType credentialType_ = Credential::RSA_PUBKEY;
#elif defined(OT_CRYPTO_SUPPORTED_KEY_RSA)
    NymParameterType nymType_ = NymParameterType::LEGACY;
    Credential::CredentialType credentialType_ = Credential::RSA_PUBKEY;
#else
    NymParameterType nymType_ = NymParameterType::ERROR;
    Credential::CredentialType credentialType_ = Credential::ERROR_TYPE;
#endif

//----------------------------------------
// CRYPTO ALGORITHMS
//----------------------------------------
#if defined(OT_CRYPTO_SUPPORTED_KEY_RSA)
  int32_t nBits_ = 1024;
#endif

};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP
