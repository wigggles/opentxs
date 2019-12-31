// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

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

#if OT_BLOCKCHAIN
namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio
}  // namespace boost
#endif  // OT_BLOCKCHAIN

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
class Core;
class Context;
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
struct FilterDatabase;
struct FilterOracle;
struct HeaderDatabase;
struct HeaderOracle;
struct Network;
struct PeerDatabase;
struct PeerManager;
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

namespace ui
{
namespace internal
{
namespace blank
{
struct AccountListItem;
struct AccountSummaryItem;
struct ActivitySummaryItem;
struct ActivityThreadItem;
struct BalanceItem;
struct ContactItem;
struct ContactListItem;
struct ContactSection;
struct ContactSubsection;
struct IssuerItem;
struct PayableListItem;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
}  // namespace blank

struct AccountActivity;
struct AccountList;
struct AccountListItem;
struct AccountSummary;
struct AccountSummaryItem;
struct ActivitySummary;
struct ActivitySummaryItem;
struct ActivityThread;
struct ActivityThreadItem;
struct BalanceItem;
struct Contact;
struct ContactItem;
struct ContactList;
struct ContactListItem;
struct ContactSection;
struct ContactSubsection;
struct IssuerItem;
struct MessagableList;
struct PayableList;
struct PayableListItem;
struct Profile;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
}  // namespace internal

namespace implementation
{
class AccountActivity;
class AccountList;
class AccountSummary;
class IssuerItem;
class ActivitySummary;
class ActivityThread;
class Contact;
class ContactList;
class ContactSection;
class ContactSubsection;
class MessagableList;
class PayableList;
class Profile;
class ProfileSection;
class ProfileSubsection;

using CustomData = std::vector<const void*>;

// Account activity
using AccountActivityPrimaryID = OTNymID;
using AccountActivityExternalInterface = ui::AccountActivity;
using AccountActivityInternalInterface = ui::internal::AccountActivity;
/** WorkflowID, state */
using AccountActivityRowID = std::pair<OTIdentifier, proto::PaymentEventType>;
using AccountActivityRowInterface = ui::BalanceItem;
using AccountActivityRowInternal = ui::internal::BalanceItem;
using AccountActivityRowBlank = ui::internal::blank::BalanceItem;
using AccountActivitySortKey = std::chrono::system_clock::time_point;

// Account list
using AccountListPrimaryID = OTNymID;
using AccountListExternalInterface = ui::AccountList;
using AccountListInternalInterface = ui::internal::AccountList;
using AccountListRowID = OTIdentifier;
using AccountListRowInterface = ui::AccountListItem;
using AccountListRowInternal = ui::internal::AccountListItem;
using AccountListRowBlank = ui::internal::blank::AccountListItem;
// type, notary ID
using AccountListSortKey = std::pair<proto::ContactItemType, std::string>;

// Account summary
using AccountSummaryPrimaryID = OTNymID;
using AccountSummaryExternalInterface = ui::AccountSummary;
using AccountSummaryInternalInterface = ui::internal::AccountSummary;
using AccountSummaryRowID = OTNymID;
using AccountSummaryRowInterface = ui::IssuerItem;
using AccountSummaryRowInternal = ui::internal::IssuerItem;
using AccountSummaryRowBlank = ui::internal::blank::IssuerItem;
using AccountSummarySortKey = std::pair<bool, std::string>;

using IssuerItemPrimaryID = OTNymID;
using IssuerItemExternalInterface = AccountSummaryRowInterface;
using IssuerItemInternalInterface = ui::internal::IssuerItem;
using IssuerItemRowID = std::pair<OTIdentifier, proto::ContactItemType>;
using IssuerItemRowInterface = ui::AccountSummaryItem;
using IssuerItemRowInternal = ui::internal::AccountSummaryItem;
using IssuerItemRowBlank = ui::internal::blank::AccountSummaryItem;
using IssuerItemSortKey = std::string;

// Activity summary
using ActivitySummaryPrimaryID = OTNymID;
using ActivitySummaryExternalInterface = ui::ActivitySummary;
using ActivitySummaryInternalInterface = ui::internal::ActivitySummary;
using ActivitySummaryRowID = OTIdentifier;
using ActivitySummaryRowInterface = ui::ActivitySummaryItem;
using ActivitySummaryRowInternal = ui::internal::ActivitySummaryItem;
using ActivitySummaryRowBlank = ui::internal::blank::ActivitySummaryItem;
using ActivitySummarySortKey =
    std::pair<std::chrono::system_clock::time_point, std::string>;

// Activity thread
using ActivityThreadPrimaryID = OTNymID;
using ActivityThreadExternalInterface = ui::ActivityThread;
using ActivityThreadInternalInterface = ui::internal::ActivityThread;
/** item id, box, accountID, taskID */
using ActivityThreadRowID = std::tuple<OTIdentifier, StorageBox, OTIdentifier>;
using ActivityThreadRowInterface = ui::ActivityThreadItem;
using ActivityThreadRowInternal = ui::internal::ActivityThreadItem;
using ActivityThreadRowBlank = ui::internal::blank::ActivityThreadItem;
/** timestamp, index */
using ActivityThreadSortKey =
    std::pair<std::chrono::system_clock::time_point, std::uint64_t>;

// Contact
using ContactPrimaryID = OTIdentifier;
using ContactExternalInterface = ui::Contact;
using ContactInternalInterface = ui::internal::Contact;
using ContactRowID = proto::ContactSectionName;
using ContactRowInterface = ui::ContactSection;
using ContactRowInternal = ui::internal::ContactSection;
using ContactRowBlank = ui::internal::blank::ContactSection;
using ContactSortKey = int;

using ContactSectionPrimaryID = ContactPrimaryID;
using ContactSectionExternalInterface = ContactRowInterface;
using ContactSectionInternalInterface = ui::internal::ContactSection;
using ContactSectionRowID =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ContactSectionRowInterface = ui::ContactSubsection;
using ContactSectionRowInternal = ui::internal::ContactSubsection;
using ContactSectionRowBlank = ui::internal::blank::ContactSubsection;
using ContactSectionSortKey = int;

using ContactSubsectionPrimaryID = ContactSectionPrimaryID;
using ContactSubsectionExternalInterface = ContactSectionRowInterface;
using ContactSubsectionInternalInterface = ui::internal::ContactSubsection;
using ContactSubsectionRowID = OTIdentifier;
using ContactSubsectionRowInterface = ui::ContactItem;
using ContactSubsectionRowInternal = ui::internal::ContactItem;
using ContactSubsectionRowBlank = ui::internal::blank::ContactItem;
using ContactSubsectionSortKey = int;

// Contact list
using ContactListPrimaryID = OTNymID;
using ContactListExternalInterface = ui::ContactList;
using ContactListInternalInterface = ui::internal::ContactList;
using ContactListRowID = OTIdentifier;
using ContactListRowInterface = ui::ContactListItem;
using ContactListRowInternal = ui::internal::ContactListItem;
using ContactListRowBlank = ui::internal::blank::ContactListItem;
using ContactListSortKey = std::string;

// Messagable list
using MessagableListPrimaryID = OTNymID;
using MessagableExternalInterface = ui::MessagableList;
using MessagableInternalInterface = ui::internal::MessagableList;
using MessagableListRowID = ContactListRowID;
using MessagableListRowInterface = ContactListRowInterface;
using MessagableListRowInternal = ContactListRowInternal;
using MessagableListRowBlank = ContactListRowBlank;
using MessagableListSortKey = std::string;

// Payable list
using PayablePrimaryID = OTNymID;
using PayableExternalInterface = ui::PayableList;
using PayableInternalInterface = ui::internal::PayableList;
using PayableListRowID = ContactListRowID;
using PayableListRowInterface = ui::PayableListItem;
using PayableListRowInternal = ui::internal::PayableListItem;
using PayableListRowBlank = ui::internal::blank::PayableListItem;
using PayableListSortKey = std::string;

// Profile
using ProfilePrimaryID = OTNymID;
using ProfileExternalInterface = ui::Profile;
using ProfileInternalInterface = ui::internal::Profile;
using ProfileRowID = proto::ContactSectionName;
using ProfileRowInterface = ui::ProfileSection;
using ProfileRowInternal = ui::internal::ProfileSection;
using ProfileRowBlank = ui::internal::blank::ProfileSection;
using ProfileSortKey = int;

using ProfileSectionPrimaryID = ProfilePrimaryID;
using ProfileSectionExternalInterface = ProfileRowInterface;
using ProfileSectionInternalInterface = ui::internal::ProfileSection;
using ProfileSectionRowID =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ProfileSectionRowInterface = ui::ProfileSubsection;
using ProfileSectionRowInternal = ui::internal::ProfileSubsection;
using ProfileSectionRowBlank = ui::internal::blank::ProfileSubsection;
using ProfileSectionSortKey = int;

using ProfileSubsectionPrimaryID = ProfileSectionPrimaryID;
using ProfileSubsectionExternalInterface = ProfileSectionRowInterface;
using ProfileSubsectionInternalInterface = ui::internal::ProfileSubsection;
using ProfileSubsectionRowID = OTIdentifier;
using ProfileSubsectionRowInterface = ui::ProfileItem;
using ProfileSubsectionRowInternal = ui::internal::ProfileItem;
using ProfileSubsectionRowBlank = ui::internal::blank::ProfileItem;
using ProfileSubsectionSortKey = int;
}  // namespace implementation
}  // namespace ui

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

bool operator==(
    const opentxs::ProtobufType& lhs,
    const opentxs::ProtobufType& rhs) noexcept;

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
    static T value(const api::Core&) { return T{}; }
};

template <typename I>
int polarity(const I value)
{
    if (0 == value) { return 0; }

    return (0 < value) ? 1 : -1;
}

template <typename Key, typename Value>
std::map<Value, Key> reverse_map(const std::map<Key, Value>& map) noexcept
{
    std::map<Value, Key> output{};

    for (const auto& [key, value] : map) { output.emplace(value, key); }

    return output;
}

proto::ContactItemType Translate(const blockchain::Type type) noexcept;
blockchain::Type Translate(const proto::ContactItemType type) noexcept;
}  // namespace opentxs

#include "Factory.hpp"
