// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Data.hpp"

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

#include "Nopayload.hpp"

namespace bitcoin = opentxs::blockchain::p2p::bitcoin;
namespace message = bitcoin::message;

namespace opentxs
{
blockchain::p2p::bitcoin::message::internal::Filterclear* Factory::
    BitcoinP2PFilterclear(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Filterclear>;

    return new ReturnType(api, std::move(header));
}

blockchain::p2p::bitcoin::message::internal::Filterclear* Factory::
    BitcoinP2PFilterclear(
        const api::internal::Core& api,
        const blockchain::Type network)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Filterclear>;

    return new ReturnType(api, network, bitcoin::Command::filterclear);
}
blockchain::p2p::bitcoin::message::internal::Getaddr* Factory::
    BitcoinP2PGetaddr(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Getaddr>;

    return new ReturnType(api, std::move(header));
}

blockchain::p2p::bitcoin::message::internal::Getaddr* Factory::
    BitcoinP2PGetaddr(
        const api::internal::Core& api,
        const blockchain::Type network)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Getaddr>;

    return new ReturnType(api, network, bitcoin::Command::getaddr);
}

blockchain::p2p::bitcoin::message::internal::Mempool* Factory::
    BitcoinP2PMempool(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Mempool>;

    return new ReturnType(api, std::move(header));
}

blockchain::p2p::bitcoin::message::internal::Mempool* Factory::
    BitcoinP2PMempool(
        const api::internal::Core& api,
        const blockchain::Type network)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Mempool>;

    return new ReturnType(api, network, bitcoin::Command::mempool);
}

blockchain::p2p::bitcoin::message::internal::Sendheaders* Factory::
    BitcoinP2PSendheaders(
        const api::internal::Core& api,
        std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Sendheaders>;

    return new ReturnType(api, std::move(header));
}

blockchain::p2p::bitcoin::message::internal::Sendheaders* Factory::
    BitcoinP2PSendheaders(
        const api::internal::Core& api,
        const blockchain::Type network)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Sendheaders>;

    return new ReturnType(api, network, bitcoin::Command::sendheaders);
}

blockchain::p2p::bitcoin::message::internal::Verack* Factory::BitcoinP2PVerack(
    const api::internal::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Verack>;

    return new ReturnType(api, std::move(header));
}

blockchain::p2p::bitcoin::message::internal::Verack* Factory::BitcoinP2PVerack(
    const api::internal::Core& api,
    const blockchain::Type network)
{
    using ReturnType =
        message::implementation::Nopayload<message::internal::Verack>;

    return new ReturnType(api, network, bitcoin::Command::verack);
}
}  // namespace opentxs
