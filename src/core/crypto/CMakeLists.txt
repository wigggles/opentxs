set(cxx-sources
  AsymmetricKeyEC.cpp
  AsymmetricKeyEd25519.cpp
  AsymmetricKeySecp256k1.cpp
  Bip32.cpp
  Bip39.cpp
  ChildKeyCredential.cpp
  ContactCredential.cpp
  Credential.cpp
  CredentialSet.cpp
  Crypto.cpp
  CryptoAsymmetric.cpp
  CryptoHash.cpp
  CryptoSymmetric.cpp
  CryptoUtil.cpp
  Ecdsa.cpp
  KeyCredential.cpp
  Letter.cpp
  Libsecp256k1.cpp
  Libsodium.cpp
  LowLevelKeyGenerator.cpp
  MasterCredential.cpp
  NymParameters.cpp
  OTASCIIArmor.cpp
  OTAsymmetricKey.cpp
  OTAsymmetricKeyOpenSSL.cpp
  OTAsymmetricKeyOpenSSLPrivdp.cpp
  OTCachedKey.cpp
  OTCallback.cpp
  OTCaller.cpp
  OTEnvelope.cpp
  OTKeypair.cpp
  OTKeyring.cpp
  OTNymOrSymmetricKey.cpp
  OTPassword.cpp
  OTPasswordData.cpp
  OTSignatureMetadata.cpp
  OTSignedFile.cpp
  OTSymmetricKey.cpp
  OpenSSL.cpp
  PaymentCode.cpp
  SymmetricKey.cpp
  TrezorCrypto.cpp
  VerificationCredential.cpp
  mkcert.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/crypto/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
)

set(MODULE_NAME opentxs-core-crypto)

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
        DESTINATION include/opentxs/core/crypto
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
