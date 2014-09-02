# Copyright (c) Monetas AG, 2014

add_subdirectory(otprotob)
add_subdirectory(basket)
add_subdirectory(cash)
add_subdirectory(trade)
add_subdirectory(cron)
add_subdirectory(script)

set(cxx-sources
  util/Timer.cpp
  util/tinythread.cpp
  util/Assert.cpp
  util/StringUtils.cpp
  transaction/Helpers.cpp
  mkcert.cpp
  OTAccount.cpp
  OTAcctList.cpp
  OTAgreement.cpp
  crypto/OTASCIIArmor.cpp
  OTAssetContract.cpp
  crypto/OTAsymmetricKey.cpp
  crypto/OTAsymmetricKeyOpenSSL.cpp
  crypto/OTAsymmetricKeyOpenSSLPrivdp.cpp
  crypto/OTCachedKey.cpp
  crypto/OTCallback.cpp
  crypto/OTCaller.cpp
  OTCheque.cpp
  OTContract.cpp
  crypto/OTCredential.cpp
  crypto/OTCrypto.cpp
  OTData.cpp
  OTDataCheck.cpp
  OTDataFolder.cpp
  crypto/OTEnvelope.cpp
  OTFolders.cpp
  OTIdentifier.cpp
  OTInstrument.cpp
  OTItem.cpp
  crypto/OTKeyCredential.cpp
  crypto/OTKeypair.cpp
  crypto/OTKeyring.cpp
  OTLedger.cpp
  OTLog.cpp
  crypto/OTLowLevelKeyData.cpp
  crypto/OTMasterkey.cpp
  OTMessage.cpp
  OTMessageBuffer.cpp
  OTMessageOutbuffer.cpp
  OTNumList.cpp
  crypto/OTNymOrSymmetricKey.cpp
  crypto/OTPassword.cpp
  crypto/OTPasswordData.cpp
  OTPaths.cpp
  OTPayload.cpp
  OTPaymentPlan.cpp
  OTPseudonym.cpp
  OTServerContract.cpp
  OTSettings.cpp
  crypto/OTSignatureMetadata.cpp
  crypto/OTSignedFile.cpp
  OTStash.cpp
  OTStashItem.cpp
  OTStorage.cpp
  OTString.cpp
  OTStringXML.cpp
  crypto/OTSubcredential.cpp
  crypto/OTSubkey.cpp
  crypto/OTSymmetricKey.cpp
  OTTrackable.cpp
  OTTransaction.cpp
  OTTransactionType.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/*.hpp")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core
  ${CMAKE_BINARY_DIR}/src/core/otprotob
)

if(KEYRING_GNOME)
  include(GNUInstallDirs)
  include_directories(${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/glib-2.0/ /usr/${CMAKE_INSTALL_LIBDIR}/glib-2.0/include/ ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/gnome-keyring-1/)
  set(keyring libgnome-keyring.so)
endif()

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress warnings about strcpy being unsafe in anyoption.cpp
  add_definitions(/wd4996)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  # we have an extra Windows-specific file to compile
  add_library(opentxs-core
    ${cxx-sources}
    ${cxx-headers}
    util/OTWindowsRegistryTools.cpp
  )
else()
  add_library(opentxs-core
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(opentxs-core otprotob opentxs-cron opentxs-script opentxs-trade opentxs-cash opentxs-basket bigint irrxml ${PROTOBUF_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARY} ${OPENTXS_SYSTEM_LIBRARIES} ${keyring})

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
          COMPONENT)
endif()
