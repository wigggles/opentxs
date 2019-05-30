// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_LETTER_HPP
#define OPENTXS_CORE_CRYPTO_LETTER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/String.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/Proto.hpp"

#include <list>
#include <map>
#include <string>

namespace opentxs
{
typedef std::multimap<std::string, crypto::key::Asymmetric*>
    mapOfAsymmetricKeys;
typedef std::
    tuple<OTString, OTString, OTString, OTString, std::shared_ptr<OTEnvelope>>
        symmetricEnvelope;
typedef std::list<symmetricEnvelope> listOfSessionKeys;
typedef std::map<proto::AsymmetricKeyType, std::string> listOfEphemeralKeys;
typedef std::multimap<std::string, const crypto::key::EllipticCurve*>
    mapOfECKeys;

/** A letter is a contract that contains the contents of an OTEnvelope along
 *  with some necessary metadata.
 */
class Letter
{
public:
    static bool Seal(
        const api::Core& api,
        const mapOfAsymmetricKeys& RecipPubKeys,
        const String& theInput,
        Data& dataOutput,
        const PasswordPrompt& reason);
    static bool Open(
        const api::Core& api,
        const Data& dataInput,
        const identity::Nym& theRecipient,
        String& theOutput,
        const PasswordPrompt& reason);

    ~Letter() = default;

private:
    static const VersionConversionMap akey_to_envelope_version_;

    static bool AddRSARecipients(
        const api::Core& api,
        const mapOfAsymmetricKeys& recipients,
        const crypto::key::Symmetric& sessionKey,
        proto::Envelope& envelope,
        const PasswordPrompt& reason);
    static bool DefaultPassword(PasswordPrompt& reason);
    static bool SortRecipients(
        const mapOfAsymmetricKeys& recipients,
        mapOfAsymmetricKeys& RSARecipients,
        mapOfECKeys& secp256k1Recipients,
        mapOfECKeys& ed25519Recipients);

    Letter() = default;
};
}  // namespace opentxs
#endif
