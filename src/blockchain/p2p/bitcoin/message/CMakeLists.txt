# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Addr.cpp
    Block.cpp
    Blocktxn.cpp
    Cfcheckpt.cpp
    Cfheaders.cpp
    Cfilter.cpp
    Cmpctblock.cpp
    Feefilter.cpp
    Filteradd.cpp
    Filterload.cpp
    Getblocks.cpp
    Getblocktxn.cpp
    Getcfcheckpt.cpp
    Getcfheaders.cpp
    Getcfilters.cpp
    Getdata.cpp
    Getheaders.cpp
    Headers.cpp
    Inv.cpp
    Merkleblock.cpp
    Message.cpp
    Notfound.cpp
    Nopayload.cpp
    Ping.cpp
    Pong.cpp
    Reject.cpp
    Sendcmpct.cpp
    Tx.cpp
    Version.cpp
)

set(cxx-headers
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/p2p/bitcoin/message/Message.hpp"
    Addr.hpp
    Block.hpp
    Blocktxn.hpp
    Cfcheckpt.hpp
    Cfheaders.hpp
    Cfilter.hpp
    Cmpctblock.hpp
    Feefilter.hpp
    Filteradd.hpp
    Filterload.hpp
    Getblocks.hpp
    Getblocktxn.hpp
    Getcfcheckpt.hpp
    Getcfheaders.hpp
    Getcfilters.hpp
    Getdata.hpp
    Getheaders.hpp
    Headers.hpp
    Inv.hpp
    Merkleblock.hpp
    Notfound.hpp
    Nopayload.hpp
    Ping.hpp
    Pong.hpp
    Reject.hpp
    Sendcmpct.hpp
    Tx.hpp
    Version.hpp
)

add_library(
  opentxs-blockchain-p2p-bitcoin-message OBJECT ${cxx-sources} ${cxx-headers}
)
target_link_libraries(
  opentxs-blockchain-p2p-bitcoin-message PRIVATE Boost::headers
                                                 opentxs::messages
)
add_dependencies(opentxs-blockchain-p2p-bitcoin-message generated_code)
