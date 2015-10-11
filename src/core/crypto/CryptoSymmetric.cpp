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

#include <opentxs/core/crypto/CryptoSymmetric.hpp>

#include <opentxs/core/OTData.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>

namespace opentxs
{

OTCrypto_Decrypt_Output::OTCrypto_Decrypt_Output()
    : m_pPassword(nullptr)
    , m_pPayload(nullptr)
{
}

OTCrypto_Decrypt_Output::~OTCrypto_Decrypt_Output()
{
    // We don't own these objects.
    // Rather, we own a pointer to ONE of them, since we are a wrapper
    // for this one or that.
    //
    m_pPassword = nullptr;
    m_pPayload = nullptr;

    // Since this is merely a wrapper class, we don't actually Release() these
    // things.
    // However, we DO have a release function, since the programmatic USER of
    // this class
    // MAY wish to Release() whatever it is wrapping.
    //
    //  Release_Envelope_Decrypt_Output();
}

OTCrypto_Decrypt_Output::OTCrypto_Decrypt_Output(
    const OTCrypto_Decrypt_Output& rhs) // passed
    : m_pPassword(nullptr),
      m_pPayload(nullptr)
{
    m_pPassword = rhs.m_pPassword;
    m_pPayload = rhs.m_pPayload;
}

OTCrypto_Decrypt_Output::OTCrypto_Decrypt_Output(OTPassword& thePassword)
    : m_pPassword(&thePassword)
    , m_pPayload(nullptr)
{
}

OTCrypto_Decrypt_Output::OTCrypto_Decrypt_Output(OTData& thePayload)
    : m_pPassword(nullptr)
    , m_pPayload(&thePayload)
{
}

void OTCrypto_Decrypt_Output::swap(OTCrypto_Decrypt_Output& other) // the swap
                                                                   // member
                                                                   // function
                                                                   // (should
                                                                   // never
                                                                   // fail!)
{
    if (&other != this) {
        std::swap(m_pPassword, other.m_pPassword);
        std::swap(m_pPayload, other.m_pPayload);
    }
}

OTCrypto_Decrypt_Output& OTCrypto_Decrypt_Output::operator=(
    OTCrypto_Decrypt_Output other) // note: argument passed by value!
{
    // swap this with other
    swap(other);

    // by convention, always return *this
    return *this;
}

// This is just a wrapper class.
void OTCrypto_Decrypt_Output::Release()
{
    OT_ASSERT((m_pPassword != nullptr) || (m_pPayload != nullptr));

    Release_Envelope_Decrypt_Output();

    // no need to call ot_super::Release here, since this class has no
    // superclass.
}

// This is just a wrapper class.
void OTCrypto_Decrypt_Output::Release_Envelope_Decrypt_Output() const
{
    if (nullptr != m_pPassword) m_pPassword->zeroMemory();

    if (nullptr != m_pPayload) m_pPayload->Release();
}

bool OTCrypto_Decrypt_Output::Concatenate(const void* pAppendData,
                                          uint32_t lAppendSize) const
{
    OT_ASSERT((m_pPassword != nullptr) || (m_pPayload != nullptr));

    if (nullptr != m_pPassword) {
        if (static_cast<int32_t>(lAppendSize) ==
            m_pPassword->addMemory(pAppendData, lAppendSize))
            return true;
        else
            return false;
    }

    if (nullptr != m_pPayload) {
        m_pPayload->Concatenate(pAppendData, lAppendSize);
        return true;
    }
    return false;
}

} // namespace opentxs
