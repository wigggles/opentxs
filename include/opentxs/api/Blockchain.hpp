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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_API_BLOCKCHAIN_HPP
#define OPENTXS_API_BLOCKCHAIN_HPP

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>

namespace opentxs
{

class CryptoEngine;
class Storage;
class Wallet;

class Blockchain
{
public:
    std::set<Identifier> AccountList(
        const Identifier& nymID,
        const proto::ContactItemType type) const;
    std::unique_ptr<proto::Bip44Address> AllocateAddress(
        const Identifier& nymID,
        const Identifier& accountID,
        const std::string& label = "",
        const BIP44Chain chain = EXTERNAL_CHAIN) const;
    bool AssignAddress(
        const Identifier& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const Identifier& contactID,
        const BIP44Chain chain = EXTERNAL_CHAIN) const;
    std::unique_ptr<proto::Bip44Address> LoadAddress(
        const Identifier& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const BIP44Chain chain) const;
    Identifier NewAccount(
        const Identifier& nymID,
        const proto::ContactItemType type) const;

    ~Blockchain() = default;

private:
    typedef std::map<Identifier, std::mutex> IDLock;
    friend class OT;

    CryptoEngine& crypto_;
    Storage& storage_;
    Wallet& wallet_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable IDLock account_lock_;
    proto::Bip44Address& add_address(
        const std::uint32_t index,
        proto::Bip44Account& account,
        const BIP44Chain chain) const;
    std::uint8_t address_prefix(const proto::ContactItemType type) const;

    Bip44Type bip44_type(const proto::ContactItemType type) const;
    std::string calculate_address(
        const proto::Bip44Account& account,
        const BIP44Chain chain,
        const std::uint32_t index) const;
    proto::Bip44Address& find_address(
        const std::uint32_t index,
        const BIP44Chain chain,
        proto::Bip44Account& account) const;
    std::unique_ptr<proto::Bip44Address> load_address(
        const Identifier& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const BIP44Chain chain) const;

    Blockchain(CryptoEngine& crypto, Storage& storage, Wallet& wallet);
    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain operator=(const Blockchain&) = delete;
    Blockchain operator=(Blockchain&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_API_BLOCKCHAIN_HPP
