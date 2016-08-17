# Copyright (c) Monetas AG, 2014

include_directories(SYSTEM
  ${ZEROMQ_INCLUDE_DIRS}
  ${CZMQ_INCLUDE_DIR}
)

add_subdirectory(otprotob)
add_subdirectory(trade)
add_subdirectory(cron)
add_subdirectory(script)
add_subdirectory(recurring)

set(cxx-sources
  app/App.cpp
  app/Dht.cpp
  app/Identity.cpp
  app/Settings.cpp
  app/Wallet.cpp
  contract/basket/Basket.cpp
  contract/basket/BasketContract.cpp
  contract/basket/BasketItem.cpp
  contract/peer/BailmentReply.cpp
  contract/peer/BailmentRequest.cpp
  contract/peer/OutBailmentReply.cpp
  contract/peer/OutBailmentRequest.cpp
  contract/peer/PeerObject.cpp
  contract/peer/PeerReply.cpp
  contract/peer/PeerRequest.cpp
  contract/CurrencyContract.cpp
  contract/SecurityContract.cpp
  contract/ServerContract.cpp
  contract/Signable.cpp
  contract/UnitDefinition.cpp
  crypto/AsymmetricKeyEC.cpp
  crypto/AsymmetricKeyEd25519.cpp
  crypto/AsymmetricKeySecp256k1.cpp
  crypto/Bip32.cpp
  crypto/Bip39.cpp
  crypto/ChildKeyCredential.cpp
  crypto/ContactCredential.cpp
  crypto/Credential.cpp
  crypto/CredentialSet.cpp
  crypto/Crypto.cpp
  crypto/CryptoAsymmetric.cpp
  crypto/CryptoEncodingEngine.cpp
  crypto/CryptoEngine.cpp
  crypto/CryptoHash.cpp
  crypto/CryptoHashEngine.cpp
  crypto/CryptoSymmetric.cpp
  crypto/CryptoSymmetricEngine.cpp
  crypto/CryptoUtil.cpp
  crypto/Ecdsa.cpp
  crypto/KeyCredential.cpp
  crypto/Letter.cpp
  crypto/Libsecp256k1.cpp
  crypto/Libsodium.cpp
  crypto/LowLevelKeyGenerator.cpp
  crypto/MasterCredential.cpp
  crypto/NymParameters.cpp
  crypto/OTASCIIArmor.cpp
  crypto/OTAsymmetricKey.cpp
  crypto/OTAsymmetricKeyOpenSSL.cpp
  crypto/OTAsymmetricKeyOpenSSLPrivdp.cpp
  crypto/OTCachedKey.cpp
  crypto/OTCallback.cpp
  crypto/OTCaller.cpp
  crypto/OTEnvelope.cpp
  crypto/OTKeypair.cpp
  crypto/OTKeyring.cpp
  crypto/OTNymOrSymmetricKey.cpp
  crypto/OTPassword.cpp
  crypto/OTPasswordData.cpp
  crypto/OTSignatureMetadata.cpp
  crypto/OTSignedFile.cpp
  crypto/OTSymmetricKey.cpp
  crypto/OpenSSL.cpp
  crypto/PaymentCode.cpp
  crypto/SymmetricKey.cpp
  crypto/TrezorCrypto.cpp
  crypto/VerificationCredential.cpp
  crypto/mkcert.cpp
  transaction/Helpers.cpp
  util/Assert.cpp
  util/OTDataFolder.cpp
  util/OTFolders.cpp
  util/OTPaths.cpp
  util/StringUtils.cpp
  util/Tag.cpp
  util/Timer.cpp
  Account.cpp
  AccountList.cpp
  Cheque.cpp
  Contract.cpp
  Identifier.cpp
  Instrument.cpp
  Item.cpp
  Ledger.cpp
  Log.cpp
  Message.cpp
  NumList.cpp
  Nym.cpp
  NymIDSource.cpp
  OTData.cpp
  OTStorage.cpp
  OTStringXML.cpp
  OTTrackable.cpp
  OTTransaction.cpp
  OTTransactionType.cpp
  String.cpp
)

file(GLOB cxx-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/app/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/contract/basket/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/contract/peer/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/contract/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/crypto/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/transaction/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/util/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/*.hpp"
)

include_directories(${ProtobufIncludePath})


if(KEYRING_GNOME)
  include(GNUInstallDirs)
  include_directories(${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/glib-2.0/ /usr/${CMAKE_INSTALL_LIBDIR}/glib-2.0/include/ ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/gnome-keyring-1/)
  set(keyring libgnome-keyring.so)
endif()

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

set(MODULE_NAME opentxs-core)
if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress warnings about strcpy being unsafe in anyoption.cpp
  add_definitions(/wd4996)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
    util/OTWindowsRegistryTools.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

target_link_libraries(opentxs-core
  PRIVATE
  opentxs-storage
  opentxs-recurring
  opentxs-script
  opentxs-cron
  opentxs-trade
  otprotob irrxml
  base64
  czmq_local)
target_link_libraries(opentxs-core
  PUBLIC
  opentxs-network
  ${PROTOBUF_LITE_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${ZLIB_LIBRARY}
  ${SODIUM_LIBRARY}
  ${OPENTXS_SYSTEM_LIBRARIES}
  ${keyring})

add_library(opentxs-proto SHARED IMPORTED)
add_library(opentxs-verify SHARED IMPORTED)

set_property(TARGET opentxs-proto PROPERTY IMPORTED_LOCATION ${OPENTXS_PROTO})
set_property(TARGET opentxs-verify PROPERTY IMPORTED_LOCATION ${OPENTXS_VERIFY})

target_link_libraries(opentxs-core PRIVATE opentxs-proto)
target_link_libraries(opentxs-core PRIVATE opentxs-verify)

if (LIBSECP256K1_EXPORT)
    add_library(staticlibsecp256k1 STATIC IMPORTED)
    set_property(
        TARGET staticlibsecp256k1
        PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/deps/lib/libsecp256k1.a)
    target_link_libraries(opentxs-core PRIVATE staticlibsecp256k1)
    target_link_libraries(opentxs-core PUBLIC ${GMP_LIBRARIES})
endif()

if (TREZORCRYPTO_EXPORT)
    target_link_libraries(opentxs-core PRIVATE TrezorCrypto)
endif()

set_lib_property(opentxs-core)

if(WIN32)
  add_custom_command(TARGET opentxs-core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-core.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-core.dll")

  add_custom_command(TARGET opentxs-core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-core.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-core.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-core
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
