// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTCallback.hpp"

#include "opentxs/core/Log.hpp"

#include <ostream>

namespace opentxs
{

OTCallback::~OTCallback()
{
    otErr << "OTCallback::~OTCallback:  (This should only happen ONCE ONLY -- "
             "as the application is closing.)\n";
    //    std::cout << "OTCallback::~OTCallback()" << std:: endl;
}

}  // namespace opentxs
