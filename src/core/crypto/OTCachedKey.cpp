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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/OTCachedKey.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTKeyring.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#if OT_CRYPTO_USING_OPENSSL
extern "C" {
#include <openssl/opensslconf.h>
}
#endif

#include <ostream>

#define OT_DEFAULT_PASSWORD "test"

namespace opentxs
{

std::mutex OTCachedKey::s_mutexCachedKeys;
mapOfCachedKeys OTCachedKey::s_mapCachedKeys;
std::shared_ptr<OTCachedKey> OTCachedKey::singleton_;

OTCachedKey::OTCachedKey(std::int32_t nTimeoutSeconds)
{
    init(nTimeoutSeconds);
}

OTCachedKey::OTCachedKey(const OTASCIIArmor& ascCachedKey)
{
    init(
        OTCachedKey::It()->GetTimeoutSeconds(),
        OTCachedKey::It()->IsUsingSystemKeyring());

    OT_ASSERT(ascCachedKey.Exists());

    SetCachedKey(ascCachedKey);
}

OTCachedKey::OTCachedKey(const OTCachedKey& rhs)
{
    shutdown_.store(false);
    use_system_keyring_.store(rhs.use_system_keyring_.load());
    paused_.store(rhs.paused_.load());
    time_.store(std::time(nullptr));
    timeout_.store(rhs.timeout_);

    if (rhs.master_password_) {
        master_password_.reset(new OTPassword(*rhs.master_password_));
    }

    if (rhs.key_) {
        key_.reset(new OTSymmetricKey(*rhs.key_));
    }

    secret_id_ = rhs.secret_id_;
}

void OTCachedKey::init(const std::int32_t& timeout, const bool useKeyring) const
{
    shutdown_.store(false);
    paused_.store(false);
    UseSystemKeyring(useKeyring);
    timeout_.store(timeout);
}

bool OTCachedKey::IsGenerated()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (key_) {
        return key_->IsGenerated();
    }

    return false;
}

bool OTCachedKey::HasHashCheck()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (key_) {
        return key_->HasHashCheck();
    }

    return false;
}

// if you pass in a master key ID, it will look it up on an existing cached map
// of master keys.
// Otherwise it will use "the" global Master Key (the one used for the Nyms.)
//
// static
std::shared_ptr<OTCachedKey> OTCachedKey::It(Identifier* pIdentifier)
{
    // For now we're only allowing a single global instance, unless you pass in
    // an ID, in which case we keep a map.

    std::lock_guard<std::mutex> lock(OTCachedKey::s_mutexCachedKeys);

    if (!singleton_) {
        // Default is 0 ("you have to type your PW a million times"), but it's
        // overridden in config file.
        singleton_.reset(new OTCachedKey);
    }

    OT_ASSERT(singleton_);

    // Notice if you pass nullptr (no args) then it ALWAYS returns a good
    // pointer here.

    if (nullptr == pIdentifier) {

        return singleton_;
    }

    // There is a chance of failure if you pass an ID, since maybe it's not
    // already on the map. But at least by this point we know FOR SURE that
    // pIdentifier is NOT nullptr.

    const String strIdentifier(*pIdentifier);
    const std::string str_identifier(strIdentifier.Get());

    auto it_keys = s_mapCachedKeys.find(str_identifier);

    if (s_mapCachedKeys.end() != it_keys) {
        std::shared_ptr<OTCachedKey> pShared(it_keys->second);

        if (pShared) {
            return pShared;
        } else {
            s_mapCachedKeys.erase(it_keys);
        }
    }

    // else: We can't instantiate it, since we don't have the corresponding
    // CachedKey, just its Identifier. We're forced simply to return nullptr in
    // this case.
    //
    // Therefore you should normally pass in the master key (the same one that
    // you want to cache a copy of) using the below version of It(). That
    // version creates the copy, if it's not already there.

    return std::shared_ptr<OTCachedKey>();
}

// If you pass in a master key, it will look it up on an existing cached map of
// master keys, based on the ID of the master key passed in. (Where it stores
// its own cached copy of the same master key.)
// NOTE: If you use it this way, then you must NEVER use the actual master key
// being cached (such as the one stored in a password-protected purse.) Instead,
// you must always look up the cached version, and use THAT master key, instead
// of the actual one in your  OTPurse. The only time you can use your master key
// itself is when loading it (such as when OTPurse loads its internal Master
// Key.) But thereafter, use the cached version of it for all operations and for
// saving.
std::shared_ptr<OTCachedKey> OTCachedKey::It(OTCachedKey& theSourceKey)
{
    std::shared_ptr<OTCachedKey> output;

    if (!theSourceKey.IsGenerated()) {
        otErr << "OTCachedKey::" << __FUNCTION__
              << ": theSourceKey.IsGenerated() returned false. "
                 "(Returning nullptr.)"
              << std::endl;

        return output;
    }

    std::lock_guard<std::mutex> lock_keys(OTCachedKey::s_mutexCachedKeys);

    const std::string id(String(Identifier(theSourceKey)).Get());

    // Let's see if it's already there on the map...
    const auto it = s_mapCachedKeys.find(id);

    if (s_mapCachedKeys.end() != it) {
        output = it->second;

        if (output) {

            return output;
        } else {
            s_mapCachedKeys.erase(it);
        }
    }

    // Here we make a copy of the master key and insert it into the map.
    // Then we return a pointer to it.
    OTASCIIArmor ascCachedKey;

    if (theSourceKey.SerializeTo(ascCachedKey)) {
        output = std::make_shared<OTCachedKey>(OTCachedKey(ascCachedKey));
        s_mapCachedKeys[id] = output;
    } else {
        otErr << __FUNCTION__
              << ": theSourceKey.SerializeTo(ascCachedKey) failed. "
                 "Returning nullptr."
              << std::endl;
    }

    return output;
}

// static
void OTCachedKey::Cleanup()
{
    std::lock_guard<std::mutex> lock(OTCachedKey::s_mutexCachedKeys);

    s_mapCachedKeys.clear();
}

bool OTCachedKey::isPaused() const { return paused_.load(); }

// When the master key is on pause, it won't work (Nyms will just use their
// own passwords instead of the master password.) This is important, for
// example, if you are loading up a bunch of Old Nyms. You pause before and
// after each one, and THEN convert them to the master key.
bool OTCachedKey::Pause() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!paused_.load()) {
        paused_.store(false);

        return true;
    }

    return false;
}

bool OTCachedKey::Unpause() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (paused_.load()) {
        paused_.store(false);

        return true;
    }

    return false;
}

void OTCachedKey::LowLevelReleaseThread()
{
    shutdown_.store(true);

    if (thread_) {
        thread_->join();
    }
}

OTCachedKey::~OTCachedKey()
{
    LowLevelReleaseThread();
}

std::int32_t OTCachedKey::GetTimeoutSeconds() const { return timeout_.load(); }

void OTCachedKey::SetTimeoutSeconds(std::int64_t nTimeoutSeconds)
{
    OT_ASSERT_MSG(
        nTimeoutSeconds >= (-1),
        "OTCachedKey::SetTimeoutSeconds: "
        "ASSERT: nTimeoutSeconds must be >= "
        "(-1)\n");

    timeout_.store(nTimeoutSeconds);
}

// Called by OTServer or OTWallet, or whatever instantiates those.
//
void OTCachedKey::SetCachedKey(const OTASCIIArmor& ascCachedKey)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    OT_ASSERT(ascCachedKey.Exists());

    key_.reset(new OTSymmetricKey);

    OT_ASSERT(key_);

    key_->SerializeFrom(ascCachedKey);
}

// Above version deletes the internal symmetric key if it already exists,
// and then below that, creates it again if it does not exist. Then serializes
// it up from storage via ascCachedKey (input.)
// Whereas below version, if internal symmetric key doesn't exist, simply
// returns false.  Therefore if it's "not generated" and you want to load it
// up from some input, call the above function, SetCachedKey().

// Apparently SerializeFrom (as of this writing) is only used in OTEnvelope.cpp
// whereas SetCachedKey (above) is used in OTWallet and OTServer.
//
bool OTCachedKey::SerializeFrom(const OTASCIIArmor& ascInput)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (key_) {
        return key_->SerializeFrom(ascInput);
    }

    return false;
}

bool OTCachedKey::SerializeTo(OTASCIIArmor& ascOutput)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (key_) {
        return key_->SerializeTo(ascOutput);
    }

    return false;
}

// Note: this calculates its ID based only on key_,
// and does NOT include salt, IV, iteration count, etc when
// generating the hash for the ID.
bool OTCachedKey::GetIdentifier(Identifier& theIdentifier) const
{
    std::lock_guard<std::mutex> lock((const_cast<OTCachedKey*>(this))->m_Mutex);

    if (key_) {
        if (key_->IsGenerated()) {
            key_->GetIdentifier(theIdentifier);

            return true;
        }
    }

    return false;
}

bool OTCachedKey::GetIdentifier(String& strIdentifier) const
{
    Identifier id;

    if (!GetIdentifier(id)) {
        return false;
    }

    strIdentifier = String(id);

    return true;
}

/*
 // TOdo: make this so you can pass in a password, or you can pass nullptr
 // and then it will use the GetPasswordCallback() method to collect one
 // from the user.

 OT_OPENSSL_CALLBACK * OTAsymmetricKey::GetPasswordCallback()

 #define OPENSSL_CALLBACK_FUNC(name) extern "C" (name)(char* buf, std::int32_t
 size,
 std::int32_t rwflag, void* userdata)

 */

// Caller must delete!
// static
std::shared_ptr<OTCachedKey> OTCachedKey::CreateMasterPassword(
    OTPassword& theOutput,
    const char* szDisplay,
    std::int32_t nTimeoutSeconds)
{
    std::shared_ptr<OTCachedKey> pMaster(new OTCachedKey(nTimeoutSeconds));

    const String strDisplay(
        (nullptr == szDisplay)
            ? "Creating a passphrase..."
            : szDisplay);  // todo internationalization / hardcoding.

    const bool bGotPassphrase = pMaster->GetMasterPassword(
        pMaster,
        theOutput,
        strDisplay.Get(),
        true);  // bool bVerifyTwice=false by default.
                // Really we didn't have to pass true
                // here, since it asks twice anyway,
                // when first generating the key.

    if (bGotPassphrase)  // success!
        return pMaster;

    // If we're still here, that means bGotPassphrase failed.
    //
    //    delete pMaster; pMaster = nullptr;
    return std::shared_ptr<OTCachedKey>();
}

// GetMasterPassword USES the User Passphrase to decrypt the cached key
// and return a decrypted plaintext of that cached symmetric key.
// Whereas ChangeUserPassphrase CHANGES the User Passphrase that's used
// to encrypt that cached key. The cached key itself is not changed, nor
// returned. It is merely re-encrypted.
bool OTCachedKey::ChangeUserPassphrase()
{
    if (!key_) {
        otErr << __FUNCTION__ << ": The Master Key does not appear yet to "
                                 "exist. Try creating a Nym first.\n";
        return false;
    }
    // --------------------------------------------------------------------
    const String strReason1("Enter old wallet master passphrase");

    // Returns a text OTPassword, or nullptr.
    std::shared_ptr<OTPassword> pOldUserPassphrase(
        OTSymmetricKey::GetPassphraseFromUser(&strReason1));  // bool bAskTwice
                                                              // = false

    if (!pOldUserPassphrase) {
        otErr << __FUNCTION__ << ": Error: Failed while trying to get old "
                                 "passphrase from user.\n";
        return false;
    }
    // --------------------------------------------------------------------
    const String strReason2("Create new wallet master passphrase");

    // Returns a text OTPassword, or nullptr.
    std::shared_ptr<OTPassword> pNewUserPassphrase(
        OTSymmetricKey::GetPassphraseFromUser(
            &strReason2, true));  // bool bAskTwice = false by default.

    if (!pNewUserPassphrase) {
        otErr << __FUNCTION__ << ": Error: Failed while trying to get new "
                                 "passphrase from user.\n";
        return false;
    }
    // --------------------------------------------------------------------
    LowLevelReleaseThread();
    std::lock_guard<std::mutex> lock(m_Mutex);
    ResetMasterPassword(lock);

    return key_->ChangePassphrase(*pOldUserPassphrase, *pNewUserPassphrase);
}

// Called by the password callback function.
// The password callback uses this to get the password for any individual Nym.
// This will also generate the master password, if one does not already exist.
//
bool OTCachedKey::GetMasterPassword(
    std::shared_ptr<OTCachedKey>& mySharedPtr,
    OTPassword& theOutput,
    const char* szDisplay,
    __attribute__((unused)) bool bVerifyTwice)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::string str_display(
        nullptr != szDisplay ? szDisplay : "(Display string was blank.)");

    const char* szFunc = "OTCachedKey::GetMasterPassword";

    if (master_password_) {
        otInfo << szFunc
               << ": Master password was available. (Returning it now.)\n";
        theOutput = *master_password_;
        ResetTimer();

        return true;
    }

    otInfo << szFunc << ": Master password wasn't loaded. Instantiating...\n";

    // If master_password_ is null, (which below this point it is) then...
    //
    // Either it hasn't been created yet, in which case we need to instantiate
    // it, OR it expired, in which case m_pMasterPassword is nullptr,
    // but m_pThread isn't, and still needs cleaning up before we
    // instantiate another one!

    LowLevelReleaseThread();
    master_password_.reset(OT::App().Crypto().AES().InstantiateBinarySecret());

    /*
    How does this work?

    When trying to open a normal nym, the password callback realizes we are
    calling it
    in "NOT master mode", so instead of just collecting the passphrase and
    giving it
    back to OpenSSL, it calls this function first, which returns the master
    password
    (so that IT can be given to OpenSSL instead.)

    If the master wasn't already loaded (common) then we call the callback in
    here ourselves.
    Notice it's recursive! But this time, the callback sees we ARE in master
    mode, so it doesn't
    call this function again (which would be an infinite loop.) Instead, it
    collects the password
    as normal, only instead of passing it back to the caller via the buffer, it
    uses the
    passUserInput by attaching it to thePWData before the call. That way the
    callback function
    can set passUserInput with whatever it retrieved from the user, and then
    back in this function
    again we can get the passUserInput and use it to unlock the MASTER
    passphrase, which we set
    onto theOutput.

    When this function returns true, the callback (0th level of recursion) uses
    theOutput
    as the "passphrase" for all Nyms, passing it to OpenSSL.

    This way, OpenSSL gets a random key instead of a passphrase, and the
    passphrase is just used
    for encrypting that random key whenever its timer has run out.

    */

    bool bReturnVal = false;

    // CALL the callback directly. (To retrieve a passphrase so I can use it in
    // GenerateKey and GetRawKey.)
    //
    // std::int32_t OT_OPENSSL_CALLBACK (char* buf, std::int32_t size,
    // std::int32_t rwflag,
    // void *userdata);
    //
    // For us, it will set passUserInput to the password from the user, and
    // return a simple 1 or 0 (instead of the length.) buf and size can be
    // nullptr / 0, and rwflag should be passed in from somewhere.
    //
    // key_ is the encrypted form of the master key. Therefore we want to hash
    // it, in order to get the ID for lookups on the keychain.
    //
    OTPassword* pDerivedKey = nullptr;
    std::unique_ptr<OTPassword> theDerivedAngel;

    if (!key_) {
        key_.reset(new OTSymmetricKey);
    }

    OT_ASSERT(key_);

    if (!key_->IsGenerated())  // doesn't already exist.
    {
        otWarn << szFunc << ": Master key didn't exist. Need to collect a "
                            "passphrase from the user, "
                            "so we can generate a master key...\n ";

        bVerifyTwice = true;  // we force it, in this case.
    } else  // If the symmetric key itself ALREADY exists (which it usually
            // will...)
    {  // then we might have also already stashed the derived key on the system
        // keychain. Let's check there first before asking the user to enter his
        // passphrase...
        //

        pDerivedKey =
            OT::App().Crypto().AES().InstantiateBinarySecret();  // pDerivedKey
                                                                 // is
        // instantiated here to
        // use as output argument
        // below.
        //
        // *** ATTEMPT to RETRIEVE the *Derived Key* from THE SYSTEM KEYCHAIN
        // ***
        //
        const bool bFoundOnKeyring =
            IsUsingSystemKeyring() &&
            OTKeyring::RetrieveSecret(
                secret_id_,
                *pDerivedKey,  // (Output) RETRIEVED PASSWORD.
                str_display);  // optional display string.

        if (bFoundOnKeyring)  // We found it -- but does it WORK?
        {
            const bool bCachedKey =
                key_->GetRawKeyFromDerivedKey(*pDerivedKey, *master_password_);

            //
            // Note: What IS the secret? We don't want it to be the user's
            // passphrase that he TYPES.
            // We also don't want it to be the eventual (random) key that
            // unlocks the private keys.
            // Rather, we want it to be the intermediary key, generated from the
            // user's passphrase via
            // a key-derivation algorithm, which is then used to unlock the
            // (random) symmetric key that
            // actually unlocks the private keys.
            // This way the symmetric key itself can be kept locked at ALL
            // times, and instead, we have the
            // derived key on the timer, use it to unlock the symmetric key
            // EVERY TIME we use that, and
            // IMMEDIATELY throw it away afterwards, since we can still open it
            // again (until the timeout) by
            // using the derived key.
            // This is slick because the user doesn't directly enter the derived
            // key, and neither is it
            // directly used for unlocking private keys -- so it's preferable to
            // store in RAM than those things.
            //
            //
            // 1. Make sure the above description is actually what we DO do now.
            // (UPDATE: for keyring, yes. For OT internally, no.)
            // 2. Make sure the derived key, as described above, is also what is
            // stored as the SECRET, here! (UPDATE: Yes!)
            //    (i.e. in other processes such as Mac Keychain or Gnome.)
            // 3. Done. Need to add ability for OTIdentifier to hash
            // OTSymmetricKey, so we can use it for strUser above. DONE.
            //
            // UPDATE: the master key cached inside OT (on a timer) is not the
            // derived key, but the master key itself
            // that's used on the private keys. However, the one we're caching
            // in the system keyring IS the derived key,
            // and not the master key. So for example, if an attacker obtained
            // the derived key from the system keyring,
            //

            if (bCachedKey)  // It works!
            {
                otWarn << szFunc << ": Finished calling "
                                    "key_->GetRawKeyFromDerivedKey "
                                    "(Success.)\n";
                theOutput = *master_password_;  // Return it to the caller.
                theDerivedAngel.reset(
                    pDerivedKey);  // Set our own copy to be destroyed later. It
                                   // continues below as "NOT nullptr".
                bReturnVal = true;  // Success.
            } else                  // It didn't unlock with the one we found.
            {
                otOut << szFunc << ": Unable to unlock master key using "
                                   "derived key found on system keyring.\n";
                delete pDerivedKey;
                pDerivedKey = nullptr;  // Below, this function checks
                                        // pDerivedKey for nullptr.
            }
        } else  // NOT found on keyring.
        {
            if (IsUsingSystemKeyring())  // We WERE using the keying, but
                                         // we DIDN'T find the derived key.
                otWarn << szFunc
                       << ": Unable to find derived key on system keyring.\n";
            // (Otherwise if we WEREN'T using the system keyring, then of course
            // we didn't find any derived key cached there.)
            delete pDerivedKey;
            pDerivedKey = nullptr;  // Below, this function checks pDerivedKey
                                    // for nullptr.
        }
    }

    // NOT found on Keyring...
    //
    if (nullptr == pDerivedKey)  // Master key was not cached in OT, nor was it
                                 // found in the system keychain.
    {  // Therefore we HAVE to ask the user for a passphrase and decrypt it
        // ourselves,
        // since we DO have an encrypted version of the key...

        // This time we DEFINITELY force the user input, since we already played
        // our hand.
        // If the master key was still in memory we would have returned already,
        // above.
        // Then we tried to find it on the keyring and we couldn't find it, so
        // now we have
        // to actually ask the user to enter it.
        //

        std::string default_password(OT_DEFAULT_PASSWORD);  // default password
        OTPassword passwordDefault;
        passwordDefault.zeroMemory();
        passwordDefault.setPassword(
            default_password.c_str(),
            static_cast<std::int32_t>(default_password.length()));

        OTPassword passUserInput;
        passUserInput.zeroMemory();  // text mode.
        OTPasswordData thePWData(
            str_display.c_str(),
            &passUserInput,
            mySharedPtr);  // these pointers are only passed
                           // in the case where it's for a
                           // master key.
        //      otInfo << "*********Begin OTCachedKey::GetMasterPassword:
        // Calling souped-up password cb...\n * *  * *  * *  * *  * ");

        // It's possible this is the first time this is happening, and the
        // master key
        // hasn't even been generated yet. In which case, we generate it here...
        //
        bool bGenerated = key_->IsGenerated();

        if (!bGenerated)  // This Symmetric Key hasn't been generated before....
        {
#ifndef OT_NO_PASSWORD
            if (!OTAsymmetricKey::GetPasswordCallback()(
                    nullptr,
                    0,
                    bVerifyTwice ? 1 : 0,
                    static_cast<void*>(&thePWData))) {
                otErr << __FUNCTION__ << ": Failed to get password from user!";
                return false;
            }
#endif  // OT_NO_PASSWORD
            // If the length of the user supplied password is less than 4
            // characters std::int64_t, we are going to use the default
            // password!
            bool bUsingDefaultPassword = false;
            {
                if (4 > std::string(passUserInput.getPassword()).length()) {
                    otOut << "\n Password entered was less than 4 characters "
                             "int64_t! This is NOT secure!!\n"
                             "... Assuming password is for testing only... "
                             "setting to default password: "
                          << OT_DEFAULT_PASSWORD << " \n";
                    bUsingDefaultPassword = true;
                }
            }

            bGenerated = key_->GenerateKey(
                bUsingDefaultPassword ? passwordDefault : passUserInput,
                &pDerivedKey);  // derived key is optional here.
            //
            // Note: since I passed &pDerivedKey in the above call, then **I**
            // am responsible to
            // check it for nullptr, and delete it if there's something there!
            //
            if (nullptr != pDerivedKey)
                theDerivedAngel.reset(pDerivedKey);
            else
                otErr << __FUNCTION__ << ": FYI: Derived key is still nullptr "
                                         "after calling "
                                         "OTSymmetricKey::GenerateKey.\n";

        } else  // key_->IsGenerated() == true. (Symmetric Key is
                // already generated.)
        {

            // Generate derived key from passphrase.
            //
            // We generate the derived key here so that
            // GetRawKeyFromPassphrase() call (below)
            // works with it being passed in. (Because the above call to
            // GenerateKey also grabs
            // a copy of the derived key and passes it in below to the same
            // GetRawKeyFromPassphrase.)
            //
            // So WHY are we keeping a copy of the derived key through these
            // calls? Otherwise they
            // would all individually generate it, which is a waste of
            // resources. Also, we want to have
            // our grubby hands on the derived key at the end so we can add it
            // to the system keyring
            // (below), and we'd just end up having to derive it AGAIN in order
            // to do so.
            //
            if (key_->HasHashCheck()) {
                pDerivedKey = key_->CalculateDerivedKeyFromPassphrase(
                    passwordDefault);  // asserts already.

                if (nullptr == pDerivedKey) {
                    otOut << "\n\n"
                          << __FUNCTION__
                          << ": Please enter your password.\n\n";

                    for (;;)  // bad passphase (as the calculate key returned
                              // nullptr)
                    {
                        if (!OTAsymmetricKey::GetPasswordCallback()(
                                nullptr,
                                0,
                                false,
                                static_cast<void*>(&thePWData))) {
                            otErr << "\n\n"
                                  << __FUNCTION__
                                  << ": Failed to get password from user!\n\n";
                            return false;
                        }
                        pDerivedKey = key_->CalculateDerivedKeyFromPassphrase(
                            passUserInput);                 // asserts already.
                        if (nullptr != pDerivedKey) break;  // success

                        otOut << "\n\n"
                              << __FUNCTION__
                              << ": Wrong Password, Please Try Again.\n\n";
                    }
                }
            } else {
                otOut << "\n Please enter your current password twice, (not a "
                         "new password!!) \n";

                if (!OTAsymmetricKey::GetPasswordCallback()(
                        nullptr, 0, true, static_cast<void*>(&thePWData))) {
                    otErr << __FUNCTION__
                          << ": Failed to get password from user!";
                    return false;
                }

                pDerivedKey = key_->CalculateNewDerivedKeyFromPassphrase(
                    passUserInput);  // asserts already.
                OT_ASSERT(nullptr != pDerivedKey);
            }
            theDerivedAngel.reset(pDerivedKey);

            otWarn << szFunc << ": FYI, symmetric key was already generated. "
                                "Proceeding to try and use it...\n";

            // bGenerated is true, if we're even in this block in the first
            // place.
            // (No need to set it twice.)
        }

        // Below this point, pDerivedKey could still be null.
        // (And we only clean it up later if we created it.)
        // Also, bGenerated could still be false. (Like if it wasn't
        // generated, then generation itself failed, then it's still false.)
        //
        // Also, even if it was already generated, or if it wasn't but then
        // successfully did,
        //

        if (bGenerated)  // If SymmetricKey (*this) is already generated.
        {
            otInfo << szFunc
                   << ": Calling key_->GetRawKeyFromPassphrase()...\n";

            // Once we have the user's password, then we use it to GetKey from
            // the OTSymmetricKey (which
            // is encrypted) and that retrieves the cleartext master password
            // which we set here and also
            // return a copy of.
            //
            // Note: if pDerivedKey was derived above already, which it should
            // have been, then it will
            // be not-nullptr, and will be used here, and will be used
            // subsequently
            // for adding to the system
            // keychain. Otherwise, it will be nullptr, and
            // GetRawKeyFromPassphrase
            // will thus just derive its
            // own copy of the derived key internally. It will still work, but
            // then back up here, it will
            // NOT be added to the system keyring, since it's still nullptr back
            // up
            // here.
            // (FYI.)
            //
            const bool bCachedKey = key_->GetRawKeyFromPassphrase(
                passUserInput, *master_password_, pDerivedKey);
            if (bCachedKey) {
                otInfo << szFunc << ": Finished calling "
                                    "key_->GetRawKeyFromPassphrase "
                                    "(Success.)\n";
                theOutput = *master_password_;  // Success!

                // Store the derived key to the system keyring.
                //
                if (IsUsingSystemKeyring() && (nullptr != pDerivedKey)) {

                    secret_id_ = String(Identifier(*key_));
                    OTKeyring::StoreSecret(
                        secret_id_,
                        *pDerivedKey,  // (Input) Derived Key BEING STORED.
                        str_display);  // optional display string.
                } else
                    otWarn << szFunc << ": Strange: Problem with either: "
                                        "IsUsingSystemKeyring"
                                        " ("
                           << (IsUsingSystemKeyring() ? "true" : "false")
                           << ") "
                              "or: (nullptr != pDerivedKey) ("
                           << ((nullptr != pDerivedKey) ? "true" : "false")
                           << ")\n";

                bReturnVal = true;
            } else
                otOut << szFunc
                      << ": key_->GetRawKeyFromPassphrase() failed.\n";
        }  // bGenerated
        else
            otErr << szFunc << ": bGenerated is still false, even after trying "
                               "to generate it, yadda yadda yadda.\n";

    }  // nullptr == pDerivedKey

    if (bReturnVal)  // Start the thread!
    {
        otInfo << szFunc << ": Starting thread for Master Key...\n";
        ResetTimer();
        shutdown_.store(false);
        thread_.reset(new std::thread(&OTCachedKey::ThreadTimeout, this));

    } else if (GetTimeoutSeconds() != (-1)) {
        master_password_.reset();
    }
    // Since we have set the cleartext master password, We also have to fire up
    // the thread
    // so it can timeout and be destroyed. In the meantime, it'll be stored in
    // an OTPassword
    // which has these security precautions:
    /*
    1. Zeros memory in a secure and cross-platform way, in its destructor.
    2. OT_Init() uses setrlimit to prevent core dumps.
    3. Uses VirtualLock and mlock to reduce/prevent swapping RAM to hard drive.
    4. (SOON) will use VirtualProtect on Windows (standard API for protected
    memory)
    5. (SOON) and similarly have option in config file for ssh-agent, gpg-agent,
    etc.
    6. Even without those things,the master password is stored in an encrypted
    form after it times out.
    7. While decrypted (while timer is going) it's still got the above security
    mechanisms,
    plus options for standard protected-memory APIs are made available wherever
    possible.
    8. The actual passphrase the user types is not stored in memory, except just
    std::int64_t enough to
    use it to derive another key, used to unlock the actual key (for a temporary
    period of time.)
    9. Meanwhile the actual key is stored in encrypted form on disk, and the
    derived key isn't stored anywhere.
    10. Ultimately external hardware, and smart cards, are the way to go. But OT
    should still do the best possible.
    */

    return bReturnVal;
}

void OTCachedKey::ThreadTimeout()
{
    while (!shutdown_.load()) {
        const auto limit = std::chrono::seconds(GetTimeoutSeconds());
        const auto now = std::chrono::seconds(std::time(nullptr));
        const auto last = std::chrono::seconds(time_.load());
        const auto duration = now - last;

        if (limit >= std::chrono::seconds(0)) {
            if (duration > limit) {
                if (GetTimeoutSeconds() != (-1)) {
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    master_password_.reset();
                    lock.unlock();
                }

            }
        }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    master_password_.reset();

    if (IsUsingSystemKeyring()) {
        OTKeyring::DeleteSecret(secret_id_, "");
    }
}

// If you actually want to create a new key, and a new passphrase, then use this
// to destroy every last vestige of the old one. (Which will cause a new one to
// be automatically generated the next time OT requests the master key.) NOTE:
// Make SURE you have all your Nyms loaded up and unlocked before you call this.
// Then save them all again so they will be properly stored with the new master
// key.
void OTCachedKey::ResetMasterPassword(const std::lock_guard<std::mutex>&)
{
    master_password_.reset();

    if (key_) {
        if (IsUsingSystemKeyring()) {
            OTKeyring::DeleteSecret(secret_id_, "");
        }

        key_.reset();
    }
}

void OTCachedKey::ResetTimer() const { time_.store(std::time(nullptr)); }

bool OTCachedKey::IsUsingSystemKeyring() const
{
    return use_system_keyring_.load();
}

void OTCachedKey::UseSystemKeyring(const bool bUsing) const
{
    use_system_keyring_.store(bUsing);
}
}  // namespace opentxs
