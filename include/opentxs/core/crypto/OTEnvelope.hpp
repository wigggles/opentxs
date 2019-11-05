// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTENVELOPE_HPP
#define OPENTXS_CORE_CRYPTO_OTENVELOPE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Data.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

typedef std::multimap<std::string, crypto::key::Asymmetric*>
    mapOfAsymmetricKeys;
typedef std::set<const identity::Nym*> setOfNyms;

class OTEnvelope
{
public:
    OPENTXS_EXPORT OTEnvelope(const api::internal::Core& api);
    OPENTXS_EXPORT explicit OTEnvelope(
        const api::internal::Core& api,
        const Armored& theArmoredText);

    /** Retrieve ciphertext in ascii armored form */
    OPENTXS_EXPORT bool GetCiphertext(Armored& theArmoredText) const;
    /** Load ascii armored ciphertext */
    OPENTXS_EXPORT bool SetCiphertext(const Armored& theArmoredText);

    OPENTXS_EXPORT bool Seal(
        const setOfNyms& recipients,
        const String& theInput,
        const PasswordPrompt& reason);
    OPENTXS_EXPORT bool Seal(
        const identity::Nym& theRecipient,
        const String& theInput,
        const PasswordPrompt& reason);
    OPENTXS_EXPORT bool Seal(
        const mapOfAsymmetricKeys& recipientKeys,
        const String& theInput,
        const PasswordPrompt& reason);
    OPENTXS_EXPORT bool Seal(
        const crypto::key::Asymmetric& RecipPubKey,
        const String& theInput,
        const PasswordPrompt& reason);
    OPENTXS_EXPORT bool Open(
        const identity::Nym& theRecipient,
        String& theOutput,
        const PasswordPrompt& reason);

    OPENTXS_EXPORT ~OTEnvelope() = default;

private:
    friend Letter;

    const api::internal::Core& api_;
    OTData ciphertext_;

    OTEnvelope() = delete;
};
}  // namespace opentxs
#endif
