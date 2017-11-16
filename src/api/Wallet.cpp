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

#include "opentxs/stdafx.hpp"

#include "opentxs/api/Wallet.hpp"

#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Server.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/Context.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/core/String.hpp"

#include <functional>

#define OT_METHOD "opentxs::Wallet::"

namespace opentxs::api
{

Wallet::Wallet(Native& ot)
    : ot_(ot)
{
}

std::shared_ptr<class Context> Wallet::context(
    const Identifier& localNymID,
    const Identifier& remoteNymID)
{
    const std::string local = String(localNymID).Get();
    const std::string remote = String(remoteNymID).Get();
    const ContextID context = {local, remote};
    auto it = context_map_.find(context);
    const bool inMap = (it != context_map_.end());

    if (inMap) {
        return it->second;
    }

    // Load from storage, if it exists.
    std::shared_ptr<proto::Context> serialized;
    const bool loaded = ot_.DB().Load(
        String(localNymID).Get(), String(remoteNymID).Get(), serialized, true);

    if (!loaded) {
        return nullptr;
    }

    if (local != serialized->localnym()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect localnym in protobuf"
              << std::endl;

        return nullptr;
    }

    if (remote != serialized->remotenym()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect localnym in protobuf"
              << std::endl;

        return nullptr;
    }

    auto& entry = context_map_[context];

    // Obtain nyms.
    const auto localNym = Nym(localNymID);
    const auto remoteNym = Nym(remoteNymID);

    if (!localNym) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load local nym."
              << std::endl;

        return nullptr;
    }

    if (!remoteNym) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load remote nym."
              << std::endl;

        return nullptr;
    }

    switch (serialized->type()) {
        case proto::CONSENSUSTYPE_SERVER: {
            auto& zmq = ot_.ZMQ();
            const auto& server = serialized->servercontext().serverid();
            auto& connection = zmq.Server(server);
            entry.reset(new class ServerContext(
                *serialized, localNym, remoteNym, connection));
        } break;
        case proto::CONSENSUSTYPE_CLIENT: {
            OT_ASSERT(ot_.ServerMode());

            const auto& serverID = ot_.Server().ID();
            entry.reset(new class ClientContext(
                *serialized, localNym, remoteNym, serverID));
        } break;
        default: {
            return nullptr;
        }
    }

    OT_ASSERT(entry);

    const bool valid = entry->Validate();

    if (!valid) {
        context_map_.erase(context);

        otErr << OT_METHOD << __FUNCTION__ << ": invalid signature on context."
              << std::endl;

        return nullptr;
    }

    return entry;
}

std::shared_ptr<const class Context> Wallet::Context(
    const Identifier& notaryID,
    const Identifier& clientNymID)
{
    Identifier serverID = notaryID;
    Identifier local, remote;

    if (ot_.ServerMode()) {
        local = ot_.Server().NymID();
        remote = clientNymID;
    } else {
        local = clientNymID;
        remote = ServerToNym(serverID);
    }

    return context(local, remote);
}

std::shared_ptr<const class ClientContext> Wallet::ClientContext(
    const Identifier&,  // Not used for now.
    const Identifier& remoteNymID)
{
    OT_ASSERT(ot_.ServerMode());

    const auto& serverNymID = ot_.Server().NymID();
    auto base = context(serverNymID, remoteNymID);
    auto output = std::dynamic_pointer_cast<const class ClientContext>(base);

    return output;
}

std::shared_ptr<const class ServerContext> Wallet::ServerContext(
    const Identifier& localNymID,
    const Identifier& remoteID)
{
    Identifier serverID = remoteID;
    auto remoteNymID = ServerToNym(serverID);
    auto base = context(localNymID, remoteNymID);

    auto output = std::dynamic_pointer_cast<const class ServerContext>(base);

    return output;
}

Editor<class Context> Wallet::mutable_Context(
    const Identifier& notaryID,
    const Identifier& clientNymID)
{
    Identifier serverID = notaryID;
    const bool serverMode = ot_.ServerMode();
    Identifier local, remote;

    if (serverMode) {
        local = ot_.Server().NymID();
        remote = clientNymID;
    } else {
        local = clientNymID;
        remote = ServerToNym(serverID);
    }

    auto base = context(local, remote);
    std::function<void(class Context*)> callback =
        [&](class Context* in) -> void { this->save(in); };

    OT_ASSERT(base);

    return Editor<class Context>(base.get(), callback);
}

Editor<class ClientContext> Wallet::mutable_ClientContext(
    const Identifier&,  // Not used for now.
    const Identifier& remoteNymID)
{
    OT_ASSERT(ot_.ServerMode());

    const auto& serverID = ot_.Server().ID();
    const auto& serverNymID = ot_.Server().NymID();
    Lock lock(context_map_lock_);
    auto base = context(serverNymID, remoteNymID);
    std::function<void(class Context*)> callback =
        [&](class Context* in) -> void { this->save(in); };

    if (base) {
        OT_ASSERT(proto::CONSENSUSTYPE_CLIENT == base->Type());
    } else {
        // Obtain nyms.
        const auto local = Nym(serverNymID);

        OT_ASSERT_MSG(local, "Local nym does not exist in the wallet.");

        const auto remote = Nym(remoteNymID);

        OT_ASSERT_MSG(remote, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {String(serverNymID).Get(),
                                     String(remoteNymID).Get()};
        auto& entry = context_map_[contextID];
        entry.reset(new class ClientContext(local, remote, serverID));
        base = entry;
    }

    OT_ASSERT(base);

    auto child = dynamic_cast<class ClientContext*>(base.get());

    OT_ASSERT(nullptr != child);

    return Editor<class ClientContext>(child, callback);
}

Editor<class ServerContext> Wallet::mutable_ServerContext(
    const Identifier& localNymID,
    const Identifier& remoteID)
{
    Lock lock(context_map_lock_);

    Identifier serverID = remoteID;
    Identifier remoteNymID = ServerToNym(serverID);

    auto base = context(localNymID, remoteNymID);

    std::function<void(class Context*)> callback =
        [&](class Context* in) -> void { this->save(in); };

    if (base) {
        OT_ASSERT(proto::CONSENSUSTYPE_SERVER == base->Type());
    } else {
        // Obtain nyms.
        const auto localNym = Nym(localNymID);

        OT_ASSERT_MSG(localNym, "Local nym does not exist in the wallet.");

        const auto remoteNym = Nym(remoteNymID);

        OT_ASSERT_MSG(remoteNym, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {String(localNymID).Get(),
                                     String(remoteNymID).Get()};
        auto& entry = context_map_[contextID];
        auto& zmq = ot_.ZMQ();
        auto& connection = zmq.Server(String(serverID).Get());
        entry.reset(
            new class ServerContext(localNym, remoteNym, serverID, connection));
        base = entry;
    }

    OT_ASSERT(base);

    auto child = dynamic_cast<class ServerContext*>(base.get());

    OT_ASSERT(nullptr != child);

    return Editor<class ServerContext>(child, callback);
}

void Wallet::save(class Context* context) const
{
    if (nullptr == context) {
        return;
    }

    Lock lock(context->lock_);

    context->update_signature(lock);

    OT_ASSERT(context->validate(lock));

    ot_.DB().Store(context->contract(lock));
}

ConstNym Wallet::Nym(
    const Identifier& id,
    const std::chrono::milliseconds& timeout)
{
    const std::string nym = String(id).Get();
    Lock mapLock(nym_map_lock_);
    bool inMap = (nym_map_.find(nym) != nym_map_.end());
    bool valid = false;

    if (!inMap) {
        std::shared_ptr<proto::CredentialIndex> serialized;

        std::string alias;
        bool loaded = ot_.DB().Load(nym, serialized, alias, true);

        if (loaded) {
            auto& pNym = nym_map_[nym].second;
            pNym.reset(new class Nym(id));

            if (pNym) {
                if (pNym->LoadCredentialIndex(*serialized)) {
                    valid = pNym->VerifyPseudonym();
                    pNym->alias_ = alias;
                }
            }
        } else {
            ot_.DHT().GetPublicNym(nym);

            if (timeout > std::chrono::milliseconds(0)) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = std::chrono::milliseconds(100);

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (nym_map_.find(nym) != nym_map_.end());
                    mapLock.unlock();

                    if (found) {
                        break;
                    }
                }

                return Nym(id);  // timeout of zero prevents infinite recursion
            }
        }
    } else {
        auto& pNym = nym_map_[nym].second;
        if (pNym) {
            valid = pNym->VerifyPseudonym();
        }
    }

    if (valid) {
        return nym_map_[nym].second;
    }

    return nullptr;
}

ConstNym Wallet::Nym(const proto::CredentialIndex& publicNym)
{
    const auto& id = publicNym.nymid();
    Identifier nym(id);

    auto existing = Nym(Identifier(nym));

    if (existing) {
        if (existing->Revision() >= publicNym.revision()) {

            return existing;
        }
    }
    existing.reset();

    std::unique_ptr<class Nym> candidate(new class Nym(nym));

    if (candidate) {
        candidate->LoadCredentialIndex(publicNym);

        if (candidate->VerifyPseudonym()) {
            candidate->WriteCredentials();
            Lock mapLock(nym_map_lock_);
            nym_map_.erase(id);
            mapLock.unlock();
        }
    }

    return Nym(nym);
}

NymData Wallet::mutable_Nym(const Identifier& id)
{
    const std::string nym = String(id).Get();
    auto exists = Nym(id);

    if (false == bool(exists)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nym << " not found."
              << std::endl;
    }

    Lock mapLock(nym_map_lock_);
    auto it = nym_map_.find(nym);

    if (nym_map_.end() == it) {
        return NymData(nullptr);
    }

    return NymData(it->second.second);
}

ObjectList Wallet::NymList() const { return ot_.DB().NymList(); }

std::mutex& Wallet::peer_lock(const std::string& nymID) const
{
    Lock map_lock(peer_map_lock_);
    auto& output = peer_lock_[nymID];
    map_lock.unlock();

    return output;
}

std::shared_ptr<proto::PeerReply> Wallet::PeerReply(
    const Identifier& nym,
    const Identifier& reply,
    const StorageBox& box) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));
    std::shared_ptr<proto::PeerReply> output;

    ot_.DB().Load(nymID, String(reply).Get(), box, output);

    return output;
}

bool Wallet::PeerReplyComplete(const Identifier& nym, const Identifier& replyID)
    const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));
    std::shared_ptr<proto::PeerReply> reply;
    const bool haveReply = ot_.DB().Load(
        nymID, String(replyID).Get(), StorageBox::SENTPEERREPLY, reply, false);

    if (!haveReply) {
        otErr << OT_METHOD << __FUNCTION__ << ": sent reply not found."
              << std::endl;

        return false;
    }

    // This reply may have been loaded by request id.
    const auto& realReplyID = reply->id();

    const bool savedReply =
        ot_.DB().Store(*reply, nymID, StorageBox::FINISHEDPEERREPLY);

    if (!savedReply) {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to save finished reply."
              << std::endl;

        return false;
    }

    const bool removedReply = ot_.DB().RemoveNymBoxItem(
        nymID, StorageBox::SENTPEERREPLY, realReplyID);

    if (!removedReply) {
        otErr << OT_METHOD << __FUNCTION__
              << ": failed to delete finished reply from sent box."
              << std::endl;
    }

    return removedReply;
}

bool Wallet::PeerReplyCreate(
    const Identifier& nym,
    const proto::PeerRequest& request,
    const proto::PeerReply& reply) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    if (reply.cookie() != request.id()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": reply cookie does not match request id." << std::endl;

        return false;
    }

    if (reply.type() != request.type()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": reply type does not match request type." << std::endl;

        return false;
    }

    const bool createdReply =
        ot_.DB().Store(reply, nymID, StorageBox::SENTPEERREPLY);

    if (!createdReply) {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to save sent reply."
              << std::endl;

        return false;
    }

    const bool processedRequest =
        ot_.DB().Store(request, nymID, StorageBox::PROCESSEDPEERREQUEST);

    if (!processedRequest) {
        otErr << OT_METHOD << __FUNCTION__
              << ": failed to save processed request." << std::endl;

        return false;
    }

    const bool movedRequest = ot_.DB().RemoveNymBoxItem(
        nymID, StorageBox::INCOMINGPEERREQUEST, request.id());

    if (!processedRequest) {
        otErr << OT_METHOD << __FUNCTION__
              << ": failed to delete processed request from incoming box."
              << std::endl;
    }

    return movedRequest;
}

bool Wallet::PeerReplyCreateRollback(
    const Identifier& nym,
    const Identifier& request,
    const Identifier& reply) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));
    const std::string requestID = String(request).Get();
    const std::string replyID = String(reply).Get();
    std::shared_ptr<proto::PeerRequest> requestItem;
    bool output = true;
    time_t notUsed = 0;
    const bool loadedRequest = ot_.DB().Load(
        nymID,
        requestID,
        StorageBox::PROCESSEDPEERREQUEST,
        requestItem,
        notUsed);

    if (loadedRequest) {
        const bool requestRolledBack = ot_.DB().Store(
            *requestItem, nymID, StorageBox::INCOMINGPEERREQUEST);

        if (requestRolledBack) {
            const bool purgedRequest = ot_.DB().RemoveNymBoxItem(
                nymID, StorageBox::PROCESSEDPEERREQUEST, requestID);
            if (!purgedRequest) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed to delete request from processed box."
                      << std::endl;
                output = false;
            }
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to save request to incoming box." << std::endl;
            output = false;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Did not find the request in the processed box."
              << std::endl;
        output = false;
    }

    const bool removedReply =
        ot_.DB().RemoveNymBoxItem(nymID, StorageBox::SENTPEERREPLY, replyID);

    if (!removedReply) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to delete reply from sent box." << std::endl;
        output = false;
    }

    return output;
}

ObjectList Wallet::PeerReplySent(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(nymID, StorageBox::SENTPEERREPLY);
}

ObjectList Wallet::PeerReplyIncoming(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(nymID, StorageBox::INCOMINGPEERREPLY);
}

ObjectList Wallet::PeerReplyFinished(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(nymID, StorageBox::FINISHEDPEERREPLY);
}

ObjectList Wallet::PeerReplyProcessed(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(nymID, StorageBox::PROCESSEDPEERREPLY);
}

bool Wallet::PeerReplyReceive(const Identifier& nym, const PeerObject& reply)
    const
{
    if (proto::PEEROBJECT_RESPONSE != reply.Type()) {
        otErr << OT_METHOD << __FUNCTION__ << ": This is not a peer reply."
              << std::endl;

        return false;
    }

    if (!reply.Request()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null request." << std::endl;

        return false;
    }

    if (!reply.Reply()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null reply." << std::endl;

        return false;
    }

    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));
    auto requestID = reply.Request()->ID();

    std::shared_ptr<proto::PeerRequest> request;
    std::time_t notUsed;
    const bool haveRequest = ot_.DB().Load(
        nymID,
        String(requestID).Get(),
        StorageBox::SENTPEERREQUEST,
        request,
        notUsed,
        false);

    if (!haveRequest) {
        otErr << OT_METHOD << __FUNCTION__
              << ": the request for this reply does not exist in the sent box."
              << std::endl;

        return false;
    }

    const bool receivedReply = ot_.DB().Store(
        reply.Reply()->Contract(), nymID, StorageBox::INCOMINGPEERREPLY);

    if (!receivedReply) {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to save incoming reply."
              << std::endl;

        return false;
    }

    const bool finishedRequest =
        ot_.DB().Store(*request, nymID, StorageBox::FINISHEDPEERREQUEST);

    if (!finishedRequest) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to save request to finished box." << std::endl;

        return false;
    }

    const bool removedRequest = ot_.DB().RemoveNymBoxItem(
        nymID, StorageBox::SENTPEERREQUEST, String(requestID).Get());

    if (!finishedRequest) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to delete finished request from sent box."
              << std::endl;
    }

    return removedRequest;
}

std::shared_ptr<proto::PeerRequest> Wallet::PeerRequest(
    const Identifier& nym,
    const Identifier& request,
    const StorageBox& box,
    std::time_t& time) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));
    std::shared_ptr<proto::PeerRequest> output;

    ot_.DB().Load(nymID, String(request).Get(), box, output, time);

    return output;
}

bool Wallet::PeerRequestComplete(
    const Identifier& nym,
    const Identifier& replyID) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));
    std::shared_ptr<proto::PeerReply> reply;
    const bool haveReply = ot_.DB().Load(
        nymID,
        String(replyID).Get(),
        StorageBox::INCOMINGPEERREPLY,
        reply,
        false);

    if (!haveReply) {
        otErr << OT_METHOD << __FUNCTION__
              << ": the reply does not exist in the incoming box." << std::endl;

        return false;
    }

    // This reply may have been loaded by request id.
    const auto& realReplyID = reply->id();

    const bool storedReply =
        ot_.DB().Store(*reply, nymID, StorageBox::PROCESSEDPEERREPLY);

    if (!storedReply) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to save reply to processed box." << std::endl;

        return false;
    }

    const bool removedReply = ot_.DB().RemoveNymBoxItem(
        nymID, StorageBox::INCOMINGPEERREPLY, realReplyID);

    if (!removedReply) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to delete completed reply from incoming box."
              << std::endl;
    }

    return removedReply;
}

bool Wallet::PeerRequestCreate(
    const Identifier& nym,
    const proto::PeerRequest& request) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().Store(
        request, String(nym).Get(), StorageBox::SENTPEERREQUEST);
}

bool Wallet::PeerRequestCreateRollback(
    const Identifier& nym,
    const Identifier& request) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().RemoveNymBoxItem(
        String(nym).Get(), StorageBox::SENTPEERREQUEST, String(request).Get());
}

bool Wallet::PeerRequestDelete(
    const Identifier& nym,
    const Identifier& request,
    const StorageBox& box) const
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREQUEST: {
            return ot_.DB().RemoveNymBoxItem(
                String(nym).Get(), box, String(request).Get());
        }
        default: {
            return false;
        }
    }
}

ObjectList Wallet::PeerRequestSent(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(String(nym).Get(), StorageBox::SENTPEERREQUEST);
}

ObjectList Wallet::PeerRequestIncoming(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(
        String(nym).Get(), StorageBox::INCOMINGPEERREQUEST);
}

ObjectList Wallet::PeerRequestFinished(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(
        String(nym).Get(), StorageBox::FINISHEDPEERREQUEST);
}

ObjectList Wallet::PeerRequestProcessed(const Identifier& nym) const
{
    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().NymBoxList(
        String(nym).Get(), StorageBox::PROCESSEDPEERREQUEST);
}

bool Wallet::PeerRequestReceive(
    const Identifier& nym,
    const PeerObject& request) const
{
    if (proto::PEEROBJECT_REQUEST != request.Type()) {
        otErr << OT_METHOD << __FUNCTION__ << ": This is not a peer request."
              << std::endl;

        return false;
    }

    if (!request.Request()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null request." << std::endl;

        return false;
    }

    const std::string nymID = String(nym).Get();
    Lock lock(peer_lock(nymID));

    return ot_.DB().Store(
        request.Request()->Contract(), nymID, StorageBox::INCOMINGPEERREQUEST);
}

bool Wallet::PeerRequestUpdate(
    const Identifier& nym,
    const Identifier& request,
    const StorageBox& box) const
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREQUEST: {
            return ot_.DB().SetPeerRequestTime(
                String(nym).Get(), String(request).Get(), box);
        }
        default: {
            return false;
        }
    }
}

bool Wallet::RemoveServer(const Identifier& id)
{
    std::string server(String(id).Get());
    Lock mapLock(server_map_lock_);
    auto deleted = server_map_.erase(server);

    if (0 != deleted) {
        return ot_.DB().RemoveServer(server);
    }

    return false;
}

bool Wallet::RemoveUnitDefinition(const Identifier& id)
{
    std::string unit(String(id).Get());
    Lock mapLock(unit_map_lock_);
    auto deleted = unit_map_.erase(unit);

    if (0 != deleted) {
        return ot_.DB().RemoveUnitDefinition(unit);
    }

    return false;
}

ConstServerContract Wallet::Server(
    const Identifier& id,
    const std::chrono::milliseconds& timeout)
{
    const String strID(id);
    const std::string server = strID.Get();
    Lock mapLock(server_map_lock_);
    bool inMap = (server_map_.find(server) != server_map_.end());
    bool valid = false;

    if (!inMap) {
        std::shared_ptr<proto::ServerContract> serialized;

        std::string alias;
        bool loaded = ot_.DB().Load(server, serialized, alias, true);

        if (loaded) {
            auto nym = Nym(Identifier(serialized->nymid()));

            if (!nym && serialized->has_publicnym()) {
                nym = Nym(serialized->publicnym());
            }

            if (nym) {
                auto& pServer = server_map_[server];
                pServer.reset(ServerContract::Factory(nym, *serialized));

                if (pServer) {
                    valid = true;  // Factory() performs validation
                    pServer->Signable::SetAlias(alias);
                }
            }
        } else {
            ot_.DHT().GetServerContract(server);

            if (timeout > std::chrono::milliseconds(0)) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = std::chrono::milliseconds(100);

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found =
                        (server_map_.find(server) != server_map_.end());
                    mapLock.unlock();

                    if (found) {
                        break;
                    }
                }

                return Server(
                    id);  // timeout of zero prevents infinite recursion
            }
        }
    } else {
        auto& pServer = server_map_[server];
        if (pServer) {
            valid = pServer->Validate();
        }
    }

    if (valid) {
        return server_map_[server];
    }

    return nullptr;
}

ConstServerContract Wallet::Server(
    std::unique_ptr<class ServerContract>& contract)
{
    std::string server = String(contract->ID()).Get();

    if (contract) {
        if (contract->Validate()) {
            if (ot_.DB().Store(contract->Contract(), contract->Alias())) {
                Lock mapLock(server_map_lock_);
                server_map_[server].reset(contract.release());
                mapLock.unlock();
            }
        }
    }

    return Server(Identifier(server));
}

ConstServerContract Wallet::Server(const proto::ServerContract& contract)
{
    std::string server = contract.id();
    auto nym = Nym(Identifier(contract.nymid()));

    if (!nym && contract.has_publicnym()) {
        nym = Nym(contract.publicnym());
    }

    if (nym) {
        std::unique_ptr<ServerContract> candidate(
            ServerContract::Factory(nym, contract));

        if (candidate) {
            if (candidate->Validate()) {
                if (ot_.DB().Store(candidate->Contract(), candidate->Alias())) {
                    Lock mapLock(server_map_lock_);
                    server_map_[server].reset(candidate.release());
                    mapLock.unlock();
                }
            }
        }
    }

    return Server(Identifier(server));
}

ConstServerContract Wallet::Server(
    const std::string& nymid,
    const std::string& name,
    const std::string& terms,
    const std::list<ServerContract::Endpoint>& endpoints)
{
    std::string server;

    auto nym = Nym(Identifier(nymid));

    if (nym) {
        std::unique_ptr<ServerContract> contract;
        contract.reset(ServerContract::Create(nym, endpoints, terms, name));

        if (contract) {

            return (Server(contract));
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: failed to create contract." << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Error: nym does not exist."
              << std::endl;
    }

    return Server(Identifier(server));
}

ObjectList Wallet::ServerList() { return ot_.DB().ServerList(); }

bool Wallet::SetNymAlias(const Identifier& id, const std::string& alias)
{
    Lock mapLock(nym_map_lock_);

    auto it = nym_map_.find(String(id).Get());

    if (nym_map_.end() != it) {
        nym_map_.erase(it);
    }

    return ot_.DB().SetNymAlias(String(id).Get(), alias);
}

Identifier Wallet::ServerToNym(Identifier& input)
{
    Identifier output;
    auto nym = Nym(input);
    const bool inputIsNymID = bool(nym);

    if (inputIsNymID) {
        output = input;
        const auto list = ServerList();
        std::size_t matches = 0;

        for (const auto& item : list) {
            const auto& serverID = item.first;
            auto server = Server(Identifier(serverID));

            OT_ASSERT(server);

            if (server->Nym()->ID() == input) {
                matches++;
                // set input to the notary ID
                input = server->ID();
            }
        }

        OT_ASSERT(2 > matches);
    } else {
        auto contract = Server(input);

        if (contract) {
            output = Identifier(contract->Contract().nymid());
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Non-existent server: " << String(input) << std::endl;
        }
    }

    return output;
}

bool Wallet::SetServerAlias(const Identifier& id, const std::string& alias)
{
    const std::string server = String(id).Get();
    const bool saved = ot_.DB().SetServerAlias(server, alias);

    if (saved) {
        Lock mapLock(server_map_lock_);
        server_map_.erase(server);

        return true;
    }

    return false;
}

bool Wallet::SetUnitDefinitionAlias(
    const Identifier& id,
    const std::string& alias)
{
    const std::string unit = String(id).Get();
    const bool saved = ot_.DB().SetUnitDefinitionAlias(unit, alias);

    if (saved) {
        Lock mapLock(unit_map_lock_);
        unit_map_.erase(unit);

        return true;
    }

    return false;
}

ObjectList Wallet::UnitDefinitionList()
{
    return ot_.DB().UnitDefinitionList();
}

ConstUnitDefinition Wallet::UnitDefinition(
    const Identifier& id,
    const std::chrono::milliseconds& timeout)
{
    const String strID(id);
    const std::string unit = strID.Get();
    Lock mapLock(unit_map_lock_);
    bool inMap = (unit_map_.find(unit) != unit_map_.end());
    bool valid = false;

    if (!inMap) {
        std::shared_ptr<proto::UnitDefinition> serialized;

        std::string alias;
        bool loaded = ot_.DB().Load(unit, serialized, alias, true);

        if (loaded) {
            auto nym = Nym(Identifier(serialized->nymid()));

            if (!nym && serialized->has_publicnym()) {
                nym = Nym(serialized->publicnym());
            }

            if (nym) {
                auto& pUnit = unit_map_[unit];
                pUnit.reset(UnitDefinition::Factory(nym, *serialized));

                if (pUnit) {
                    valid = true;  // Factory() performs validation
                    pUnit->Signable::SetAlias(alias);
                }
            }
        } else {
            ot_.DHT().GetUnitDefinition(unit);

            if (timeout > std::chrono::milliseconds(0)) {
                mapLock.unlock();
                auto start = std::chrono::high_resolution_clock::now();
                auto end = start + timeout;
                const auto interval = std::chrono::milliseconds(100);

                while (std::chrono::high_resolution_clock::now() < end) {
                    std::this_thread::sleep_for(interval);
                    mapLock.lock();
                    bool found = (unit_map_.find(unit) != unit_map_.end());
                    mapLock.unlock();

                    if (found) {
                        break;
                    }
                }

                return UnitDefinition(id);  // timeout of zero prevents
                                            // infinite recursion
            }
        }
    } else {
        auto& pUnit = unit_map_[unit];
        if (pUnit) {
            valid = pUnit->Validate();
        }
    }

    if (valid) {
        return unit_map_[unit];
    }

    return nullptr;
}

ConstUnitDefinition Wallet::UnitDefinition(
    std::unique_ptr<class UnitDefinition>& contract)
{
    std::string unit = String(contract->ID()).Get();

    if (contract) {
        if (contract->Validate()) {
            if (ot_.DB().Store(contract->Contract(), contract->Alias())) {
                Lock mapLock(unit_map_lock_);
                unit_map_[unit].reset(contract.release());
                mapLock.unlock();
            }
        }
    }

    return UnitDefinition(Identifier(unit));
}

ConstUnitDefinition Wallet::UnitDefinition(
    const proto::UnitDefinition& contract)
{
    std::string unit = contract.id();
    auto nym = Nym(Identifier(contract.nymid()));

    if (!nym && contract.has_publicnym()) {
        nym = Nym(contract.publicnym());
    }

    if (nym) {
        std::unique_ptr<class UnitDefinition> candidate(
            UnitDefinition::Factory(nym, contract));

        if (candidate) {
            if (candidate->Validate()) {
                if (ot_.DB().Store(candidate->Contract(), candidate->Alias())) {
                    Lock mapLock(unit_map_lock_);
                    unit_map_[unit].reset(candidate.release());
                    mapLock.unlock();
                }
            }
        }
    }

    return UnitDefinition(Identifier(unit));
}

ConstUnitDefinition Wallet::UnitDefinition(
    const std::string& nymid,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::string& tla,
    const std::uint32_t& power,
    const std::string& fraction)
{
    std::string unit;

    auto nym = Nym(Identifier(nymid));

    if (nym) {
        std::unique_ptr<class UnitDefinition> contract;
        contract.reset(UnitDefinition::Create(
            nym, shortname, name, symbol, terms, tla, power, fraction));
        if (contract) {

            return (UnitDefinition(contract));
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: failed to create contract." << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Error: nym does not exist."
              << std::endl;
    }

    return UnitDefinition(Identifier(unit));
}

ConstUnitDefinition Wallet::UnitDefinition(
    const std::string& nymid,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms)
{
    std::string unit;

    auto nym = Nym(Identifier(nymid));

    if (nym) {
        std::unique_ptr<class UnitDefinition> contract;
        contract.reset(
            UnitDefinition::Create(nym, shortname, name, symbol, terms));
        if (contract) {

            return (UnitDefinition(contract));
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: failed to create contract." << std::endl;
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Error: nym does not exist."
              << std::endl;
    }

    return UnitDefinition(Identifier(unit));
}
}  // namespace opentxs::api
