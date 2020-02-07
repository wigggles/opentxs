// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTO_HPP
#define OPENTXS_PROTO_HPP

// IWYU pragma: begin_exports
#include <opentxs-proto/Types.hpp>
#include <opentxs-proto/Check.hpp>
// IWYU pragma: end_exports

namespace opentxs
{
using ProtobufType = ::google::protobuf::MessageLite;

static const proto::HashType StandardHash{proto::HASHTYPE_BLAKE2B256};
}  // namespace opentxs
#endif
