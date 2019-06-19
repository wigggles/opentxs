// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"
#include "Internal.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"

#include "internal/api/Api.hpp"

#include "opentxs/OT.hpp"

namespace opentxs
{
api::Context* OT::instance_pointer_{nullptr};
OTFlag OT::running_{Flag::Factory(true)};

const api::Context& OT::App()
{
    OT_ASSERT(nullptr != instance_pointer_);

    return *instance_pointer_;
}

void OT::Cleanup()
{
    if (nullptr != instance_pointer_) {
        auto ot = dynamic_cast<api::internal::Context*>(instance_pointer_);

        if (nullptr != ot) { ot->shutdown(); }

        delete instance_pointer_;
        instance_pointer_ = nullptr;
    }
}

const api::client::Manager& OT::ClientFactory(
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    auto& ot = Start(args, gcInterval, externalPasswordCallback);

    return ot.StartClient(args, 0);
}

void OT::Join()
{
    while (nullptr != instance_pointer_) {
        Log::Sleep(std::chrono::milliseconds(250));
    }
}

const opentxs::Flag& OT::Running() { return running_; }

const api::client::Manager& OT::RecoverClient(
    const ArgList& args,
    const std::string& words,
    const std::string& passphrase,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    auto& ot = Start(args, gcInterval, externalPasswordCallback);
    auto& client = ot.StartClient(args, 0);
    auto reason = client.Factory().PasswordPrompt("Recovering a BIP-39 seed");

    if (0 < words.size()) {
        OTPassword wordList{};
        OTPassword phrase{};
        wordList.setPassword(words);
        phrase.setPassword(passphrase);
        client.Seeds().ImportSeed(wordList, phrase, reason);
    }

    return client;
}

const api::server::Manager& OT::ServerFactory(
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    auto& ot = Start(args, gcInterval, externalPasswordCallback);

    return ot.StartServer(args, 0, false);
}

const api::Context& OT::Start(
    const ArgList& args,
    const std::chrono::seconds gcInterval,
    OTCaller* externalPasswordCallback)
{
    OT_ASSERT(nullptr == instance_pointer_);

    instance_pointer_ =
        Factory::Context(running_, args, gcInterval, externalPasswordCallback);

    OT_ASSERT(nullptr != instance_pointer_);

    auto ot = dynamic_cast<api::internal::Context*>(instance_pointer_);

    OT_ASSERT(nullptr != ot);

    ot->Init();

    return *instance_pointer_;
}
}  // namespace opentxs
