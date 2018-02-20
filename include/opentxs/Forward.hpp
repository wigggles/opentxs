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
class Message;
class ReplySocket;
class RequestSocket;
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
class OT_ME;
class OTAPI_Exec;
class OTASCIIArmor;
class OTAsymmetricKey;
class OTCachedKey;
class OTClient;
class OTCronItem;
class OTDataFolder;
class OTEnvelope;
class OTFolders;
class OTKeypair;
class OTME_too;
class OTNym_or_SymmetricKey;
class OTPassword;
class OTPasswordData;
class OTPaths;
class OTPayment;
class OTPaymentPlan;
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
using OTServerConnection = Pimpl<network::ServerConnection>;
}  // namespace opentxs
#endif  // OPENTXS_FORWARD_HPP
