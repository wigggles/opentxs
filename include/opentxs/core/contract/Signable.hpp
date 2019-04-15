// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_SIGNABLE_HPP
#define OPENTXS_CORE_CONTRACT_SIGNABLE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{
using SerializedSignature = std::shared_ptr<proto::Signature>;

class Signable
{
public:
    virtual std::string Alias() const;
    OTIdentifier ID() const;
    virtual std::string Name() const = 0;
    Nym_p Nym() const;
    virtual const std::string& Terms() const;
    virtual OTData Serialize() const = 0;
    bool Validate() const;
    const std::uint32_t& Version() const;

    virtual void SetAlias(const std::string& alias);

    virtual ~Signable() = default;

protected:
    using Signatures = std::list<SerializedSignature>;

    std::string alias_;
    OTIdentifier id_;
    const Nym_p nym_;
    Signatures signatures_;
    std::uint32_t version_ = 0;
    std::string conditions_;  // Human-readable portion
    mutable std::mutex lock_;

    /** Calculate the ID and verify that it matches the existing id_ value */
    bool CheckID(const Lock& lock) const;
    virtual OTIdentifier id(const Lock& lock) const;
    virtual bool validate(const Lock& lock) const = 0;
    virtual bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const;
    bool verify_write_lock(const Lock& lock) const;

    /** Calculate and unconditionally set id_ */
    bool CalculateID(const Lock& lock);
    virtual bool update_signature(const Lock& lock);

    /** Calculate identifier */
    virtual OTIdentifier GetID(const Lock& lock) const = 0;

    explicit Signable(const Nym_p& nym);
    explicit Signable(const Nym_p& nym, const std::uint32_t version);
    explicit Signable(
        const Nym_p& nym,
        const std::uint32_t version,
        const std::string& conditions);
    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    Signable& operator=(const Signable&) = delete;
    Signable& operator=(Signable&&) = delete;
};
}  // namespace opentxs
#endif
