# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#// clang-format off
set(MODULE_NAME opentxs-crypto-library)

set(cxx-sources
  AsymmetricProvider.cpp
  Bitcoin.cpp
  EcdsaProvider.cpp
  HashingProvider.cpp
  OpenSSL.cpp
  OpenSSL_BIO.cpp
  Secp256k1.cpp
  Sodium.cpp
  Trezor.cpp
)

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/AsymmetricProvider.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/EcdsaProvider.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/EncodingProvider.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/HashingProvider.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/SymmetricProvider.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Bitcoin.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/OpenSSL.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Ripemd160.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Secp256k1.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Sodium.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Trezor.hpp"
  AsymmetricProvider.hpp
  AsymmetricProviderNull.hpp
  Bitcoin.hpp
  EcdsaProvider.hpp
  OpenSSL.hpp
  OpenSSL_BIO.hpp
  Secp256k1.hpp
  Sodium.hpp
  Trezor.hpp
)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/crypto/library
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})

target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/")

if(TREZORCRYPTO_EXPORT)
  target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/trezor/crypto")
endif()

target_include_directories(${MODULE_NAME} PRIVATE ${OT_SODIUM_HEADERS})
target_include_directories(${MODULE_NAME} PRIVATE ${OT_SSL_HEADERS})

if(LIBSECP256K1_EXPORT)
  target_include_directories(${MODULE_NAME} PRIVATE ${OT_SECP256K1_HEADERS})
endif()

if(LIBBITCOIN_EXPORT)
  target_include_directories(${MODULE_NAME} PRIVATE ${LIBBITCOIN_INCLUDE_DIRS})
endif()
#// clang-format on
