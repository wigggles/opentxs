# Copyright (c) Monetas AG, 2014

add_subdirectory(irrxml)
add_subdirectory(base64)

if(CASH_LUCRE_EXPORT)
  ### Build lucre as library
  set(lucre-sources
    ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
  )

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/ ${OPENSSL_INCLUDE_DIR})

  if(WIN32)
    # suppress sscanf/sprintf being unsafe warnings
    add_definitions("/wd4996")
  endif()

  add_library(lucre OBJECT
    ${lucre-sources}
  )

set_property(TARGET lucre PROPERTY POSITION_INDEPENDENT_CODE 1)
endif(CASH_LUCRE_EXPORT)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (TREZORCRYPTO_EXPORT)
    set(trezor-sources
        trezor-crypto/aes/aescrypt.c
        trezor-crypto/aes/aeskey.c
        trezor-crypto/aes/aes_modes.c
        trezor-crypto/aes/aestab.c
        trezor-crypto/chacha20poly1305/chacha20poly1305.c
        trezor-crypto/chacha20poly1305/chacha_merged.c
        trezor-crypto/chacha20poly1305/poly1305-donna.c
        trezor-crypto/chacha20poly1305/rfc7539.c
        trezor-crypto/ed25519-donna/curve25519-donna-32bit.c
        trezor-crypto/ed25519-donna/curve25519-donna-helpers.c
        trezor-crypto/ed25519-donna/curve25519-donna-scalarmult-base.c
        trezor-crypto/ed25519-donna/ed25519.c
        trezor-crypto/ed25519-donna/ed25519-donna-32bit-tables.c
        trezor-crypto/ed25519-donna/ed25519-donna-basepoint-table.c
        trezor-crypto/ed25519-donna/ed25519-donna-impl-base.c
        trezor-crypto/ed25519-donna/ed25519-keccak.c
        trezor-crypto/ed25519-donna/ed25519-sha3.c
        trezor-crypto/ed25519-donna/modm-donna-32bit.c
        trezor-crypto/address.c
        trezor-crypto/base32.c
        trezor-crypto/base58.c
        trezor-crypto/bignum.c
        trezor-crypto/bip32.c
        trezor-crypto/bip39.c
        trezor-crypto/blake256.c
        trezor-crypto/blake2b.c
        trezor-crypto/blake2s.c
        trezor-crypto/cash_addr.c
        trezor-crypto/curves.c
        trezor-crypto/ecdsa.c
        trezor-crypto/hasher.c
        trezor-crypto/hmac.c
        trezor-crypto/memzero.c
        trezor-crypto/nem.c
        trezor-crypto/nist256p1.c
        trezor-crypto/pbkdf2.c
        trezor-crypto/rand.c
        trezor-crypto/rc4.c
        trezor-crypto/ripemd160.c
        trezor-crypto/script.c
        trezor-crypto/secp256k1.c
        trezor-crypto/segwit_addr.c
        trezor-crypto/sha2.c
        trezor-crypto/sha3.c
    )
    set(trezor-headers
        trezor-crypto/aes/aes.h
        trezor-crypto/aes/aesopt.h
        trezor-crypto/aes/aestab.h
        trezor-crypto/aes/brg_types.h
        trezor-crypto/chacha20poly1305/chacha20poly1305.h
        trezor-crypto/chacha20poly1305/ecrypt-config.h
        trezor-crypto/chacha20poly1305/ecrypt-machine.h
        trezor-crypto/chacha20poly1305/ecrypt-portable.h
        trezor-crypto/chacha20poly1305/ecrypt-sync.h
        trezor-crypto/chacha20poly1305/poly1305-donna-32.h
        trezor-crypto/chacha20poly1305/poly1305-donna.h
        trezor-crypto/chacha20poly1305/rfc7539.h
        trezor-crypto/ed25519-donna/curve25519-donna-32bit.h
        trezor-crypto/ed25519-donna/curve25519-donna-helpers.h
        trezor-crypto/ed25519-donna/curve25519-donna-scalarmult-base.h
        trezor-crypto/ed25519-donna/ed25519-donna-32bit-tables.h
        trezor-crypto/ed25519-donna/ed25519-donna-basepoint-table.h
        trezor-crypto/ed25519-donna/ed25519-donna.h
        trezor-crypto/ed25519-donna/ed25519-donna-impl-base.h
        trezor-crypto/ed25519-donna/ed25519-donna-portable.h
        trezor-crypto/ed25519-donna/ed25519.h
        trezor-crypto/ed25519-donna/ed25519-hash-custom.h
        trezor-crypto/ed25519-donna/ed25519-hash-custom-keccak.h
        trezor-crypto/ed25519-donna/ed25519-hash-custom-sha3.h
        trezor-crypto/ed25519-donna/ed25519-keccak.h
        trezor-crypto/ed25519-donna/ed25519-sha3.h
        trezor-crypto/ed25519-donna/modm-donna-32bit.h
        trezor-crypto/address.h
        trezor-crypto/base32.h
        trezor-crypto/base58.h
        trezor-crypto/bignum.h
        trezor-crypto/bip32.h
        trezor-crypto/bip39_english.h
        trezor-crypto/bip39.h
        trezor-crypto/blake256.h
        trezor-crypto/blake2b.h
        trezor-crypto/blake2_common.h
        trezor-crypto/blake2s.h
        trezor-crypto/cash_addr.h
        trezor-crypto/check_mem.h
        trezor-crypto/curves.h
        trezor-crypto/ecdsa.h
        trezor-crypto/hasher.h
        trezor-crypto/hmac.h
        trezor-crypto/memzero.h
        trezor-crypto/nem.h
        trezor-crypto/nem_serialize.h
        trezor-crypto/nist256p1.h
        trezor-crypto/options.h
        trezor-crypto/pbkdf2.h
        trezor-crypto/rand.h
        trezor-crypto/rc4.h
        trezor-crypto/ripemd160.h
        trezor-crypto/script.h
        trezor-crypto/secp256k1.h
        trezor-crypto/segwit_addr.h
        trezor-crypto/sha2.h
        trezor-crypto/sha3.h
    )
    include_directories(trezor-crypto)
    include_directories(trezor-crypto/aes)
    include_directories(trezor-crypto/chacha20poly1305)
    include_directories(trezor-crypto/ed25519-donna)
    set(CMAKE_C_FLAGS "-std=c99")
    add_library(TrezorCrypto OBJECT ${trezor-sources} ${trezor-headers})
    set_property(TARGET TrezorCrypto PROPERTY POSITION_INDEPENDENT_CODE 1)
endif()
