# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  Asymmetric.cpp
  Ed25519.cpp
  EllipticCurve.cpp
  HD.cpp
  Keypair.cpp
  RSA.cpp
  Secp256k1.cpp
  Symmetric.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Asymmetric.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/EllipticCurve.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Ed25519.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/HD.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Keypair.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/RSA.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Secp256k1.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/crypto/key/Symmetric.hpp"
)
set(
  cxx-headers
  ${cxx-install-headers}
  Asymmetric.hpp
  Ed25519.hpp
  EllipticCurve.hpp
  HD.hpp
  Keypair.hpp
  Null.hpp
  RSA.hpp
  Secp256k1.hpp
  Symmetric.hpp
  SymmetricNull.hpp
)

add_library(opentxs-crypto-key OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-crypto-key SYSTEM
  PRIVATE "${sodium_INCLUDE_DIR}"
)

if(OPENSSL_EXPORT)
  target_include_directories(
    opentxs-crypto-key SYSTEM
    PRIVATE "${OPENSSL_INCLUDE_DIR}"
  )
endif()

set_property(TARGET opentxs-crypto-key PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/crypto/key
  COMPONENT dev
)
