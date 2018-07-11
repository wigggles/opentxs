#// clang-format off
set(MODULE_NAME opentxs-crypto-key)

set(cxx-sources
  Asymmetric.cpp
  Ed25519.cpp
  EllipticCurve.cpp
  Keypair.cpp
  LegacySymmetric.cpp
  RSA.cpp
  RSA_private.cpp
  Secp256k1.cpp
  Symmetric.cpp
)

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/Asymmetric.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/EllipticCurve.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/Ed25519.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/Keypair.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/LegacySymmetric.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/RSA.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/Secp256k1.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/key/Symmetric.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  Keypair.hpp
  RSA_private.hpp
)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
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
        DESTINATION include/opentxs/crypto/key
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
#// clang-format on
