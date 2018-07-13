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
        trezor-crypto/aes_modes.c
        trezor-crypto/aescrypt.c
        trezor-crypto/aeskey.c
        trezor-crypto/aestab.c
        trezor-crypto/base58.c
        trezor-crypto/bignum.c
        trezor-crypto/bip32.c
        trezor-crypto/bip39.c
        trezor-crypto/curves.c
        trezor-crypto/ecdsa.c
        trezor-crypto/hmac.c
        trezor-crypto/nist256p1.c
        trezor-crypto/pbkdf2.c
        trezor-crypto/rand.c
        trezor-crypto/ripemd160.c
        trezor-crypto/script.c
        trezor-crypto/secp256k1.c
        trezor-crypto/sha2.c
        trezor-crypto/sha3.c
        trezor-crypto/curve25519-donna/curve25519.c
        trezor-crypto/ed25519-donna/ed25519.c
    )
    set(trezor-headers
        trezor-crypto/aes.h
        trezor-crypto/aesopt.h
        trezor-crypto/aestab.h
        trezor-crypto/base58.h
        trezor-crypto/bignum.h
        trezor-crypto/bip32.h
        trezor-crypto/bip39.h
        trezor-crypto/bip39_english.h
        trezor-crypto/curves.h
        trezor-crypto/ecdsa.h
        trezor-crypto/hmac.h
        trezor-crypto/macros.h
        trezor-crypto/nist256p1.h
        trezor-crypto/options.h
        trezor-crypto/pbkdf2.h
        trezor-crypto/rand.h
        trezor-crypto/ripemd160.h
        trezor-crypto/script.h
        trezor-crypto/secp256k1.h
        trezor-crypto/sha2.h
        trezor-crypto/sha3.h
        trezor-crypto/curve25519-donna/curve25519.h
        trezor-crypto/ed25519-donna/ed25519.h
    )
    include_directories(trezor-crypto/curve25519-donna)
    include_directories(trezor-crypto/ed25519-donna)
    set(CMAKE_C_FLAGS "-std=c99")
    set_source_files_properties(
        trezor-crypto/aeskey.c
        PROPERTIES
        COMPILE_FLAGS
        -Wno-sequence-point
    )
    add_library(TrezorCrypto OBJECT ${trezor-sources} ${trezor-headers})
    set_property(TARGET TrezorCrypto PROPERTY POSITION_INDEPENDENT_CODE 1)
endif()
