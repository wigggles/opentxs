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
#include <tuple>
#include <string>
#include <vector>

namespace opentxs
{

class Message;
class Nym;
class String;

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

typedef std::vector<std::shared_ptr<std::string>> DhtResults;

typedef std::function<void(bool)> DhtDoneCallback;
typedef std::function<bool(const DhtResults&)> DhtResultsCallback;

typedef std::function<bool(
    const std::uint32_t, const std::string&, std::string&)> Digest;
typedef std::function<std::string()> Random;
typedef std::function<bool(const bool)> EmptyBucket;


/** C++11 representation of a claim. This version is more useful than the
 *  protobuf version, since it contains the claim ID.
 */
typedef std::tuple<
    std::string,             // claim identifier
    std::uint32_t,           // section
    std::uint32_t,           // type
    std::string,             // value
    std::int64_t,            // start time
    std::int64_t,            // end time
    std::set<std::uint32_t>> // attributes
        Claim;

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

typedef std::int64_t TransactionNumber;
typedef std::int64_t RequestNumber;

enum class ClaimPolarity : std::uint8_t {
    NEUTRAL  = 0,
    POSITIVE = 1,
    NEGATIVE = 2
};

enum class StorageBox : std::uint8_t {
    SENTPEERREQUEST  = 0,
    INCOMINGPEERREQUEST = 1,
    SENTPEERREPLY = 2,
    INCOMINGPEERREPLY = 3,
    FINISHEDPEERREQUEST = 4,
    FINISHEDPEERREPLY = 5,
    PROCESSEDPEERREQUEST = 6,
    PROCESSEDPEERREPLY = 7,
    MAILINBOX = 8,
    MAILOUTBOX = 9
};

enum class Bip43Purpose : std::uint32_t {
    PAYCODE = 47,                                           // BIP-47
    NYM = 0x4f544e4d                                        // OTNM
};

enum class Bip44Type : std::uint32_t {
    BITCOIN = 0
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
    origin_market_offer, // finalReceipt
    origin_payment_plan, // finalReceipt, paymentReceipt
    origin_smart_contract, // finalReceipt, paymentReceipt
    origin_pay_dividend, // SOME voucherReceipts are from a payDividend.
    origin_error_state
};

enum class SendResult : std::uint8_t {
    ERROR_SENDING = 0,
    TIMEOUT_RECEIVING = 1,
    HAVE_REPLY = 2
};

enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,
    ACTIVE = 1,
    STALLED = 2
};

typedef std::pair<SendResult, std::unique_ptr<std::string>> NetworkReplyRaw;
typedef std::pair<SendResult, std::unique_ptr<String>> NetworkReplyString;
typedef std::pair<SendResult, std::unique_ptr<Message>> NetworkReplyMessage;

enum class ClientCommandType : std::uint8_t {
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

    // register user account on a specific server, with public key. Nym ID
    // will be hash of said public key.
    registerNym = 2,

    // Delete user account from a specific server.
    unregisterNym = 3,

    // Get the next request number from the server (for this user). Most
    // requests must be accompanied by a request number, which increments
    // for each Nym with each request.
    getRequestNumber = 4,

    // Every transaction requires a transaction number. If your wallet
    // doesn't have one, then here it can request the server to send one
    // over. (Or several.)
    getTransactionNumbers = 5,

    // Used by AcceptEntireNymbox() as it's setting everything up.
    processNymbox = 6
};

enum class ThreadStatus : std::uint8_t {
    ERROR = 0,
    RUNNING = 1,
    FINISHED = 2,
    SHUTDOWN = 3,
};

enum class Messagability : std::int8_t {
    NO_SERVER_CLAIM = -3,
    INVALID_SENDER = -2,
    MISSING_SENDER = -1,
    READY = 0,
    MISSING_RECIPIENT = 1,
    UNREGISTERED = 2,
};
} // namespace opentxs

#endif // OPENTXS_CORE_TYPES_HPP
