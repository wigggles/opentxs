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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_MULTIPARTMESSAGE_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_MULTIPARTMESSAGE_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/network/zeromq/MultipartMessage.hpp"

namespace opentxs::network::zeromq::implementation
{
class MultipartMessage : virtual public zeromq::MultipartMessage
{
public:
    const Message& at(const std::size_t index) const override;
    Message& at(const std::size_t index) override;
    const FrameIterator begin() const override;
    FrameIterator begin() override;
    //    FrameIterator Body_begin() const override;
    //    FrameIterator Body_end() const override;
    //    const Message& Body_at(const std::size_t index) const =
    //    0;
    // const FrameSection& Body() const override;
    const FrameIterator end() const override;
    FrameIterator end() override;
    //    const FrameSection& Header() const override;
    //    FrameIterator Header_begin() const override;
    //    FrameIterator Header_end() const override;
    //    const Message& Header_at(const std::size_t index) const
    //    override;
    std::size_t size() const override;

    Message& AddFrame() override;
    Message& AddFrame(const opentxs::Data& input) override;
    Message& AddFrame(const std::string& input) override;

    ~MultipartMessage() = default;

private:
    friend network::zeromq::MultipartMessage;

    std::vector<OTZMQMessage> messages_{};

    MultipartMessage* clone() const override;

    MultipartMessage();
    MultipartMessage(const MultipartMessage&) = delete;
    MultipartMessage(MultipartMessage&&) = default;
    MultipartMessage& operator=(const MultipartMessage&) = delete;
    MultipartMessage& operator=(MultipartMessage&&) = default;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_MULTIPARTMESSAGE_HPP
