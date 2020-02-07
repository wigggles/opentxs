// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"

#include <set>

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Reject final : virtual public bitcoin::Message
{
public:
    const std::string& getMessage() const noexcept { return message_; }
    bitcoin::RejectCode getRejectCode() const noexcept { return code_; }
    const std::string& getReason() const noexcept { return reason_; }
    OTData getExtraData() const noexcept { return Data::Factory(extra_); }

    ~Reject() final = default;

    OTData payload() const noexcept final;

private:
    friend opentxs::Factory;

    const std::string message_;
    const bitcoin::RejectCode code_{};
    const std::string reason_;
    const OTData extra_;

    Reject(
        const api::internal::Core& api,
        const blockchain::Type network,
        const std::string& message,
        const bitcoin::RejectCode code,
        const std::string& reason,
        const Data& extra) noexcept;
    Reject(
        const api::internal::Core& api,
        std::unique_ptr<Header> header,
        const std::string& message,
        const bitcoin::RejectCode code,
        const std::string& reason,
        const Data& extra) noexcept(false);
    Reject(const Reject&) = delete;
    Reject(Reject&&) = delete;
    Reject& operator=(const Reject&) = delete;
    Reject& operator=(Reject&&) = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
