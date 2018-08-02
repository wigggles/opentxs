// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/OT.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "Bip32.hpp"

//#define OT_METHOD "opentxs::crypto::implementation::Bip32::"

namespace opentxs::crypto
{
std::string Print(const proto::HDPath& node)
{
    std::stringstream output{};
    output << node.root();

    for (const auto& child : node.child()) {
        output << " / ";
        const auto max = static_cast<std::uint32_t>(Bip32Child::HARDENED);

        if (max > child) {
            output << std::to_string(child);
        } else {
            output << std::to_string(child - max) << "'";
        }
    }

    return output.str();
}
}  // namespace opentxs::crypto
#endif
