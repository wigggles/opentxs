// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_SWIGWRAP_HPP
#define OPENTXS_CLIENT_SWIGWRAP_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{
class Manager;
}  // namespace implementation
}  // namespace client
}  // namespace api

class SwigWrap
{
public:
private:
    friend api::client::implementation::Manager;

    static const api::client::Manager* client_;

    SwigWrap();
    ~SwigWrap() = default;
};
}  // namespace opentxs
#endif
