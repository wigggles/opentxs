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

#ifndef OPENTXS_FORWARD_HPP
#define OPENTXS_FORWARD_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Pimpl.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Cash;
class Issuer;
class Pair;
class ServerAction;
class Sync;
class Wallet;
}  // namespace opentxs::api::client

namespace crypto
{
class Encode;
class Hash;
class Symmetric;
class Util;
}  // namespace opentxs::api::crypto

namespace storage
{
class Storage;
}  // namespace opentxs::api::storage

namespace network
{
class Dht;
class ZMQ;
}  // namespace opentxs::api::network

class Activity;
class Api;
class Blockchain;
class ContactManager;
class Crypto;
class Identity;
class Native;
class Server;
class Settings;
class UI;
}  // namespace opentxs::api

namespace client
{
class ServerAction;
class Wallet;
}  // namespace opentxs::client

namespace network
{
namespace zeromq
{
class Context;
class ListenCallback;
class Message;
class PairSocket;
class PublishSocket;
class PullSocket;
class PushSocket;
class ReplyCallback;
class ReplySocket;
class RequestSocket;
class SubscribeSocket;
}  // namespace opentxs::network::zeromq

class Dht;
class OpenDHT;
class ServerConnection;
class ZMQ;
}  // namespace opentxs::network

namespace server
{
class MessageProcessor;
class Server;
}  // namespace opentxs::server

namespace ui
{
class ActivityThread;
class ActivityThreadItem;
class ActivitySummary;
class ActivitySummaryItem;
class ContactList;
class ContactListItem;
class ListRow;
class MessagableList;
}  // namespace opentxs::ui

class Account;
class AccountVisitor;
class AsymmetricKeyEC;
class Basket;
class BasketContract;
class Bip32;
class Bip39;
class Cheque;
class ClientContext;
class Contact;
class ContactData;
class Context;
class Contract;
class CurrencyContract;
class Credential;
class CredentialSet;
class CryptoAsymmetric;
class CryptoEncoding;
class CryptoHash;
class CryptoSymmetric;
class CryptoSymmetricNew;
class Data;
class Ecdsa;
class Flag;
class Identifier;
class Item;
class Ledger;
class Letter;
class Log;
class MasterCredential;
class Message;
#if OT_CASH
class Mint;
#endif  // OT_CASH
class NumList;
class Nym;
class NymData;
class NymParameters;
class OT;
class OT_API;
class OTAPI_Exec;
class OTASCIIArmor;
class OTAsymmetricKey;
class OTCachedKey;
class OTCallback;
class OTCaller;
class OTClient;
class OTCronItem;
class OTDataFolder;
class OTEnvelope;
class OTFolders;
class OTKeypair;
class OTNym_or_SymmetricKey;
class OTPassword;
class OTPasswordData;
class OTPaths;
class OTPayment;
class OTPaymentPlan;
class OTRecordList;
class OTSignature;
class OTSignatureMetadata;
class OTSmartContract;
class OTSymmetricKey;
class OTTransaction;
class OTWallet;
class PayDividendVisitor;
class PaymentCode;
class PeerObject;
class Purse;
class ServerContext;
class ServerContract;
class Signals;
class StorageDriver;
class StoragePlugin;
class String;
class SymmetricKey;
#if OT_CASH
class Token;
#endif  // OT_CASH
class TransactionStatement;
class UnitDefinition;

using OTData = Pimpl<Data>;
using OTFlag = Pimpl<Flag>;
using OTPaymentCode = Pimpl<PaymentCode>;
using OTServerConnection = Pimpl<network::ServerConnection>;
using OTUIActivitySummary = Pimpl<ui::ActivitySummary>;
using OTUIActivitySummaryItem = Pimpl<ui::ActivitySummaryItem>;
using OTUIActivityThread = Pimpl<ui::ActivityThread>;
using OTUIActivityThreadItem = Pimpl<ui::ActivityThreadItem>;
using OTUIContactList = Pimpl<ui::ContactList>;
using OTUIContactListItem = Pimpl<ui::ContactListItem>;
using OTUIMessagableList = Pimpl<ui::MessagableList>;
using OTZMQContext = Pimpl<network::zeromq::Context>;
using OTZMQListenCallback = Pimpl<network::zeromq::ListenCallback>;
using OTZMQMessage = Pimpl<network::zeromq::Message>;
using OTZMQPairSocket = Pimpl<network::zeromq::PairSocket>;
using OTZMQPublishSocket = Pimpl<network::zeromq::PublishSocket>;
using OTZMQPullSocket = Pimpl<network::zeromq::PullSocket>;
using OTZMQPushSocket = Pimpl<network::zeromq::PushSocket>;
using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
using OTZMQReplySocket = Pimpl<network::zeromq::ReplySocket>;
using OTZMQRequestSocket = Pimpl<network::zeromq::RequestSocket>;
using OTZMQSubscribeSocket = Pimpl<network::zeromq::SubscribeSocket>;
}  // namespace opentxs
#endif  // OPENTXS_FORWARD_HPP
