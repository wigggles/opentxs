# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(add_opentx_test target_name file_name)
  set(cxx-sources "${PROJECT_SOURCE_DIR}/tests/main.cpp" "${file_name}"
                  "${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp")

  include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests
                      ${GTEST_INCLUDE_DIRS})

  add_executable(${target_name} ${cxx-sources})
  target_link_libraries(
    ${target_name}
    opentxs::libopentxs
    ${GTEST_LIBRARY}
    ${ZMQ_LIBRARIES}
    Boost::filesystem
  )
  target_include_directories(${target_name} PRIVATE ${ZMQ_INCLUDE_DIRS})
  set_target_properties(
    ${target_name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
  )

  add_test(${target_name} ${PROJECT_BINARY_DIR}/tests/${target_name}
           --gtest_output=xml:gtestresults.xml)
endfunction()

if(OT_CRYPTO_SUPPORTED_KEY_HD)
  add_subdirectory(blockchain)
endif()

if(OT_CASH_EXPORT)
  add_subdirectory(blind)
endif()

add_subdirectory(client)
add_subdirectory(core)
add_subdirectory(contact)
add_subdirectory(crypto)
add_subdirectory(identity)
add_subdirectory(integration)
add_subdirectory(network/zeromq)
add_subdirectory(otx)
add_subdirectory(rpc)
add_subdirectory(ui)
