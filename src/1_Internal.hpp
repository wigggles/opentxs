// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/Proto.hpp"

#pragma once

// IWYU pragma: begin_exports
#include "opentxs/Forward.hpp"  // IWYU pragma: associated
// IWYU pragma: end_exports

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"

#define PAYMENT_CODE_VERSION 1
#define PEER_MESSAGE_VERSION 2
#define PEER_PAYMENT_VERSION 5
#define PEER_CASH_VERSION 7
#define PEER_OBJECT_PEER_REQUEST 7
#define PEER_OBJECT_PEER_REPLY 7
#define OT_CONTACT_VERSION 3
#define CONTACT_CONTACT_DATA_VERSION 6
#define MESSAGE_SEND_ERROR -1
#define MESSAGE_NOT_SENT_NO_ERROR 0
#define MESSAGE_SENT 1
#define REPLY_NOT_RECEIVED -1
#define MESSAGE_SUCCESS_FALSE 0
#define MESSAGE_SUCCESS_TRUE 1
#define FIRST_REQUEST_NUMBER 1

#define OT_ZMQ_NEW_FILTER_SIGNAL 246
#define OT_ZMQ_NEW_NYM_SIGNAL 247
#define OT_ZMQ_CONNECT_SIGNAL 248
#define OT_ZMQ_DISCONNECT_SIGNAL 249
#define OT_ZMQ_RECEIVE_SIGNAL 250
#define OT_ZMQ_SEND_SIGNAL 251
#define OT_ZMQ_REGISTER_SIGNAL 252
#define OT_ZMQ_REORG_SIGNAL 253
#define OT_ZMQ_STATE_MACHINE_SIGNAL 254
#define OT_ZMQ_SHUTDOWN_SIGNAL 255

using OTZMQWorkType = std::uint8_t;

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
#if OT_BLOCKCHAIN
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database
#endif  // OT_BLOCKCHAIN

namespace internal
{
struct BalanceList;
struct BalanceNode;
struct BalanceTree;
struct Deterministic;
struct HD;
struct Imported;
struct PaymentCode;
}  // namespace internal
}  // namespace blockchain

namespace implementation
{
class UI;
}  // namespace implementation

namespace internal
{
struct Activity;
struct Blockchain;
struct Contacts;
struct Manager;
struct Pair;
}  // namespace internal
}  // namespace client

namespace crypto
{
namespace internal
{
struct Asymmetric;
}  // namespace internal
}  // namespace crypto

namespace implementation
{
class Context;
class Core;
class Storage;
}  // namespace implementation

namespace internal
{
struct Context;
struct Core;
struct Factory;
struct Log;
}  // namespace internal

namespace network
{
namespace implementation
{
class Context;
class Dht;
class ZMQ;
}  // namespace implementation
}  // namespace network

namespace server
{
namespace implementation
{
class Factory;
class Manager;
}  // namespace implementation
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace server

namespace storage
{
namespace implementation
{
class Storage;
}  // namespace implementation

class Driver;
class Multiplex;
class Plugin;
class StorageInternal;
}  // namespace storage

class Core;
}  // namespace api

namespace blind
{
namespace implementation
{
class Purse;
}  // namespace implementation

namespace token
{
namespace implementation
{
class Token;
}  // namespace implementation
}  // namespace token
}  // namespace blind

#if OT_BLOCKCHAIN
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
class Inventory;
struct EncodedInput;
struct EncodedOutpoint;
struct EncodedOutput;
struct EncodedTransaction;
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
namespace internal
{
struct Header;
}  // namespace internal
}  // namespace bitcoin
}  // namespace block

namespace client
{
namespace internal
{
struct BlockOracle;
struct FilterDatabase;
struct FilterOracle;
struct HeaderDatabase;
struct HeaderOracle;
struct IO;
struct Network;
struct PeerDatabase;
struct PeerManager;
struct ThreadPool;
struct Wallet;
}  // namespace internal

class UpdateTransaction;
}  // namespace client

namespace internal
{
struct Database;
struct GCS;
}  // namespace internal

namespace p2p
{
namespace bitcoin
{
namespace message
{
namespace internal
{
struct Addr;
struct Block;
struct Blocktxn;
struct Cfcheckpt;
struct Cfheaders;
struct Cfilter;
struct Filteradd;
struct Filterclear;
struct Filterload;
struct Getaddr;
struct Getcfcheckpt;
struct Getcfheaders;
struct Getcfilters;
struct Getdata;
struct Getheaders;
struct Headers;
struct Inv;
struct Mempool;
struct Notfound;
struct Ping;
struct Pong;
struct Sendheaders;
struct Verack;
struct Version;
}  // namespace internal

class Checkorder;
class Cmpctblock;
class Feefilter;
class Getblocks;
class Getblocktxn;
class Merkleblock;
class Reject;
class Reply;
class Sendcmpct;
class Submitorder;
class Tx;
}  // namespace message

class Body;
class Header;
class Message;
class Peer;
}  // namespace bitcoin

namespace internal
{
struct Address;
struct Peer;
}  // namespace internal
}  // namespace p2p
}  // namespace blockchain
#endif  // OT_BLOCKCHAIN

namespace crypto
{
class Pbkdf2;
class Ripemd160;

namespace implementation
{
class OpenSSL;
}  // namespace implementation
}  // namespace crypto

namespace identity
{
namespace credential
{
namespace internal
{
struct Base;
struct Contact;
struct Key;
struct Primary;
struct Secondary;
struct Verification;
}  // namespace internal
}  // namespace credential

namespace internal
{
struct Authority;
struct Nym;
}  // namespace internal

namespace wot
{
namespace verification
{
namespace internal
{
struct Group;
struct Item;
struct Nym;
struct Set;
}  // namespace internal
}  // namespace verification
}  // namespace wot
}  // namespace identity

namespace internal
{
struct ClientContext;
struct Context;
struct NymFile;
struct ServerContext;
}  // namespace internal

namespace network
{
namespace zeromq
{
namespace implementation
{
class Context;
class Proxy;
}  // namespace implementation
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace client
{
namespace internal
{
struct Operation;
struct StateMachine;
}  // namespace internal
}  // namespace client
}  // namespace otx

namespace rpc
{
namespace internal
{
struct RPC;
}  // namespace internal
}  // namespace rpc

namespace server
{
class ReplyMessage;
}  // namespace server

namespace storage
{
namespace implementation
{
class StorageMultiplex;
}  // namespace implementation

class Accounts;
class Bip47Channels;
class BlockchainTransactions;
class Contacts;
class Contexts;
class Credentials;
class Issuers;
class Mailbox;
class Notary;
class Nym;
class Nyms;
class PaymentWorkflows;
class PeerReplies;
class PeerRequests;
class Seeds;
class Servers;
class Thread;
class Threads;
class Tree;
class Txos;
class Units;
}  // namespace storage

class DhtConfig;
#if OT_CRYPTO_USING_LIBSECP256K1
class Libsecp256k1;
#endif
class Libsodium;
class LowLevelKeyGenerator;
#if OT_CRYPTO_USING_OPENSSL
class OpenSSL;
#endif
class StorageConfig;

auto operator==(
    const opentxs::ProtobufType& lhs,
    const opentxs::ProtobufType& rhs) noexcept -> bool;

template <typename T>
void dedup(std::vector<T>& vector) noexcept
{
    std::sort(vector.begin(), vector.end());
    vector.erase(std::unique(vector.begin(), vector.end()), vector.end());
}

template <typename I>
struct HDIndex {
    Bip32Index value_{};

    operator Bip32Index() const { return value_; }

    HDIndex(const I in)
        : value_(static_cast<Bip32Index>(in))
    {
    }

    HDIndex(const I lhs, const Bip32Child rhs)
        : value_(static_cast<Bip32Index>(lhs) | static_cast<Bip32Index>(rhs))
    {
    }
};

template <typename Bip43Purpose>
HDIndex(const Bip43Purpose, const Bip32Child)->HDIndex<Bip43Purpose>;

template <typename T>
struct make_blank {
    static auto value(const api::Core&) -> T { return T{}; }
};

template <typename I>
auto polarity(const I value) -> int
{
    if (0 == value) { return 0; }

    return (0 < value) ? 1 : -1;
}

template <typename Key, typename Value>
auto reverse_map(const std::map<Key, Value>& map) noexcept
    -> std::map<Value, Key>
{
    std::map<Value, Key> output{};

    for (const auto& [key, value] : map) { output.emplace(value, key); }

    return output;
}

auto Translate(const blockchain::Type type) noexcept -> proto::ContactItemType;
auto Translate(const proto::ContactItemType type) noexcept -> blockchain::Type;
}  // namespace opentxs

// IWYU pragma: begin_exports
#include "Factory.hpp"
// IWYU pragma: end_exports
