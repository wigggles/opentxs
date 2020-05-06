// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Proto.hpp"  // IWYU pragma: associated

#include "protobuf/Check.hpp"

#define PROTO_NAME "instrument revision"

namespace opentxs::proto
{
bool CheckProto_1(const InstrumentRevision& input, const bool silent)
{
    CHECK_IDENTIFIER(id)
    CHECK_EXISTS(revision)
    CHECK_EXISTS(item)

    return true;
}

bool CheckProto_2(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(2)
}

bool CheckProto_3(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(3)
}

bool CheckProto_4(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(4)
}

bool CheckProto_5(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(5)
}

bool CheckProto_6(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(6)
}

bool CheckProto_7(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(7)
}

bool CheckProto_8(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(8)
}

bool CheckProto_9(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(9)
}

bool CheckProto_10(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(10)
}

bool CheckProto_11(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(11)
}

bool CheckProto_12(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(12)
}

bool CheckProto_13(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(13)
}

bool CheckProto_14(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(14)
}

bool CheckProto_15(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(15)
}

bool CheckProto_16(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(16)
}

bool CheckProto_17(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(17)
}

bool CheckProto_18(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(18)
}

bool CheckProto_19(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(19)
}

bool CheckProto_20(const InstrumentRevision& input, const bool silent)
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto