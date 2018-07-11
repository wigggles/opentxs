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

#ifndef OPENTXS_CORE_CRYPTO_OTASYMMETRICKEYOPENSSL_HPP
#define OPENTXS_CORE_CRYPTO_OTASYMMETRICKEYOPENSSL_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/Proto.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
#ifndef OT_KEY_TIMER
// TODO:
// 1. Add this value to the config file so it becomes merely a default value
// here.
// 2. This timer solution isn't the full solution but only a stopgap measure.
// See notes in ReleaseKeyLowLevel for more -- ultimate solution will involve
// the callback itself, and some kind of encrypted storage of hashed passwords,
// using session keys, as well as an option to use ssh-agent and other standard
// APIs for protected memory.
//
// UPDATE: Am in the process now of adding the actual Master key. Therefore
// OT_MASTER_KEY_TIMEOUT was added for the actual mechanism, while OT_KEY_TIMER
// (a stopgap measure) was set to 0, which makes it of no effect. Probably
// OT_KEY_TIMER will be removed entirely (we'll see.)
#define OT_KEY_TIMER 30
// TODO: Next release, as users get converted to file format 2.0 (master key)
// then reduce this timer from 30 to 0. (30 is just to help them convert.)
//#define OT_KEY_TIMER 0
//#define OT_MASTER_KEY_TIMEOUT 300  // This is in OTEnvelope.h
// FYI: 1800 seconds is 30 minutes, 300 seconds is 5 mins.
#endif  // OT_KEY_TIMER

class RSA : public Asymmetric
{
private:
    typedef Asymmetric ot_super;
    friend class Asymmetric;  // For the factory.
    /** base64-encoded, string form of key. (Encrypted too, for private keys.
     * Should store it in this form most of the time.) m_p_ascKey is the most
     * basic value. m_pKey is derived from it, for example. */
    OTASCIIArmor* m_p_ascKey = nullptr;
    explicit RSA(const proto::AsymmetricKey& serializedKey);
    explicit RSA(const String& publicKey);

public:
    const crypto::AsymmetricProvider& engine() const override;
    bool IsEmpty() const override;
    /** Don't ever call this. It's only here because it's impossible to get rid
     * of unless and until RSA key support is removed entirely. */
    bool SaveCertToString(
        String& strOutput,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;

    virtual bool SetPrivateKey(
        const String& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    virtual bool SetPublicKeyFromPrivateKey(
        const String& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    virtual bool GetPrivateKey(
        String& strOutput,
        const Asymmetric* pPubkey,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;

    bool GetPublicKey(String& strKey) const override;
    virtual bool SetPublicKey(const String& strKey);

    bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const override;

    class d;

    d* dp;

    proto::HashType SigHashType() const override
    {
        return proto::HASHTYPE_SHA256;
    }

    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;

protected:
    RSA();
    explicit RSA(const proto::KeyRole role);

public:
    virtual ~RSA();
    void Release() override;
    void Release_AsymmetricKey_OpenSSL();

protected:
    void ReleaseKeyLowLevel_Hook() const override;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#endif  // OPENTXS_CORE_CRYPTO_OTASYMMETRICKEYOPENSSL_HPP
