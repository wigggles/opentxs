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

#include <opentxs/core/crypto/Bip32.hpp>

#include <opentxs/core/crypto/OTCachedKey.hpp>

namespace opentxs
{

BinarySecret Bip32::GetHDSeed() const
{
    BinarySecret decryptedCachedKey;

    std::shared_ptr<OTCachedKey> encryptedCachedKey(OTCachedKey::It());
    if (encryptedCachedKey) {
        std::string pReason = "loading the master HD seed for this wallet";
        encryptedCachedKey->GetMasterPassword(
            encryptedCachedKey,
            *decryptedCachedKey,
            pReason.c_str());
    }
    return decryptedCachedKey;
}

serializedAsymmetricKey Bip32::GetHDKey(const proto::HDPath path) const
{
    uint32_t depth = path.child_size();
    if (0 == depth) {
        BinarySecret seed = GetHDSeed();

        return SeedToPrivateKey(*seed);
    } else {
        proto::HDPath newpath = path;
        newpath.mutable_child()->RemoveLast();
        serializedAsymmetricKey parentnode = GetHDKey(newpath);

        return GetChild(
            *parentnode,
            path.child(depth));
    }
}

} // namespace opentxs
