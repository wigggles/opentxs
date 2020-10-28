# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(key)
add_subdirectory(library)

set(cxx-sources
    Bip32.cpp
    Bip39.cpp
    bip39_word_list.cpp
    Envelope.cpp
    HDNode.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip32.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Bip39.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/Envelope.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    Bip32.hpp
    Bip39.hpp
    Envelope.hpp
    HDNode.hpp
)

add_library(opentxs-crypto OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-crypto PRIVATE Boost::headers opentxs::messages)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/crypto
    COMPONENT dev
  )
endif()
