// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICDECRYPTOUTPUT_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICDECRYPTOUTPUT_HPP

#include "opentxs/Forward.hpp"

#include <cstdint>

namespace opentxs
{
// Sometimes I want to decrypt into an OTPassword (for encrypted symmetric
// keys being decrypted) and sometimes I want to decrypt into an Data
// (For most other types of data.) This class allows me to do it either way
// without duplicating the static Decrypt() function, by wrapping both
// types.
//
class CryptoSymmetricDecryptOutput
{
private:
    OTPassword* m_pPassword{nullptr};
    Data* m_pPayload{nullptr};

    CryptoSymmetricDecryptOutput();

public:
    EXPORT ~CryptoSymmetricDecryptOutput();

    EXPORT CryptoSymmetricDecryptOutput(
        const CryptoSymmetricDecryptOutput& rhs);

    EXPORT CryptoSymmetricDecryptOutput(OTPassword& thePassword);
    EXPORT CryptoSymmetricDecryptOutput(Data& thePayload);

    EXPORT void swap(CryptoSymmetricDecryptOutput& other);

    EXPORT CryptoSymmetricDecryptOutput& operator=(
        CryptoSymmetricDecryptOutput other);  // passed by value.

    EXPORT bool Concatenate(const void* pAppendData, std::uint32_t lAppendSize)
        const;

    EXPORT void Release();  // Someday make this virtual, if we ever subclass
                            // it.
    EXPORT void Release_Envelope_Decrypt_Output() const;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRICDECRYPTOUTPUT_HPP
