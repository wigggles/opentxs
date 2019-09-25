// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NYMIDSOURCE_HPP
#define OPENTXS_CORE_NYMIDSOURCE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <memory>

namespace opentxs
{
namespace identity
{
class Source
{
public:
    EXPORT virtual OTString asString() const noexcept = 0;
    EXPORT virtual OTString Description() const noexcept = 0;
    EXPORT virtual proto::SourceType Type() const noexcept = 0;
    EXPORT virtual OTNymID NymID() const noexcept = 0;
    EXPORT virtual std::shared_ptr<proto::NymIDSource> Serialize() const
        noexcept = 0;
    EXPORT virtual bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature,
        const PasswordPrompt& reason) const noexcept = 0;
    EXPORT virtual bool Sign(
        const identity::credential::Primary& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept = 0;

    EXPORT virtual ~Source() = default;

protected:
    Source() = default;

private:
    Source(const Source&) = delete;
    Source(Source&&) = delete;
    Source& operator=(const Source&);
    Source& operator=(Source&&);
};
}  // namespace identity
}  // namespace opentxs
#endif
