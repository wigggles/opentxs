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

#ifndef OPENTXS_FORWARD_INTERNAL_HPP
#define OPENTXS_FORWARD_INTERNAL_HPP

#include "opentxs/Forward.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{
class Wallet;
}  // namespace api::client::implementation
}  // namespace api::client

namespace implementation
{
class Api;
class Crypto;
class Native;
class UI;
}  // namespace api::implementation

namespace network
{
namespace implementation
{
class Context;
class Dht;
class ZMQ;
}  // namespace api::network::implementation
}  // namespace api::network
}  // namespace api

namespace storage
{
class Root;
}  // namespace opentxs::storage

namespace ui
{
namespace implementation
{
class ActivityThread;
class ActivityThreadItem;
class ActivitySummary;
class ActivitySummaryItem;
class ContactList;
class ContactListInterface;
class ContactListItem;
class MessagableList;
}  // namespace opentxs::ui::implementation
}  // namespace opentxs::ui

class DhtConfig;
#if OT_CRYPTO_USING_LIBSECP256K1
class Libsecp256k1;
#endif
class Libsodium;
#if OT_CRYPTO_USING_OPENSSL
class OpenSSL;
#endif
class StorageConfig;
class StorageMultiplex;
#if OT_CRYPTO_USING_TREZOR
class TrezorCrypto;
#endif
}  // namespace opentxs
#endif  // OPENTXS_FORWARD_INTERNAL_HPP
