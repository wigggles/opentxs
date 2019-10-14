// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Legacy final : virtual public api::Legacy
{
public:
    const char* Account() const noexcept final { return account_; }
    const char* Cert() const noexcept final { return cert_; }
    std::string ClientConfigFilePath(const int instance) const noexcept final;
    std::string ClientDataFolder(const int instance) const noexcept final;
    const char* Common() const noexcept final { return common_; }
    const char* Contract() const noexcept final { return contract_; }
    const char* Cron() const noexcept final { return cron_; }
    std::string CryptoConfigFilePath() const noexcept final;
    const char* ExpiredBox() const noexcept final { return expired_box_; }
    const char* Inbox() const noexcept final { return inbox_; }
    std::string LogConfigFilePath() const noexcept final;
    const char* Market() const noexcept final { return market_; }
    const char* Mint() const noexcept final { return mint_; }
    const char* Nym() const noexcept final { return nym_; }
    const char* Nymbox() const noexcept final { return nymbox_; }
    const char* Outbox() const noexcept final { return outbox_; }
    std::string PIDFilePath() const noexcept final;
    const char* PaymentInbox() const noexcept final { return payment_inbox_; }
    const char* Purse() const noexcept final { return purse_; }
    const char* Receipt() const noexcept final { return receipt_; }
    const char* RecordBox() const noexcept final { return record_box_; }
    std::string ServerConfigFilePath(const int instance) const noexcept final;
    std::string ServerDataFolder(const int instance) const noexcept final;
    const char* Spent() const noexcept final { return spent_; }
    const char* UserAcct() const noexcept final { return useracct_; }

    ~Legacy() final = default;

private:
    friend opentxs::Factory;

    static const char* account_;
    static const char* cert_;
    static const char* common_;
    static const char* contract_;
    static const char* cron_;
    static const char* expired_box_;
    static const char* inbox_;
    static const char* market_;
    static const char* mint_;
    static const char* nym_;
    static const char* nymbox_;
    static const char* outbox_;
    static const char* payment_inbox_;
    static const char* purse_;
    static const char* receipt_;
    static const char* record_box_;
    static const char* spent_;
    static const char* useracct_;
    const std::string client_data_folder_{""};
    const std::string server_data_folder_{""};
    const std::string client_config_file_{""};
    const std::string crypto_config_file_{""};
    const std::string log_config_file_{""};
    const std::string server_config_file_{""};
    const std::string pid_file_{""};

    std::string get_path(const std::string& fragment, const int instance = 0)
        const noexcept;
    std::string get_file(const std::string& fragment, const int instance = 0)
        const noexcept;

    Legacy() noexcept;
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    Legacy& operator=(const Legacy&) = delete;
    Legacy& operator=(Legacy&&) = delete;
};
}  // namespace opentxs::api::implementation
