// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::implementation
{
typedef std::deque<std::shared_ptr<Message>> dequeOfMail;
typedef std::map<std::string, OTIdentifier> mapOfIdentifiers;

class NymFile final : public opentxs::internal::NymFile, Lockable
{
public:
    bool CompareID(const identifier::Nym& rhs) const final;
    void DisplayStatistics(String& strOutput) const final;
    bool GetInboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const final;  // client-side
    bool GetOutboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const final;  // client-side
    std::shared_ptr<Message> GetOutpaymentsByIndex(
        const std::int32_t nIndex) const final;
    std::shared_ptr<Message> GetOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const PasswordPrompt& reason,
        std::unique_ptr<OTPayment>* pReturnPayment = nullptr,
        std::int32_t* pnReturnIndex = nullptr) const final;
    std::int32_t GetOutpaymentsCount() const final;
    const std::int64_t& GetUsageCredits() const final
    {
        sLock lock(shared_lock_);

        return m_lUsageCredits;
    }
    const identifier::Nym& ID() const final { return target_nym_->ID(); }
    std::string PaymentCode() const final { return target_nym_->PaymentCode(); }
    bool SerializeNymFile(String& output) const final;

    void AddOutpayments(std::shared_ptr<Message> theMessage) final;
    std::set<std::string>& GetSetAssetAccounts() final
    {
        sLock lock(shared_lock_);

        return m_setAccounts;
    }
    bool RemoveOutpaymentsByIndex(const std::int32_t nIndex) final;
    bool RemoveOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const PasswordPrompt& reason) final;
    bool SaveSignedNymFile(const identity::Nym& SIGNER_NYM);
    bool SetInboxHash(
        const std::string& acct_id,
        const Identifier& theInput) final;  // client-side
    bool SetOutboxHash(
        const std::string& acct_id,
        const Identifier& theInput) final;  // client-side
    void SetUsageCredits(const std::int64_t& lUsage) final
    {
        eLock lock(shared_lock_);

        m_lUsageCredits = lUsage;
    }

    ~NymFile() final;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    const Nym_p target_nym_{nullptr};
    const Nym_p signer_nym_{nullptr};
    std::int64_t m_lUsageCredits{-1};
    bool m_bMarkForDeletion{false};
    OTString m_strNymFile;
    OTString m_strVersion;
    OTString m_strDescription;

    // Whenever client downloads Inbox, its hash is stored here. (When
    // downloading account, can compare ITS inbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers m_mapInboxHash;
    // Whenever client downloads Outbox, its hash is stored here. (When
    // downloading account, can compare ITS outbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers m_mapOutboxHash;
    // Any outoing payments sent by this Nym. (And not yet deleted.) (payments
    // screen.)
    dequeOfMail m_dequeOutpayments;
    // (SERVER side)
    // A list of asset account IDs. Server side only (client side uses wallet;
    // has multiple servers.)
    std::set<std::string> m_setAccounts;

    bool GetHash(
        const mapOfIdentifiers& the_map,
        const std::string& str_id,
        Identifier& theOutput) const;

    void ClearAll();
    bool DeserializeNymFile(
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials = nullptr,
        const OTPassword* pImportPassword = nullptr);
    template <typename T>
    bool deserialize_nymfile(
        const T& lock,
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials,
        const OTPassword* pImportPassword = nullptr);
    bool LoadSignedNymFile(const PasswordPrompt& reason) final;
    template <typename T>
    bool load_signed_nymfile(const T& lock, const PasswordPrompt& reason);
    void RemoveAllNumbers(const String& pstrNotaryID = String::Factory());
    bool SaveSignedNymFile(const PasswordPrompt& reason) final;
    template <typename T>
    bool save_signed_nymfile(const T& lock, const PasswordPrompt& reason);
    template <typename T>
    bool serialize_nymfile(const T& lock, String& strNym) const;
    bool SerializeNymFile(const char* szFoldername, const char* szFilename);
    bool SetHash(
        mapOfIdentifiers& the_map,
        const std::string& str_id,
        const Identifier& theInput);

    NymFile(const api::internal::Core& core, Nym_p targetNym, Nym_p signerNym);
    NymFile() = delete;
    NymFile(const NymFile&) = delete;
    NymFile(NymFile&&) = delete;
    NymFile& operator=(const NymFile&) = delete;
    NymFile& operator=(NymFile&&) = delete;
};
}  // namespace opentxs::implementation
