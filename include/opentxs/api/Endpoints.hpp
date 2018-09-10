// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_ENDPOINTS_HPP
#define OPENTXS_API_ENDPOINTS_HPP

#include "opentxs/Forward.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(ZMQEndpoints) opentxs::api::Endpoints;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
class Endpoints
{
public:
    EXPORT virtual std::string AccountUpdate() const = 0;
    EXPORT virtual std::string ConnectionStatus() const = 0;
    EXPORT virtual std::string ContactUpdate() const = 0;
    EXPORT virtual std::string DhtRequestNym() const = 0;
    EXPORT virtual std::string DhtRequestServer() const = 0;
    EXPORT virtual std::string DhtRequestUnit() const = 0;
    EXPORT virtual std::string InternalProcessPushNotification() const = 0;
    EXPORT virtual std::string InternalPushNotification() const = 0;
    EXPORT virtual std::string IssuerUpdate() const = 0;
    EXPORT virtual std::string NymDownload() const = 0;
    EXPORT virtual std::string PairEvent() const = 0;
    EXPORT virtual std::string PendingBailment() const = 0;
    EXPORT virtual std::string ServerReplyReceived() const = 0;
    EXPORT virtual std::string ServerRequestSent() const = 0;
    EXPORT virtual std::string ServerUpdate() const = 0;
    EXPORT virtual std::string TaskComplete() const = 0;
    EXPORT virtual std::string ThreadUpdate(
        const std::string& thread) const = 0;
    EXPORT virtual std::string WidgetUpdate() const = 0;
    EXPORT virtual std::string WorkflowAccountUpdate() const = 0;

    EXPORT virtual ~Endpoints() = default;

protected:
    Endpoints() = default;

private:
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
