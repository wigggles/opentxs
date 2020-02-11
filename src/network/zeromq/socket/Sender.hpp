// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::network::zeromq::socket::implementation
{
template <typename Interface, typename ImplementationParent = Socket>
class Sender : virtual public Interface, virtual public ImplementationParent
{
protected:
    Sender() noexcept;

    ~Sender() override = default;

private:
    bool send(zeromq::Message& data) const noexcept override;

    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    Sender& operator=(const Sender&) = delete;
    Sender& operator=(Sender&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
