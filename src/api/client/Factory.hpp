// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::client::implementation
{
class Factory final : public opentxs::api::implementation::Factory
{
public:
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& message) const final;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment) const final;
#if OT_CASH
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse) const final;
#endif
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const final;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const OTPeerRequest request,
        const VersionNumber version) const final;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const final;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const final;

    ~Factory() final = default;

private:
    friend opentxs::Factory;

    const api::client::internal::Manager& client_;

    Factory(const api::client::internal::Manager& client);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::client::implementation
