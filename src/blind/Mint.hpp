// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/blind/Mint.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

#include <map>

namespace opentxs::blind::mint::implementation
{
class Mint : virtual public opentxs::blind::Mint, virtual public Contract
{
public:
    OTIdentifier AccountID() const override { return m_CashAccountID; }
    bool Expired() const override;
    std::int64_t GetDenomination(std::int32_t nIndex) const override;
    std::int32_t GetDenominationCount() const override
    {
        return m_nDenominationCount;
    }

    Time GetExpiration() const override { return m_EXPIRATION; }
    std::int64_t GetLargestDenomination(std::int64_t lAmount) const override;
    bool GetPrivate(Armored& theArmor, std::int64_t lDenomination)
        const override;
    bool GetPublic(Armored& theArmor, std::int64_t lDenomination)
        const override;
    std::int32_t GetSeries() const override { return m_nSeries; }
    Time GetValidFrom() const override { return m_VALID_FROM; }
    Time GetValidTo() const override { return m_VALID_TO; }
    const identifier::UnitDefinition& InstrumentDefinitionID() const override
    {
        return m_InstrumentDefinitionID;
    }

    void GenerateNewMint(
        const api::Wallet& wallet,
        const std::int32_t nSeries,
        const Time VALID_FROM,
        const Time VALID_TO,
        const Time MINT_EXPIRATION,
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
        const std::size_t keySize,
        const PasswordPrompt& reason) override;
    bool LoadContract(const PasswordPrompt& reason) override;
    bool LoadMint(const PasswordPrompt& reason, const char* szAppend = nullptr)
        override;
    void Release() override;
    void Release_Mint() override;
    void ReleaseDenominations() override;
    bool SaveMint(const char* szAppend = nullptr) override;
    void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) override
    {
        m_InstrumentDefinitionID = newID;
    }
    void SetSavePrivateKeys(bool bDoIt = true) override
    {
        m_bSavePrivateKeys = bDoIt;
    }
    void UpdateContents(const PasswordPrompt& reason) override;
    bool VerifyContractID() const override;
    bool VerifyMint(
        const identity::Nym& theOperator,
        const PasswordPrompt& reason) override;

    ~Mint() override;

protected:
    using mapOfArmor = std::map<std::int64_t, OTArmored>;

    std::int32_t ProcessXMLNode(
        irr::io::IrrXMLReader*& xml,
        const PasswordPrompt& reason) override;

    void InitMint();

    mapOfArmor m_mapPrivate;
    mapOfArmor m_mapPublic;
    OTServerID m_NotaryID;
    OTNymID m_ServerNymID;
    OTUnitID m_InstrumentDefinitionID;
    std::int32_t m_nDenominationCount;
    bool m_bSavePrivateKeys;
    std::int32_t m_nSeries;
    Time m_VALID_FROM;
    Time m_VALID_TO;
    Time m_EXPIRATION;
    OTIdentifier m_CashAccountID;

    Mint(const api::Core& core);
    Mint(
        const api::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    Mint(
        const api::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);

private:
    Mint() = delete;
};
}  // namespace opentxs::blind::mint::implementation
