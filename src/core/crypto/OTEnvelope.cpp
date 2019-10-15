// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTEnvelope.hpp"

#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/crypto/Letter.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/identity/Nym.hpp"

#include <cstdint>
#include <ostream>

//#define OT_METHOD "opentxs::OTEnvelope::"

namespace opentxs
{
OTEnvelope::OTEnvelope(const api::internal::Core& api)
    : api_(api)
    , ciphertext_(Data::Factory())
{
}

OTEnvelope::OTEnvelope(
    const api::internal::Core& api,
    const Armored& theArmoredText)
    : api_(api)
    , ciphertext_(Data::Factory())
{
    SetCiphertext(theArmoredText);
}

bool OTEnvelope::GetCiphertext(Armored& theArmoredText) const
{
    if (ciphertext_->empty()) { return false; }

    return theArmoredText.SetData(ciphertext_.get(), true);
}

bool OTEnvelope::SetCiphertext(const Armored& theArmoredText)
{
    ciphertext_ = Data::Factory();

    return theArmoredText.GetData(ciphertext_, true);
}

bool OTEnvelope::Seal(
    const setOfNyms& recipients,
    const String& theInput,
    const PasswordPrompt& reason)
{
    mapOfAsymmetricKeys recipientKeys;

    for (auto& it : recipients) {
        recipientKeys.insert(std::pair<std::string, crypto::key::Asymmetric*>(
            "",
            const_cast<crypto::key::Asymmetric*>(&(it->GetPublicEncrKey()))));
    }

    if (!recipientKeys.empty()) {
        return Seal(recipientKeys, theInput, reason);
    } else {
        return false;
    }
}

bool OTEnvelope::Seal(
    const identity::Nym& theRecipient,
    const String& theInput,
    const PasswordPrompt& reason)
{
    return Seal(theRecipient.GetPublicEncrKey(), theInput, reason);
}

bool OTEnvelope::Seal(
    const crypto::key::Asymmetric& RecipPubKey,
    const String& theInput,
    const PasswordPrompt& reason)
{
    mapOfAsymmetricKeys recipientKeys;
    recipientKeys.insert(std::pair<std::string, crypto::key::Asymmetric*>(
        "", const_cast<crypto::key::Asymmetric*>(&RecipPubKey)));

    return Seal(recipientKeys, theInput, reason);
}

bool OTEnvelope::Seal(
    const mapOfAsymmetricKeys& recipientKeys,
    const String& theInput,
    const PasswordPrompt& reason)
{
    OT_ASSERT_MSG(
        !recipientKeys.empty(),
        "OTEnvelope::Seal: ASSERT: RecipPubKeys.size() > 0");

    ciphertext_ = Data::Factory();

    return Letter::Seal(api_, recipientKeys, theInput, ciphertext_, reason);
}

bool OTEnvelope::Open(
    const identity::Nym& theRecipient,
    String& theOutput,
    const PasswordPrompt& reason)
{
    if (ciphertext_->empty()) { return false; }

    return Letter::Open(api_, ciphertext_, theRecipient, theOutput, reason);
}
}  // namespace opentxs
