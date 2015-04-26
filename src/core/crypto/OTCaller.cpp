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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/OTCaller.hpp>
#include <opentxs/core/crypto/OTCallback.hpp>
#include <opentxs/core/Log.hpp>

namespace opentxs
{

OTCaller::~OTCaller()
{
    otOut << "OTCaller::~OTCaller: (This should only happen as the application "
             "is closing.)\n";

    delCallback();
}

// A display string is set here before the Java dialog is shown, so that the
// string can be displayed on that dialog.
//
const char* OTCaller::GetDisplay() const
{
    // I'm using the OTPassword class to store the display string, in addition
    // to
    // storing the password itself. (For convenience.)
    //
    return reinterpret_cast<const char*>(m_Display.getPassword_uint8());
}

// A display string is set here before the Java dialog is shown, so that the
// string can be displayed on that dialog.
//
void OTCaller::SetDisplay(const char* szDisplay, int32_t nLength)
{
    // I'm using the OTPassword class to store the display string, in addition
    // to
    // storing the password itself. (For convenience.)
    //
    m_Display.setPassword_uint8(reinterpret_cast<const uint8_t*>(szDisplay),
                                nLength);
}

// The password will be stored here by the Java dialog, so that the C callback
// can retrieve it and pass it to OpenSSL
//
bool OTCaller::GetPassword(OTPassword& theOutput) const // Get the password....
{
    otOut << "OTCaller::GetPassword: FYI, returning password after invoking a "
             "(probably Java) password dialog.\n";

    theOutput.setPassword_uint8(m_Password.getPassword_uint8(),
                                m_Password.getPasswordSize());

    return true;
}

void OTCaller::ZeroOutPassword() // Then ZERO IT OUT so copies aren't floating
                                 // around.
{
    if (m_Password.getPasswordSize() > 0) m_Password.zeroMemory();
}

void OTCaller::delCallback()
{
    //    if (nullptr != _callback)  // TODO this may be a memory leak.
    //        delete _callback;    // But I know we're currently crashing from
    // deleting same object twice.
    // And since the object comes from Java, who am I to delete it? Let Java
    // clean it up.
    if (isCallbackSet())
        otOut << "OTCaller::delCallback: WARNING: setting existing callback "
                 "object pointer to nullptr. "
                 "(This message doesn't trigger if it was already nullptr.)\n";

    _callback = nullptr;
}

void OTCaller::setCallback(OTCallback* cb)
{
    otOut << "OTCaller::setCallback: Attempting to set the password OTCallback "
             "pointer...\n";

    if (nullptr == cb) {
        otOut << "OTCaller::setCallback: ERROR: nullptr password OTCallback "
                 "object passed in. (Returning.)\n";
        return;
    }

    delCallback(); // Sets _callback to nullptr, but LOGS first, if it was
                   // already
                   // set.

    _callback = cb;
    otOut << "OTCaller::setCallback: FYI, the password OTCallback pointer was "
             "set.\n";
}

bool OTCaller::isCallbackSet() const
{
    return (nullptr == _callback) ? false : true;
}

void OTCaller::callOne()
{
    ZeroOutPassword(); // Make sure there isn't some old password still in here.

    if (isCallbackSet()) {
        otOut
            << "OTCaller::callOne: FYI, Executing password callback (one)...\n";
        _callback->runOne(GetDisplay(), m_Password);
    }
    else {
        otOut << "OTCaller::callOne: WARNING: Failed attempt to trigger "
                 "password callback (one), due to \"it hasn't been set "
                 "yet.\"\n";
    }
}

void OTCaller::callTwo()
{
    ZeroOutPassword(); // Make sure there isn't some old password still in here.

    if (isCallbackSet()) {
        otOut
            << "OTCaller::callTwo: FYI, Executing password callback (two)...\n";
        _callback->runTwo(GetDisplay(), m_Password);
    }
    else {
        otOut << "OTCaller::callTwo: WARNING: Failed attempt to trigger "
                 "password callback (two), due to \"it hasn't been set "
                 "yet.\"\n";
    }
}

/*
 WCHAR szPassword[MAX_PATH];

 // Retrieve the password
 if (GetPasswordFromUser(szPassword, MAX_PATH))

 UsePassword(szPassword); // <===========

 // WINDOWS MEMORY ZEROING CODE:
 SecureZeroMemory(szPassword, sizeof(szPassword));

 */

/*
 SOURCE: https://www.securecoding.cert.org
 TODO security: research all of these items and implement them in OT properly
along with all other code scanning and security measures.

 https://www.securecoding.cert.org/confluence/display/cplusplus/MSC06-CPP.+Be+aware+of+compiler+optimization+when+dealing+with+sensitive+data


 Compliant Code Example (Windows)
 This compliant solution uses a SecureZeroMemory() function provided by many
versions of the Microsoft Visual Studio compiler.
 The documentation for the SecureZeroMemory() function guarantees that the
compiler does not optimize out this call when zeroing memory.

 void getPassword(void) {
  char pwd[64];
  if (retrievePassword(pwd, sizeof(pwd))) {
    // checking of password, secure operations, etc
  }
  SecureZeroMemory(pwd, sizeof(pwd));
}

Compliant Solution (Windows)
The #pragma directives in this compliant solution instruct the compiler to avoid
optimizing the enclosed code.
 This #pragma directive is supported on some versions of Microsoft Visual Studio
and may be supported on other compilers.
 Check compiler documentation to ensure its availability and its optimization
guarantees.

void getPassword(void) {
    char pwd[64];
    if (retrievePassword(pwd, sizeof(pwd))) {
        // checking of password, secure operations, etc
    }
#pragma optimize("", off)
    memset(pwd, 0, sizeof(pwd));
#pragma optimize("", on)
}

Compliant Solution
This compliant solution uses the volatile type qualifier to inform the compiler
that the memory should be overwritten
 and that the call to the memset_s() function should not be optimized out.
Unfortunately, this compliant solution may
 not be as efficient as possible due to the nature of the volatile type
qualifier preventing the compiler from optimizing
 the code at all. Typically, some compilers are smart enough to replace calls to
memset() with equivalent assembly instructions
 that are much more efficient than the memset() implementation. Implementing a
memset_s() function as shown in the example may
 prevent the compiler from using the optimal assembly instructions and may
result in less efficient code. Check compiler
 documentation and the assembly output from the compiler.

// memset_s.c
void *memset_s(void* v, int32_t c, size_t n) {
    volatile uint8_t *p = v;
    while (n--)
        *p++ = c;

    return v;
}

// getPassword.c
extern void *memset_s(void* v, int32_t c, size_t n);

void getPassword(void) {
    char pwd[64];

    if (retrievePassword(pwd, sizeof(pwd))) {
        // checking of password, secure operations, etc
    }
    memset_s(pwd, 0, sizeof(pwd));
}
However, it should be noted that both calling functions and accessing volatile
qualified objects can still be optimized out
 (while maintaining strict conformance to the standard), so the above may still
not work.
 */

} // namespace opentxs
