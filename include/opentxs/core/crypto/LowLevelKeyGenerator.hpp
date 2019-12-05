// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_LOWLEVELKEYGENERATOR_HPP
#define OPENTXS_CORE_CRYPTO_LOWLEVELKEYGENERATOR_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include <memory>

namespace opentxs
{
class LowLevelKeyGenerator
{
private:
    /** Used for passing x509's and EVP_PKEYs around, so a replacement crypto
     *  engine will not require changes to any function parameters throughout
     * the rest of OT. */
    class LowLevelKeyGeneratordp;
#if OT_CRYPTO_USING_OPENSSL
    class LowLevelKeyGeneratorOpenSSLdp;
#endif

    std::unique_ptr<LowLevelKeyGeneratordp> dp;
    std::unique_ptr<NymParameters> pkeyData_;

    LowLevelKeyGenerator() = delete;
    LowLevelKeyGenerator(const LowLevelKeyGenerator&) = delete;
    LowLevelKeyGenerator& operator=(const LowLevelKeyGenerator&) = delete;
    void Cleanup();

public:
    /** By default, LowLevelKeyGenerator cleans up the members. But if you set
     * this to false, it will NOT cleanup. */
    bool m_bCleanup;

    LowLevelKeyGenerator(const NymParameters& pkeyData);

    bool MakeNewKeypair();
    bool SetOntoKeypair(
        crypto::key::Keypair& theKeypair,
        const PasswordPrompt& passwordData);

    ~LowLevelKeyGenerator();
};
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#endif
