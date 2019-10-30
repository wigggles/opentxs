// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_FACTORY_HPP
#define OPENTXS_API_FACTORY_HPP

#include "opentxs/Forward.hpp"

#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace api
{
class Factory
{
public:
    EXPORT virtual OTArmored Armored() const = 0;
    EXPORT virtual OTArmored Armored(const std::string& input) const = 0;
    EXPORT virtual OTArmored Armored(const opentxs::Data& input) const = 0;
    EXPORT virtual OTArmored Armored(const opentxs::String& input) const = 0;
    EXPORT virtual OTArmored Armored(
        const opentxs::OTEnvelope& input) const = 0;
    EXPORT virtual OTArmored Armored(const ProtobufType& input) const = 0;
    EXPORT virtual OTString Armored(
        const ProtobufType& input,
        const std::string& header) const = 0;

    EXPORT virtual OTAsymmetricKey AsymmetricKey(
        const NymParameters& params,
        const opentxs::PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const = 0;
    EXPORT virtual OTAsymmetricKey AsymmetricKey(
        const proto::AsymmetricKey& serialized,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Basket> Basket() const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const = 0;

    EXPORT virtual std::unique_ptr<OTPassword> BinarySecret() const = 0;

#if OT_BLOCKCHAIN
    EXPORT virtual OTBlockchainAddress BlockchainAddress(
        const blockchain::p2p::Protocol protocol,
        const blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const std::set<blockchain::p2p::Service>& services) const = 0;
    EXPORT virtual std::unique_ptr<blockchain::block::Header> BlockHeader(
        const proto::BlockchainBlockHeader& serialized) const = 0;
    EXPORT virtual std::unique_ptr<blockchain::block::Header> BlockHeader(
        const blockchain::Type type,
        const Data& raw) const = 0;
    EXPORT virtual std::unique_ptr<blockchain::block::Header> BlockHeader(
        const blockchain::Type type,
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height) const = 0;
#endif  // OT_BLOCKCHAIN

    EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque() const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const = 0;

    EXPORT virtual std::unique_ptr<OTCron> Cron() const = 0;

    EXPORT virtual std::unique_ptr<OTCronItem> CronItem(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const = 0;

    EXPORT virtual OTData Data() const = 0;
    EXPORT virtual OTData Data(const opentxs::Armored& input) const = 0;
    EXPORT virtual OTData Data(const ProtobufType& input) const = 0;
    EXPORT virtual OTData Data(
        const opentxs::network::zeromq::Frame& input) const = 0;
    EXPORT virtual OTData Data(const std::uint8_t input) const = 0;
    EXPORT virtual OTData Data(const std::uint32_t input) const = 0;
    EXPORT virtual OTData Data(const std::string input, const StringStyle mode)
        const = 0;
    EXPORT virtual OTData Data(
        const std::vector<unsigned char>& input) const = 0;
    EXPORT virtual OTData Data(const std::vector<std::byte>& input) const = 0;

    EXPORT virtual OTIdentifier Identifier() const = 0;
    EXPORT virtual OTIdentifier Identifier(
        const std::string& serialized) const = 0;
    EXPORT virtual OTIdentifier Identifier(
        const opentxs::String& serialized) const = 0;
    EXPORT virtual OTIdentifier Identifier(
        const opentxs::Contract& contract) const = 0;
    EXPORT virtual OTIdentifier Identifier(const opentxs::Item& item) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const String& serialized,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const = 0;  // From owner we can get acct
                                                   // ID, server ID, and
                                                   // transaction Num
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const = 0;  // From owner we can get acct
                                                   // ID, server ID, and
                                                   // transaction Num
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNumber,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const = 0;

    EXPORT virtual OTKeypair Keypair(
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual OTKeypair Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual OTKeypair Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const opentxs::PasswordPrompt& reason) const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual OTKeypair Keypair(
        const std::string& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const EcdsaCurve& curve,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const = 0;

    EXPORT virtual std::unique_ptr<OTMarket> Market() const = 0;
    EXPORT virtual std::unique_ptr<OTMarket> Market(
        const char* szFilename) const = 0;
    EXPORT virtual std::unique_ptr<OTMarket> Market(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Message> Message() const = 0;

#if OT_CASH
    EXPORT virtual std::unique_ptr<blind::Mint> Mint() const = 0;
    EXPORT virtual std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const = 0;
    EXPORT virtual std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const = 0;
#endif

    EXPORT virtual OTNymID NymID() const = 0;
    EXPORT virtual OTNymID NymID(const std::string& serialized) const = 0;
    EXPORT virtual OTNymID NymID(const opentxs::String& serialized) const = 0;
    EXPORT virtual OTNymID NymIDFromPaymentCode(
        const std::string& serialized) const = 0;

    EXPORT virtual std::unique_ptr<OTOffer> Offer()
        const = 0;  // The constructor contains
                    // the 3 variables needed to
                    // identify any market.
    EXPORT virtual std::unique_ptr<OTOffer> Offer(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const = 0;

    EXPORT virtual OTPasswordPrompt PasswordPrompt(
        const std::string& text) const = 0;

    EXPORT virtual std::unique_ptr<OTPayment> Payment() const = 0;
    EXPORT virtual std::unique_ptr<OTPayment> Payment(
        const String& strPayment) const = 0;
    EXPORT virtual std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const = 0;

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& base58,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const = 0;
#endif

    EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan() const = 0;
    EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const = 0;
    EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& message,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment,
        const opentxs::PasswordPrompt& reason) const = 0;
#if OT_CASH
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse,
        const opentxs::PasswordPrompt& reason) const = 0;
#endif
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const std::shared_ptr<const PeerReply> reply,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized,
        const opentxs::PasswordPrompt& reason) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const = 0;

    EXPORT virtual OTZMQPipeline Pipeline(
        std::function<void(opentxs::network::zeromq::Message&)> callback)
        const = 0;

#if OT_CASH
    EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const ServerContext& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type = proto::CASHTYPE_LUCRE) const = 0;
    EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const proto::Purse& serialized) const = 0;
    EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type = proto::CASHTYPE_LUCRE) const = 0;
#endif  // OT_CASH

    EXPORT virtual std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const = 0;

    EXPORT virtual OTServerID ServerID() const = 0;
    EXPORT virtual OTServerID ServerID(const std::string& serialized) const = 0;
    EXPORT virtual OTServerID ServerID(
        const opentxs::String& serialized) const = 0;

    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile() const = 0;
    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME) const = 0;
    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME) const = 0;
    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME) const = 0;

    EXPORT virtual std::unique_ptr<OTSmartContract> SmartContract() const = 0;
    EXPORT virtual std::unique_ptr<OTSmartContract> SmartContract(
        const identifier::Server& NOTARY_ID) const = 0;

    /** Generate a blank, invalid key */
    EXPORT virtual OTSymmetricKey SymmetricKey() const = 0;
    /** Derive a new, random symmetric key
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] password Optional key password information.
     *  \param[in] mode The symmetric algorithm for which to generate an
     *                  appropriate key
     */
    EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const = 0;
    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] seed A binary or text seed to be expanded into a secret key
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF.
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type) const = 0;
    /** Construct a symmetric key from an existing OTPassword
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const OTPassword& raw,
        const opentxs::PasswordPrompt& reason) const = 0;

    EXPORT virtual std::unique_ptr<OTTrade> Trade() const = 0;
    EXPORT virtual std::unique_ptr<OTTrade> Trade(
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const = 0;

    EXPORT virtual std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem,
        const opentxs::PasswordPrompt& reason) const = 0;

    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        originType theOriginType = originType::not_applicable) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const = 0;
    // THIS factory only used when loading an abbreviated box receipt
    // (inbox, nymbox, or outbox receipt).
    // The full receipt is loaded only after the abbreviated ones are loaded,
    // and verified against them.
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        originType theOriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        const Time the_DATE_SIGNED,
        transactionType theType,
        const String& strHash,
        const std::int64_t& lAdjustment,
        const std::int64_t& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const = 0;

    EXPORT virtual OTUnitID UnitID() const = 0;
    EXPORT virtual OTUnitID UnitID(const std::string& serialized) const = 0;
    EXPORT virtual OTUnitID UnitID(const opentxs::String& serialized) const = 0;

    EXPORT virtual ~Factory() = default;

protected:
    Factory() = default;

private:
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
