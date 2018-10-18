// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTCaller.hpp"

#include "opentxs/core/crypto/OTCallback.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Log.hpp"

#include <cstdint>
#include <ostream>

#define OT_METHOD "opentxs::OTCaller"

namespace opentxs
{

OTCaller::~OTCaller() { delCallback(); }

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
void OTCaller::SetDisplay(const char* szDisplay, std::int32_t nLength)
{
    // I'm using the OTPassword class to store the display string, in addition
    // to
    // storing the password itself. (For convenience.)
    //
    m_Display.setPassword_uint8(
        reinterpret_cast<const std::uint8_t*>(szDisplay), nLength);
}

// The password will be stored here by the Java dialog, so that the C callback
// can retrieve it and pass it to OpenSSL
//
bool OTCaller::GetPassword(OTPassword& theOutput) const  // Get the password....
{
    theOutput.setPassword_uint8(
        m_Password.getPassword_uint8(), m_Password.getPasswordSize());

    return true;
}

void OTCaller::ZeroOutPassword()  // Then ZERO IT OUT so copies aren't floating
                                  // around.
{
    if (m_Password.getPasswordSize() > 0) m_Password.zeroMemory();
}

void OTCaller::delCallback()
{
    if (isCallbackSet()) { _callback = nullptr; }
}

void OTCaller::setCallback(OTCallback* cb)
{
    if (nullptr == cb) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR: nullptr password OTCallback "
            "object passed in. (Returning).")
            .Flush();
        return;
    }

    delCallback();  // Sets _callback to nullptr, but LOGS first, if it was
                    // already set.
    _callback = cb;
}

bool OTCaller::isCallbackSet() const
{
    return (nullptr == _callback) ? false : true;
}

void OTCaller::callOne()
{
    ZeroOutPassword();  // Make sure there isn't some old password still in
                        // here.

    if (isCallbackSet()) {
        _callback->runOne(GetDisplay(), m_Password);
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": WARNING: Failed attempt to trigger "
            "password callback (one), due to it hasn't been set "
            "yet.")
            .Flush();
    }
}

void OTCaller::callTwo()
{
    ZeroOutPassword();  // Make sure there isn't some old password still in
                        // here.

    if (isCallbackSet()) {
        _callback->runTwo(GetDisplay(), m_Password);
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": WARNING: Failed attempt to trigger "
            "password callback (two), due to it hasn't been set "
            "yet.")
            .Flush();
    }
}
}  // namespace opentxs
