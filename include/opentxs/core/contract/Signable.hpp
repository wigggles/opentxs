// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_SIGNABLE_HPP
#define OPENTXS_CORE_CONTRACT_SIGNABLE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <string>

namespace opentxs
{
namespace contract
{
class Signable
{
public:
    using Signature = std::shared_ptr<proto::Signature>;

    OPENTXS_EXPORT virtual std::string Alias() const = 0;
    OPENTXS_EXPORT virtual OTIdentifier ID() const = 0;
    OPENTXS_EXPORT virtual std::string Name() const = 0;
    OPENTXS_EXPORT virtual Nym_p Nym() const = 0;
    OPENTXS_EXPORT virtual const std::string& Terms() const = 0;
    OPENTXS_EXPORT virtual OTData Serialize() const = 0;
    OPENTXS_EXPORT virtual bool Validate(
        const opentxs::PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual VersionNumber Version() const = 0;

    OPENTXS_EXPORT virtual void SetAlias(const std::string& alias) = 0;

    OPENTXS_EXPORT virtual ~Signable() = default;

protected:
    Signable() noexcept = default;

private:
#ifdef _WIN32
public:
#endif
    OPENTXS_EXPORT virtual Signable* clone() const noexcept = 0;
#ifdef _WIN32
private:
#endif

    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    Signable& operator=(const Signable&) = delete;
    Signable& operator=(Signable&&) = delete;
};
}  // namespace contract
}  // namespace opentxs
#endif
