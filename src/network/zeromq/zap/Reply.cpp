// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "network/zeromq/Message.hpp"

#include <sstream>
#include <string>

#include "Reply.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Reply>;

namespace opentxs::network::zeromq::zap
{
OTZMQZAPReply Reply::Factory(
    const Request& request,
    const zap::Status& code,
    const std::string& status,
    const std::string& userID,
    const Data& metadata,
    const std::string& version)
{
    return OTZMQZAPReply(new implementation::Reply(
        request, code, status, userID, metadata, version));
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
const Reply::CodeMap Reply::code_map_{
    {Status::Success, "200"},
    {Status::TemporaryError, "300"},
    {Status::AuthFailure, "400"},
    {Status::SystemError, "500"},
};
const Reply::CodeReverseMap Reply::code_reverse_map_{
    invert_code_map(code_map_)};

Reply::Reply(
    const Request& request,
    const zap::Status& code,
    const std::string& status,
    const std::string& userID,
    const Data& metadata,
    const std::string& version)
    : zeromq::implementation::Message()
{
    if (0 < request.Header().size()) {
        for (const auto& frame : request.Header()) {
            messages_.emplace_back(frame);
        }
    }

    messages_.emplace_back(Frame::Factory());
    messages_.emplace_back(Frame::Factory(version));
    messages_.emplace_back(Frame::Factory(request.RequestID()));
    messages_.emplace_back(Frame::Factory(code_to_string(code)));
    messages_.emplace_back(Frame::Factory(status));
    messages_.emplace_back(Frame::Factory(userID));
    messages_.emplace_back(Frame::Factory(metadata));
}

Reply::Reply(const Reply& rhs)
    : zeromq::Message()
    , zeromq::zap::Reply()
    , zeromq::implementation::Message()
{
    messages_ = rhs.messages_;
}

std::string Reply::code_to_string(const zap::Status& code)
{
    try {
        return code_map_.at(code);
    } catch (...) {

        return "";
    }
}

Reply::CodeReverseMap Reply::invert_code_map(const CodeMap& input)
{
    CodeReverseMap output{};

    for (const auto& [type, string] : input) { output.emplace(string, type); }

    return output;
}

std::string Reply::Debug() const
{
    std::stringstream output{};
    const auto body = Body();

    if (VERSION_POSITION < body.size()) {
        output << "Version: " << std::string(body.at(VERSION_POSITION)) << "\n";
    }

    if (REQUEST_ID_POSITION < body.size()) {
        const auto& requestID = body.at(REQUEST_ID_POSITION);

        if (0 < requestID.size()) {
            output << "Request ID: 0x" << Data::Factory(requestID)->asHex()
                   << "\n";
        }
    }

    if (STATUS_CODE_POSITION < body.size()) {
        output << "Status Code: " << std::string(body.at(STATUS_CODE_POSITION))
               << "\n";
    }

    if (STATUS_TEXT_POSITION < body.size()) {
        output << "Status Text: " << std::string(body.at(STATUS_TEXT_POSITION))
               << "\n";
    }

    if (USER_ID_POSITION < body.size()) {
        output << "User ID: " << std::string(body.at(USER_ID_POSITION)) << "\n";
    }

    if (METADATA_POSITION < body.size()) {
        const auto& metadata = body.at(METADATA_POSITION);

        if (0 < metadata.size()) {
            output << "Metadata: 0x" << Data::Factory(metadata)->asHex()
                   << "\n";
        }
    }

    return output.str();
}

OTData Reply::Metadata() const
{
    const auto& frame = Body_at(METADATA_POSITION);

    return Data::Factory(frame.data(), frame.size());
}

OTData Reply::RequestID() const
{
    const auto& frame = Body_at(REQUEST_ID_POSITION);

    return Data::Factory(frame.data(), frame.size());
}

zap::Status Reply::string_to_code(const std::string& string)
{
    try {
        return code_reverse_map_.at(string);
    } catch (...) {

        return Status::Unknown;
    }
}
}  // namespace opentxs::network::zeromq::zap::implementation
