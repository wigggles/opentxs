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

#include "opentxs/core/crypto/SymmetricKey.hpp"

#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricNew.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
SymmetricKey::SymmetricKey(
    const CryptoSymmetricNew& engine)
        : engine_(engine)
        , version_(1)
        , type_(proto::SKEYTYPE_RAW)
{
}

SymmetricKey::SymmetricKey(
    const CryptoSymmetricNew& engine,
    const proto::SymmetricKey serialized)
        : engine_(engine)
        , version_(serialized.version())
        , type_(serialized.type())
        , key_size_(serialized.size())
        , seed_salt_(
            new OTData(serialized.salt().data(), serialized.salt().size()))
        , operations_(serialized.operations())
        , difficulty_(serialized.difficulty())
        , encrypted_key_(new proto::Ciphertext(serialized.key()))
{
}

SymmetricKey::SymmetricKey(
    const CryptoSymmetricNew& engine,
    __attribute__((unused)) const OTPassword& seed,
    __attribute__((unused)) const std::string& salt,
    __attribute__((unused)) const std::uint64_t operations,
    __attribute__((unused)) const std::uint64_t difficulty,
    __attribute__((unused)) const std::size_t size,
    const proto::SymmetricKeyType type)
        : engine_(engine)
        , version_(1)
        , type_(type)
{
}

SymmetricKey::SymmetricKey(
    const CryptoSymmetricNew& engine,
    __attribute__((unused)) const OTAsymmetricKeyEC& privateKey,
    __attribute__((unused)) const OTAsymmetricKeyEC& publicKey,
    __attribute__((unused)) const std::size_t size)
        : engine_(engine)
        , version_(1)
        , type_(proto::SKEYTYPE_ECDH)
{
}

std::unique_ptr<SymmetricKey> SymmetricKey::Factory(
    const CryptoSymmetricNew& engine,
    const OTPasswordData& password,
    const proto::SymmetricMode mode)
{
    std::unique_ptr<SymmetricKey> output;
    output.reset(new SymmetricKey(engine));

    if (!output) { return output; }

    proto::SymmetricMode realMode = proto::SMODE_ERROR;

    if (mode == realMode) {
        realMode = engine.DefaultMode();
    } else {
        realMode = mode;
    }

    const auto size = output->engine_.KeySize(realMode);
    output->key_size_ = size;
    OTPassword key;

    if (!output->Allocate(size, key, false)) { return output; }

    output->EncryptKey(key, password);

    return output;
}

std::unique_ptr<SymmetricKey> SymmetricKey::Factory(
    const CryptoSymmetricNew& engine,
    const proto::SymmetricKey serialized)
{
    std::unique_ptr<SymmetricKey> output;
    const bool valid = proto::Check(
        serialized,
        serialized.version(),
        serialized.version());

    if (valid) {
        output.reset(new SymmetricKey(engine, serialized));
    }

    return output;
}

std::unique_ptr<SymmetricKey> SymmetricKey::Factory(
    __attribute__((unused)) const CryptoSymmetricNew& engine,
    __attribute__((unused)) const OTPassword& seed,
    __attribute__((unused)) const std::string& salt,
    __attribute__((unused)) const std::uint64_t operations,
    __attribute__((unused)) const std::uint64_t difficulty,
    __attribute__((unused)) const std::size_t size,
    __attribute__((unused)) const proto::SymmetricKeyType type)
{
    std::unique_ptr<SymmetricKey> output;

    return output;
}

std::unique_ptr<SymmetricKey> SymmetricKey::Factory(
    const CryptoSymmetricNew& engine,
    const OTPassword& raw)
{
    std::unique_ptr<SymmetricKey> output;

    if (!raw.isMemory()) { return output; }

    output.reset(new SymmetricKey(engine));

    if (!output) { return output; }

    OTPasswordData password("Encrypting a symmetric key.");
    output->EncryptKey(raw, password);
    output->key_size_ = raw.getMemorySize();

    return output;
}

std::unique_ptr<SymmetricKey> SymmetricKey::Factory(
    __attribute__((unused)) const CryptoSymmetricNew& engine,
    __attribute__((unused)) const OTAsymmetricKeyEC& privateKey,
    __attribute__((unused)) const OTAsymmetricKeyEC& publicKey,
    __attribute__((unused)) const std::string& salt,
    __attribute__((unused)) const std::size_t size)
{
    std::unique_ptr<SymmetricKey> output;

    return output;
}

bool SymmetricKey::Allocate(
    const std::size_t size,
    OTPassword& container,
    const bool text)
{
    std::int32_t result;

    if (text) {
        result = container.randomizePassword(size);
    } else {
        result = container.randomizeMemory(size);
    }

    if (0 > result) { return false; }

    return (size == static_cast<std::uint32_t>(result));
}

bool SymmetricKey::Decrypt(
    const proto::Ciphertext& input,
    const OTPasswordData& keyPassword,
    std::uint8_t* plaintext)
{
    if (!plaintext_key_) {
        if (!DecryptKey(keyPassword)) { return false; }
    }

    const bool output = engine_.Decrypt(
        input,
        plaintext_key_->getMemory_uint8(),
        plaintext_key_->getMemorySize(),
        plaintext);

    return output;
}

bool SymmetricKey::Decrypt(
    const proto::Ciphertext& ciphertext,
    const OTPasswordData& keyPassword,
    OTPassword& plaintext)
{
    if (!Allocate(ciphertext.data().size(), plaintext, ciphertext.text())) {

        return false;
    }

    std::uint8_t* output = nullptr;

    if (ciphertext.text()) {
        output = plaintext.getPasswordWritable();
    } else {
        output = static_cast<std::uint8_t*>(plaintext.getMemoryWritable());
    }

    return (Decrypt(ciphertext, keyPassword, output));
}

bool SymmetricKey::DecryptKey(const OTPasswordData& keyPassword)
{
    if (!encrypted_key_) { return false; }

    if (!plaintext_key_) {
        plaintext_key_.reset(new OTPassword());

        OT_ASSERT(plaintext_key_);

        // Allocate space for plaintext (same size as ciphertext)
        if (!Allocate(encrypted_key_->data().size(), *plaintext_key_)) {

            return false;
        }
    }

    OTPassword key;
    GetPassword(keyPassword, key);

    return engine_.Decrypt(
        *encrypted_key_,
        static_cast<const std::uint8_t*>(key.getMemory()),
        key.getMemorySize(),
        static_cast<std::uint8_t*>(plaintext_key_->getMemoryWritable()));
}

bool SymmetricKey::Encrypt(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* iv,
    const std::size_t ivSize,
    const proto::SymmetricMode mode,
    const OTPasswordData& keyPassword,
    proto::Ciphertext& ciphertext,
    const bool text)
{
    if (nullptr == input) { return false; }

    if (!plaintext_key_) {
        if (!DecryptKey(keyPassword)) { return false; }
    }

    if (proto::SMODE_ERROR == mode) {
        ciphertext.set_mode(engine_.DefaultMode());
    } else {
        ciphertext.set_mode(mode);
    }

    if ((0 == ivSize) || (nullptr == iv)) {
        OTPassword blankIV;
        blankIV.randomizeMemory(engine_.IvSize(ciphertext.mode()));
        ciphertext.set_iv(blankIV.getMemory(), blankIV.getMemorySize());
    } else {
        ciphertext.set_iv(iv, ivSize);
    }

    ciphertext.set_text(text);

    return engine_.Encrypt(
        input,
        inputSize,
        plaintext_key_->getMemory_uint8(),
        plaintext_key_->getMemorySize(),
        ciphertext);
}

bool SymmetricKey::Encrypt(
    const OTPassword& plaintext,
    const OTData& iv,
    const OTPasswordData& keyPassword,
    proto::Ciphertext& ciphertext,
    const bool attachKey,
    const proto::SymmetricMode mode)
{
    const std::uint8_t* input = nullptr;
    std::size_t inputSize = 0;

    if (plaintext.isPassword()) {
        input = reinterpret_cast<const std::uint8_t*>(plaintext.getPassword());
        inputSize = plaintext.getPasswordSize();
    } else {
        input = static_cast<const std::uint8_t*>(plaintext.getMemory());
        inputSize = plaintext.getMemorySize();
    }

    const bool success = Encrypt(
        input,
        inputSize,
        static_cast<const std::uint8_t*>(iv.GetPointer()),
        iv.GetSize(),
        mode,
        keyPassword,
        ciphertext,
        plaintext.isPassword());

    if (success && attachKey) {
        Serialize(*ciphertext.mutable_key());
    }

    return success;
}

bool SymmetricKey::EncryptKey(
    const OTPassword& plaintextKey,
    const OTPasswordData& keyPassword)
{
    encrypted_key_.reset(new proto::Ciphertext);

    OT_ASSERT(encrypted_key_);

    encrypted_key_->set_mode(engine_.DefaultMode());
    OTPassword blankIV;
    blankIV.randomizeMemory(engine_.IvSize(encrypted_key_->mode()));
    encrypted_key_->set_iv(blankIV.getMemory(), blankIV.getMemorySize());
    encrypted_key_->set_text(false);

    OTPassword key;
    GetPassword(keyPassword, key);

    return engine_.Encrypt(
        plaintextKey.getMemory_uint8(),
        plaintextKey.getMemorySize(),
        key.getMemory_uint8(),
        key.getMemorySize(),
        *encrypted_key_);
}

bool SymmetricKey::GetPassword(
    const OTPasswordData& keyPassword,
    OTPassword& password)
{
    if (keyPassword.Override()) {
        password = *keyPassword.Override();

        return true;
    } else {
        std::unique_ptr<OTPassword> master(new OTPassword);

        OT_ASSERT(master);

        master->randomizeMemory(OTPassword::DEFAULT_SIZE);
        const auto length = souped_up_pass_cb(
            static_cast<char*>(master->getMemoryWritable()),
            OTPassword::DEFAULT_SIZE,
            false,
            reinterpret_cast<void*>(const_cast<OTPasswordData*>(&keyPassword)));
        bool result = false;

        if (0 < length) {
            password.setMemory(master->getMemory(), length);
            result = true;
        }

        return result;
    }
}

bool SymmetricKey::Serialize(proto::SymmetricKey& output) const
{
    output.set_version(version_);
    output.set_type(type_);
    output.set_size(key_size_);
    *output.mutable_key() = *encrypted_key_;

    if (proto::SKEYTYPE_ARGON2) {
        if (seed_salt_) {
            output.set_salt(seed_salt_->GetPointer(), seed_salt_->GetSize());
        }

        output.set_operations(operations_);
        output.set_difficulty(difficulty_);
    }

    if (!encrypted_key_) { return false; }

    return Check(output, version_, version_);
}
} // namespace opentxs
