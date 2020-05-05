// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTO_HPP
#define OPENTXS_PROTO_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#include <opentxs/protobuf/APIArgument.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/AcceptPendingPayment.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/AccountData.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/AccountEvent.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/AddClaim.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/AddContact.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/AsymmetricKey.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/Authority.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/Bailment.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/BailmentReply.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/BasketItem.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/BasketParams.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/Bip47Address.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/Bip47Channel.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/Bip47Direction.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/BitcoinBlockHeaderFields.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlindedSeriesList.pb.h>      // IWYU pragma: export
#include <opentxs/protobuf/BlockchainActivity.pb.h>     // IWYU pragma: export
#include <opentxs/protobuf/BlockchainAddress.pb.h>      // IWYU pragma: export
#include <opentxs/protobuf/BlockchainBlockHeader.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainBlockLocalData.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainExternalAddress.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainFilterHeader.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainInputWitness.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainPeerAddress.pb.h>   // IWYU pragma: export
#include <opentxs/protobuf/BlockchainPreviousOutput.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainTransaction.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainTransactionInput.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainTransactionOutput.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/BlockchainWalletKey.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/ChildCredentialParameters.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/Ciphertext.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/Claim.pb.h>                // IWYU pragma: export
#include <opentxs/protobuf/ClientContext.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/ConnectionInfo.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/ConnectionInfoReply.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/Contact.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/ContactData.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/ContactEvent.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/ContactItem.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/ContactSection.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/Context.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/CreateInstrumentDefinition.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/CreateNym.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/Credential.pb.h>      // IWYU pragma: export
#include <opentxs/protobuf/CurrencyParams.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Envelope.pb.h>      // IWYU pragma: export
#include <opentxs/protobuf/EquityParams.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/EthereumBlockHeaderFields.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/Faucet.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/GCS.pb.h>                 // IWYU pragma: export
#include <opentxs/protobuf/GetWorkflow.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/HDAccount.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/HDPath.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/HDSeed.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/InstrumentRevision.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/Issuer.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/KeyCredential.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/ListenAddress.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/LucreTokenData.pb.h>      // IWYU pragma: export
#include <opentxs/protobuf/MasterCredentialParameters.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/ModifyAccount.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/MoveFunds.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/NoticeAcknowledgement.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/Nym.pb.h>                    // IWYU pragma: export
#include <opentxs/protobuf/NymIDSource.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/OTXPush.pb.h>                // IWYU pragma: export
#include <opentxs/protobuf/OutBailment.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/OutBailmentReply.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/PairEvent.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/PaymentCode.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/PaymentEvent.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/PaymentWorkflow.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/PeerObject.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/PeerReply.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/PeerRequest.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/PeerRequestHistory.pb.h>     // IWYU pragma: export
#include <opentxs/protobuf/PeerRequestWorkflow.pb.h>    // IWYU pragma: export
#include <opentxs/protobuf/PendingBailment.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/PendingCommand.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/Purse.pb.h>                  // IWYU pragma: export
#include <opentxs/protobuf/PurseExchange.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/RPCCommand.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/RPCPush.pb.h>                // IWYU pragma: export
#include <opentxs/protobuf/RPCResponse.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/RPCStatus.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/RPCTask.pb.h>                // IWYU pragma: export
#include <opentxs/protobuf/Seed.pb.h>                   // IWYU pragma: export
#include <opentxs/protobuf/SendMessage.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/SendPayment.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/ServerContext.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/ServerContract.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/ServerReply.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/ServerRequest.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/SessionData.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/Signature.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/SourceProof.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/SpentTokenList.pb.h>         // IWYU pragma: export
#include <opentxs/protobuf/StorageAccountIndex.pb.h>    // IWYU pragma: export
#include <opentxs/protobuf/StorageAccounts.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/StorageBip47AddressIndex.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageBip47ChannelList.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageBip47Contexts.pb.h>     // IWYU pragma: export
#include <opentxs/protobuf/StorageBip47NymAddressIndex.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageBlockchainAccountList.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageBlockchainTransactions.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageBlockchainTxo.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageContactAddressIndex.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StorageContactNymIndex.pb.h>   // IWYU pragma: export
#include <opentxs/protobuf/StorageContacts.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/StorageCredentials.pb.h>       // IWYU pragma: export
#include <opentxs/protobuf/StorageIDList.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/StorageIssuers.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/StorageItemHash.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/StorageItems.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/StorageNotary.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/StorageNym.pb.h>               // IWYU pragma: export
#include <opentxs/protobuf/StorageNymList.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/StoragePaymentWorkflows.pb.h>  // IWYU pragma: export
#include <opentxs/protobuf/StoragePurse.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/StorageRoot.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/StorageSeeds.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/StorageServers.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/StorageThread.pb.h>            // IWYU pragma: export
#include <opentxs/protobuf/StorageThreadItem.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/StorageTxoIndex.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/StorageUnits.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/StorageWorkflowIndex.pb.h>     // IWYU pragma: export
#include <opentxs/protobuf/StorageWorkflowType.pb.h>      // IWYU pragma: export
#include <opentxs/protobuf/StoreSecret.pb.h>              // IWYU pragma: export
#include <opentxs/protobuf/SymmetricKey.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/TaggedKey.pb.h>                // IWYU pragma: export
#include <opentxs/protobuf/TaskComplete.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/Token.pb.h>                    // IWYU pragma: export
#include <opentxs/protobuf/TransactionData.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/UnitAccountMap.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/UnitDefinition.pb.h>           // IWYU pragma: export
#include <opentxs/protobuf/Verification.pb.h>             // IWYU pragma: export
#include <opentxs/protobuf/VerificationGroup.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/VerificationIdentity.pb.h>     // IWYU pragma: export
#include <opentxs/protobuf/VerificationOffer.pb.h>        // IWYU pragma: export
#include <opentxs/protobuf/VerificationSet.pb.h>          // IWYU pragma: export
#include <opentxs/protobuf/VerifyClaim.pb.h>              // IWYU pragma: export

#pragma GCC diagnostic pop
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/APIArgument.hpp"
#include "opentxs/protobuf/verify/AcceptPendingPayment.hpp"
#include "opentxs/protobuf/verify/AccountData.hpp"
#include "opentxs/protobuf/verify/AccountEvent.hpp"
#include "opentxs/protobuf/verify/AddClaim.hpp"
#include "opentxs/protobuf/verify/AddContact.hpp"
#include "opentxs/protobuf/verify/AsymmetricKey.hpp"
#include "opentxs/protobuf/verify/Authority.hpp"
#include "opentxs/protobuf/verify/Bailment.hpp"
#include "opentxs/protobuf/verify/BailmentReply.hpp"
#include "opentxs/protobuf/verify/BasketItem.hpp"
#include "opentxs/protobuf/verify/BasketParams.hpp"
#include "opentxs/protobuf/verify/Bip47Address.hpp"
#include "opentxs/protobuf/verify/Bip47Channel.hpp"
#include "opentxs/protobuf/verify/Bip47Direction.hpp"
#include "opentxs/protobuf/verify/BitcoinBlockHeaderFields.hpp"
#include "opentxs/protobuf/verify/BlindedSeriesList.hpp"
#include "opentxs/protobuf/verify/BlockchainActivity.hpp"
#include "opentxs/protobuf/verify/BlockchainAddress.hpp"
#include "opentxs/protobuf/verify/BlockchainBlockHeader.hpp"
#include "opentxs/protobuf/verify/BlockchainBlockLocalData.hpp"
#include "opentxs/protobuf/verify/BlockchainExternalAddress.hpp"
#include "opentxs/protobuf/verify/BlockchainFilterHeader.hpp"
#include "opentxs/protobuf/verify/BlockchainInputWitness.hpp"
#include "opentxs/protobuf/verify/BlockchainPeerAddress.hpp"
#include "opentxs/protobuf/verify/BlockchainPreviousOutput.hpp"
#include "opentxs/protobuf/verify/BlockchainTransaction.hpp"
#include "opentxs/protobuf/verify/BlockchainTransactionInput.hpp"
#include "opentxs/protobuf/verify/BlockchainTransactionOutput.hpp"
#include "opentxs/protobuf/verify/BlockchainWalletKey.hpp"
#include "opentxs/protobuf/verify/ChildCredentialParameters.hpp"
#include "opentxs/protobuf/verify/Ciphertext.hpp"
#include "opentxs/protobuf/verify/Claim.hpp"
#include "opentxs/protobuf/verify/ClientContext.hpp"
#include "opentxs/protobuf/verify/ConnectionInfo.hpp"
#include "opentxs/protobuf/verify/ConnectionInfoReply.hpp"
#include "opentxs/protobuf/verify/Contact.hpp"
#include "opentxs/protobuf/verify/ContactData.hpp"
#include "opentxs/protobuf/verify/ContactEvent.hpp"
#include "opentxs/protobuf/verify/ContactItem.hpp"
#include "opentxs/protobuf/verify/ContactSection.hpp"
#include "opentxs/protobuf/verify/Context.hpp"
#include "opentxs/protobuf/verify/CreateInstrumentDefinition.hpp"
#include "opentxs/protobuf/verify/CreateNym.hpp"
#include "opentxs/protobuf/verify/Credential.hpp"
#include "opentxs/protobuf/verify/CurrencyParams.hpp"
#include "opentxs/protobuf/verify/Envelope.hpp"
#include "opentxs/protobuf/verify/EquityParams.hpp"
#include "opentxs/protobuf/verify/EthereumBlockHeaderFields.hpp"
#include "opentxs/protobuf/verify/Faucet.hpp"
#include "opentxs/protobuf/verify/GCS.hpp"
#include "opentxs/protobuf/verify/GetWorkflow.hpp"
#include "opentxs/protobuf/verify/HDAccount.hpp"
#include "opentxs/protobuf/verify/HDPath.hpp"
#include "opentxs/protobuf/verify/HDSeed.hpp"
#include "opentxs/protobuf/verify/InstrumentRevision.hpp"
#include "opentxs/protobuf/verify/Issuer.hpp"
#include "opentxs/protobuf/verify/KeyCredential.hpp"
#include "opentxs/protobuf/verify/ListenAddress.hpp"
#include "opentxs/protobuf/verify/LucreTokenData.hpp"
#include "opentxs/protobuf/verify/MasterCredentialParameters.hpp"
#include "opentxs/protobuf/verify/ModifyAccount.hpp"
#include "opentxs/protobuf/verify/MoveFunds.hpp"
#include "opentxs/protobuf/verify/NoticeAcknowledgement.hpp"
#include "opentxs/protobuf/verify/Nym.hpp"
#include "opentxs/protobuf/verify/NymIDSource.hpp"
#include "opentxs/protobuf/verify/OTXPush.hpp"
#include "opentxs/protobuf/verify/OutBailment.hpp"
#include "opentxs/protobuf/verify/OutBailmentReply.hpp"
#include "opentxs/protobuf/verify/PairEvent.hpp"
#include "opentxs/protobuf/verify/PaymentCode.hpp"
#include "opentxs/protobuf/verify/PaymentEvent.hpp"
#include "opentxs/protobuf/verify/PaymentWorkflow.hpp"
#include "opentxs/protobuf/verify/PeerObject.hpp"
#include "opentxs/protobuf/verify/PeerReply.hpp"
#include "opentxs/protobuf/verify/PeerRequest.hpp"
#include "opentxs/protobuf/verify/PeerRequestHistory.hpp"
#include "opentxs/protobuf/verify/PeerRequestWorkflow.hpp"
#include "opentxs/protobuf/verify/PendingBailment.hpp"
#include "opentxs/protobuf/verify/PendingCommand.hpp"
#include "opentxs/protobuf/verify/Purse.hpp"
#include "opentxs/protobuf/verify/PurseExchange.hpp"
#include "opentxs/protobuf/verify/RPCCommand.hpp"
#include "opentxs/protobuf/verify/RPCPush.hpp"
#include "opentxs/protobuf/verify/RPCResponse.hpp"
#include "opentxs/protobuf/verify/RPCStatus.hpp"
#include "opentxs/protobuf/verify/RPCTask.hpp"
#include "opentxs/protobuf/verify/Seed.hpp"
#include "opentxs/protobuf/verify/SendMessage.hpp"
#include "opentxs/protobuf/verify/SendPayment.hpp"
#include "opentxs/protobuf/verify/ServerContext.hpp"
#include "opentxs/protobuf/verify/ServerContract.hpp"
#include "opentxs/protobuf/verify/ServerReply.hpp"
#include "opentxs/protobuf/verify/ServerRequest.hpp"
#include "opentxs/protobuf/verify/SessionData.hpp"
#include "opentxs/protobuf/verify/Signature.hpp"
#include "opentxs/protobuf/verify/Signature.hpp"
#include "opentxs/protobuf/verify/SourceProof.hpp"
#include "opentxs/protobuf/verify/SpentTokenList.hpp"
#include "opentxs/protobuf/verify/StorageAccountIndex.hpp"
#include "opentxs/protobuf/verify/StorageAccounts.hpp"
#include "opentxs/protobuf/verify/StorageBip47AddressIndex.hpp"
#include "opentxs/protobuf/verify/StorageBip47ChannelList.hpp"
#include "opentxs/protobuf/verify/StorageBip47Contexts.hpp"
#include "opentxs/protobuf/verify/StorageBip47NymAddressIndex.hpp"
#include "opentxs/protobuf/verify/StorageBlockchainAccountList.hpp"
#include "opentxs/protobuf/verify/StorageBlockchainTransactions.hpp"
#include "opentxs/protobuf/verify/StorageBlockchainTxo.hpp"
#include "opentxs/protobuf/verify/StorageContactAddressIndex.hpp"
#include "opentxs/protobuf/verify/StorageContactNymIndex.hpp"
#include "opentxs/protobuf/verify/StorageContacts.hpp"
#include "opentxs/protobuf/verify/StorageCredentials.hpp"
#include "opentxs/protobuf/verify/StorageIDList.hpp"
#include "opentxs/protobuf/verify/StorageIssuers.hpp"
#include "opentxs/protobuf/verify/StorageItemHash.hpp"
#include "opentxs/protobuf/verify/StorageItems.hpp"
#include "opentxs/protobuf/verify/StorageNotary.hpp"
#include "opentxs/protobuf/verify/StorageNym.hpp"
#include "opentxs/protobuf/verify/StorageNymList.hpp"
#include "opentxs/protobuf/verify/StoragePaymentWorkflows.hpp"
#include "opentxs/protobuf/verify/StoragePurse.hpp"
#include "opentxs/protobuf/verify/StorageRoot.hpp"
#include "opentxs/protobuf/verify/StorageSeeds.hpp"
#include "opentxs/protobuf/verify/StorageServers.hpp"
#include "opentxs/protobuf/verify/StorageThread.hpp"
#include "opentxs/protobuf/verify/StorageThreadItem.hpp"
#include "opentxs/protobuf/verify/StorageTxoIndex.hpp"
#include "opentxs/protobuf/verify/StorageUnits.hpp"
#include "opentxs/protobuf/verify/StorageWorkflowIndex.hpp"
#include "opentxs/protobuf/verify/StorageWorkflowType.hpp"
#include "opentxs/protobuf/verify/StoreSecret.hpp"
#include "opentxs/protobuf/verify/SymmetricKey.hpp"
#include "opentxs/protobuf/verify/TaggedKey.hpp"
#include "opentxs/protobuf/verify/TaskComplete.hpp"
#include "opentxs/protobuf/verify/Token.hpp"
#include "opentxs/protobuf/verify/TransactionData.hpp"
#include "opentxs/protobuf/verify/UnitAccountMap.hpp"
#include "opentxs/protobuf/verify/UnitDefinition.hpp"
#include "opentxs/protobuf/verify/Verification.hpp"
#include "opentxs/protobuf/verify/VerificationGroup.hpp"
#include "opentxs/protobuf/verify/VerificationIdentity.hpp"
#include "opentxs/protobuf/verify/VerificationOffer.hpp"
#include "opentxs/protobuf/verify/VerificationSet.hpp"
#include "opentxs/protobuf/verify/VerifyClaim.hpp"
#include "opentxs/protobuf/verify/VerifyCredentials.hpp"

namespace opentxs
{
using ProtobufType = ::google::protobuf::MessageLite;

static const proto::HashType StandardHash{proto::HASHTYPE_BLAKE2B256};
}  // namespace opentxs
#endif
