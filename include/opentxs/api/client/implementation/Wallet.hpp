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

#ifndef OPENTXS_API_CLIENT_IMPLEMENTATION_WALLET_HPP
#define OPENTXS_API_CLIENT_IMPLEMENTATION_WALLET_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/client/Wallet.hpp"

#include <map>
#include <mutex>
#include <tuple>

namespace opentxs
{

class ClientContext;
class Context;
class Message;
class PeerObject;
class ServerContext;

namespace api
{
class Native;

namespace implementation
{
class Native;
}  // namespace implementation

namespace client
{
namespace implementation
{

class Wallet : virtual public opentxs::api::client::Wallet
{
public:
    std::shared_ptr<const opentxs::Context> Context(
        const Identifier& notaryID,
        const Identifier& clientNymID) override;
    std::shared_ptr<const opentxs::ClientContext> ClientContext(
        const Identifier& localNymID,
        const Identifier& remoteNymID) override;
    std::shared_ptr<const opentxs::ServerContext> ServerContext(
        const Identifier& localNymID,
        const Identifier& remoteID) override;
    Editor<opentxs::Context> mutable_Context(
        const Identifier& notaryID,
        const Identifier& clientNymID) override;
    Editor<opentxs::ClientContext> mutable_ClientContext(
        const Identifier& localNymID,
        const Identifier& remoteNymID) override;
    Editor<opentxs::ServerContext> mutable_ServerContext(
        const Identifier& localNymID,
        const Identifier& remoteID) override;
    std::set<Identifier> IssuerList(const Identifier& nymID) const override;
    std::shared_ptr<const class Issuer> Issuer(
        const Identifier& nymID,
        const Identifier& issuerID) const override;
    Editor<class Issuer> mutable_Issuer(
        const Identifier& nymID,
        const Identifier& issuerID) const override;
    ConstNym Nym(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) override;
    ConstNym Nym(const proto::CredentialIndex& nym) override;
    NymData mutable_Nym(const Identifier& id) override;
    ObjectList NymList() const override;
    std::shared_ptr<proto::PeerReply> PeerReply(
        const Identifier& nym,
        const Identifier& reply,
        const StorageBox& box) const override;
    bool PeerReplyComplete(
        const Identifier& nym,
        const Identifier& replyOrRequest) const override;
    bool PeerReplyCreate(
        const Identifier& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const override;
    bool PeerReplyCreateRollback(
        const Identifier& nym,
        const Identifier& request,
        const Identifier& reply) const override;
    ObjectList PeerReplySent(const Identifier& nym) const override;
    ObjectList PeerReplyIncoming(const Identifier& nym) const override;
    ObjectList PeerReplyFinished(const Identifier& nym) const override;
    ObjectList PeerReplyProcessed(const Identifier& nym) const override;
    bool PeerReplyReceive(const Identifier& nym, const PeerObject& reply)
        const override;
    std::shared_ptr<proto::PeerRequest> PeerRequest(
        const Identifier& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time) const override;
    bool PeerRequestComplete(const Identifier& nym, const Identifier& reply)
        const override;
    bool PeerRequestCreate(
        const Identifier& nym,
        const proto::PeerRequest& request) const override;
    bool PeerRequestCreateRollback(
        const Identifier& nym,
        const Identifier& request) const override;
    bool PeerRequestDelete(
        const Identifier& nym,
        const Identifier& request,
        const StorageBox& box) const override;
    ObjectList PeerRequestSent(const Identifier& nym) const override;
    ObjectList PeerRequestIncoming(const Identifier& nym) const override;
    ObjectList PeerRequestFinished(const Identifier& nym) const override;
    ObjectList PeerRequestProcessed(const Identifier& nym) const override;
    bool PeerRequestReceive(const Identifier& nym, const PeerObject& request)
        const override;
    bool PeerRequestUpdate(
        const Identifier& nym,
        const Identifier& request,
        const StorageBox& box) const override;
    bool RemoveServer(const Identifier& id) override;
    bool RemoveUnitDefinition(const Identifier& id) override;
    ConstServerContract Server(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) override;
    ConstServerContract Server(const proto::ServerContract& contract) override;
    ConstServerContract Server(
        const std::string& nymid,
        const std::string& name,
        const std::string& terms,
        const std::list<ServerContract::Endpoint>& endpoints) override;
    ObjectList ServerList() override;
    bool SetNymAlias(const Identifier& id, const std::string& alias) override;
    bool SetServerAlias(const Identifier& id, const std::string& alias)
        override;
    bool SetUnitDefinitionAlias(const Identifier& id, const std::string& alias)
        override;
    ObjectList UnitDefinitionList() override;
    ConstUnitDefinition UnitDefinition(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) override;
    ConstUnitDefinition UnitDefinition(
        const proto::UnitDefinition& contract) override;
    ConstUnitDefinition UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t& power,
        const std::string& fraction) override;
    ConstUnitDefinition UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms) override;

    ~Wallet();

private:
    typedef std::pair<std::mutex, std::shared_ptr<class Nym>> NymLock;
    typedef std::map<std::string, NymLock> NymMap;
    typedef std::map<std::string, std::shared_ptr<class ServerContract>>
        ServerMap;
    typedef std::map<std::string, std::shared_ptr<class UnitDefinition>>
        UnitMap;
    typedef std::pair<std::string, std::string> ContextID;
    typedef std::map<ContextID, std::shared_ptr<class Context>> ContextMap;
    typedef std::pair<Identifier, Identifier> IssuerID;
    typedef std::pair<std::mutex, std::shared_ptr<class Issuer>> IssuerLock;
    typedef std::map<IssuerID, IssuerLock> IssuerMap;

    friend class opentxs::api::implementation::Native;

    Native& ot_;
    NymMap nym_map_;
    ServerMap server_map_;
    UnitMap unit_map_;
    ContextMap context_map_;
    mutable IssuerMap issuer_map_;
    mutable std::mutex nym_map_lock_;
    mutable std::mutex server_map_lock_;
    mutable std::mutex unit_map_lock_;
    mutable std::mutex context_map_lock_;
    mutable std::mutex issuer_map_lock_;
    mutable std::mutex peer_map_lock_;
    mutable std::map<std::string, std::mutex> peer_lock_;
    mutable std::mutex nymfile_map_lock_;
    mutable std::map<Identifier, std::mutex> nymfile_lock_;

    std::mutex& nymfile_lock(const Identifier& nymID) const;
    std::mutex& peer_lock(const std::string& nymID) const;
    void save(class Context* context) const;
    void save(const Lock& lock, class Issuer* in) const;

    std::shared_ptr<class Context> context(
        const Identifier& localNymID,
        const Identifier& remoteNymID);
    IssuerLock& issuer(
        const Identifier& nymID,
        const Identifier& issuerID,
        const bool create) const;

    /**   Save an instantiated server contract to storage and add to internal
     *    map.
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the instantiated ServerContract object
     */
    ConstServerContract Server(std::unique_ptr<ServerContract>& contract);
    Identifier ServerToNym(Identifier& serverID);

    /**   Save an instantiated unit definition to storage and add to internal
     *    map.
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the instantiated UnitDefinition object
     */
    ConstUnitDefinition UnitDefinition(
        std::unique_ptr<class UnitDefinition>& contract);

    Wallet(Native& ot);
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace implementation
}  // namespace client
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_API_CLIENT_IMPLEMENTATION_WALLET_HPP
