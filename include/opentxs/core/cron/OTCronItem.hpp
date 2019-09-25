// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Base class for OTTrade and OTAgreement.
// OTCron contains lists of these for regular processing.

#ifndef OPENTXS_CORE_CRON_OTCRONITEM_HPP
#define OPENTXS_CORE_CRON_OTCRONITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/OTTrackable.hpp"
#include "opentxs/Types.hpp"

#include <deque>

namespace opentxs
{
class OTCronItem : public OTTrackable
{
public:
    virtual originType GetOriginType() const = 0;

    // To force the Nym to close out the closing number on the receipt.
    bool DropFinalReceiptToInbox(
        const identifier::Nym& NYM_ID,
        const Identifier& ACCOUNT_ID,
        const std::int64_t& lNewTransactionNumber,
        const std::int64_t& lClosingNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory());

    // Notify the Nym that the OPENING number is now closed, so he can remove it
    // from his issued list.
    bool DropFinalReceiptToNymbox(
        const identifier::Nym& NYM_ID,
        const TransactionNumber& lNewTransactionNumber,
        const String& strOrigCronItem,
        const originType theOriginType,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory());
    virtual bool CanRemoveItemFromCron(const ClientContext& context);
    virtual void HarvestOpeningNumber(ServerContext& context);
    virtual void HarvestClosingNumbers(ServerContext& context);
    // pActivator and pRemover are both "SOMETIMES nullptr"
    // I don't default the parameter, because I want to force the programmer to
    // choose.

    // Called in OTCron::AddCronItem.
    void HookActivationOnCron(
        const PasswordPrompt& reason,
        bool bForTheFirstTime = false);  // This calls
                                         // onActivate,
                                         // which is
                                         // virtual.

    // Called in OTCron::RemoveCronItem as well as OTCron::ProcessCron.
    // This calls onFinalReceipt, then onRemovalFromCron. Both are virtual.
    void HookRemovalFromCron(
        const api::Wallet& wallet,
        Nym_p pRemover,
        std::int64_t newTransactionNo,
        const PasswordPrompt& reason);

    inline bool IsFlaggedForRemoval() const { return m_bRemovalFlag; }
    inline void FlagForRemoval() { m_bRemovalFlag = true; }
    inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }

    EXPORT static std::unique_ptr<OTCronItem> LoadCronReceipt(
        const api::Core& core,
        const TransactionNumber& lTransactionNum,
        const PasswordPrompt& reason);  // Server-side only.
    EXPORT static std::unique_ptr<OTCronItem> LoadActiveCronReceipt(
        const api::Core& core,
        const TransactionNumber& lTransactionNum,
        const identifier::Server& notaryID,
        const PasswordPrompt& reason);  // Client-side only.
    EXPORT static bool EraseActiveCronReceipt(
        const std::string& dataFolder,
        const TransactionNumber& lTransactionNum,
        const identifier::Nym& nymID,
        const identifier::Server& notaryID);  // Client-side only.
    EXPORT static bool GetActiveCronTransNums(
        NumList& output,  // Client-side
                          // only.
        const std::string& dataFolder,
        const identifier::Nym& nymID,
        const identifier::Server& notaryID);
    inline void SetCreationDate(const time64_t& CREATION_DATE)
    {
        m_CREATION_DATE = CREATION_DATE;
    }
    inline const time64_t& GetCreationDate() const { return m_CREATION_DATE; }

    EXPORT bool SetDateRange(
        time64_t VALID_FROM = OT_TIME_ZERO,
        time64_t VALID_TO = OT_TIME_ZERO);
    inline void SetLastProcessDate(const time64_t& THE_DATE)
    {
        m_LAST_PROCESS_DATE = THE_DATE;
    }
    inline const time64_t& GetLastProcessDate() const
    {
        return m_LAST_PROCESS_DATE;
    }

    inline void SetProcessInterval(const std::int64_t& THE_DATE)
    {
        m_PROCESS_INTERVAL = THE_DATE;
    }
    inline const std::int64_t& GetProcessInterval() const
    {
        return m_PROCESS_INTERVAL;
    }

    inline OTCron* GetCron() const { return m_pCron; }
    void setServerNym(Nym_p serverNym) { serverNym_ = serverNym; }
    void setNotaryID(const identifier::Server& notaryID);
    // When first adding anything to Cron, a copy needs to be saved in a folder
    // somewhere.
    EXPORT bool SaveCronReceipt();  // server side only
    EXPORT bool SaveActiveCronReceipt(
        const identifier::Nym& theNymID);  // client
                                           // side
                                           // only

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    virtual bool ProcessCron(
        const PasswordPrompt& reason);  // OTCron calls this
                                        // regularly, which is my
                                        // chance to expire, etc.
                                        // From OTTrackable
                                        // (parent class of this)
    ~OTCronItem() override;

    void InitCronItem();

    void Release() override;
    void Release_CronItem();
    EXPORT bool GetCancelerID(identifier::Nym& theOutput) const;
    EXPORT bool IsCanceled() const { return m_bCanceled; }

    // When canceling a cron item before it
    // has been activated, use this.
    EXPORT bool CancelBeforeActivation(
        const identity::Nym& theCancelerNym,
        const PasswordPrompt& reason);

    // These are for     std::deque<std::int64_t> m_dequeClosingNumbers;
    // They are numbers used for CLOSING a transaction. (finalReceipt.)
    EXPORT std::int64_t GetClosingTransactionNoAt(std::uint32_t nIndex) const;
    EXPORT std::int32_t GetCountClosingNumbers() const;

    EXPORT void AddClosingTransactionNo(
        const std::int64_t& lClosingTransactionNo);

    // HIGHER LEVEL ABSTRACTIONS:
    EXPORT std::int64_t GetOpeningNum() const;
    EXPORT std::int64_t GetClosingNum() const;
    virtual bool IsValidOpeningNumber(const std::int64_t& lOpeningNum) const;

    virtual std::int64_t GetOpeningNumber(
        const identifier::Nym& theNymID) const;
    virtual std::int64_t GetClosingNumber(const Identifier& theAcctID) const;
    std::int32_t ProcessXMLNode(
        irr::io::IrrXMLReader*& xml,
        const PasswordPrompt& reason) override;

protected:
    std::deque<std::int64_t> m_dequeClosingNumbers;  // Numbers used for CLOSING
                                                     // a transaction.
                                                     // (finalReceipt.)
    OTNymID m_pCancelerNymID;

    bool m_bCanceled{false};  // This defaults to false. But if someone cancels
                              // it (BEFORE it is ever activated, just to nip it
                              // in the bud and harvest the numbers, and send
                              // the notices, etc) -- then we set this to true,
                              // and we also set the canceler Nym ID. (So we can
                              // see these values later and know whether it was
                              // canceled before activation, and if so, who did
                              // it.)

    bool m_bRemovalFlag{false};  // Set this to true and the cronitem will be
                                 // removed from Cron on next process.
    // (And its offer will be removed from the Market as well, if appropriate.)
    virtual void onActivate(const PasswordPrompt& reason) {
    }  // called by HookActivationOnCron().

    virtual void onFinalReceipt(
        OTCronItem& theOrigCronItem,
        const std::int64_t& lNewTransactionNumber,
        Nym_p theOriginator,
        Nym_p pRemover,
        const PasswordPrompt& reason) = 0;  // called by
                                            // HookRemovalFromCron().

    virtual void onRemovalFromCron(const PasswordPrompt& reason) {
    }  // called by HookRemovalFromCron().
    void ClearClosingNumbers();

    OTCronItem(
        const api::Core& core,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTCronItem(
        const api::Core& core,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& ACCT_ID,
        const identifier::Nym& NYM_ID);
    OTCronItem(const api::Core& core);

private:
    typedef OTTrackable ot_super;

    OTCron* m_pCron{nullptr};
    Nym_p serverNym_{nullptr};
    OTIdentifier notaryID_;
    time64_t m_CREATION_DATE{0};  // The date, in seconds, when the CronItem was
                                  // authorized.
    time64_t m_LAST_PROCESS_DATE{0};  // The last time this item was processed.
    std::int64_t m_PROCESS_INTERVAL{0};  // How often to Process Cron on this
                                         // item.

    OTCronItem() = delete;
    OTCronItem(const OTCronItem&) = delete;
    OTCronItem(OTCronItem&&) = delete;
    OTCronItem& operator=(const OTCronItem&) = delete;
    OTCronItem& operator=(OTCronItem&&) = delete;
};
}  // namespace opentxs
#endif
