// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "opentxs/crypto/Bip39.hpp"

namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

class OTPassword;
class Secret;
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
class Bip39 final : public opentxs::crypto::Bip39
{
public:
    auto SeedToWords(const Secret& seed, Secret& words) const noexcept
        -> bool final;
    void WordsToSeed(
        const Secret& words,
        Secret& seed,
        const Secret& passphrase) const noexcept final;

    Bip39(const api::Crypto& crypto) noexcept;
    ~Bip39() final = default;

private:
    using WordList = std::vector<const char*>;
    using Words = std::map<std::string, WordList>;
    using MnemonicWords = std::vector<std::string>;

    static const Words words_;
    static const std::size_t BitsPerWord;
    static const std::uint8_t ByteBits;
    static const std::size_t DictionarySize;
    static const std::size_t EntropyBitDivisor;
    static const std::size_t HmacIterationCount;
    static const std::size_t HmacOutputSizeBytes;
    static const std::string PassphrasePrefix;
    static const std::size_t ValidMnemonicWordMultiple;

    const api::Crypto& crypto_;

    static auto bitShift(std::size_t theBit) noexcept -> std::byte;

    auto entropy_to_words(const Secret& entropy, Secret& words) const noexcept
        -> bool;
    void words_to_root(
        const Secret& words,
        Secret& bip32RootNode,
        const Secret& passphrase) const noexcept;

    Bip39() = delete;
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    auto operator=(const Bip39&) -> Bip39& = delete;
    auto operator=(Bip39 &&) -> Bip39& = delete;
};
}  // namespace opentxs::crypto::implementation
