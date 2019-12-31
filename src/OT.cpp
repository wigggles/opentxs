// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"
#include "Internal.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"

#include "internal/api/Api.hpp"

#include <atomic>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>

#include "opentxs/OT.hpp"

namespace opentxs
{
api::internal::Context* instance_pointer_{nullptr};
OTFlag running_{Flag::Factory(true)};

const api::Context& Context()
{
    if (nullptr == instance_pointer_) {
        std::runtime_error("Context is not initialized");
    }

    return *instance_pointer_;
}

void Cleanup()
{
    if (nullptr != instance_pointer_) {
        instance_pointer_->shutdown();
        delete instance_pointer_;
        instance_pointer_ = nullptr;
    }
}

const api::Context& InitContext(
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    if (nullptr != instance_pointer_) {
        std::runtime_error("Context is not initialized");
    }

    instance_pointer_ =
        Factory::Context(running_, args, gcInterval, externalPasswordCallback);

    OT_ASSERT(nullptr != instance_pointer_);

    instance_pointer_->Init();

    return *instance_pointer_;
}

void Join()
{
    while (nullptr != instance_pointer_) {
        Sleep(std::chrono::milliseconds(250));
    }
}
}  // namespace opentxs
