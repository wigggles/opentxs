// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/key/Symmetric.hpp"

#include "internal/api/crypto/Crypto.hpp"
#include "internal/api/Api.hpp"

#include "StorageParent.hpp"
#include "Scheduler.hpp"

#include <mutex>

namespace opentxs::api::implementation
{
class Core : virtual public api::internal::Core,
             public api::implementation::StorageParent,
             public Scheduler
{
public:
    const crypto::Asymmetric& Asymmetric() const override
    {
        return asymmetric_;
    }
    const api::Settings& Config() const override { return config_; }
    const api::Crypto& Crypto() const override { return crypto_; }
    const std::string& DataFolder() const override { return data_folder_; }
    const api::network::Dht& DHT() const override;
    const api::Endpoints& Endpoints() const override;
    const api::Factory& Factory() const override { return factory_; }
    INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const override;
    bool GetSecret(
        const opentxs::Lock& lock,
        OTPassword& secret,
        const PasswordPrompt& reason,
        const bool twice) const override;
    int Instance() const override { return instance_; }
    std::mutex& Lock() const override { return master_key_lock_; }
    const opentxs::crypto::key::Symmetric& MasterKey(
        const opentxs::Lock& lock) const override;
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& Seeds() const override;
#endif
    void SetMasterKeyTimeout(
        const std::chrono::seconds& timeout) const override;
    const api::storage::Storage& Storage() const override;
    const api::crypto::Symmetric& Symmetric() const override
    {
        return symmetric_;
    }
    const api::Wallet& Wallet() const override;
    const opentxs::network::zeromq::Context& ZeroMQ() const override
    {
        return zmq_context_;
    }

    virtual ~Core();

protected:
    std::unique_ptr<api::internal::Factory> factory_p_;
    const api::internal::Factory& factory_;
    const opentxs::network::zeromq::Context& zmq_context_;
    const api::crypto::internal::Asymmetric& asymmetric_;
    const api::crypto::Symmetric& symmetric_;
    const int instance_{0};
    std::unique_ptr<api::Endpoints> endpoints_;
#if OT_CRYPTO_WITH_BIP39
    std::unique_ptr<api::HDSeed> seeds_;
#endif
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::network::Dht> dht_;
    mutable std::mutex master_key_lock_;
    mutable std::unique_ptr<OTPassword> master_secret_;
    mutable OTSymmetricKey master_key_;
    mutable std::thread password_timeout_;
    mutable std::chrono::seconds password_duration_;
    mutable Time last_activity_;
    mutable std::atomic<bool> timeout_thread_running_;

    static std::unique_ptr<api::internal::Factory> make_factory(
        const api::internal::Core& me,
        api::internal::Factory* provided);
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
        api::internal::Factory* factory = nullptr);

private:
    void bump_password_timer(const opentxs::Lock& lock) const;
    void password_timeout() const;

    void storage_gc_hook() override;

    Core() = delete;
    Core(const Core&) = delete;
    Core(Core&&) = delete;
    Core& operator=(const Core&) = delete;
    Core& operator=(Core&&) = delete;
};
}  // namespace opentxs::api::implementation
