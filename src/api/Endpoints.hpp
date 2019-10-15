// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Endpoints final : public opentxs::api::Endpoints
{
public:
    std::string AccountUpdate() const final;
    std::string ConnectionStatus() const final;
    std::string ContactUpdate() const final;
    std::string DhtRequestNym() const final;
    std::string DhtRequestServer() const final;
    std::string DhtRequestUnit() const final;
    std::string FindNym() const final;
    std::string FindServer() const final;
    std::string FindUnitDefinition() const final;
    std::string InternalProcessPushNotification() const final;
    std::string InternalPushNotification() const final;
    std::string IssuerUpdate() const final;
    std::string NymDownload() const final;
    std::string PairEvent() const final;
    std::string PeerReplyUpdate() const final;
    std::string PeerRequestUpdate() const final;
    std::string PendingBailment() const final;
    std::string ServerReplyReceived() const final;
    std::string ServerRequestSent() const final;
    std::string ServerUpdate() const final;
    std::string TaskComplete() const final;
    std::string ThreadUpdate(const std::string& thread) const final;
    std::string WidgetUpdate() const final;
    std::string WorkflowAccountUpdate() const final;

    ~Endpoints() final = default;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& zmq_;
    const int instance_;

    std::string build_inproc_path(const std::string& path, const int version)
        const;
    std::string build_inproc_path(
        const std::string& path,
        const int version,
        const std::string& suffix) const;

    Endpoints(const opentxs::network::zeromq::Context& zmq, const int instance);
    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace opentxs::api::implementation
