# Copyright (c) 2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(irrxml)
add_subdirectory(base64)

if(CASH_LUCRE_EXPORT)
  # Build lucre as library
  set(lucre-sources ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp)

  add_library(lucre OBJECT ${lucre-sources})

  if(OT_OPENSSL_FLAVOR_LIBRESSL)
    target_compile_definitions(lucre PUBLIC OT_BUNDLED_LIBRESSL)
  endif()

  target_include_directories(
    lucre SYSTEM
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/"
      "${OPENSSL_INCLUDE_DIR}"
  )
  if(NOT MSVC)
    target_compile_options(
      lucre
      PRIVATE
        -Wno-effc++
        -Wno-format
        -Wno-old-style-cast
        -Wno-sign-conversion
        -Wno-unused-variable
        -Wno-zero-as-null-pointer-constant
    )
  endif()

  if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    target_compile_options(
      lucre
      PRIVATE -Wno-shorten-64-to-32 -Wno-string-conversion
    )
  endif()

  set_property(TARGET lucre PROPERTY POSITION_INDEPENDENT_CODE 1)
endif(CASH_LUCRE_EXPORT)

if(TREZORCRYPTO_EXPORT)
  set(
    trezor-headers
    trezor/crypto/aes/aes.h
    trezor/crypto/aes/aesopt.h
    trezor/crypto/aes/aestab.h
    trezor/crypto/ed25519-donna/curve25519-donna-32bit.h
    trezor/crypto/ed25519-donna/curve25519-donna-helpers.c
    trezor/crypto/ed25519-donna/curve25519-donna-scalarmult-base.h
    trezor/crypto/ed25519-donna/ed25519-donna-32bit-tables.h
    trezor/crypto/ed25519-donna/ed25519-donna-basepoint-table.h
    trezor/crypto/ed25519-donna/ed25519-donna-impl-base.h
    trezor/crypto/ed25519-donna/ed25519-donna.h
    trezor/crypto/ed25519-donna/ed25519-hash-custom.h
    trezor/crypto/ed25519-donna/ed25519-keccak.h
    trezor/crypto/ed25519-donna/ed25519-sha3.h
    trezor/crypto/ed25519-donna/ed25519.h
    trezor/crypto/ed25519-donna/modm-donna-32bit.h
    trezor/crypto/address.h
    trezor/crypto/base58.h
    trezor/crypto/bignum.h
    trezor/crypto/bip32.h
    trezor/crypto/bip39.h
    trezor/crypto/bip39_english.h
    trezor/crypto/blake256.h
    trezor/crypto/blake2b.h
    trezor/crypto/curves.h
    trezor/crypto/ecdsa.h
    trezor/crypto/groestl.h
    trezor/crypto/hasher.h
    trezor/crypto/hmac.h
    trezor/crypto/hmac_drbg.h
    trezor/crypto/memzero.h
    trezor/crypto/nist256p1.h
    trezor/crypto/options.h
    trezor/crypto/pbkdf2.h
    trezor/crypto/rand.h
    trezor/crypto/rfc6979.h
    trezor/crypto/ripemd160.h
    trezor/crypto/secp256k1.h
    trezor/crypto/sha2.h
    trezor/crypto/sha3.h
  )
  set(
    trezor-sources
    trezor/crypto/aes/aes_modes.c
    trezor/crypto/aes/aescrypt.c
    trezor/crypto/aes/aeskey.c
    trezor/crypto/aes/aestab.c
    trezor/crypto/ed25519-donna/curve25519-donna-32bit.c
    trezor/crypto/ed25519-donna/curve25519-donna-helpers.c
    trezor/crypto/ed25519-donna/curve25519-donna-scalarmult-base.c
    trezor/crypto/ed25519-donna/ed25519-donna-32bit-tables.c
    trezor/crypto/ed25519-donna/ed25519-donna-basepoint-table.c
    trezor/crypto/ed25519-donna/ed25519-donna-impl-base.c
    trezor/crypto/ed25519-donna/ed25519-keccak.c
    trezor/crypto/ed25519-donna/ed25519-sha3.c
    trezor/crypto/ed25519-donna/ed25519.c
    trezor/crypto/ed25519-donna/modm-donna-32bit.c
    trezor/crypto/address.c
    trezor/crypto/base58.c
    trezor/crypto/bignum.c
    trezor/crypto/bip32.c
    trezor/crypto/bip39.c
    trezor/crypto/blake256.c
    trezor/crypto/blake2b.c
    trezor/crypto/curves.c
    trezor/crypto/ecdsa.c
    trezor/crypto/groestl.c
    trezor/crypto/hasher.c
    trezor/crypto/hmac.c
    trezor/crypto/hmac_drbg.c
    trezor/crypto/memzero.c
    trezor/crypto/nist256p1.c
    trezor/crypto/pbkdf2.c
    trezor/crypto/rand.c
    trezor/crypto/rfc6979.c
    trezor/crypto/ripemd160.c
    trezor/crypto/secp256k1.c
    trezor/crypto/sha2.c
    trezor/crypto/sha3.c
  )
  add_library(TrezorCrypto OBJECT ${trezor-sources} ${trezor-headers})
  set_property(TARGET TrezorCrypto PROPERTY POSITION_INDEPENDENT_CODE 1)
  set_property(TARGET TrezorCrypto PROPERTY C_STANDARD 99)
  set_property(TARGET TrezorCrypto PROPERTY C_STANDARD_REQUIRED ON)
  set_property(TARGET TrezorCrypto PROPERTY CMAKE_C_EXTENSIONS OFF)
  target_compile_definitions(TrezorCrypto PRIVATE "-DRAND_PLATFORM_INDEPENDENT")
  target_include_directories(
    TrezorCrypto SYSTEM
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor/crypto/"
  )
  target_include_directories(
    TrezorCrypto SYSTEM
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor/crypto/aes"
  )
  target_include_directories(
    TrezorCrypto SYSTEM
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/trezor/crypto/ed25519-donna"
  )
endif()

set(hashes-sources ${CMAKE_CURRENT_SOURCE_DIR}/siphash/src/siphash.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/smhasher/src/MurmurHash3.cpp)

set(hashes-headers ${CMAKE_CURRENT_SOURCE_DIR}/siphash/src/siphash.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/smhasher/src/MurmurHash3.h)

add_library(external-hashes OBJECT ${hashes-sources} ${hashes-headers})

if(NOT MSVC)
  target_compile_options(
    external-hashes
    PRIVATE
      -Wno-cast-align
      -Wno-effc++
      -Wno-implicit-fallthrough
      -Wno-old-style-cast
      -Wno-shadow
      -Wno-sign-conversion
      -Wno-switch-default
      -Wno-zero-as-null-pointer-constant
  )
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  target_compile_options(
    external-hashes
    PRIVATE -Wno-reserved-id-macro -Wno-shadow-field-in-constructor
  )
endif()

set_property(TARGET external-hashes PROPERTY POSITION_INDEPENDENT_CODE 1)
