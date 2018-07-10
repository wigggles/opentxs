#// clang-format off
set(MODULE_NAME opentxs-crypto-library)

set(cxx-sources
  AsymmetricProvider.cpp
  EcdsaProvider.cpp
  HashingProvider.cpp
  LegacySymmetricProvider.cpp
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
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/LegacySymmetricProvider.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/SymmetricProvider.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/OpenSSL.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Secp256k1.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Sodium.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/crypto/library/Trezor.hpp"
  AsymmetricProvider.hpp
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
        DESTINATION include/opentxs/crypto/library
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
#// clang-format on
