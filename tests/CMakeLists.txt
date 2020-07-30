# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(add_opentx_test_target target_name cxx-sources)
  include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests
                      ${GTEST_INCLUDE_DIRS})

  add_executable(${target_name} ${cxx-sources})
  target_link_libraries(
    ${target_name}
    opentxs::libopentxs
    ${GTEST_LIBRARY}
    Threads::Threads
    ZLIB::ZLIB
    Boost::system
    Boost::filesystem
    protobuf::libprotobuf-lite
    "${OT_ZMQ_TARGET}"
    unofficial-sodium::sodium
  )

  if(LIB_ANDROID_LOG)
    target_link_libraries(${target_name} ${LIB_ANDROID_LOG})
  endif()

  if(Boost_stacktrace_basic_FOUND)
    target_link_libraries(${target_name} Boost::stacktrace_basic)
  endif()

  if(OT_BLOCKCHAIN_EXPORT)
    target_link_libraries(${target_name} Boost::thread)
  endif()

  if(DHT_EXPORT)
    target_link_libraries(${target_name} opendht ${GNUTLS_LIBRARIES})
  endif()

  if(FS_EXPORT)
    target_link_libraries(${target_name} Boost::iostreams)
  endif()

  if(SQLITE_EXPORT)
    target_link_libraries(${target_name} SQLite::SQLite3)
  endif()

  if(LMDB_EXPORT)
    target_link_libraries(${target_name} lmdb)
  endif()

  if(OPENSSL_EXPORT)
    target_link_libraries(${target_name} OpenSSL::Crypto OpenSSL::SSL)
  endif()

  if(LIBSECP256K1_EXPORT)
    target_link_libraries(${target_name} unofficial::secp256k1)
  endif()

  target_include_directories(
    ${target_name} SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/"
  )
  set_target_properties(
    ${target_name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
  )

  add_test(${target_name} ${PROJECT_BINARY_DIR}/tests/${target_name}
           --gtest_output=xml:gtestresults.xml)
endfunction()

function(add_opentx_test target_name file_name)
  set(cxx-sources "${PROJECT_SOURCE_DIR}/tests/main.cpp" "${file_name}"
                  "${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp")

  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

function(add_opentx_low_level_test target_name file_name)
  set(cxx-sources "${PROJECT_SOURCE_DIR}/tests/lowlevel.cpp" "${file_name}"
                  "${PROJECT_SOURCE_DIR}/tests/OTLowLevelTestEnvironment.cpp")

  add_opentx_test_target("${target_name}" "${cxx-sources}")
endfunction()

add_subdirectory(blockchain)

if(OT_CASH_EXPORT)
  add_subdirectory(blind)
endif()

add_subdirectory(client)
add_subdirectory(core)
add_subdirectory(contact)
add_subdirectory(context)
add_subdirectory(crypto)
add_subdirectory(identity)
add_subdirectory(integration)
add_subdirectory(network/zeromq)
add_subdirectory(otx)

if(NOT WIN32)
  add_subdirectory(rpc)
endif()

add_subdirectory(ui)
