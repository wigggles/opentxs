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

#ifndef OPENTXS_CORE_CRYPTO_OTKEYRING_HPP
#define OPENTXS_CORE_CRYPTO_OTKEYRING_HPP

#include "opentxs/Version.hpp"

#include <string>

// It's better to turn-on one of these, by using the Makefile,
// instead of hardcoding it here, which is entirely unnecessary.
//
// Nevertheless, I have added this comment anyway, in case that's
// what you want to do (for testing or whatever.)
//
//#define OT_KEYRING_WINDOWS  1
//#define OT_KEYRING_MAC      1
//#define OT_KEYRING_GNOME    1
//#define OT_KEYRING_KWALLET  1
//#define OT_KEYRING_FLATFILE 1

namespace opentxs
{

class String;
class OTPassword;

class OTKeyring
{
public:
    // NOTE: Normally the "username" in our context is related to the
    // master key. OTCachedKey will call OTKeyring::RetrieveSecret, and
    // will pass in probably a hash of the encrypted master key as the
    // "username", and will retrieve the cleartext master key as the
    // "password" (which it then uses as the password for all private
    // keys in the wallet.)
    //
    // OT *already* caches its master key inside the OTCachedKey object
    // itself, for X minutes. So then, why have this additional step,
    // where we store it using whatever is the standard API for
    // "protected memory" on the user's computer? The answer is, so that
    // we can cache the password BETWEEN runs of the OT engine, and not
    // just DURING the CURRENT run. For example, only this sort of
    // caching makes it possible to set the user free from still having
    // to enter his password for every run of an OT script.
    //
    // I don't even know if I recommend using this OTKeyring, since I
    // don't necessarily trust the operating system's protected memory,
    // and since the password is vulnerable during the time it is cached
    // by the OS, since any other process ALSO running as the same user
    // is able to ask the OS what the cached value is.
    //
    // (So be careful what software you run on your computer...)
    // In fact, perhaps OT should ONLY run as a certain user, which is
    // not used for anything else. Have it be setuid. That way, even if
    // a hacker can trick you into running his trojan as your own user,
    // he will be unable to query the OS for your master key, since your
    // user is not what was used to cache the PW in the first place (a
    // special "OT user" being used instead.) And thus cannot be used to
    // retrieve it, either.
    //
    // INTERFACE:
    //
    EXPORT static bool StoreSecret(
        const String& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    EXPORT static bool RetrieveSecret(
        const String& strUser,
        OTPassword& thePassword,
        const std::string& str_display);

    EXPORT static bool DeleteSecret(
        const String& strUser,
        const std::string& str_display);

private:
#if defined(OT_KEYRING_WINDOWS) && defined(_WIN32)
    EXPORT static bool Windows_StoreSecret(
        const OTString& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    EXPORT static bool Windows_RetrieveSecret(
        const OTString& strUser,
        OTPassword& thePassword,
        const std::string& str_display);

    EXPORT static bool Windows_DeleteSecret(
        const OTString& strUser,
        const std::string& str_display);
//#endif
#elif defined(OT_KEYRING_MAC) && defined(__APPLE__)
    static bool Mac_StoreSecret(
        const OTString& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    static bool Mac_RetrieveSecret(
        const OTString& strUser,
        OTPassword& thePassword,
        const std::string& str_display);

    static bool Mac_DeleteSecret(
        const OTString& strUser,
        const std::string& str_display);
//#endif
#elif defined(OT_KEYRING_IOS) && defined(__APPLE__)
    static bool IOS_StoreSecret(
        const OTString& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    static bool IOS_RetrieveSecret(
        const OTString& strUser,
        OTPassword& thePassword,
        const std::string& str_display);

    static bool IOS_DeleteSecret(
        const OTString& strUser,
        const std::string& str_display);
//#endif
#elif defined(OT_KEYRING_GNOME)
    static bool Gnome_StoreSecret(
        const OTString& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    static bool Gnome_RetrieveSecret(
        const OTString& strUser,
        OTPassword& thePassword,
        const std::string& str_display);  // unused

    static bool Gnome_DeleteSecret(
        const OTString& strUser,
        const std::string& str_display);  // unused
                                          //#endif
#elif defined(OT_KEYRING_KWALLET)
    static KWallet::Wallet* s_pWallet;
    static KApplication* s_pApp;
    static bool KWallet_StoreSecret(
        const OTString& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    static bool KWallet_RetrieveSecret(
        const OTString& strUser,
        OTPassword& thePassword,
        const std::string& str_display);

    static bool KWallet_DeleteSecret(
        const OTString& strUser,
        const std::string& str_display);
#elif defined(OT_KEYRING_FLATFILE)  // Do not use! Unsafe! For testing only!
    static bool FlatFile_StoreSecret(
        const String& strUser,
        const OTPassword& thePassword,
        const std::string& str_display);

    static bool FlatFile_RetrieveSecret(
        const String& strUser,
        OTPassword& thePassword,
        const std::string& str_display);

    static bool FlatFile_DeleteSecret(
        const String& strUser,
        const std::string& str_display);

public:
    EXPORT static void FlatFile_SetPasswordFolder(std::string folder);
    EXPORT static const char* FlatFile_GetPasswordFolder();

private:
    EXPORT static std::string s_str_passwd_folder;  // NOTE: Do not ever use
                                                    // this. OT_KEYRING_FLATFILE
                                                    // should NEVER be defined!
                                                    // No! For testing only.
#endif
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTKEYRING_HPP
