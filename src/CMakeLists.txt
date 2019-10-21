# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(api)

if(OT_CASH_EXPORT)
  set(blind $<TARGET_OBJECTS:opentxs-blind>)
  add_subdirectory(blind)
else()
  set(blind "")
endif()

add_subdirectory(client)
add_subdirectory(consensus)
add_subdirectory(contact)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(ext)
add_subdirectory(identity)
add_subdirectory(iterator)
add_subdirectory(network)
add_subdirectory(otx)
add_subdirectory(rpc)
add_subdirectory(server)
add_subdirectory(storage)
add_subdirectory(ui)
add_subdirectory(util)

if(TREZORCRYPTO_EXPORT)
  set(trezor $<TARGET_OBJECTS:TrezorCrypto>)
  set(trezor-wrapper $<TARGET_OBJECTS:opentxs-crypto-library-trezor>)
else()
  set(trezor "")
  set(trezor-wrapper "")
endif()

if(LIBSECP256K1_EXPORT)
  set(
    libsecp256k1-wrapper
    $<TARGET_OBJECTS:opentxs-crypto-library-libsecp256k1>
  )
else()
  set(libsecp256k1-wrapper "")
endif()

if(OPENSSL_EXPORT)
  set(openssl-wrapper $<TARGET_OBJECTS:opentxs-crypto-library-openssl>)
else()
  set(openssl-wrapper "")
endif()

if(OT_BLOCKCHAIN_EXPORT)
  set(
    blockchain
    $<TARGET_OBJECTS:opentxs-blockchain-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-block-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-block>
    $<TARGET_OBJECTS:opentxs-blockchain-client-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-client>
    $<TARGET_OBJECTS:opentxs-blockchain-p2p-bitcoin-message>
    $<TARGET_OBJECTS:opentxs-blockchain-p2p-bitcoin>
    $<TARGET_OBJECTS:opentxs-blockchain-p2p>
    $<TARGET_OBJECTS:opentxs-blockchain>
  )
  add_subdirectory(blockchain)
else()
  set(blockchain "")
endif()

if(CASH_LUCRE_EXPORT)
  set(lucre $<TARGET_OBJECTS:lucre>)
else()
  set(lucre "")
endif()

if(HD_EXPORT)
  set(hd $<TARGET_OBJECTS:opentxs-api-client-blockchain>)
else()
  set(hd "")
endif()

set(
  object-deps
  $<TARGET_OBJECTS:base64>
  $<TARGET_OBJECTS:external-hashes>
  $<TARGET_OBJECTS:irrxml>
  $<TARGET_OBJECTS:opentxs-api-client>
  $<TARGET_OBJECTS:opentxs-api-crypto-base58>
  $<TARGET_OBJECTS:opentxs-api-crypto>
  $<TARGET_OBJECTS:opentxs-api-network>
  $<TARGET_OBJECTS:opentxs-api-server>
  $<TARGET_OBJECTS:opentxs-api-storage>
  $<TARGET_OBJECTS:opentxs-api>
  ${blockchain}
  $<TARGET_OBJECTS:opentxs-client>
  $<TARGET_OBJECTS:opentxs-consensus>
  $<TARGET_OBJECTS:opentxs-contact>
  $<TARGET_OBJECTS:opentxs-core-contract-basket>
  $<TARGET_OBJECTS:opentxs-core-contract-peer>
  $<TARGET_OBJECTS:opentxs-core-contract>
  $<TARGET_OBJECTS:opentxs-core-crypto>
  $<TARGET_OBJECTS:opentxs-core-transaction>
  $<TARGET_OBJECTS:opentxs-core-util>
  $<TARGET_OBJECTS:opentxs-core>
  $<TARGET_OBJECTS:opentxs-cron>
  $<TARGET_OBJECTS:opentxs-crypto-key>
  $<TARGET_OBJECTS:opentxs-crypto-library>
  $<TARGET_OBJECTS:opentxs-crypto>
  $<TARGET_OBJECTS:opentxs-ext>
  $<TARGET_OBJECTS:opentxs-identity-wot-verification>
  $<TARGET_OBJECTS:opentxs-identity-credential>
  $<TARGET_OBJECTS:opentxs-identity>
  $<TARGET_OBJECTS:opentxs-iterator>
  $<TARGET_OBJECTS:opentxs-network-zeromq-curve>
  $<TARGET_OBJECTS:opentxs-network-zeromq-socket>
  $<TARGET_OBJECTS:opentxs-network-zeromq-zap>
  $<TARGET_OBJECTS:opentxs-network-zeromq>
  $<TARGET_OBJECTS:opentxs-network>
  $<TARGET_OBJECTS:opentxs-otx-client>
  $<TARGET_OBJECTS:opentxs-otx>
  $<TARGET_OBJECTS:opentxs-recurring>
  $<TARGET_OBJECTS:opentxs-rpc>
  $<TARGET_OBJECTS:opentxs-script>
  $<TARGET_OBJECTS:opentxs-server>
  $<TARGET_OBJECTS:opentxs-storage-drivers>
  $<TARGET_OBJECTS:opentxs-storage-tree>
  $<TARGET_OBJECTS:opentxs-storage>
  $<TARGET_OBJECTS:opentxs-trade>
  $<TARGET_OBJECTS:opentxs-ui>
  $<TARGET_OBJECTS:opentxs-util>
  $<TARGET_OBJECTS:otprotob>
  ${blind}
  ${hd}
  ${libsecp256k1-wrapper}
  ${lucre}
  ${openssl-wrapper}
  ${trezor-wrapper}
  ${trezor}
)

set(
  cxx-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Factory.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Globals.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/OT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Proto.cpp
)

set(
  cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Exclusive.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Forward.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/opentxs.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/OT.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Pimpl.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Proto.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Proto.tpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Qt.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Shared.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/SharedPimpl.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Types.hpp"
)

set(
  cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/Internal.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Exclusive.tpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Shared.tpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.hpp"
)

add_library(opentxs ${cxx-sources} ${cxx-headers} ${object-deps})
add_library(opentxs::libopentxs ALIAS opentxs)

target_link_libraries(
  opentxs
  PUBLIC libopentxs-proto
  PRIVATE
    Threads::Threads
    ZLIB::ZLIB
    Boost::system
    Boost::filesystem
)

if(Boost_stacktrace_basic_FOUND)
  target_link_libraries(opentxs PRIVATE Boost::stacktrace_basic)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  target_link_libraries(opentxs PRIVATE Boost::thread)
endif()

if(OT_USE_VCPKG_TARGETS)
  target_link_libraries(opentxs PRIVATE unofficial-sodium:sodium)
  target_link_libraries(opentxs PRIVATE libzmq libzmq-static)
else()
  target_link_libraries(opentxs PRIVATE sodium)
  target_link_libraries(opentxs PRIVATE ${ZMQ_LIBRARIES})
endif()

if(CMAKE_DL_LIBS)
  target_link_libraries(opentxs PRIVATE ${CMAKE_DL_LIBS})
endif()

if(LIB_RT)
  target_link_libraries(opentxs PRIVATE ${LIB_RT})
endif()

if(LIB_ANDROID_LOG)
  target_link_libraries(opentxs PRIVATE ${LIB_ANDROID_LOG})
endif()

if(DHT_EXPORT)
  target_link_libraries(opentxs PRIVATE opendht ${GNUTLS_LIBRARIES})
endif()

if(FS_EXPORT)
  target_link_libraries(opentxs PRIVATE Boost::iostreams)
endif()

if(SQLITE_EXPORT)
  target_link_libraries(opentxs PRIVATE SQLite::SQLite3)
endif()

if(LMDB_EXPORT)
  target_link_libraries(opentxs PRIVATE ${LMDB_LIBRARIES})
endif()

if(OPENSSL_EXPORT)
  target_link_libraries(opentxs PRIVATE OpenSSL::Crypto OpenSSL::SSL)
endif()

if(LIBSECP256K1_EXPORT)
  target_link_libraries(opentxs PRIVATE ${LIBSECP256K1_LIBRARY})
endif()

if(OT_WITH_QT)
  target_link_libraries(
    opentxs
    PUBLIC ${Qt5Qml_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Core_LIBRARIES}
  )
endif()

target_include_directories(opentxs SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/")
set_target_properties(
  opentxs
  PROPERTIES
    VERSION
    "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}"
    SOVERSION
    1
)

install(
  TARGETS opentxs DESTINATION ${CMAKE_INSTALL_LIBDIR}
  EXPORT opentxs-targets
  COMPONENT main
)

install(FILES ${cxx-install-headers} DESTINATION include/opentxs COMPONENT dev)
