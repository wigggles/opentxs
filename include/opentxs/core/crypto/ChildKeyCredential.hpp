// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_CHILDKEYCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_CHILDKEYCREDENTIAL_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/KeyCredential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/Proto.hpp"

#include <memory>

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 crypto::key::Asymmetrics (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

namespace opentxs
{

class CredentialSet;

class ChildKeyCredential : public KeyCredential
{

private:
    typedef KeyCredential ot_super;
    friend class Credential;

    serializedCredential serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;

    ChildKeyCredential(
        const api::Core& api,
        CredentialSet& other,
        const proto::Credential& serializedCred);
    ChildKeyCredential(
        const api::Core& api,
        CredentialSet& other,
        const NymParameters& nymParameters);
    ChildKeyCredential() = delete;
    ChildKeyCredential(const ChildKeyCredential&) = delete;
    ChildKeyCredential(ChildKeyCredential&&) = delete;
    ChildKeyCredential& operator=(const ChildKeyCredential&) = delete;
    ChildKeyCredential& operator=(ChildKeyCredential&&) = delete;

public:
    virtual ~ChildKeyCredential() = default;
};

}  // namespace opentxs

#endif
