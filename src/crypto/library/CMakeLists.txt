# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  AsymmetricProvider.cpp
  EcdsaProvider.cpp
  HashingProvider.cpp
  Sodium.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/AsymmetricProvider.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/EcdsaProvider.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/EncodingProvider.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/HashingProvider.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/SymmetricProvider.hpp"
)
set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/Ripemd160.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/Sodium.hpp"
  AsymmetricProvider.hpp
  AsymmetricProviderNull.hpp
  EcdsaProvider.hpp
  Sodium.hpp
)

add_library(opentxs-crypto-library OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-crypto-library SYSTEM
  PRIVATE "${sodium_INCLUDE_DIR}"
)
set_property(TARGET opentxs-crypto-library PROPERTY POSITION_INDEPENDENT_CODE 1)

if(TREZORCRYPTO_EXPORT)
  add_library(
    opentxs-crypto-library-trezor OBJECT
    Trezor.cpp Trezor.hpp
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/Trezor.hpp"
  )
  target_include_directories(
    opentxs-crypto-library-trezor SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/trezor/crypto"
  )
  set_property(
    TARGET opentxs-crypto-library-trezor
    PROPERTY POSITION_INDEPENDENT_CODE 1
  )
endif()

if(LIBSECP256K1_EXPORT)
  add_library(
    opentxs-crypto-library-libsecp256k1 OBJECT
    Secp256k1.cpp Secp256k1.hpp
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/Secp256k1.hpp"
  )
  target_include_directories(
    opentxs-crypto-library-libsecp256k1 SYSTEM
    PRIVATE "${LIBSECP256K1_INCLUDE_DIR}"
  )
  set_property(
    TARGET opentxs-crypto-library-libsecp256k1
    PROPERTY POSITION_INDEPENDENT_CODE 1
  )
endif()

if(OPENSSL_EXPORT)
  add_library(
    opentxs-crypto-library-openssl OBJECT
    OpenSSL.cpp
    OpenSSL_BIO.cpp
    OpenSSL.hpp
    OpenSSL_BIO.hpp
    "${opentxs_SOURCE_DIR}/include/opentxs/crypto/library/OpenSSL.hpp"
  )
  target_include_directories(
    opentxs-crypto-library-openssl SYSTEM
    PRIVATE "${OPENSSL_INCLUDE_DIR}" "${Boost_INCLUDE_DIRS}"
  )
  set_property(
    TARGET opentxs-crypto-library-openssl
    PROPERTY POSITION_INDEPENDENT_CODE 1
  )
endif()

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/crypto/library
  COMPONENT dev
)
