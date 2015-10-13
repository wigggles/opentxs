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

#ifndef OPENTXS_CORE_CRYPTO_MASTERCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_MASTERCREDENTIAL_HPP

#include "KeyCredential.hpp"
#include <opentxs/core/crypto/NymParameters.hpp>

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
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials


namespace opentxs
{

class String;
class CredentialSet;

class MasterCredential : public KeyCredential
{
private: // Private prevents erroneous use by other classes.
    typedef KeyCredential ot_super;
    MasterCredential() = delete;
public:
    virtual bool VerifyInternally();  // Verify that m_strNymID is the same as
                                      // the hash of m_strSourceForNymID. Also
                                      // verify that *this ==
                                      // m_pOwner->m_MasterCredential (the master
                                      // credential.) Then verify the
                                      // (self-signed) signature on *this.
    bool VerifyAgainstSource() const; // Should actually curl the URL, or lookup
                                      // the blockchain value, or verify Cert
                                      // against Cert Authority, etc. Due to the
                                      // network slowdown of this step, we will
                                      // eventually make a separate identity
                                      // verification server.
    bool VerifySource_HTTP(const String strSource) const;
    bool VerifySource_HTTPS(const String strSource) const; // It's deliberate
                                                           // that strSource
                                                           // isn't passed by
                                                           // reference here.
    bool VerifySource_Bitcoin(const String strSource) const;
    bool VerifySource_Namecoin(const String strSource) const;
    bool VerifySource_Freenet(const String strSource) const;
    bool VerifySource_TOR(const String strSource) const;
    bool VerifySource_I2P(const String strSource) const;
    bool VerifySource_CA(const String strSource) const;
    bool VerifySource_Pubkey(const String strSource) const;
    MasterCredential(CredentialSet& theOwner);
    MasterCredential(CredentialSet& theOwner, const Credential::CredentialType masterType);
    MasterCredential(CredentialSet& theOwner, const std::shared_ptr<NymParameters>& nymParameters, const String* psourceForNymID = nullptr);
    virtual ~MasterCredential();
    virtual void UpdateContents();
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_MASTERCREDENTIAL_HPP
