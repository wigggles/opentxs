// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_LEDGER_HPP
#define OPENTXS_CORE_LEDGER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Contract.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"

#include <cstdint>
#include <map>
#include <set>

namespace opentxs
{
namespace api
{
namespace implementation
{
class Factory;
}  // namespace implementation
}  // namespace api

// transaction ID is a std::int64_t, assigned by the server. Each transaction
// has one. FIRST the server issues the ID. THEN we create the blank transaction
// object with the ID in it and store it in our inbox. THEN if we want to send a
// transaction, we use the blank to do so. If there is no blank available, we
// message the server and request one.
typedef std::map<TransactionNumber, std::shared_ptr<OTTransaction>>
    mapOfTransactions;

// the "inbox" and "outbox" functionality is implemented in this class
class Ledger : public OTTransactionType
{
public:
    ledgerType m_Type{ledgerType::error_state};

    bool m_bLoadedLegacyData{false};  // So the server can tell if it just
                                      // loaded a legacy box or a hashed box.
                                      // (Legacy boxes stored ALL of the
                                      // receipts IN the box. No more.)

    inline ledgerType GetType() const { return m_Type; }

    EXPORT bool LoadedLegacyData() const { return m_bLoadedLegacyData; }

    // This function assumes that this is an INBOX.
    // If you don't use an INBOX to call this method, then it will return
    // nullptr immediately. If you DO use an inbox, then it will create a
    // balanceStatement item to go onto your transaction.  (Transactions require
    // balance statements. And when you get the atBalanceStatement reply from
    // the server, KEEP THAT RECEIPT. Well, OT will do that for you.) You only
    // have to keep the latest receipt, unlike systems that don't store balance
    // agreement.  We also store a list of issued transactions, the new balance,
    // and the outbox hash.
    EXPORT std::unique_ptr<Item> GenerateBalanceStatement(
        std::int64_t lAdjustment,
        const OTTransaction& theOwner,
        const ServerContext& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const PasswordPrompt& reason) const;
    EXPORT std::unique_ptr<Item> GenerateBalanceStatement(
        std::int64_t lAdjustment,
        const OTTransaction& theOwner,
        const ServerContext& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const std::set<TransactionNumber>& without,
        const PasswordPrompt& reason) const;

    EXPORT void ProduceOutboxReport(
        Item& theBalanceItem,
        const PasswordPrompt& reason);

    EXPORT bool AddTransaction(std::shared_ptr<OTTransaction> theTransaction);
    EXPORT bool RemoveTransaction(
        const TransactionNumber number);  // if false,
                                          // transaction
                                          // wasn't
                                          // found.

    EXPORT std::set<std::int64_t> GetTransactionNums(
        const std::set<std::int32_t>* pOnlyForIndices = nullptr) const;

    EXPORT std::shared_ptr<OTTransaction> GetTransaction(
        transactionType theType);
    EXPORT std::shared_ptr<OTTransaction> GetTransaction(
        const TransactionNumber number) const;
    EXPORT std::shared_ptr<OTTransaction> GetTransactionByIndex(
        std::int32_t nIndex) const;
    EXPORT std::shared_ptr<OTTransaction> GetFinalReceipt(
        std::int64_t lReferenceNum);
    EXPORT std::shared_ptr<OTTransaction> GetTransferReceipt(
        std::int64_t lNumberOfOrigin,
        const PasswordPrompt& reason);
    EXPORT std::shared_ptr<OTTransaction> GetChequeReceipt(
        std::int64_t lChequeNum,
        const PasswordPrompt& reason);
    EXPORT std::int32_t GetTransactionIndex(
        const TransactionNumber number);  // if not
                                          // found,
                                          // returns -1
    EXPORT std::shared_ptr<OTTransaction> GetReplyNotice(
        const std::int64_t& lRequestNum);

    // This calls OTTransactionType::VerifyAccount(), which calls
    // VerifyContractID() as well as VerifySignature().
    //
    // But first, this OTLedger version also loads the box receipts,
    // if doing so is appropriate. (message ledger == not appropriate.)
    //
    // Use this method instead of Contract::VerifyContract, which
    // expects/uses a pubkey from inside the contract in order to verify
    // it.
    //
    EXPORT bool VerifyAccount(
        const identity::Nym& theNym,
        const PasswordPrompt& reason) override;
    // For ALL abbreviated transactions, load the actual box receipt for each.
    EXPORT bool LoadBoxReceipts(
        const PasswordPrompt& reason,
        std::set<std::int64_t>* psetUnloaded = nullptr);  // if psetUnloaded
                                                          // passed
                                                          // in, then use it to
                                                          // return the #s that
                                                          // weren't there.
    EXPORT bool SaveBoxReceipts();  // For all "full version" transactions, save
                                    // the actual box receipt for each.
    // Verifies the abbreviated form exists first, and then loads the
    // full version and compares the two. Returns success / fail.
    //
    EXPORT bool LoadBoxReceipt(
        const std::int64_t& lTransactionNum,
        const PasswordPrompt& reason);
    // Saves the Box Receipt separately.
    EXPORT bool SaveBoxReceipt(const std::int64_t& lTransactionNum);
    // "Deletes" it by adding MARKED_FOR_DELETION to the bottom of the file.
    EXPORT bool DeleteBoxReceipt(const std::int64_t& lTransactionNum);

    EXPORT bool LoadInbox(const PasswordPrompt& reason);
    EXPORT bool LoadNymbox(const PasswordPrompt& reason);
    EXPORT bool LoadOutbox(const PasswordPrompt& reason);

    // If you pass the identifier in, the hash is recorded there
    EXPORT bool SaveInbox();
    EXPORT bool SaveInbox(Identifier& pInboxHash);
    EXPORT bool SaveNymbox();
    EXPORT bool SaveNymbox(Identifier& pNymboxHash);
    EXPORT bool SaveOutbox();
    EXPORT bool SaveOutbox(Identifier& pOutboxHash);

    EXPORT bool CalculateHash(Identifier& theOutput) const;
    EXPORT bool CalculateInboxHash(Identifier& theOutput) const;
    EXPORT bool CalculateOutboxHash(Identifier& theOutput) const;
    EXPORT bool CalculateNymboxHash(Identifier& theOutput) const;
    EXPORT bool SavePaymentInbox();
    EXPORT bool LoadPaymentInbox(const PasswordPrompt& reason);

    EXPORT bool SaveRecordBox();
    EXPORT bool LoadRecordBox(const PasswordPrompt& reason);

    EXPORT bool SaveExpiredBox();
    EXPORT bool LoadExpiredBox(const PasswordPrompt& reason);
    EXPORT bool LoadLedgerFromString(
        const String& theStr,
        const PasswordPrompt& reason);  // Auto-detects
                                        // ledger type.
    // (message/nymbox/inbox/outbox)
    EXPORT bool LoadInboxFromString(
        const String& strBox,
        const PasswordPrompt& reason);
    EXPORT bool LoadOutboxFromString(
        const String& strBox,
        const PasswordPrompt& reason);
    EXPORT bool LoadNymboxFromString(
        const String& strBox,
        const PasswordPrompt& reason);
    EXPORT bool LoadPaymentInboxFromString(
        const String& strBox,
        const PasswordPrompt& reason);
    EXPORT bool LoadRecordBoxFromString(
        const String& strBox,
        const PasswordPrompt& reason);
    EXPORT bool LoadExpiredBoxFromString(
        const String& strBox,
        const PasswordPrompt& reason);
    // inline for the top one only.
    inline std::int32_t GetTransactionCount() const
    {
        return static_cast<std::int32_t>(m_mapTransactions.size());
    }
    EXPORT std::int32_t GetTransactionCountInRefTo(
        std::int64_t lReferenceNum) const;
    EXPORT std::int64_t GetTotalPendingValue(
        const PasswordPrompt& reason);  // for inbox only, allows you
                                        // to
    // lookup the total value of pending
    // transfers within.
    EXPORT const mapOfTransactions& GetTransactionMap() const;

    EXPORT void Release() override;
    EXPORT void Release_Ledger();

    EXPORT void ReleaseTransactions();
    // ONLY call this if you need to load a ledger where you don't already know
    // the person's NymID
    // For example, if you need to load someone ELSE's inbox in order to send
    // them a transfer, then
    // you only know their account number, not their user ID. So you call this
    // function to get it
    // loaded up, and the NymID will hopefully be loaded up with the rest of
    // it.
    EXPORT void InitLedger();

    [[deprecated]] EXPORT bool GenerateLedger(
        const Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        const PasswordPrompt& reason,
        bool bCreateFile = false);
    EXPORT bool CreateLedger(
        const identifier::Nym& theNymID,
        const Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false);

    EXPORT static char const* _GetTypeString(ledgerType theType);
    EXPORT char const* GetTypeString() const { return _GetTypeString(m_Type); }

    EXPORT ~Ledger() override;

protected:
    bool LoadGeneric(
        ledgerType theType,
        const PasswordPrompt& reason,
        const String& pString = String::Factory());
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(
        irr::io::IrrXMLReader*& xml,
        const PasswordPrompt& reason) override;
    bool SaveGeneric(ledgerType theType);
    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

private:  // Private prevents erroneous use by other classes.
    friend api::implementation::Factory;

    typedef OTTransactionType ot_super;

    mapOfTransactions m_mapTransactions;  // a ledger contains a map of
                                          // transactions.

    std::tuple<bool, std::string, std::string, std::string> make_filename(
        const ledgerType theType);

    bool generate_ledger(
        const identifier::Nym& theNymID,
        const Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile);
    bool save_box(
        const ledgerType type,
        Identifier& hash,
        bool (Ledger::*calc)(Identifier&) const);

    Ledger(const api::Core& core);
    Ledger(
        const api::Core& core,
        const Identifier& theAccountID,
        const identifier::Server& theNotaryID);
    Ledger(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const identifier::Server& theNotaryID);
    Ledger() = delete;
};
}  // namespace opentxs
#endif
