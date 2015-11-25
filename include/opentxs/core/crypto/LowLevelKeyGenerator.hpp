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

#ifndef OPENTXS_CORE_CRYPTO_LOWLEVELKEYGENERATOR_HPP
#define OPENTXS_CORE_CRYPTO_LOWLEVELKEYGENERATOR_HPP

#include <opentxs/core/crypto/OTPasswordData.hpp>

#include <memory>

namespace opentxs
{

class OTCaller;
class OTKeypair;

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

class NymParameters;
/// LowLevelKeyGenerator
/// Used for passing x509's and EVP_PKEYs around, so a replacement
/// crypto engine will not require changes to any function parameters
/// throughout the rest of OT.
class LowLevelKeyGenerator
{
private:
    class LowLevelKeyGeneratordp;

    std::unique_ptr<NymParameters> pkeyData_;

    LowLevelKeyGenerator() = delete;
    LowLevelKeyGenerator(const LowLevelKeyGenerator&) = delete;
    LowLevelKeyGenerator& operator=(const LowLevelKeyGenerator&) = delete;
    void Cleanup();
    LowLevelKeyGeneratordp* dp = nullptr;

#if defined(OT_CRYPTO_USING_OPENSSL)
    class LowLevelKeyGeneratorOpenSSLdp;
#endif

#if defined(OT_CRYPTO_USING_LIBSECP256K1)
    class LowLevelKeyGeneratorSecp256k1dp;
#endif

public:
    bool m_bCleanup = true; // By default, LowLevelKeyGenerator cleans up the members. But
                     // if you set this to false, it will NOT cleanup.
    bool MakeNewKeypair();
    bool SetOntoKeypair(OTKeypair& theKeypair, OTPasswordData& passwordData);

    LowLevelKeyGenerator(const NymParameters& pkeyData);
    ~LowLevelKeyGenerator();

};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_LOWLEVELKEYGENERATOR_HPP
