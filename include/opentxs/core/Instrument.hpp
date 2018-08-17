// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_INSTRUMENT_HPP
#define OPENTXS_CORE_INSTRUMENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"

#include <cstdint>

namespace opentxs
{
class Instrument : public OTScriptable
{
public:
    EXPORT void Release() override;

    void Release_Instrument();
    EXPORT bool VerifyCurrentDate();  // Verify whether the CURRENT date is
                                      // WITHIN the VALID FROM / TO dates.
    EXPORT bool IsExpired();  // Verify whether the CURRENT date is AFTER the
                              // the
                              // "VALID TO" date.
    inline time64_t GetValidFrom() const { return m_VALID_FROM; }
    inline time64_t GetValidTo() const { return m_VALID_TO; }

    inline const Identifier& GetInstrumentDefinitionID() const
    {
        return m_InstrumentDefinitionID;
    }
    inline const Identifier& GetNotaryID() const { return m_NotaryID; }
    void InitInstrument();

    EXPORT virtual ~Instrument();

protected:
    OTIdentifier m_InstrumentDefinitionID;
    OTIdentifier m_NotaryID;
    // Expiration Date (valid from/to date)
    // The date, in seconds, when the instrument is valid FROM.
    time64_t m_VALID_FROM{0};
    // The date, in seconds, when the instrument expires.
    time64_t m_VALID_TO{0};

    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    inline void SetValidFrom(time64_t TIME_FROM) { m_VALID_FROM = TIME_FROM; }
    inline void SetValidTo(time64_t TIME_TO) { m_VALID_TO = TIME_TO; }
    inline void SetInstrumentDefinitionID(
        const Identifier& INSTRUMENT_DEFINITION_ID)
    {
        m_InstrumentDefinitionID = INSTRUMENT_DEFINITION_ID;
    }
    inline void SetNotaryID(const Identifier& NOTARY_ID)
    {
        m_NotaryID = NOTARY_ID;
    }

    Instrument(const api::Core& core);
    Instrument(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);

private:
    Instrument() = delete;
};
}  // namespace opentxs
#endif
