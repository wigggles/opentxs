// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_MINT_HPP
#define OPENTXS_BLIND_MINT_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH
#include "opentxs/core/Contract.hpp"

#include <cstdint>
#include <ctime>

namespace opentxs
{
namespace blind
{
class Mint : virtual public Contract
{
public:
    EXPORT virtual OTIdentifier AccountID() const = 0;
    EXPORT virtual bool Expired() const = 0;
    EXPORT virtual std::int64_t GetDenomination(std::int32_t nIndex) const = 0;
    EXPORT virtual std::int32_t GetDenominationCount() const = 0;
    EXPORT virtual time64_t GetExpiration() const = 0;
    EXPORT virtual std::int64_t GetLargestDenomination(
        std::int64_t lAmount) const = 0;
    EXPORT virtual bool GetPrivate(
        Armored& theArmor,
        std::int64_t lDenomination) const = 0;
    EXPORT virtual bool GetPublic(Armored& theArmor, std::int64_t lDenomination)
        const = 0;
    EXPORT virtual std::int32_t GetSeries() const = 0;
    EXPORT virtual time64_t GetValidFrom() const = 0;
    EXPORT virtual time64_t GetValidTo() const = 0;
    EXPORT virtual const identifier::UnitDefinition& InstrumentDefinitionID()
        const = 0;

    EXPORT virtual bool AddDenomination(
        const identity::Nym& theNotary,
        const std::int64_t denomination,
        const std::size_t keySize) = 0;
    EXPORT virtual void GenerateNewMint(
        const api::Wallet& wallet,
        std::int32_t nSeries,
        time64_t VALID_FROM,
        time64_t VALID_TO,
        time64_t MINT_EXPIRATION,
        const identifier::UnitDefinition& theInstrumentDefinitionID,
        const identifier::Server& theNotaryID,
        const identity::Nym& theNotary,
        const std::int64_t nDenom1,
        const std::int64_t nDenom2,
        const std::int64_t nDenom3,
        const std::int64_t nDenom4,
        const std::int64_t nDenom5,
        const std::int64_t nDenom6,
        const std::int64_t nDenom7,
        const std::int64_t nDenom8,
        const std::int64_t nDenom9,
        const std::int64_t nDenom10,
        const std::size_t keySize) = 0;
    EXPORT virtual bool LoadMint(const char* szAppend = nullptr) = 0;
    EXPORT virtual void Release_Mint() = 0;
    EXPORT virtual void ReleaseDenominations() = 0;
    EXPORT virtual bool SaveMint(const char* szAppend = nullptr) = 0;
    EXPORT virtual void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) = 0;
    EXPORT virtual void SetSavePrivateKeys(bool bDoIt = true) = 0;
    EXPORT virtual bool SignToken(
        const identity::Nym& notary,
        blind::Token& token) = 0;
    EXPORT virtual bool VerifyMint(const identity::Nym& theOperator) = 0;
    EXPORT virtual bool VerifyToken(
        const identity::Nym& notary,
        const blind::Token& token) = 0;

    EXPORT ~Mint() override = default;

protected:
    Mint() = default;

private:
    Mint(const Mint&) = delete;
    Mint(Mint&&) = delete;
    Mint& operator=(const Mint&) = delete;
    Mint& operator=(Mint&&) = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif  // OT_CASH
#endif
