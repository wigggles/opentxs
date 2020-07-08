// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

namespace opentxs
{
using OTZMQWorkType = std::uint8_t;

#define OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL 243
#define OT_ZMQ_NEW_PEER_SIGNAL 244
#define OT_ZMQ_NEW_BLOCK_HEADER_SIGNAL 245
#define OT_ZMQ_NEW_FILTER_SIGNAL 246
#define OT_ZMQ_NEW_NYM_SIGNAL 247
#define OT_ZMQ_CONNECT_SIGNAL 248
#define OT_ZMQ_DISCONNECT_SIGNAL 249
#define OT_ZMQ_RECEIVE_SIGNAL 250
#define OT_ZMQ_SEND_SIGNAL 251
#define OT_ZMQ_REGISTER_SIGNAL 252
#define OT_ZMQ_REORG_SIGNAL 253
#define OT_ZMQ_STATE_MACHINE_SIGNAL 254
#define OT_ZMQ_SHUTDOWN_SIGNAL 255
}  // namespace opentxs
