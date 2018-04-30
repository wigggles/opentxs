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
/** AccountInfo: accountID, nymID, serverID, unitID*/
using AccountInfo =
    std::tuple<OTIdentifier, OTIdentifier, OTIdentifier, OTIdentifier>;

class OTWallet : Lockable
{
public:
    EXPORT std::set<AccountInfo> AccountList() const;
    EXPORT void DisplayStatistics(String& strOutput) const;
    EXPORT bool GetAccount(
        const std::size_t iIndex,
        Identifier& THE_ID,
        String& THE_NAME) const;
    EXPORT std::size_t GetAccountCount() const;
    EXPORT std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const;

    EXPORT void AddAccount(std::shared_ptr<Account>& theAcct);
    // Low level.
    EXPORT bool addExtraKey(
        const std::string& str_id,
        std::shared_ptr<OTSymmetricKey> pKey);
#if OT_CASH
    // While waiting on server response to a withdrawal, we keep the private
    // coin data here so we can unblind the response. This information is so
    // important (as important as the digital cash token itself, until the
    // unblinding is done) that we need to save the file right away.
    EXPORT void AddPendingWithdrawal(const Purse& thePurse);
#endif
    // When the wallet's master passphrase changes, the extra symmetric keys
    // need to be updated to reflect that.
    EXPORT bool ChangePassphrasesOnExtraKeys(
        const OTPassword& oldPassphrase,
        const OTPassword& newPassphrase);
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
    EXPORT bool Decrypt_ByKeyID(
        const std::string& key_id,
        const String& strCiphertext,
        String& strOutput,
        const String* pstrDisplay = nullptr);
    EXPORT bool Encrypt_ByKeyID(
        const std::string& key_id,
        const String& strPlaintext,
        String& strOutput,
        const String* pstrDisplay = nullptr,
        bool bBookends = true);
    EXPORT std::shared_ptr<Account> GetAccount(const Identifier& theAccountID);
    EXPORT std::shared_ptr<Account> GetAccountPartialMatch(
        std::string PARTIAL_ID);
    // Low level.
    EXPORT std::shared_ptr<OTSymmetricKey> getExtraKey(
        const std::string& str_id) const;
    EXPORT std::shared_ptr<Account> GetIssuerAccount(
        const Identifier& theInstrumentDefinitionID);
    EXPORT std::shared_ptr<OTSymmetricKey> getOrCreateExtraKey(
        const std::string& str_KeyID,
        const std::string* pReason = nullptr);  // Use this one.
    EXPORT std::shared_ptr<Account> GetOrLoadAccount(
        const Nym& theNym,
        const Identifier& ACCT_ID,
        const Identifier& NOTARY_ID,
        const char* szFuncName = nullptr);
#if OT_CASH
    EXPORT Purse* GetPendingWithdrawal();
#endif  // OT_CASH
    EXPORT std::string GetPhrase();
    EXPORT std::string GetSeed();
    EXPORT std::string GetWords();
    EXPORT std::shared_ptr<Account> LoadAccount(
        const Nym& theNym,
        const Identifier& ACCT_ID,
        const Identifier& NOTARY_ID,
        const char* szFuncName = nullptr);
    EXPORT bool LoadWallet(const char* szFilename = nullptr);
    // These functions are low-level. They don't check for dependent data before
    // deleting, and they don't save the wallet after they do.
    //
    // (You have to handle that at a higher level.) higher level version of
    // these two will require a server message, in addition to removing from
    // wallet. (To delete them on server side.)
    EXPORT bool RemoveAccount(const Identifier& theTargetID);
#if OT_CASH
    EXPORT void RemovePendingWithdrawal();
#endif  // OT_CASH
    // These functions are low-level. They don't check for dependent data before
    // deleting, and they don't save the wallet after they do.
    //
    // (You have to handle that at a higher level.) higher level version of
    // these two will require a server message, in addition to removing from
    // wallet. (To delete them on server side.)
    EXPORT bool SaveWallet(const char* szFilename = nullptr);

    EXPORT ~OTWallet();

private:
    friend OT_API;

    /** AccountEntry nymID, serverID, unitID, account */
    using AccountEntry = std::tuple<
        OTIdentifier,
        OTIdentifier,
        OTIdentifier,
        std::shared_ptr<Account>>;
    using mapOfAccounts = std::map<OTIdentifier, AccountEntry>;
    using mapOfSymmetricKeys =
        std::map<std::string, std::shared_ptr<OTSymmetricKey>>;

    const api::Crypto& crypto_;
    const api::storage::Storage& storage_;
#if OT_CASH
    // While waiting on server response to withdrawal, store private coin data
    // here for unblinding
    Purse* m_pWithdrawalPurse{nullptr};
#endif  // OT_CASH
    String m_strName{};
    String m_strVersion{};
    String m_strFilename{};
    String m_strDataFolder{};
    mapOfAccounts m_mapAccounts;
    // Let's say you have some private data that you want to store safely.
    // For example, your Bitmessage user/pass. Perhaps you want to throw
    // your Bitmessage connect string into your client-side sql*lite DB.
    // But you can't leave the password there in plaintext form! So instead,
    // you create a symmetric key to encrypt it with (stored here on this
    // map.)
    // Therefore your data, such as your Bitmessage password, is stored in
    // encrypted form to a symmetric key stored in the wallet. Then that
    // symmetric key is encrypted to the master password in the wallet.
    // If the master password ever changes, the symmetric keys on this map
    // can be re-encrypted to the new master password. Meanwhile the Bitmessage
    // connection string ITSELF, in your sql*lite DB, doesn't need to be re-
    // encrypted at all, since it's encrypted to the symmetric key, which,
    // though itself may be re-encrypted to another master password, the actual
    // contents of the symmetric key haven't changed.
    //
    // (This way you can change the wallet master passphrase, WITHOUT having
    // to go through your sql*lite database re-encrypting all the crap in there
    // that you might have encrypted previously before you changed your wallet
    // password.)
    //
    // That's why these are "extra" keys -- because you can create as many of
    // them as you want, and just use them for encrypting various data on the
    // client side.
    //
    mapOfSymmetricKeys m_mapExtraKeys{};
    // All the Nyms that use the Master key are listed here (makes it easy
    // to see which ones are converted already.)

    void add_account(const Lock& lock, std::shared_ptr<Account>& theAcct);
    bool add_extra_key(
        const Lock& lock,
        const std::string& str_id,
        std::shared_ptr<OTSymmetricKey> pKey);
    std::shared_ptr<Account> get_account(
        const Lock& lock,
        const Identifier& theAccountID);
    std::shared_ptr<Account> load_account(
        const Lock& lock,
        const Nym& theNym,
        const Identifier& ACCT_ID,
        const Identifier& NOTARY_ID,
        const char* szFuncName = nullptr);
    void release(const Lock& lock);
    bool save_contract(const Lock& lock, String& strContract);
    bool save_wallet(const Lock& lock, const char* szFilename = nullptr);
    bool verify_account(
        const Lock& lock,
        const Nym& theNym,
        Account& theAcct,
        const Identifier& NOTARY_ID,
        const String& strAcctID,
        const char* szFuncName = nullptr);

    OTWallet(const api::Crypto& crypto, const api::storage::Storage& storage);
    OTWallet() = delete;
    OTWallet(const OTWallet&) = delete;
    OTWallet(OTWallet&&) = delete;
    OTWallet& operator=(const OTWallet&) = delete;
    OTWallet& operator=(OTWallet&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OTWALLET_HPP
