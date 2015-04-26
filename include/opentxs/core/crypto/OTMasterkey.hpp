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

#ifndef OPENTXS_CORE_CRYPTO_OTMASTERKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTMASTERKEY_HPP

#include "OTKeyCredential.hpp"

// A nym contains a list of master credentials, via OTCredential.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each credential is like a master key for the Nym's identity,
// which can issue its own subkeys.
//
// Each subkey has 3 key pairs: encryption, signing, and authentication.
// Not all subcredentials are a subkey. For example, you might have a
// subcredential that uses Google Authenticator, and thus doesn't contain
// any keys, because it uses alternate methods for its own authentication.
//
// Each OTCredential contains a "master" subkey, and a list of subcredentials
// (some of them subkeys) signed by that master.
//
// The same class (subcredential/subkey) is used because there are master
// credentials and subcredentials, so we're using inheritance for
// "subcredential"
// and "subkey" to encapsulate the credentials, so we don't have to repeat code
// across both.
// We're using a "has-a" model here, since the OTCredential "has a" master
// subkey, and also "has a" list of subcredentials, some of which are subkeys.
//
// Each subcredential must be signed by the subkey that is the master key.
// Each subkey has 3 key pairs: encryption, signing, and authentication.
//
// Each key pair has 2 OTAsymmetricKeys (public and private.)
//
// I'm thinking that the Nym should also have a key pair (for whatever is
// its current key pair, copied from its credentials.)
//
// the master should never be able to do any actions except for sign subkeys.
// the subkeys, meanwhile should only be able to do actions, and not issue
// any new keys.

namespace opentxs
{

class String;
class OTCredential;

class OTMasterkey : public OTKeyCredential
{
private: // Private prevents erroneous use by other classes.
    typedef OTKeyCredential ot_super;
    friend class OTCredential;

public:
    virtual bool VerifyInternally();  // Verify that m_strNymID is the same as
                                      // the hash of m_strSourceForNymID. Also
                                      // verify that *this ==
                                      // m_pOwner->m_MasterKey (the master
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
    OTMasterkey();
    OTMasterkey(OTCredential& theOwner);
    virtual ~OTMasterkey();
    virtual void UpdateContents();
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTMASTERKEY_HPP
