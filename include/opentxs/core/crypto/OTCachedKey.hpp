// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTCACHEDKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTCACHEDKEY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace opentxs
{
namespace api
{
namespace implementation
{
class Crypto;
}  // namespace implementation
}  // namespace api

/**
This class handles the functionality of caching the master key for X seconds as
an OTPassword, and then deleting it. It also caches the encrypted version in an
crypto::key::LegacySymmetric, which can be unlocked to an OTPassword again for X
more seconds (by entering the passphrase...)

How does OTCachedKey work, in a nutshell?

 -- It's a singleton. There's only one "master key" for the application and it's
available globally like this: OTCachedKey::It(). For example:
OTCachedKey::It()->IsGenerated()
 -- m_pSymmetricKey contains a symmetric key in ENCRYPTED form, which loads up
from the wallet, and which is always kept in RAM (as on the hard drive) ONLY in
encrypted form.
 -- m_pMasterPassword is the DECRYPTED version of m_pSymmetricKey. When a user
enters his password, a key derivation hash is used to transform it, and the
result is used to get an unlocked copy of m_pSymmetricKey, putting the cleartext
version into m_pMasterPassword.
 -- Why store a decrypted version in memory? So we can set a timer using another
thread, and thereby return X seconds later, and destroy that decrypted version.
In the meantime, until it is destroyed, it will be used automatically by OT,
every time a password is needed until those X seconds are over.
 -- That's still not good enough: the decrypted version should still be stored
in protected memory, using some standard API such as gpg-agent or Windows
VirtualProtect. And whatever is stored there, should ALSO be encrypted to a
session key which is re-generated for every run of the application. Furthermore,
the decrypted version being stored isn't the user's actual passphrase, nor is it
the key derived from it via some password hash algorithm, but instead is a
random number that was generated in order to be used as the master key.
 -- This means that the actual password the user enters, is never stored
anywhere. Neither is the key derived from that password, since both can
instantly be erased after using them to unlock the master key. Furthermore the
master key is encrypted to a temporary session key, which is different for every
run of the application, so it is never stored cleartext in memory, either, nor
is it thus available to anyone with backdoors into protected memory.
 -- Not only does the session key change after each run of the application, but
it is possible to replace the master key without forcing a change of the
password itself. Likewise it is also possible to change the password, without
changing the underlying key that it opens. Adding the session key will further
ensure that neither password nor master key ever appears in RAM (for very
std::int64_t) nor on the harddrive in cleartext form.
 -- Adding the use of a standard protected memory API such as gpg-agent means
there is no risk of swapping or core dumps revealing vital information from
within OT itself. Nevertheless, the OTPassword class takes pains to prevent core
dumps and swapping, and also uses tricks to zero sensitive memory after it has
been used.
 -- The protected memory API has no access to what it is storing, due to the
session key. I will make this configurable so that users can choose their own
authentication mechanisms.
 -- Since a thread is used to implement a timer for wiping the master key after
X seconds, a mutex appears in the class below, which must be locked by any
functions in order to use the master key. (They do this behind the scenes
automatically.)
 -- An OT operation will behave in this way:  The user tries to withdraw cash,
and thus must sign a withdrawal request. An OpenSSL call is used to sign the
request, and the OT password callback is passed to the OpenSSL call, which
OpenSSL will call in the event that it needs a password. When that happens, OT
will, instead of popping up the password dialog to the user, will notice that
the master key password has been decrypted already and is within the X seconds
window, so it will use that password to unlock the master key, which it passes
to OpenSSL instead of asking the user to enter his passphrase. If the timer had
already been expired by that time, then the master key password would have been
destroyed already, meaning OT has no way to open the master key, so OT would
then pop up the passphrase dialog and ask the user to enter his passphrase, use
that to derive a key, and use that to unlock the master key password, and then
use that to unlock the master key and pass that back to OpenSSL via the callback
function.
 -- OT might do several operations within a SINGLE USE CASE. For example, for a
cash withdrawal, OT might sign the transaction item, sign the balance agreement
item, sign the main transaction containing those items, AND sign the message
containing that transaction. Perhaps OT has to use that private key 4 or 5 times
in a row, all within a fraction of a second. Should the user therefore have to
enter his passphrase 4 or 5 times, for a cash withdrawal? Isn't once enough? But
it's not that simple, because each operation opens the master key just
std::int64_t
enough for OpenSSL to perform the requested operation, and then destroys it
again immediately after the operation is completed. (Even when the timeout is
active, what's being stored isn't the master key itself, but the master key
password used to open it JUST LONG ENOUGH to use it, and then destroy it again.)
The critical point to understand is that, especially with the introduction of
the session key, EVEN though a timeout is happening, the master key itself is
STILL destroyed after each operation, even 4 or 5 times in a row really fast,
for some single use case transaction to occur.
 -- If you set your timeout to -1, then the master key passphrase will never
expire. But if your timeout is set to 0, then you will have to type your
password 4 or 5 times for a single transaction. Get it? If you set it to 300
(seconds) then you will only have to type your passphrase once and then your
application will work for 5 minutes without having to type it again, not even
once more, until that 5 minutes are up, and then you will definitely have to
type that password again before doing another operation.
 -- This is an area where much can be gained through the study of code
obfuscation, randomizing memory locations, zeroing memory after use, using
protected memory APIs, using session keys, using outsourced authentication
systems such as ssh-agent instead of handling it internally, stack smashing
techniques such as canaries, etc etc. This is always a moving target. */

// This is only the hard-coded default; it's also configurable in the opt file.
#define OT_MASTER_KEY_TIMEOUT 300

class OTCachedKey
{
public:
    EXPORT static std::shared_ptr<OTCachedKey> CreateMasterPassword(
        const api::Core& api,
        OTPassword& theOutput,
        const char* szDisplay = nullptr,
        std::int32_t nTimeoutSeconds = OT_MASTER_KEY_TIMEOUT);

    EXPORT explicit OTCachedKey(
        const api::Core& api,
        const Armored& ascCachedKey);
    EXPORT bool GetIdentifier(Identifier& theIdentifier) const;
    EXPORT bool GetIdentifier(String& strIdentifier) const;
    /** For Nyms, which have a global master key serving as their "passphrase"
     * (for that wallet), The password callback uses OTCachedKey::It() to get
     * the instance, and then GetMasterPassword to get the passphrase for any
     * individual Nym. Otherwise, OTCachedKey::It(crypto::key::LegacySymmetric
     * *) looks up a cached master key based on the ID of the key passed in. For
     * example, OTPurse has a symmetric key and master key (optionally, vs using
     * a Nym.) The symmetric key contains the actual key for the tokens, and the
     * master key is used for the passphrase, which may be cached, or may have
     * timed out, and then re-retrieved from the user (either way). The purse,
     * rather than using the global master key to get the passphrase, (which
     * _would_ happen if the purse is encrypted to a nym) will instead use its
     * own internal master key to get its passphrase (also retrieving from the
     * user if necessary.) */
    EXPORT bool GetMasterPassword(
        const OTCachedKey& passwordPassword,
        OTPassword& theOutput,
        const char* szDisplay = nullptr,
        bool bVerifyTwice = false) const;
    EXPORT bool HasHashCheck() const;
    EXPORT bool IsGenerated() const;
    EXPORT bool isPaused() const;
    EXPORT bool IsUsingSystemKeyring() const;
    EXPORT bool SerializeTo(Armored& ascOutput) const;

    /** GetMasterPassword USES the User Passphrase to decrypt the cached key and
     * return a decrypted plaintext of that cached symmetric key. Whereas
     * ChangeUserPassphrase CHANGES the User Passphrase that's used to encrypt
     * that cached key. The cached key itself is not changed, nor returned. It
     * is merely re-encrypted. */
    EXPORT bool ChangeUserPassphrase();
    EXPORT bool Pause();
    EXPORT void Reset();
    EXPORT bool SerializeFrom(const Armored& ascInput);
    /* These two functions are used by the Server or OTWallet that
     * actually keeps the master key. The owner sets the master key pointer on
     * initialization, and then later when the password callback code in
     * crypto::key::Asymmetric needs to access the master key, it can use
     * GetMasterPassword to access it. */
    EXPORT void SetCachedKey(const Armored& ascCachedKey);
    EXPORT void SetTimeoutSeconds(const std::int64_t nTimeoutSeconds);
    EXPORT void UseSystemKeyring(const bool bUsing = true);
    EXPORT bool Unpause();

    EXPORT ~OTCachedKey();

private:
    friend class api::implementation::Crypto;

    const api::Core& api_;
    mutable std::mutex general_lock_;
    mutable std::mutex master_password_lock_;
    mutable OTFlag shutdown_;
    /** if set to true, then additionally use the local OS's standard API for
     * storing/retrieving secrets. (Store the master key here whenever it's
     * decrypted, and try to retrieve from here whenever it's needed, before
     * resorting to asking the user to type his passphrase.) This is
     * configurable in the config file. */
    mutable OTFlag use_system_keyring_;
    /** If you want to force the old system, PAUSE the master key (REMEMBER to
     * Unpause when done!) */
    mutable OTFlag paused_;
    mutable OTFlag thread_exited_;
    mutable std::atomic<std::time_t> time_{0};
    /** The master password will be stored internally for X seconds, and then
     * destroyed. */
    mutable std::atomic<std::int64_t> timeout_{0};
    /** The thread used for destroying the password after the timeout period. */
    mutable std::unique_ptr<std::thread> thread_;
    /** Created when password is passed in; destroyed by Timer after X seconds.
     */
    mutable std::unique_ptr<OTPassword> master_password_;
    /** Encrypted form of the master key. Serialized by OTWallet or Server. */
    mutable OTLegacySymmetricKey key_;
    mutable OTString secret_id_;

    void release_thread() const;
    /** The cleartext version (m_pMasterPassword) is deleted and set nullptr
     * after a Timer of X seconds. (Timer thread calls this.) The INSTANCE that
     * owns the thread also passes a pointer to ITSELF. (So we can access
     * password, mutex, timeout value, etc.) This function calls
     * DestroyMasterPassword. */
    void reset_timer(const Lock& lock) const;
    void timeout_thread() const;

    void reset_master_password();

    explicit OTCachedKey(
        const api::Core& api,
        const std::int32_t nTimeoutSeconds = OT_MASTER_KEY_TIMEOUT);
    OTCachedKey() = delete;
    OTCachedKey(const OTCachedKey&) = delete;
    OTCachedKey(OTCachedKey&&) = delete;
    OTCachedKey& operator=(const OTCachedKey&) = delete;
    OTCachedKey& operator=(OTCachedKey&&) = delete;
};
}  // namespace opentxs
#endif
