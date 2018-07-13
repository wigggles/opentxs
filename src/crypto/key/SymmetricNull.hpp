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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_KEY_SYMMETRICNULL_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_KEY_SYMMETRICNULL_HPP

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class SymmetricNull final : virtual public key::Symmetric
{
public:
    bool ChangePassword(const OTPasswordData&, const OTPassword&) override
    {
        return false;
    }
    bool Decrypt(const proto::Ciphertext&, const OTPasswordData&, std::string&)
        override
    {
        return false;
    }
    bool Decrypt(const proto::Ciphertext&, const OTPasswordData&, Data&)
        override
    {
        return false;
    }
    bool Decrypt(const proto::Ciphertext&, const OTPasswordData&, OTPassword&)
        override
    {
        return false;
    }
    bool Encrypt(
        const std::string&,
        const Data&,
        const OTPasswordData&,
        proto::Ciphertext&,
        const bool = true,
        const proto::SymmetricMode = proto::SMODE_ERROR) override
    {
        return false;
    }
    bool Encrypt(
        const String&,
        const Data&,
        const OTPasswordData&,
        proto::Ciphertext&,
        const bool = true,
        const proto::SymmetricMode = proto::SMODE_ERROR) override
    {
        return false;
    }
    bool Encrypt(
        const OTPassword&,
        const Data&,
        const OTPasswordData&,
        proto::Ciphertext&,
        const bool = true,
        const proto::SymmetricMode = proto::SMODE_ERROR) override
    {
        return false;
    }
    OTIdentifier ID() override { return Identifier::Factory(); }
    bool Serialize(proto::SymmetricKey&) const override { return false; }
    bool Unlock(const OTPasswordData&) override { return false; }

    operator bool() const override { return false; }

    SymmetricNull() = default;
    ~SymmetricNull() = default;

private:
    SymmetricNull* clone() const override { return nullptr; }

    SymmetricNull(const SymmetricNull&) = default;
    SymmetricNull(SymmetricNull&&) = delete;
    SymmetricNull& operator=(const SymmetricNull&) = delete;
    SymmetricNull& operator=(SymmetricNull&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_KEY_SYMMETRICNULL_HPP
