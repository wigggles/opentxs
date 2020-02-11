// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/Proto.hpp"

#include "internal/api/Api.hpp"
#include "SymmetricNull.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Symmetric.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Symmetric>;

#define OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS 3
#define OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY 8388608

#define OT_METHOD "opentxs::crypto::key::implementation::Symmetric::"

namespace opentxs
{
crypto::key::Symmetric* Factory::SymmetricKey()
{
    return new crypto::key::implementation::SymmetricNull;
}

using ReturnType = crypto::key::implementation::Symmetric;

crypto::key::Symmetric* Factory::SymmetricKey(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& reason,
    const proto::SymmetricMode mode)
{
    auto output = std::make_unique<ReturnType>(api, engine);

    if (false == bool(output)) { return nullptr; }

    const auto realMode{mode == proto::SMODE_ERROR ? engine.DefaultMode()
                                                   : mode};
    Lock lock(output->lock_);
    const auto size = output->engine_.KeySize(realMode);
    output->key_size_ = size;
    output->plaintext_key_ = std::make_unique<OTPassword>();

    OT_ASSERT(output->plaintext_key_);

    auto& key = *output->plaintext_key_;

    if (false == output->allocate(lock, size, key, false)) { return nullptr; }

    output->encrypt_key(lock, key, reason);

    return output.release();
}

crypto::key::Symmetric* Factory::SymmetricKey(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized)
{
    std::unique_ptr<crypto::key::implementation::Symmetric> output;
    const bool valid = proto::Validate(serialized, VERBOSE);

    if (valid) {
        output.reset(new crypto::key::implementation::Symmetric(
            api, engine, serialized));
    }

    return output.release();
}

crypto::key::Symmetric* Factory::SymmetricKey(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const OTPassword& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const proto::SymmetricKeyType type)
{
    std::unique_ptr<crypto::key::implementation::Symmetric> output;
    std::string salt{};
    crypto::key::implementation::Symmetric::Allocate(
        engine.SaltSize(type), salt, false);

    const std::uint64_t ops =
        (0 == operations) ? OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS : operations;
    const std::uint64_t mem =
        (0 == difficulty) ? OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY : difficulty;

    output.reset(new crypto::key::implementation::Symmetric(
        api, engine, seed, salt, size, ops, mem, type));

    return output.release();
}

crypto::key::Symmetric* Factory::SymmetricKey(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const OTPassword& raw,
    const opentxs::PasswordPrompt& reason)
{
    std::unique_ptr<crypto::key::implementation::Symmetric> output;

    if (false == raw.isMemory()) { return nullptr; }

    output.reset(new crypto::key::implementation::Symmetric(api, engine));

    if (false == bool(output)) { return nullptr; }

    Lock lock(output->lock_);
    output->encrypt_key(lock, raw, reason);
    output->key_size_ = raw.getMemorySize();

    return output.release();
}
}  // namespace opentxs

namespace opentxs::crypto::key
{
OTSymmetricKey Symmetric::Factory()
{
    return OTSymmetricKey{new implementation::SymmetricNull};
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
Symmetric::Symmetric(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const VersionNumber version,
    const proto::SymmetricKeyType type,
    const std::size_t keySize,
    std::string* salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    OTPassword* plaintextKey,
    proto::Ciphertext* encryptedKey)
    : key::Symmetric()
    , api_(api)
    , engine_(engine)
    , version_(version)
    , type_(type)
    , key_size_(keySize)
    , operations_(operations)
    , difficulty_(difficulty)
    , salt_(salt)
    , plaintext_key_(plaintextKey)
    , encrypted_key_(encryptedKey)
    , lock_()
{
}

Symmetric::Symmetric(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine)
    : Symmetric(
          api,
          engine,
          1,
          proto::SKEYTYPE_RAW,
          0,
          nullptr,
          0,
          0,
          nullptr,
          nullptr)
{
}

Symmetric::Symmetric(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized)
    : Symmetric(
          api,
          engine,
          serialized.version(),
          serialized.type(),
          serialized.size(),
          new std::string(serialized.salt()),
          serialized.operations(),
          serialized.difficulty(),
          nullptr,
          new proto::Ciphertext(serialized.key()))
{
}

Symmetric::Symmetric(
    const api::internal::Core& api,
    const crypto::SymmetricProvider& engine,
    const OTPassword& seed,
    const std::string& salt,
    const std::size_t size,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricKeyType type)
    : Symmetric(
          api,
          engine,
          1,
          type,
          size,
          new std::string(salt),
          operations,
          difficulty,
          new OTPassword,
          nullptr)
{
    OT_ASSERT(salt_);
    OT_ASSERT(plaintext_key_);
    OT_ASSERT(0 != operations);
    OT_ASSERT(0 != difficulty);
    OT_ASSERT(0 != size);

    Lock lock(lock_);
    auto& plain = get_plaintext(lock);
    auto& salt_m = get_salt(lock);
    allocate(lock, key_size_, *plain, false);
    const auto bytes = seed.Bytes();

    const bool derived = engine.Derive(
        reinterpret_cast<const std::uint8_t*>(bytes.data()),
        bytes.size(),
        reinterpret_cast<const std::uint8_t*>(salt_m->data()),
        salt_m->size(),
        operations_,
        difficulty_,
        type_,
        static_cast<std::uint8_t*>(plain->getMemoryWritable()),
        plain->getMemorySize());

    OT_ASSERT(derived);
}

Symmetric::Symmetric(const Symmetric& rhs)
    : Symmetric(
          rhs.api_,
          rhs.engine_,
          rhs.version_,
          rhs.type_,
          rhs.key_size_,
          nullptr,
          rhs.operations_,
          rhs.difficulty_,
          nullptr,
          nullptr)
{
    if (rhs.salt_) { salt_.reset(new std::string(*rhs.salt_)); }

    if (rhs.plaintext_key_) {
        plaintext_key_.reset(new OTPassword(*rhs.plaintext_key_));
    }

    if (rhs.encrypted_key_) {
        encrypted_key_.reset(new proto::Ciphertext(*rhs.encrypted_key_));
    }
}

bool Symmetric::Allocate(const std::size_t size, Data& container)
{
    container.SetSize(size);

    return (size == container.size());
}

bool Symmetric::Allocate(const std::size_t size, String& container)
{
    std::vector<char> blank{};
    blank.assign(size, 0x7f);

    OT_ASSERT(blank.size() == size);

    container.Set(blank.data(), blank.size());

    return (size == container.GetLength());
}

bool Symmetric::Allocate(
    const std::size_t size,
    std::string& container,
    const bool random)
{
    container.resize(size, 0x0);

    if (random) {
        OTPassword::randomizeMemory(
            static_cast<void*>(const_cast<char*>(container.data())),
            container.size());
    }

    return (size == container.size());
}

bool Symmetric::allocate(
    const Lock& lock,
    const std::size_t size,
    OTPassword& container,
    const bool text) const
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

bool Symmetric::ChangePassword(
    const opentxs::PasswordPrompt& reason,
    const OTPassword& newPassword)
{
    Lock lock(lock_);
    auto& plain = get_plaintext(lock);

    if (unlock(lock, reason)) {
        OTPasswordPrompt copy{reason};
        copy->SetPassword(newPassword);

        return encrypt_key(lock, *plain, copy);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
        .Flush();

    return false;
}

Symmetric* Symmetric::clone() const { return new Symmetric(*this); }

bool Symmetric::decrypt(
    const Lock& lock,
    const proto::Ciphertext& input,
    const opentxs::PasswordPrompt& reason,
    std::uint8_t* plaintext) const
{
    auto& plain = get_plaintext(lock);

    if (false == bool(plain)) {
        if (false == unlock(lock, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    const bool output = engine_.Decrypt(
        input, plain->getMemory_uint8(), plain->getMemorySize(), plaintext);

    if (false == output) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to decrypt key.").Flush();

        return false;
    }

    return output;
}

bool Symmetric::Decrypt(
    const proto::Ciphertext& ciphertext,
    const opentxs::PasswordPrompt& reason,
    const AllocateOutput plaintext) const
{
    Lock lock(lock_);

    if (false == bool(plaintext)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing output allocator")
            .Flush();

        return false;
    }

    auto output = plaintext(ciphertext.data().size());

    if (false == output.valid(ciphertext.data().size())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to allocate space for decryption.")
            .Flush();

        return false;
    }

    return decrypt(lock, ciphertext, reason, output.as<std::uint8_t>());
}

bool Symmetric::encrypt(
    const Lock& lock,
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* iv,
    const std::size_t ivSize,
    const proto::SymmetricMode mode,
    const opentxs::PasswordPrompt& reason,
    proto::Ciphertext& ciphertext,
    const bool text) const
{
    if (nullptr == input) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null input.").Flush();

        return false;
    }

    auto& plain = get_plaintext(lock);

    if (false == bool(plain)) {
        if (false == unlock(lock, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    ciphertext.set_version(1);

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

    OT_ASSERT(nullptr != plain->getMemory_uint8());

    return engine_.Encrypt(
        input,
        inputSize,
        plain->getMemory_uint8(),
        plain->getMemorySize(),
        ciphertext);
}

bool Symmetric::Encrypt(
    const ReadView plaintext,
    const opentxs::PasswordPrompt& reason,
    proto::Ciphertext& ciphertext,
    const bool attachKey,
    const proto::SymmetricMode mode,
    const ReadView iv) const
{
    Lock lock(lock_);
    const bool success = encrypt(
        lock,
        reinterpret_cast<const std::uint8_t*>(plaintext.data()),
        plaintext.size(),
        reinterpret_cast<const std::uint8_t*>(iv.data()),
        iv.size(),
        mode,
        reason,
        ciphertext,
        true);

    if (success && attachKey) { serialize(lock, *ciphertext.mutable_key()); }

    return success;
}

bool Symmetric::encrypt_key(
    const Lock& lock,
    const OTPassword& plaintextKey,
    const opentxs::PasswordPrompt& reason,
    const proto::SymmetricKeyType type) const
{
    auto& encrypted = get_encrypted(lock);
    encrypted.reset(new proto::Ciphertext);

    OT_ASSERT(encrypted);

    encrypted->set_mode(engine_.DefaultMode());
    OTPassword blankIV;
    blankIV.randomizeMemory(engine_.IvSize(encrypted->mode()));
    encrypted->set_iv(blankIV.getMemory(), blankIV.getMemorySize());
    encrypted->set_text(false);
    OTPassword key;
    get_password(lock, reason, key);
    const auto saltSize = engine_.SaltSize(type);
    auto& salt = get_salt(lock);

    if (!salt) { salt.reset(new std::string); }

    OT_ASSERT(salt);

    if (salt->size() != saltSize) {
        if (!Allocate(saltSize, *salt, true)) { return false; }
    }

    Symmetric secondaryKey(
        api_,
        engine_,
        key,
        *salt,
        engine_.KeySize(encrypted->mode()),
        OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS,
        OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY);

    return engine_.Encrypt(
        plaintextKey.getMemory_uint8(),
        plaintextKey.getMemorySize(),
        secondaryKey.plaintext_key_->getMemory_uint8(),
        secondaryKey.plaintext_key_->getMemorySize(),
        *encrypted);
}

std::unique_ptr<proto::Ciphertext>& Symmetric::get_encrypted(
    const Lock& lock) const
{
    return encrypted_key_;
}

bool Symmetric::get_password(
    const Lock& lock,
    const opentxs::PasswordPrompt& reason,
    OTPassword& password) const
{
    if (reason.Password()) {
        password = *reason.Password();

        return true;
    } else {
        std::unique_ptr<OTPassword> master(new OTPassword);

        OT_ASSERT(master);

        master->randomizeMemory(master->getBlockSize());
        auto* callback = api_.GetInternalPasswordCallback();

        OT_ASSERT(nullptr != callback);

        const auto length = (*callback)(
            static_cast<char*>(master->getMemoryWritable()),
            master->getBlockSize(),
            0,
            const_cast<PasswordPrompt*>(&reason));
        bool result = false;

        if (0 < length) {
            password.setMemory(master->getMemory(), length);
            result = true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to obtain master password")
                .Flush();
        }

        return result;
    }
}

std::unique_ptr<OTPassword>& Symmetric::get_plaintext(const Lock& lock) const
{
    return plaintext_key_;
}

std::unique_ptr<std::string>& Symmetric::get_salt(const Lock& lock) const
{
    return salt_;
}

OTIdentifier Symmetric::ID(const opentxs::PasswordPrompt& reason) const
{
    Lock lock(lock_);
    auto& plain = get_plaintext(lock);

    if (false == bool(plain)) {
        if (false == unlock(lock, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return api_.Factory().Identifier();
        }
    }

    OT_ASSERT(plain);

    return api_.Factory().Identifier(plain->Bytes());
}

bool Symmetric::RawKey(
    const opentxs::PasswordPrompt& reason,
    OTPassword& output) const
{
    Lock lock(lock_);
    auto& plain = get_plaintext(lock);

    if (false == bool(plain)) {
        if (false == unlock(lock, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    OT_ASSERT(plain);

    if (plain->isMemory()) {
        output.setMemory(plain->getMemory(), plain->getMemorySize());
    } else {
        output.setPassword(plain->getPassword(), plain->getPasswordSize());
    }

    return true;
}

bool Symmetric::serialize(const Lock& lock, proto::SymmetricKey& output) const
{
    auto& encrypted = get_encrypted(lock);
    auto& salt = get_salt(lock);

    if (!encrypted) { return false; }

    output.set_version(version_);
    output.set_type(type_);
    output.set_size(key_size_);
    *output.mutable_key() = *encrypted;

    if (salt) { output.set_salt(*salt); }

    output.set_operations(operations_);
    output.set_difficulty(difficulty_);

    return proto::Validate(output, VERBOSE);
}

bool Symmetric::Serialize(proto::SymmetricKey& output) const
{
    Lock lock(lock_);

    return serialize(lock, output);
}

bool Symmetric::unlock(const Lock& lock, const opentxs::PasswordPrompt& reason)
    const
{
    auto& encrypted = get_encrypted(lock);
    auto& plain = get_plaintext(lock);

    if (false == bool(encrypted)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Master key not loaded.").Flush();

        return false;
    }

    if (plain) {
        if (0 < plain->Bytes().size()) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Already unlocked").Flush();

            return true;
        }
    } else {
        plain.reset(new OTPassword);

        OT_ASSERT(plain);

        // Allocate space for plaintext (same size as ciphertext)
        if (!allocate(lock, encrypted->data().size(), *plain)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to allocate space for plaintext master key.")
                .Flush();

            return false;
        }
    }

    auto& salt = get_salt(lock);
    auto key = OTPassword{};

    if (false == get_password(lock, reason, key)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to obtain master password.")
            .Flush();

        return false;
    }

    auto secondaryKey = Symmetric{api_,
                                  engine_,
                                  key,
                                  *salt,
                                  engine_.KeySize(encrypted->mode()),
                                  OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS,
                                  OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY};
    const auto output = engine_.Decrypt(
        *encrypted,
        secondaryKey.plaintext_key_->getMemory_uint8(),
        secondaryKey.plaintext_key_->getMemorySize(),
        static_cast<std::uint8_t*>(plain->getMemoryWritable()));

    if (output) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Key unlocked").Flush();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Failed to unlock key").Flush();
    }

    return output;
}

bool Symmetric::Unlock(const opentxs::PasswordPrompt& reason) const
{
    Lock lock(lock_);

    return unlock(lock, reason);
}
}  // namespace opentxs::crypto::key::implementation
