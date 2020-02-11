// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/key/Symmetric.hpp"

#include "internal/api/crypto/Crypto.hpp"
#include "internal/api/Api.hpp"

#include "StorageParent.hpp"
#include "Scheduler.hpp"
#include "ZMQ.hpp"

#include <mutex>

namespace opentxs::api::implementation
{
class Core : virtual public api::internal::Core,
             public ZMQ,
             public Scheduler,
             public api::implementation::StorageParent
{
public:
    static const api::internal::Core& get_api(
        const PasswordPrompt& reason) noexcept;

    const crypto::Asymmetric& Asymmetric() const final { return asymmetric_; }
    const api::Settings& Config() const final { return config_; }
    const api::Crypto& Crypto() const final { return crypto_; }
    const std::string& DataFolder() const final { return data_folder_; }
    const api::network::Dht& DHT() const final;
    const api::Endpoints& Endpoints() const final { return endpoints_; }
    const api::Factory& Factory() const final { return factory_; }
    INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const final;
    bool GetSecret(
        const opentxs::Lock& lock,
        OTPassword& secret,
        const PasswordPrompt& reason,
        const bool twice) const final;
    int Instance() const final { return instance_; }
    const api::Legacy& Legacy() const noexcept final
    {
        return parent_.Legacy();
    }
    std::mutex& Lock() const final { return master_key_lock_; }
    const opentxs::crypto::key::Symmetric& MasterKey(
        const opentxs::Lock& lock) const final;
    const api::HDSeed& Seeds() const final;
    void SetMasterKeyTimeout(const std::chrono::seconds& timeout) const final;
    const api::storage::Storage& Storage() const final;
    const api::crypto::Symmetric& Symmetric() const final { return symmetric_; }
    const api::Wallet& Wallet() const final;
    const opentxs::network::zeromq::Context& ZeroMQ() const final
    {
        return zmq_context_;
    }

    ~Core() override;

protected:
    std::unique_ptr<api::internal::Factory> factory_p_;
    const api::internal::Factory& factory_;
    const api::crypto::internal::Asymmetric& asymmetric_;
    const api::crypto::Symmetric& symmetric_;
    std::unique_ptr<api::HDSeed> seeds_;
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::network::Dht> dht_;
    mutable std::mutex master_key_lock_;
    mutable std::unique_ptr<OTPassword> master_secret_;
    mutable OTSymmetricKey master_key_;
    mutable std::thread password_timeout_;
    mutable std::chrono::seconds password_duration_;
    mutable Time last_activity_;
    mutable std::atomic<bool> timeout_thread_running_;

    static OTSymmetricKey make_master_key(
        const api::internal::Context& parent,
        const api::Factory& factory,
        const proto::Ciphertext& encrypted_secret_,
        std::unique_ptr<OTPassword>& master_secret_,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage);

    void cleanup();

    Core(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const std::string& dataFolder,
        const int instance,
        const bool dhtDefault,
        std::unique_ptr<api::internal::Factory> factory);

private:
    void bump_password_timer(const opentxs::Lock& lock) const;
    void password_timeout() const;

    void storage_gc_hook() final;

    Core() = delete;
    Core(const Core&) = delete;
    Core(Core&&) = delete;
    Core& operator=(const Core&) = delete;
    Core& operator=(Core&&) = delete;
};
}  // namespace opentxs::api::implementation
