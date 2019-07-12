// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "Socket.hpp"

#include "Sender.hpp"

//#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Sender::"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename Interface, typename ImplementationParent>
Sender<Interface, ImplementationParent>::Sender() noexcept
    : Interface()
{
}

template <typename Interface, typename ImplementationParent>
bool Sender<Interface, ImplementationParent>::send(
    zeromq::Message& message) const noexcept
{
    Lock lock(this->lock_);

    if (false == this->running_.get()) { return false; }

    return this->send_message(lock, message);
}
}  // namespace opentxs::network::zeromq::socket::implementation
