// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_OPENDHT_HPP
#define OPENTXS_NETWORK_OPENDHT_HPP

#include "opentxs/Forward.hpp"

#if OT_DHT
#include "opentxs/Types.hpp"

#include <string>

namespace opentxs
{
namespace network
{
class OpenDHT
{
public:
    EXPORT virtual void Insert(
        const std::string& key,
        const std::string& value,
        DhtDoneCallback cb = {}) const = 0;
    EXPORT virtual void Retrieve(
        const std::string& key,
        DhtResultsCallback vcb,
        DhtDoneCallback dcb = {}) const = 0;

    EXPORT virtual ~OpenDHT() = default;

protected:
    OpenDHT() = default;

private:
    OpenDHT(const OpenDHT&) = delete;
    OpenDHT(OpenDHT&&) = delete;
    OpenDHT& operator=(const OpenDHT&) = delete;
    OpenDHT& operator=(OpenDHT&&) = delete;
};
}  // namespace network
}  // namespace opentxs
#endif  // OT_DHT
#endif
