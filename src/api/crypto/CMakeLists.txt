# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base58)

set(cxx-sources
    Asymmetric.cpp
    Config.cpp
    Crypto.cpp
    Encode.cpp
    Hash.cpp
    Symmetric.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Asymmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Config.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Encode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Hash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Symmetric.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/api/crypto/Util.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    Asymmetric.hpp
    Config.hpp
    Crypto.hpp
    Encode.hpp
    Hash.hpp
    Symmetric.hpp
)

add_library(opentxs-api-crypto OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-api-crypto SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(
  opentxs-api-crypto PRIVATE Boost::headers opentxs::messages
)

if(OPENSSL_EXPORT)
  target_compile_definitions(
    opentxs-api-crypto PUBLIC OT_CRYPTO_USING_OPENSSL=1
  )
else()
  target_compile_definitions(
    opentxs-api-crypto PUBLIC OT_CRYPTO_USING_OPENSSL=0
  )
endif()

set_property(TARGET opentxs-api-crypto PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/api/crypto
  COMPONENT dev
)
