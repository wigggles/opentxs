// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Endpoints final : public opentxs::api::Endpoints
{
public:
    std::string AccountUpdate() const noexcept final;
    std::string BlockchainReorg() const noexcept final;
    std::string ConnectionStatus() const noexcept final;
    std::string ContactUpdate() const noexcept final;
    std::string DhtRequestNym() const noexcept final;
    std::string DhtRequestServer() const noexcept final;
    std::string DhtRequestUnit() const noexcept final;
    std::string FindNym() const noexcept final;
    std::string FindServer() const noexcept final;
    std::string FindUnitDefinition() const noexcept final;
    std::string InternalBlockchainAsioContext() const noexcept final;
    std::string InternalProcessPushNotification() const noexcept final;
    std::string InternalPushNotification() const noexcept final;
    std::string IssuerUpdate() const noexcept final;
    std::string NymDownload() const noexcept final;
    std::string PairEvent() const noexcept final;
    std::string PeerReplyUpdate() const noexcept final;
    std::string PeerRequestUpdate() const noexcept final;
    std::string PendingBailment() const noexcept final;
    std::string ServerReplyReceived() const noexcept final;
    std::string ServerRequestSent() const noexcept final;
    std::string ServerUpdate() const noexcept final;
    std::string Shutdown() const noexcept final;
    std::string TaskComplete() const noexcept final;
    std::string ThreadUpdate(const std::string& thread) const noexcept final;
    std::string WidgetUpdate() const noexcept final;
    std::string WorkflowAccountUpdate() const noexcept final;

    ~Endpoints() final = default;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& zmq_;
    const int instance_;

    std::string build_inproc_path(const std::string& path, const int version)
        const noexcept;
    std::string build_inproc_path(
        const std::string& path,
        const int version,
        const std::string& suffix) const noexcept;

    Endpoints(
        const opentxs::network::zeromq::Context& zmq,
        const int instance) noexcept;
    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace opentxs::api::implementation
