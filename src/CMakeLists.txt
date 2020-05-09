# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(api)

if(OT_CASH_EXPORT)
  add_subdirectory(blind)
  set(object-deps $<TARGET_OBJECTS:opentxs-blind>)
else()
  set(object-deps "")
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
add_subdirectory(protobuf)
add_subdirectory(rpc)
add_subdirectory(server)
add_subdirectory(storage)
add_subdirectory(ui)
add_subdirectory(util)

if(LIBSECP256K1_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-crypto-library-libsecp256k1>)
endif()

if(OPENSSL_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:opentxs-crypto-library-openssl>)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(blockchain)
  list(
    APPEND
      object-deps
      $<TARGET_OBJECTS:opentxs-api-client-blockchain-database>
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
endif()

if(CASH_LUCRE_EXPORT)
  list(APPEND object-deps $<TARGET_OBJECTS:lucre>)
endif()

list(
  APPEND
    object-deps
    $<TARGET_OBJECTS:base64>
    $<TARGET_OBJECTS:external-hashes>
    $<TARGET_OBJECTS:irrxml>
    $<TARGET_OBJECTS:messages>
    $<TARGET_OBJECTS:opentxs-api-client-blockchain>
    $<TARGET_OBJECTS:opentxs-api-client>
    $<TARGET_OBJECTS:opentxs-api-crypto-base58>
    $<TARGET_OBJECTS:opentxs-api-crypto>
    $<TARGET_OBJECTS:opentxs-api-network>
    $<TARGET_OBJECTS:opentxs-api-server>
    $<TARGET_OBJECTS:opentxs-api-storage>
    $<TARGET_OBJECTS:opentxs-api>
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
    $<TARGET_OBJECTS:opentxs-identity-credential>
    $<TARGET_OBJECTS:opentxs-identity-wot-verification>
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
    $<TARGET_OBJECTS:proto-verify>
)

set(
  cxx-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Bytes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Globals.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/OT.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Proto.cpp
)

set(
  cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Bytes.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Exclusive.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Forward.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/OT.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Pimpl.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Proto.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Proto.tpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Qt.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Shared.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/SharedPimpl.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/Types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/opentxs.hpp"
)

set(
  cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/0_stdafx.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/1_Internal.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Exclusive.tpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Factory.hpp"
)

add_library(opentxs ${cxx-sources} ${cxx-headers} ${object-deps})
add_library(opentxs::libopentxs ALIAS opentxs)

target_link_libraries(
  opentxs
  PRIVATE
    Threads::Threads
    ZLIB::ZLIB
    Boost::system
    Boost::filesystem
    protobuf::libprotobuf-lite
)

if(Boost_stacktrace_basic_FOUND)
  target_link_libraries(opentxs PRIVATE Boost::stacktrace_basic)
endif()

if(OT_BLOCKCHAIN_EXPORT)
  target_link_libraries(opentxs PRIVATE Boost::thread)
endif()

if(OT_USE_VCPKG_TARGETS)
  target_link_libraries(opentxs PRIVATE unofficial-sodium::sodium)
  target_link_libraries(opentxs PRIVATE libzmq libzmq-static)
else()
  target_link_libraries(opentxs PRIVATE ${SODIUM_LIBRARIES})
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

if(FS_EXPORT OR OPENTXS_BLOCK_STORAGE_ENABLED)
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
target_include_directories(opentxs SYSTEM PUBLIC "${Protobuf_INCLUDE_DIRS}")
set_target_properties(
  opentxs
  PROPERTIES
    VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
    SOVERSION
    1
)

add_custom_target(generated_code)
add_dependencies(generated_code otprotob)
add_dependencies(generated_code messages)

include(GenerateExportHeader)
generate_export_header(
  opentxs
  EXPORT_FILE_NAME
  "${${PROJECT_NAME}_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
)

install(
  TARGETS opentxs DESTINATION ${CMAKE_INSTALL_LIBDIR}
  EXPORT opentxs-targets
  COMPONENT main
)

if(ANDROID)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_SUFFIX)
  get_target_property(Boost_filesystem_LIBRARY Boost::filesystem
                      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX})
  get_target_property(Boost_stacktrace_basic_LIBRARY Boost::stacktrace_basic
                      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX})
  get_target_property(Boost_system_LIBRARY Boost::system
                      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX})
  get_target_property(Boost_thread_LIBRARY Boost::thread
                      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX})

  install(
    FILES
      "${Boost_filesystem_LIBRARY}"
      "${Boost_stacktrace_basic_LIBRARY}"
      "${Boost_system_LIBRARY}"
      "${Boost_thread_LIBRARY}"
      "${SODIUM_LIBRARIES}"
      "${ZMQ_LIBRARIES}"
      "${Protobuf_LITE_LIBRARIES}"
      "${ZLIB_LIBRARIES}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT main
  )

  if(LMDB_EXPORT)
    install(
      FILES "${LMDB_LIBRARIES}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(OPENSSL_EXPORT)
    install(
      FILES "${OPENSSL_CRYPTO_LIBRARY}" "${OPENSSL_SSL_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(LIBSECP256K1_EXPORT)
    install(
      FILES "${LIBSECP256K1_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()
endif()

install(
  FILES
    ${cxx-install-headers}
    "${${PROJECT_NAME}_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
  DESTINATION include/opentxs
  COMPONENT dev
)
