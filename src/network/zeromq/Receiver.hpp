// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <mutex>
#include <thread>

namespace opentxs::network::zeromq::implementation
{
class Receiver
{
protected:
    Receiver(std::mutex& lock, void* socket, const bool startThread);

    virtual ~Receiver();

    std::mutex& receiver_lock_;
    // Not owned by this class
    void* receiver_socket_{nullptr};
    OTFlag receiver_run_;
    std::unique_ptr<std::thread> receiver_thread_{nullptr};

    virtual bool have_callback() const { return false; }

    virtual void process_incoming(const Lock& lock, Message& message) = 0;
    virtual void thread();

private:
    Receiver() = delete;
    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver& operator=(Receiver&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
