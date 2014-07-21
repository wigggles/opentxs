# Copyright (c) Monetas AG, 2014

add_subdirectory(otprotob)

set(cxx-sources
  Timer.cpp
  tinythread.cpp
  anyoption.cpp
  mkcert.cpp
  OTAccount.cpp
  OTAcctList.cpp
  OTAgent.cpp
  OTAgreement.cpp
  OTASCIIArmor.cpp
  OTAssert.cpp
  OTAssetContract.cpp
  OTAsymmetricKey.cpp
  OTAsymmetricKeyOpenSSL.cpp
  OTAsymmetricKeyOpenSSLPrivdp.cpp
  OTBasket.cpp
  OTBasketItem.cpp
  OTBylaw.cpp
  OTCachedKey.cpp
  OTCallback.cpp
  OTCaller.cpp
  OTCheque.cpp
  OTClause.cpp
  OTContract.cpp
  OTCredential.cpp
  OTCron.cpp
  OTCronItem.cpp
  OTCrypto.cpp
  OTData.cpp
  OTDataCheck.cpp
  OTDataFolder.cpp
  OTDigitalCash.cpp
  OTEnvelope.cpp
  OTFolders.cpp
  OTIdentifier.cpp
  OTInstrument.cpp
  OTItem.cpp
  OTKeyCredential.cpp
  OTKeypair.cpp
  OTKeyring.cpp
  OTLedger.cpp
  OTLog.cpp
  OTLowLevelKeyData.cpp
  OTMarket.cpp
  OTMasterkey.cpp
  OTMessage.cpp
  OTMessageBuffer.cpp
  OTMessageOutbuffer.cpp
  OTMint.cpp
  OTMintLucre.cpp
  OTNumList.cpp
  OTNymOrSymmetricKey.cpp
  OTOffer.cpp
  OTParty.cpp
  OTPartyAccount.cpp
  OTPassword.cpp
  OTPasswordData.cpp
  OTPaths.cpp
  OTPayload.cpp
  OTPayment.cpp
  OTPaymentPlan.cpp
  OTPseudonym.cpp
  OTPurse.cpp
  OTScript.cpp
  OTScriptable.cpp
  OTScriptChai.cpp
  OTServerContract.cpp
  OTSettings.cpp
  OTSignatureMetadata.cpp
  OTSignedFile.cpp
  OTSmartContract.cpp
  OTStash.cpp
  OTStashItem.cpp
  OTStorage.cpp
  StringUtils.cpp
  OTString.cpp
  OTStringXML.cpp
  OTSubcredential.cpp
  OTSubkey.cpp
  OTSymmetricKey.cpp
  OTToken.cpp
  OTTokenLucre.cpp
  OTTrackable.cpp
  OTTrade.cpp
  OTTransaction.cpp
  OTTransactionType.cpp
  OTVariable.cpp
  OTWallet.cpp
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/bigint/
  ${dependency_include_dir}/ChaiScript/include/
  ${dependency_include_dir}/irrxml/
  ${dependency_include_dir}/lucre/include/
  ${dependency_include_dir}/simpleini/
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core
  ${CMAKE_BINARY_DIR}/src/core/otprotob
)

if(KEYRING_GNOME)
  include(GNUInstallDirs)
  include_directories(${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/glib-2.0/ /usr/${CMAKE_INSTALL_LIBDIR}/glib-2.0/include/ ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/gnome-keyring-1/)
  set(keyring libgnome-keyring.so)
endif()

if(MSVC)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress warnings about strcpy being unsafe in anyoption.cpp
  add_definitions(/wd4996)

  # OTScriptChai needs this flag to avoid error C1128: number of sections exceeded object file format limit
  add_definitions(/bigobj)

  include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
  )

  # we have an extra Windows-specific file to compile
  add_library(opentxs-core
    ${cxx-sources}
    OTWindowsRegistryTools.cpp
  )

  target_link_libraries(opentxs-core otprotob bigint irrxml lucre ${PROTOBUF_LIBRARY} ${LIB_EAY_RELEASE} ${SSL_EAY_RELEASE} ${ZLIB_LIBRARY} ${OPENTXS_SYSTEM_LIBRARIES} ${keyring})
else()
  add_library(opentxs-core
    ${cxx-sources}
  )

  target_link_libraries(opentxs-core otprotob bigint irrxml lucre protobuf ssl crypto z ${OPENTXS_SYSTEM_LIBRARIES} ${keyring})
endif()

if(MSVC)
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
  install(TARGETS opentxs-core DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
