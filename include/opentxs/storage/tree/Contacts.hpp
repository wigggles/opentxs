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

#ifndef OPENTXS_STORAGE_TREE_CONTACTS_HPP
#define OPENTXS_STORAGE_TREE_CONTACTS_HPP

#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>

namespace opentxs
{
namespace storage
{

class Tree;

class Contacts : public Node
{
public:
    std::string Alias(const std::string& id) const;
    std::string AddressOwner(proto::ContactItemType chain, std::string address)
        const;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& output,
        std::string& alias,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool SetAlias(const std::string& id, const std::string& alias);
    bool Store(const proto::Contact& data, const std::string& alias);

    ~Contacts() = default;

private:
    friend class Tree;
    typedef std::pair<proto::ContactItemType, std::string> Address;

    std::map<Address, std::string> address_index_{};
    std::map<std::string, std::set<std::string>> merge_{};
    std::map<std::string, std::string> merged_{};

    const std::string& nomalize_id(const std::string& input) const;
    bool save(const std::unique_lock<std::mutex>& lock) const override;
    proto::StorageContacts serialize() const;

    void extract_addresses(const Lock& lock, const proto::Contact& data);
    void init(const std::string& hash) override;
    void reconcile_maps(const Lock& lock, const proto::Contact& data);
    void reverse_merged();

    Contacts(const StorageDriver& storage, const std::string& hash);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    Contacts operator=(const Contacts&) = delete;
    Contacts operator=(Contacts&&) = delete;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_CONTACTS_HPP
