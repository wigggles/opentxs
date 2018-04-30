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

#ifndef OPENTXS_CORE_TYPES_HPP
#define OPENTXS_CORE_TYPES_HPP

#include <cstdint>
#include <functional>
#include <list>
#include <set>
#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <shared_mutex>
#include <string>
#include <vector>

namespace opentxs
{

class Message;
class Nym;
class String;

#define PAYMENT_CODE_VERSION 1
#define PEER_MESSAGE_VERSION 2
#define PEER_PAYMENT_VERSION 5
#define NYM_CREATE_VERSION 4
#define NYM_UPGRADE_VERSION 4
#define CONTACT_CREDENTIAL_VERSION 4
#define NYM_CONTACT_DATA_VERSION 4
#define CONTACT_CONTACT_DATA_VERSION 5
#define VERIFICATION_CREDENTIAL_VERSION 1
#define KEY_CREDENTIAL_VERSION 1
#define MESSAGE_SEND_ERROR -1
#define MESSAGE_NOT_SENT_NO_ERROR 0
#define MESSAGE_SENT 1
#define REPLY_NOT_RECEIVED -1
#define MESSAGE_SUCCESS_FALSE 0
#define MESSAGE_SUCCESS_TRUE 1
#define FIRST_REQUEST_NUMBER 1

typedef std::map<std::string, std::set<std::string>> ArgList;

typedef bool CredentialIndexModeFlag;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_ONLY_IDS = true;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_FULL_CREDS = false;

typedef bool CredentialModeFlag;
static const CredentialModeFlag PRIVATE_VERSION = true;
static const CredentialModeFlag PUBLIC_VERSION = false;

typedef bool SerializationModeFlag;
static const SerializationModeFlag AS_PRIVATE = true;
static const SerializationModeFlag AS_PUBLIC = false;

typedef bool SerializationSignatureFlag;
static const SerializationSignatureFlag WITH_SIGNATURES = true;
static const SerializationSignatureFlag WITHOUT_SIGNATURES = false;

typedef bool ProtoValidationVerbosity;
static const ProtoValidationVerbosity SILENT = true;
static const ProtoValidationVerbosity VERBOSE = false;

typedef bool BIP44Chain;
static const BIP44Chain INTERNAL_CHAIN = true;
static const BIP44Chain EXTERNAL_CHAIN = false;

typedef bool BlockMode;
static const BlockMode BLOCK_MODE = true;
static const BlockMode NOBLOCK_MODE = false;

typedef std::vector<std::shared_ptr<std::string>> DhtResults;

typedef std::function<void(bool)> DhtDoneCallback;
typedef std::function<bool(const DhtResults&)> DhtResultsCallback;

typedef std::function<
    bool(const std::uint32_t, const std::string&, std::string&)>
    Digest;
typedef std::function<std::string()> Random;

typedef std::function<void()> PeriodicTask;

/** C++11 representation of a claim. This version is more useful than the
 *  protobuf version, since it contains the claim ID.
 */
typedef std::tuple<
    std::string,              // claim identifier
    std::uint32_t,            // section
    std::uint32_t,            // type
    std::string,              // value
    std::int64_t,             // start time
    std::int64_t,             // end time
    std::set<std::uint32_t>>  // attributes
    Claim;
typedef Claim ClaimTuple;

/** C++11 representation of all contact data associated with a nym, aggregating
 *  each the nym's contact credentials in the event it has more than one.
 */
typedef std::set<Claim> ClaimSet;

/** A list of object IDs and their associated aliases
 *  * string: id of the stored object
 *  * string: alias of the stored object
 */
typedef std::list<std::pair<std::string, std::string>> ObjectList;

typedef std::vector<unsigned char> RawData;

typedef std::map<std::string, Nym*> mapOfNyms;
typedef std::map<std::string, const Nym*> mapOfConstNyms;
typedef std::map<std::string, std::unique_ptr<Nym>> mapOfNymsSP;

// local ID, remote ID
using ContextID = std::pair<std::string, std::string>;
using ContextLockCallback =
    std::function<std::recursive_mutex&(const ContextID&)>;

typedef std::int32_t NetworkOperationStatus;

typedef std::int64_t TransactionNumber;
typedef std::int64_t RequestNumber;
typedef std::int64_t Amount;

using Lock = std::unique_lock<std::mutex>;
using rLock = std::unique_lock<std::recursive_mutex>;
using sLock = std::shared_lock<std::shared_mutex>;
using eLock = std::unique_lock<std::shared_mutex>;

enum class ClaimPolarity : std::uint8_t {
    NEUTRAL = 0,
    POSITIVE = 1,
    NEGATIVE = 2
};

enum class StorageBox : std::uint8_t {
    SENTPEERREQUEST = 0,
    INCOMINGPEERREQUEST = 1,
    SENTPEERREPLY = 2,
    INCOMINGPEERREPLY = 3,
    FINISHEDPEERREQUEST = 4,
    FINISHEDPEERREPLY = 5,
    PROCESSEDPEERREQUEST = 6,
    PROCESSEDPEERREPLY = 7,
    MAILINBOX = 8,
    MAILOUTBOX = 9,
    INCOMINGBLOCKCHAIN = 10,
    OUTGOINGBLOCKCHAIN = 11,
    DRAFT = 254,
    UNKNOWN = 255,
};

enum class Bip43Purpose : std::uint32_t {
    HDWALLET = 44,    // BIP-44
    PAYCODE = 47,     // BIP-47
    FS = 0x4f544653,  // OTFS
    NYM = 0x4f544e4d  // OTNM
};

enum class Bip44Type : std::uint32_t {
    BITCOIN = 0,
    TESTNET = 1,
    LITECOIN = 2,
    DOGECOIN = 3,
    REDDCOIN = 4,
    DASH = 5,
    PEERCOIN = 6,
    NAMECOIN = 7,
    FEATHERCOIN = 8,
    COUNTERPARTY = 9,
    BLACKCOIN = 10,
    BITCOINCASH = 145,
};

enum class Bip32Child : std::uint32_t {
    AUTH_KEY = 0x41555448,
    ENCRYPT_KEY = 0x454e4352,
    SIGN_KEY = 0x5349474e,
    HARDENED = 0x80000000,
};

enum class EcdsaCurve : std::uint8_t {
    ERROR = 0,
    SECP256K1 = 1,
    ED25519 = 2,
};

enum class NymParameterType : std::uint8_t {
    ERROR = 0,
    RSA = 1,
    SECP256K1 = 2,
    ED25519 = 3
};

enum class NymCapability : std::uint8_t {
    SIGN_MESSAGE = 0,
    ENCRYPT_MESSAGE = 1,
    AUTHENTICATE_CONNECTION = 2,
    SIGN_CHILDCRED = 3,
};

enum class ID : std::uint8_t {
    ERROR = 0,
    SHA256 = 1,
    BLAKE2B = 2,
};

enum class ContractType : std::uint8_t {
    ERROR = 0,
    NYM = 1,
    SERVER = 2,
    UNIT = 3,
};

// originType is DISPLAY ONLY. Used in OTTransaction and OTItem.
// sometimes an OTItem is used to represent an OTTransaction.
// (for example, processInbox transaction has a processInbox item that
// contains a list of sub-items that represent the receipts aka
// OTTransactions in the inbox.)
//
// This is used for for finalReceipts and for paymentReceipts,
// so the GUI can sort them properly without having to load up the
// original transaction and see its type.
// This won't affect the actual operation of OT itself, which ignores
// this value. It's just here to help the GUI to sort receipts that
// have already been closed, with less work necessary to do so.
//
// NOTE: I'll also use this for paymentReceipts, so I can distinguish
// smart contract receipts from payment plan receipts. In the case of
// marketReceipts, it's not that important, since we already know it's
// for a market trade. But with paymentReceipts, it's useful. (And
// finalReceipts.) Maybe I should create a "contractReceipt" to fix
// that ambiguity.
//
enum class originType : std::int8_t {
    not_applicable,
    origin_market_offer,    // finalReceipt
    origin_payment_plan,    // finalReceipt, paymentReceipt
    origin_smart_contract,  // finalReceipt, paymentReceipt
    origin_pay_dividend,    // SOME voucherReceipts are from a payDividend.
    origin_error_state
};

enum class SendResult : std::int8_t {
    TRANSACTION_NUMBERS = -3,
    INVALID_REPLY = -2,
    TIMEOUT = -1,
    ERROR = 0,
    UNNECESSARY = 1,
    VALID_REPLY = 2,
};

enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,
    ACTIVE = 1,
    STALLED = 2
};

typedef std::pair<SendResult, std::shared_ptr<std::string>> NetworkReplyRaw;
typedef std::pair<SendResult, std::shared_ptr<String>> NetworkReplyString;
typedef std::pair<SendResult, std::shared_ptr<Message>> NetworkReplyMessage;

typedef std::tuple<RequestNumber, TransactionNumber, NetworkReplyMessage>
    CommandResult;

enum class MessageType : std::uint8_t {
    badID = 0,
    // Your public key is sent along with this message so the server can
    // reply to you even without your being a registered user. Other than
    // these top two commands, all other commands can only be executed by
    // registered users.
    //
    // The server ID is a hash of the server contract. The signature on the
    // contract can be verified by a public key that appears in a standard
    // section of any server contract. The URL/port information is also
    // derived from the contract.
    //
    // Simply by importing the server contract into your wallet, you are
    // able to connect to it and encrypt all of your communications to it.
    //
    // Thus, the check server ID command really just confirms what you
    // should already know... Your wallet still wants to see that the server
    // agrees with the server ID, and that the server is able to read
    // messages that were encrypted to the public key in the contract, and
    // that the server is able to sign all of its future correspondence with
    // the same public key.
    //
    // It is the server operator's responsibility to secure the domain name
    // and web host that users will connect to when they import the
    // contract, as well as the private key that matches the public key from
    // the contract.
    pingNotary = 1,
    pingNotaryResponse = 2,
    // register user account on a specific server, with public key. Nym ID
    // will be hash of said public key.
    registerNym = 3,
    registerNymResponse = 4,
    // Delete user account from a specific server.
    unregisterNym = 5,
    unregisterNymResponse = 6,
    // Get the next request number from the server (for this user). Most
    // requests must be accompanied by a request number, which increments
    // for each Nym with each request.
    getRequestNumber = 7,
    getRequestNumberResponse = 8,
    // Every transaction requires a transaction number. If your wallet
    // doesn't have one, then here it can request the server to send one
    // over. (Or several.)
    getTransactionNumbers = 9,
    getTransactionNumbersResponse = 10,
    // Used by AcceptEntireNymbox() as it's setting everything up.
    processNymbox = 11,
    processNymboxResponse = 12,
    checkNym = 13,
    checkNymResponse = 14,
    sendNymMessage = 15,
    sendNymMessageResponse = 16,
    sendNymInstrument = 17,
    // sendNymInstrumentResponse = 18,
    unregisterAccount = 19,
    unregisterAccountResponse = 20,
    registerAccount = 21,
    registerAccountResponse = 22,
    registerInstrumentDefinition = 23,
    registerInstrumentDefinitionResponse = 24,
    issueBasket = 25,
    issueBasketResponse = 26,
    notarizeTransaction = 27,
    notarizeTransactionResponse = 28,
    getNymbox = 29,
    getNymboxResponse = 30,
    getBoxReceipt = 31,
    getBoxReceiptResponse = 32,
    getAccountData = 33,
    getAccountDataResponse = 34,
    processInbox = 35,
    processInboxResponse = 36,
    queryInstrumentDefinitions = 37,
    queryInstrumentDefinitionsResponse = 38,
    getInstrumentDefinition = 39,
    getInstrumentDefinitionResponse = 40,
    getMint = 41,
    getMintResponse = 42,
    getMarketList = 43,
    getMarketListResponse = 44,
    getMarketOffers = 45,
    getMarketOffersResponse = 46,
    getMarketRecentTrades = 47,
    getMarketRecentTradesResponse = 48,
    getNymMarketOffers = 49,
    getNymMarketOffersResponse = 50,
    triggerClause = 51,
    triggerClauseResponse = 52,
    usageCredits = 53,
    usageCreditsResponse = 54,
    registerContract = 55,
    registerContractResponse = 56,
    requestAdmin = 57,
    requestAdminResponse = 58,
    addClaim = 59,
    addClaimResponse = 60,
};

enum class ThreadStatus : std::uint8_t {
    ERROR = 0,
    RUNNING = 1,
    FINISHED_SUCCESS = 2,
    FINISHED_FAILED = 3,
    SHUTDOWN = 4,
};

enum class Messagability : std::int8_t {
    MISSING_CONTACT = -5,
    CONTACT_LACKS_NYM = -4,
    NO_SERVER_CLAIM = -3,
    INVALID_SENDER = -2,
    MISSING_SENDER = -1,
    READY = 0,
    MISSING_RECIPIENT = 1,
    UNREGISTERED = 2,
};

enum class Depositability : std::int8_t {
    ACCOUNT_NOT_SPECIFIED = -4,
    WRONG_ACCOUNT = -3,
    WRONG_RECIPIENT = -2,
    INVALID_INSTRUMENT = -1,
    READY = 0,
    NOT_REGISTERED = 1,
    NO_ACCOUNT = 2,
};

enum class BlockchainAccountType : std::uint8_t {
    ERROR = 0,
    BIP32 = 1,
    BIP44 = 2,
};

enum class SocketType : std::uint8_t {
    Error = 0,
    Request = 1,
    Reply = 2,
    Publish = 3,
    Subscribe = 4,
    Push = 5,
    Pull = 6,
    Pair = 7,
};

enum class RemoteBoxType : std::int8_t {
    Error = -1,
    Nymbox = 0,
    Inbox = 1,
    Outbox = 2,
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_TYPES_HPP
