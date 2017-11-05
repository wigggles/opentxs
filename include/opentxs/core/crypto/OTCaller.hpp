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

#ifndef OPENTXS_CORE_CRYPTO_OTCALLER_HPP
#define OPENTXS_CORE_CRYPTO_OTCALLER_HPP

#include "opentxs/Version.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"

#include <stdint.h>

namespace opentxs
{

class OTCallback;
class OTPassword;

class OTCaller
{
protected:
    OTPassword m_Password;  // The password will be stored here by the Java
    // dialog, so that the C callback can retrieve it and
    // pass it to OpenSSL
    OTPassword m_Display;  // A display string is set here before the Java
                           // dialog
                           // is shown. (OTPassword used here only for
                           // convenience.)

    OTCallback* _callback{nullptr};

public:
    OTCaller()
        : _callback(nullptr)
    {
    }
    EXPORT ~OTCaller();

    EXPORT bool GetPassword(OTPassword& theOutput) const;  // Grab the password
                                                           // when it is needed.
    EXPORT void ZeroOutPassword();  // Then ZERO IT OUT so copies aren't
                                    // floating
                                    // around...

    EXPORT const char* GetDisplay() const;
    EXPORT void SetDisplay(const char* szDisplay, int32_t nLength);

    EXPORT void delCallback();
    EXPORT void setCallback(OTCallback* cb);
    EXPORT bool isCallbackSet() const;

    EXPORT void callOne();  // Asks for password once. (For authentication when
                            // using the Nym's private key.)
    EXPORT void callTwo();  // Asks for password twice. (For confirmation during
                            // nym creation and password change.)
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTCALLER_HPP
