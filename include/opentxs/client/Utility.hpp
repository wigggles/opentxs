// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_UTILITY_HPP
#define OPENTXS_CLIENT_UTILITY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/util/Common.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <array>
#include <string>

namespace opentxs
{

inline bool VerifyStringVal(const std::string& nValue)
{
    return 0 < nValue.length();
}

EXPORT std::int32_t InterpretTransactionMsgReply(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strAttempt,
    const std::string& strResponse);
EXPORT bool VerifyMessage(const std::string& strMessage);
EXPORT NetworkOperationStatus VerifyMessageSuccess(const std::string& message);
EXPORT std::int32_t VerifyMsgBalanceAgrmntSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strMessage);
EXPORT std::int32_t VerifyMsgTrnxSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strMessage);

typedef std::array<bool, 4> OTfourbool;

class Utility
{
public:
    std::string strLastReplyReceived;
    std::int32_t delay_ms{};
    std::int32_t max_trans_dl{};

    EXPORT Utility(ServerContext& context, const api::client::Manager& api);

    EXPORT void delay() const;
    EXPORT std::int32_t getAndProcessNymbox_3(
        const std::string& notaryID,
        const std::string& nymID,
        bool& bWasMsgSent);
    EXPORT std::int32_t getAndProcessNymbox_4(
        const std::string& notaryID,
        const std::string& nymID,
        bool& bWasMsgSent,
        bool bForceDownload);
    EXPORT std::int32_t getAndProcessNymbox_8(
        const std::string& notaryID,
        const std::string& nymID,
        bool& bWasMsgSent,
        bool bForceDownload,
        std::int32_t nRequestNumber,
        bool& bFoundNymboxItem,
        bool bHarvestingForRetry,
        const OTfourbool& bMsgFoursome);
    EXPORT bool getBoxReceiptLowLevel(
        const std::string& accountID,
        std::int32_t nBoxType,
        std::int64_t strTransactionNum,
        bool& bWasSent);
    EXPORT bool getBoxReceiptWithErrorCorrection(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& accountID,
        std::int32_t nBoxType,
        std::int64_t strTransactionNum);
    EXPORT std::int32_t getInboxAccount(
        const std::string& accountID,
        bool& bWasSentInbox,
        bool& bWasSentAccount);
    EXPORT std::int32_t getInboxAccount(
        const std::string& accountID,
        bool& bWasSentInbox,
        bool& bWasSentAccount,
        bool bForceDownload);
    EXPORT bool getInboxOutboxAccount(const std::string& accountID);
    EXPORT bool getInboxOutboxAccount(
        const std::string& accountID,
        bool bForceDownload);
    EXPORT bool getIntermediaryFiles(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& accountID);
    EXPORT bool getIntermediaryFiles(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& accountID,
        bool bForceDownload);
    EXPORT std::string getLastReplyReceived() const;
    EXPORT std::int32_t getNbrTransactionCount() const;
    EXPORT std::int32_t getNymbox(
        const std::string& notaryID,
        const std::string& nymID);
    EXPORT std::int32_t getNymbox(
        const std::string& notaryID,
        const std::string& nymID,
        bool bForceDownload);
    EXPORT std::int32_t getNymboxLowLevel();
    EXPORT std::int32_t getNymboxLowLevel(bool& bWasSent);
    EXPORT bool insureHaveAllBoxReceipts(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& accountID,
        std::int32_t nBoxType);
    EXPORT bool insureHaveAllBoxReceipts(
        const std::string& notaryID,
        const std::string& nymID,
        const std::string& accountID,
        std::int32_t nBoxType,
        std::int32_t nRequestSeeking,
        bool& bFoundIt);
    EXPORT void longDelay() const;
    EXPORT std::int32_t processNymbox(
        const std::string& notaryID,
        const std::string& nymID,
        bool& bWasMsgSent,
        std::int32_t& nMsgSentRequestNumOut,
        std::int32_t& nReplySuccessOut,
        std::int32_t& nBalanceSuccessOut,
        std::int32_t& nTransSuccessOut);
    EXPORT void setLastReplyReceived(const std::string& strReply);
    EXPORT void setNbrTransactionCount(std::int32_t new_trans_dl);

    EXPORT ~Utility() = default;

protected:
    ServerContext& context_;
    const api::client::Manager& api_;

    Utility() = delete;
    Utility(const Utility&) = delete;
    Utility(Utility&&) = delete;
    Utility& operator=(const Utility&) = delete;
    Utility& operator=(Utility&&) = delete;
};
}  // namespace opentxs

#endif
