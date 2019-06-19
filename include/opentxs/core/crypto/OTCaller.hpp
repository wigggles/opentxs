// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTCALLER_HPP
#define OPENTXS_CORE_CRYPTO_OTCALLER_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{
class OTCaller final
{
public:
    EXPORT bool HaveCallback() const;

    EXPORT void AskOnce(const PasswordPrompt& prompt, OTPassword& output);
    EXPORT void AskTwice(const PasswordPrompt& prompt, OTPassword& output);
    EXPORT void SetCallback(OTCallback* callback);

    EXPORT OTCaller();

    EXPORT ~OTCaller();

private:
    OTCallback* callback_{nullptr};
};
}  // namespace opentxs
#endif
