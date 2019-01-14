// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTWALLET_HPP
#define OPENTXS_CLIENT_OTWALLET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>

namespace opentxs
{
class OTWallet final : Lockable
{
public:
    EXPORT void DisplayStatistics(String& strOutput) const;
    EXPORT std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const;
    // These allow the client application to encrypt its own sensitive data.
    // For example, let's say the client application is storing your Bitmessage
    // username and password in its database. It can't store those in the clear,
    // so it encrypts the DB's sensitive data using Encrypt_ByKeyID("sql_db")
    // and accesses the data using Decrypt_ByKeyID("sql_db").
    // The string acts as a key to look up a symmetric key which is normally
    // stored in encrypted form, using the wallet's master key. Whenever the
    // wallet's master key is available (until it times out) the client app will
    // thus be able to use these symmetric keys without having to ask the user
    // to type a passphrase.
    // (We do this for Nyms already. These methods basically give us the same
    // functionality for symmetric keys as we already had for the wallet's
    // Nyms.)
    EXPORT [[deprecated]] bool Decrypt_ByKeyID(
        const std::string& key_id,
        const String& strCiphertext,
        String& strOutput,
        const String& pstrDisplay = String::Factory());
    EXPORT [[deprecated]] bool Encrypt_ByKeyID(
        const std::string& key_id,
        const String& strPlaintext,
        String& strOutput,
        const String& pstrDisplay = String::Factory(),
        bool bBookends = true);
    EXPORT std::string GetPhrase();
    EXPORT std::string GetSeed();
    EXPORT std::string GetWords();
    EXPORT bool LoadWallet(const char* szFilename = nullptr);
    EXPORT bool SaveWallet(const char* szFilename = nullptr);

    EXPORT ~OTWallet();

private:
    friend OT_API;

    const api::Core& api_;
    OTString m_strName;
    OTString m_strVersion;
    OTString m_strFilename;
    OTString m_strDataFolder;

    void release(const Lock& lock);
    bool save_contract(const Lock& lock, String& strContract);
    bool save_wallet(const Lock& lock, const char* szFilename = nullptr);

    OTWallet(const api::Core& api);
    OTWallet() = delete;
    OTWallet(const OTWallet&) = delete;
    OTWallet(OTWallet&&) = delete;
    OTWallet& operator=(const OTWallet&) = delete;
    OTWallet& operator=(OTWallet&&) = delete;
};
}  // namespace opentxs

#endif
