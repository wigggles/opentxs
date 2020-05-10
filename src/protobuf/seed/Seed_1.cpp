// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Proto.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/verify/Seed.hpp"
#include "opentxs/protobuf/verify/VerifyCredentials.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "seed"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const Seed& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT_VA(words, SeedAllowedCiphertext(), false);
    OPTIONAL_SUBOBJECT_VA(passphrase, SeedAllowedCiphertext(), false);
    CHECK_IDENTIFIER(fingerprint);
    CHECK_EXCLUDED(index);
    CHECK_EXCLUDED(raw);

    return true;
}

auto CheckProto_2(const Seed& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT_VA(words, SeedAllowedCiphertext(), false);
    OPTIONAL_SUBOBJECT_VA(passphrase, SeedAllowedCiphertext(), false);
    CHECK_IDENTIFIER(fingerprint);
    CHECK_EXISTS(index);
    CHECK_EXCLUDED(raw);

    return true;
}

auto CheckProto_3(const Seed& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECT_VA(words, SeedAllowedCiphertext(), false);
    OPTIONAL_SUBOBJECT_VA(passphrase, SeedAllowedCiphertext(), false);
    CHECK_IDENTIFIER(fingerprint);
    CHECK_EXISTS(index);
    OPTIONAL_SUBOBJECT_VA(raw, SeedAllowedCiphertext(), false);

    if (false == input.has_raw() && false == input.has_words()) {
        FAIL_1("No payload");
    }

    return true;
}

auto CheckProto_4(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const Seed& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
