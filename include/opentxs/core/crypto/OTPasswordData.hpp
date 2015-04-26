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

#ifndef OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP
#define OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP

#include <string>
#include <memory>

namespace opentxs
{

class OTCachedKey;
class OTPassword;
class String;

/*
 To use:

 OTPassword thePass;
 (Or...)
 OTPassword thePass(strPassword, strPassword.length());

 const char * szPassword    = thePass.getPassword();
 const int32_t    nPassLength    = thePass.getPasswordSize();

 If the instance of OTPassword is not going to be destroyed immediately
 after the password is used, then make sure to call zeroMemory() after
 using the password. (Otherwise the destructor will handle this anyway.)

 (The primary purpose of this class is that it zeros its memory out when
 it is destructed.)

 This class gives me a safe way to hand-off a password, and off-load the
 handling risk to the user.  This class will be included as part of the
 OT-API SWIG interface so that it's available inside other languages.

 */

#define OT_PW_DISPLAY "Enter master passphrase for wallet."

#define OTPASSWORD_BLOCKSIZE 128 // (128 bytes max length for a password.)
#define OTPASSWORD_MEMSIZE 129   // +1 for null terminator.

// UPDATE: Increasing the size here, so we can accommodate private keys (in
// addition to passphrases.)
#define OT_LARGE_BLOCKSIZE 32767 // (32767 bytes max length for a password.)
#define OT_LARGE_MEMSIZE 32768   // +1 for null terminator.

// Default is the smaller size.
#define OT_DEFAULT_BLOCKSIZE 128
#define OT_DEFAULT_MEMSIZE 129

// https://github.com/lorf/keepassx/blob/master/src/lib/SecString.cpp

// Done:  Although we have good memory ZEROING code (for destruction)
// we don't have code yet that will keep the contents SECURE while they
// are in memory. For example, that will prevent them from being paged
// to the hard drive during swapping. Such code would make OTPassword much
// more appropriate for use cases such as storing passphrases and private
// keys, and would even allow timeout procedures...
//
// NOTE: For Windows, use VirtualLock instead of mlock.
//

/*
 #include <sys/mman.h>

 void *locking_alloc(size_t numbytes)
 {
    static short have_warned = 0;

    void *mem = malloc(numbytes);

    if (mlock(mem, numbytes) && !have_warned)
    {

        // We probably do not have permission.
        // Sometimes, it might not be possible to lock enough memory.

        fprintf(stderr, "Warning: Using insecure memory!\n");

        have_warned = 1;

    }

    return mem;
 }

The mlock() call generally locks more memory than you want. Locking is done on a
per-page basis. All of the pages the memory spans will be locked in RAM, and
will not be swapped out under any circumstances, until the process unlocks
something in the same page by using mlock().

There are some potentially negative consequences here. First, If your process
locks two buffers that happen to live on the same page, then unlocking either
one will unlock the entire page, causing both buffers to unlock. Second, when
locking lots of data, it is easy to lock more pages than necessary (the
operating system doesn't move data around once it has been allocated), which can
slow down machine performance significantly.

Unlocking a chunk of memory looks exactly the same as locking it, except that
you call munlock():
        munlock(mem, numbytes);


 // TODO: Work in some usage of CryptProtectMemory and CryptUnprotectMemory
(Windows only)
 // with sample code below.  Also should make some kind of UNIX version.


#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <cstdio>
#include <Wincrypt.h>

#define SSN_STR_LEN 12  // includes null

void main()
{
    HRESULT hr = S_OK;
    LPWSTR pSensitiveText = nullptr;
    DWORD cbSensitiveText = 0;
    DWORD cbPlainText = SSN_STR_LEN*sizeof(WCHAR);
    DWORD dwMod = 0;

    //  Memory to encrypt must be a multiple of CRYPTPROTECTMEMORY_BLOCK_SIZE.
    if (dwMod = cbPlainText % CRYPTPROTECTMEMORY_BLOCK_SIZE)
        cbSensitiveText = cbPlainText +
        (CRYPTPROTECTMEMORY_BLOCK_SIZE - dwMod);
    else
        cbSensitiveText = cbPlainText;

    pSensitiveText = (LPWSTR)LocalAlloc(LPTR, cbSensitiveText);
    if (nullptr == pSensitiveText)
    {
        wprintf(L"Memory allocation failed.\n");
        return E_OUTOFMEMORY;
    }

    //  Place sensitive string to encrypt in pSensitiveText.

    if (!CryptProtectMemory(pSensitiveText, cbSensitiveText,
        CRYPTPROTECTMEMORY_SAME_PROCESS))
    {
        wprintf(L"CryptProtectMemory failed: %d\n", GetLastError());
        SecureZeroMemory(pSensitiveText, cbSensitiveText);
        LocalFree(pSensitiveText);
        pSensitiveText = nullptr;
        return E_FAIL;
    }

    //  Call CryptUnprotectMemory to decrypt and use the memory.

    SecureZeroMemory(pSensitiveText, cbSensitiveText);
    LocalFree(pSensitiveText);
    pSensitiveText = nullptr;

    return hr;
}


#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <cstdio>
#include <Wincrypt.h>
#include <strsafe.h>

#pragma comment(lib, "crypt32.lib")

void main()
{
    LPWSTR pEncryptedText;  // contains the encrypted text
    DWORD cbEncryptedText;  // number of bytes to which
                            // pEncryptedText points

    if (CryptUnprotectMemory(pEncryptedText, cbEncryptedText,
        CRYPTPROTECTMEMORY_SAME_PROCESS))
    {
        // Use the decrypted string.
    }
    else
    {
        wprintf(L"CryptUnprotectMemory failed: %d\n",
            GetLastError());
    }

    // Clear and free memory after using
    // the decrypted string or if an error occurs.
    SecureZeroMemory(pEncryptedText, cbEncryptedText);
    LocalFree(pEncryptedText);
    pEncryptedText = nullptr;
}


 */

/*
 OTPasswordData
 This class is used for passing user data to the password callback.
 Whenever actually doing some OpenSSL call that involves a private key,
 just instantiate one of these and pass its address as the userdata for
 the OpenSSL call.  Then when the OT password callback is activated by
 OpenSSL, that pointer will be passed into the callback, so the user string
 can be displayed on the password dialog. (And also so the callback knows
 whether it was activated for a normal key or for a master key.) If it was
 activated for a normal key, then it will use the cached master key, or
 if that's timed out then it will try to decrypt a copy of it using the
 master Nym. Whereas if it WAS activated for the Master Nym, then it will
 just pop up the passphrase dialog and get his passphrase, and use that to
 decrypt the master key.

 NOTE: For internationalization later, we can add an OTPasswordData constructor
 that takes a STRING CODE instead of an actual string. We can use an enum for
 this. Then we just pass the code there, instead of the string itself, and
 the class will do the work of looking up the actual string based on that code.
 */
class OTPasswordData
{
private:
    OTPassword* m_pMasterPW; // Used only when isForCachedKey is true, for
                             // output. Points to output value from original
                             // caller (not owned.)
    const std::string m_strDisplay;
    bool m_bUsingOldSystem; // "Do NOT use CachedKey if this is true."

    // If m_pMasterPW is set, this must be set as well.
    std::shared_ptr<OTCachedKey> m_pCachedKey;

public:
    EXPORT bool isForNormalNym() const;
    EXPORT bool isForCachedKey() const;
    EXPORT const char* GetDisplayString() const;
    EXPORT bool isUsingOldSystem() const;
    EXPORT void setUsingOldSystem(bool bUsing = true);
    OTPassword* GetMasterPW() const
    {
        return m_pMasterPW;
    }
    std::shared_ptr<OTCachedKey> GetCachedKey() const
    {
        return m_pCachedKey;
    }
    EXPORT OTPasswordData(const char* szDisplay,
                          OTPassword* pMasterPW = nullptr,
                          std::shared_ptr<OTCachedKey> pCachedKey =
                              std::shared_ptr<OTCachedKey>());
    EXPORT OTPasswordData(const std::string& str_Display,
                          OTPassword* pMasterPW = nullptr,
                          std::shared_ptr<OTCachedKey> pCachedKey =
                              std::shared_ptr<OTCachedKey>());
    EXPORT OTPasswordData(const String& strDisplay,
                          OTPassword* pMasterPW = nullptr,
                          std::shared_ptr<OTCachedKey> pCachedKey =
                              std::shared_ptr<OTCachedKey>());
    EXPORT ~OTPasswordData();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP
