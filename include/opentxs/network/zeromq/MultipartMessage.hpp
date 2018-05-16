/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_NETWORK_ZEROMQ_MULTIPARTMESSAGE_HPP
#define OPENTXS_NETWORK_ZEROMQ_MULTIPARTMESSAGE_HPP

#include "opentxs/Forward.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator+=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator==;
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator!=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator<;
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator<=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator>;
%ignore opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>::operator>=;
%template(OTZMQMultipartMessage) opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>;
%rename(ZMQMultipartMessage) opentxs::network::zeromq::MultipartMessage;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class MultipartMessage
{
public:
    EXPORT static opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>
    Factory();
    //    EXPORT static
    //    opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>
    //    ReplyFactory(const MultipartMessage& request);

    EXPORT virtual const Message& at(const std::size_t index) const = 0;
    EXPORT virtual Message& at(const std::size_t index) = 0;
    EXPORT virtual const FrameIterator begin() const = 0;
    EXPORT virtual FrameIterator begin() = 0;
    //    EXPORT virtual FrameIterator Body_begin() const = 0;
    //    EXPORT virtual FrameIterator Body_end() const = 0;
    //    EXPORT virtual const Message& Body_at(const std::size_t index) const =
    //    0; EXPORT virtual const FrameSection& Body() const = 0;
    EXPORT virtual const FrameIterator end() const = 0;
    EXPORT virtual FrameIterator end() = 0;
    //    EXPORT virtual const FrameSection& Header() const = 0;
    //    EXPORT virtual FrameIterator Header_begin() const = 0;
    //    EXPORT virtual FrameIterator Header_end() const = 0;
    //    EXPORT virtual const Message& Header_at(const std::size_t index) const
    //    = 0;
    EXPORT virtual std::size_t size() const = 0;

    EXPORT virtual Message& AddFrame() = 0;
    EXPORT virtual Message& AddFrame(const opentxs::Data& input) = 0;
    EXPORT virtual Message& AddFrame(const std::string& input) = 0;

    EXPORT virtual ~MultipartMessage() = default;

protected:
    MultipartMessage() = default;

private:
    friend OTZMQMultipartMessage;

    virtual MultipartMessage* clone() const = 0;

    MultipartMessage(const MultipartMessage&) = delete;
    MultipartMessage(MultipartMessage&&) = default;
    MultipartMessage& operator=(const MultipartMessage&) = delete;
    MultipartMessage& operator=(MultipartMessage&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_MULTIPARTMESSAGE_HPP
