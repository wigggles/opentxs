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

#include <opentxs/core/crypto/TrezorCrypto.hpp>

extern "C" {
#include <trezor-crypto/bip39.h>
}

namespace opentxs
{
std::string TrezorCrypto::toWords(const OTPassword& seed) const
{
    std::string wordlist(
        ::mnemonic_from_data(
            static_cast<const uint8_t*>(seed.getMemory()),
            seed.getMemorySize()));
    return wordlist;
}

serializedAsymmetricKey TrezorCrypto::SeedToPrivateKey(const OTPassword& seed)
    const
{
    serializedAsymmetricKey derivedKey;
    HDNode node;

    int result = ::hdnode_from_seed(
        static_cast<const uint8_t*>(seed.getMemory()),
        seed.getMemorySize(),
        &node);

    if (1 == result) {
        derivedKey = HDNodeToSerialized(node);
    }

    return derivedKey;
}

serializedAsymmetricKey TrezorCrypto::GetChild(
    const proto::AsymmetricKey& parent,
    const uint32_t index) const
{
    std::shared_ptr<HDNode> node = SerializedToHDNode(parent);

    if (proto::KEYMODE_PUBLIC == parent.mode()) {
        hdnode_private_ckd(node.get(), index);
    } else {
        hdnode_public_ckd(node.get(), index);
    }
    serializedAsymmetricKey key = HDNodeToSerialized(*node);

    return key;
}

serializedAsymmetricKey TrezorCrypto::HDNodeToSerialized(const HDNode& node)
    const
{
    serializedAsymmetricKey key = std::make_shared<proto::AsymmetricKey>();

    uint8_t testkey[sizeof(node.private_key)]{};
    bool isPrivate = !memcmp(
        testkey,
        node.private_key,
        sizeof(node.private_key));

    key->set_version(1);
    key->set_type(proto::AKEYTYPE_SECP256K1);

    if (isPrivate) {
        key->set_mode(proto::KEYMODE_PRIVATE);
        key->set_key(node.private_key, sizeof(node.private_key));
    } else {
        key->set_mode(proto::KEYMODE_PUBLIC);
        key->set_key(node.public_key, sizeof(node.public_key));
    }
    key->set_chaincode(node.chain_code, sizeof(node.chain_code));

    return key;
}

std::shared_ptr<HDNode> TrezorCrypto::SerializedToHDNode(
    const proto::AsymmetricKey& serialized) const
{
    std::shared_ptr<HDNode> node = std::make_shared<HDNode>();

    OTPassword::safe_memcpy(
        &(node->chain_code[0]),
        sizeof(node->chain_code),
        serialized.chaincode().c_str(),
        serialized.chaincode().size(),
        false);

    if (proto::KEYMODE_PRIVATE == serialized.mode()) {
        OTPassword::safe_memcpy(
            &(node->private_key[0]),
            sizeof(node->private_key),
            serialized.key().c_str(),
            serialized.key().size(),
            false);
    } else {
        OTPassword::safe_memcpy(
            &(node->public_key[0]),
            sizeof(node->public_key),
            serialized.key().c_str(),
            serialized.key().size(),
            false);
    }

    return node;
}

serializedAsymmetricKey TrezorCrypto::PrivateToPublic(
    const serializedAsymmetricKey& key) const
{
    std::shared_ptr<HDNode> node = SerializedToHDNode(*key);
    hdnode_fill_public_key(node.get());

    // This will cause the next function to serialize as public
    OTPassword::zeroMemory(node->private_key, sizeof(node->private_key));

    return HDNodeToSerialized(*node);
}

} // namespace opentxs
