// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP
#define OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP

#include "opentxs/Forward.hpp"

#include <string>
#include <memory>

namespace opentxs
{

class OTCachedKey;
class OTPassword;
class String;

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
    OTPassword* m_pMasterPW{
        nullptr};  // Used only when isForCachedKey is true, for
                   // output. Points to output value from original
                   // caller (not owned.)
    const std::string m_strDisplay;
    bool m_bUsingOldSystem{false};  // "Do NOT use CachedKey if this is true."

    // If m_pMasterPW is set, this must be set as well.
    const OTCachedKey* m_pCachedKey{nullptr};

    std::unique_ptr<OTPassword> password_override_;

public:
    EXPORT bool isForNormalNym() const;
    EXPORT bool isForCachedKey() const;
    EXPORT const char* GetDisplayString() const;
    EXPORT bool isUsingOldSystem() const;
    EXPORT void setUsingOldSystem(bool bUsing = true);
    OTPassword* GetMasterPW() const { return m_pMasterPW; }
    const OTCachedKey* GetCachedKey() const { return m_pCachedKey; }
    EXPORT OTPasswordData(
        const char* szDisplay,
        OTPassword* pMasterPW = nullptr,
        const OTCachedKey* pCachedKey = nullptr);
    EXPORT OTPasswordData(
        const std::string& str_Display,
        OTPassword* pMasterPW = nullptr,
        const OTCachedKey* pCachedKey = nullptr);
    EXPORT OTPasswordData(
        const String& strDisplay,
        OTPassword* pMasterPW = nullptr,
        const OTCachedKey* pCachedKey = nullptr);

    EXPORT bool ClearOverride();
    EXPORT bool SetOverride(const OTPassword& password);
    EXPORT const std::unique_ptr<OTPassword>& Override() const;

    EXPORT ~OTPasswordData();
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP
