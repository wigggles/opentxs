// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTENVELOPE_HPP
#define OPENTXS_CORE_CRYPTO_OTENVELOPE_HPP

#include "opentxs/Forward.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>

namespace opentxs
{
typedef std::multimap<std::string, crypto::key::Asymmetric*>
    mapOfAsymmetricKeys;
typedef std::set<const identity::Nym*> setOfNyms;

class OTEnvelope
{
public:
    EXPORT OTEnvelope(const api::Core& api);
    EXPORT explicit OTEnvelope(
        const api::Core& api,
        const Armored& theArmoredText);

    /** Retrieve ciphertext in ascii armored form */
    EXPORT bool GetCiphertext(Armored& theArmoredText) const;
    /** Load ascii armored ciphertext */
    EXPORT bool SetCiphertext(const Armored& theArmoredText);

    EXPORT bool Seal(
        const setOfNyms& recipients,
        const String& theInput,
        const PasswordPrompt& reason);
    EXPORT bool Seal(
        const identity::Nym& theRecipient,
        const String& theInput,
        const PasswordPrompt& reason);
    EXPORT bool Seal(
        const mapOfAsymmetricKeys& recipientKeys,
        const String& theInput,
        const PasswordPrompt& reason);
    EXPORT bool Seal(
        const crypto::key::Asymmetric& RecipPubKey,
        const String& theInput,
        const PasswordPrompt& reason);
    EXPORT bool Open(
        const identity::Nym& theRecipient,
        String& theOutput,
        const PasswordPrompt& reason);

    EXPORT ~OTEnvelope() = default;

private:
    friend Letter;

    const api::Core& api_;
    OTData ciphertext_;

    OTEnvelope() = delete;
};
}  // namespace opentxs
#endif
