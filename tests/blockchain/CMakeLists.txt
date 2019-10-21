# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(name unittests-opentxs-blockchain)

set(
  cxx-sources
  ${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Test_BlockchainAPI.cpp
  Test_Filters.cpp
  Test_HeaderOracle.cpp
  Test_NumericHash.cpp
  Test_BlockHeader.cpp
  Test_CompactSize.cpp
  Test_Message.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests
                    ${GTEST_INCLUDE_DIRS})

add_executable(${name} ${cxx-sources})
target_link_libraries(
  ${name}
  opentxs::libopentxs
  ${GTEST_LIBRARY}
  Boost::headers
)
set_target_properties(
  ${name}
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name}
         --gtest_output=xml:gtestresults.xml)
