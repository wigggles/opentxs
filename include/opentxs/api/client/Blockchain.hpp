// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain
{
public:
    using Chain = opentxs::blockchain::Type;
    using Style = blockchain::AddressStyle;

    /// Throws std::runtime_error if type is invalid
    EXPORT virtual const blockchain::BalanceTree& Account(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept(false) = 0;
    EXPORT virtual std::set<OTIdentifier> AccountList(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept = 0;
    EXPORT virtual bool AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const Identifier& label) const noexcept = 0;
    EXPORT virtual bool AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept = 0;
    EXPORT virtual std::tuple<OTData, Style, Chain> DecodeAddress(
        const std::string& encoded) const noexcept = 0;
    EXPORT virtual std::string EncodeAddress(
        const Style style,
        const Chain chain,
        const Data& data) const noexcept = 0;
    /// Throws std::out_of_range if the specified account does not exist
    EXPORT virtual const blockchain::HD& HDSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false) = 0;
    EXPORT virtual OTIdentifier NewHDSubaccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept = 0;
    EXPORT virtual bool StoreTransaction(
        const identifier::Nym& nymID,
        const Chain chain,
        const proto::BlockchainTransaction& transaction,
        const PasswordPrompt& reason) const noexcept = 0;
    EXPORT virtual std::shared_ptr<proto::BlockchainTransaction> Transaction(
        const std::string& id) const noexcept = 0;
    EXPORT virtual bool UpdateTransactions(
        const std::map<OTData, OTIdentifier>& changed) const noexcept = 0;

    EXPORT virtual ~Blockchain() = default;

protected:
    Blockchain() noexcept = default;

private:
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#endif
