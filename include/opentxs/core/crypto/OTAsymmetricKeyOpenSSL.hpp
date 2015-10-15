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

#include <opentxs/core/crypto/OTAsymmetricKey.hpp>

namespace opentxs
{

class OTASCIIArmor;
class OTCaller;
class OTPassword;
class String;
class FormattedKey;

// Todo:
// 1. Add this value to the config file so it becomes merely a default value
// here.
// 2. This timer solution isn't the full solution but only a stopgap measure.
//    See notes in ReleaseKeyLowLevel for more -- ultimate solution will involve
//    the callback itself, and some kind of encrypted storage of hashed
// passwords,
//    using session keys, as well as an option to use ssh-agent and other
// standard
//    APIs for protected memory.
//
// UPDATE: Am in the process now of adding the actual Master key. Therefore
// OT_MASTER_KEY_TIMEOUT
// was added for the actual mechanism, while OT_KEY_TIMER (a stopgap measure)
// was set to 0, which
// makes it of no effect. Probably OT_KEY_TIMER will be removed entirely (we'll
// see.)
//
#ifndef OT_KEY_TIMER

#define OT_KEY_TIMER 30

// TODO: Next release, as users get converted to file format 2.0 (master key)
// then reduce this timer from 30 to 0. (30 is just to help them convert.)

//#define OT_KEY_TIMER 0

//#define OT_MASTER_KEY_TIMEOUT 300  // This is in OTEnvelope.h

// FYI: 1800 seconds is 30 minutes, 300 seconds is 5 mins.
#endif // OT_KEY_TIMER

#if defined(OT_CRYPTO_USING_OPENSSL)

class OTAsymmetricKey_OpenSSL : public OTAsymmetricKey
{
private:
    typedef OTAsymmetricKey ot_super;
    friend class OTAsymmetricKey; // For the factory.
public:

    virtual bool SetPrivateKey(
        const String& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    virtual bool SetPublicKeyFromPrivateKey(
        const String& strCert,
        const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);

    virtual bool SaveCertToString(
        String& strOutput, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;
    virtual bool SavePrivateKeyToString(
        String& strOutput, const String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr) const;

    virtual bool ReEncryptPrivateKey(const OTPassword& theExportPassword,
                                     bool bImporting) const;

    class OTAsymmetricKey_OpenSSLPrivdp;
    OTAsymmetricKey_OpenSSLPrivdp* dp;

protected: // CONSTRUCTOR
    OTAsymmetricKey_OpenSSL();

public: // DERSTRUCTION
    virtual ~OTAsymmetricKey_OpenSSL();
    virtual void Release();
    void Release_AsymmetricKey_OpenSSL();

protected:
    virtual void ReleaseKeyLowLevel_Hook() const;
};

#elif defined(OT_CRYPTO_USING_GPG)

#else // NO CRYPTO ENGINE DEFINED?

#endif

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTASYMMETRICKEYOPENSSL_HPP
