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

#ifndef OPENTXS_STORAGE_TREE_PEERREQUESTS_HPP
#define OPENTXS_STORAGE_TREE_PEERREQUESTS_HPP

#include "opentxs/core/app/Editor.hpp"
#include "opentxs/storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{

class Nym;

class PeerRequests : public Node
{
private:
    friend class Nym;

    void init(const std::string& hash) override;
    bool save(const std::unique_lock<std::mutex>& lock) override;
    proto::StorageNymList serialize() const;

    PeerRequests(
        const Storage& storage,
        const keyFunction& migrate,
        const std::string& hash);
    PeerRequests() = delete;
    PeerRequests(const PeerRequests&) = delete;
    PeerRequests(PeerRequests&&) = delete;
    PeerRequests operator=(const PeerRequests&) = delete;
    PeerRequests operator=(PeerRequests&&) = delete;

public:
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::PeerRequest>& output,
        const bool checking) const;

    bool Delete(const std::string& id);
    bool Store(const proto::PeerRequest& data, const std::string& alias);

    ~PeerRequests() = default;
};
}  // namespace storage
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_TREE_PEERREQUESTS_HPP
