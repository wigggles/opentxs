# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

set(cxx-sources
  Test_OTData.cpp
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${GTEST_INCLUDE_DIRS}
)

add_executable(unittests-opentxs ${cxx-sources})
target_link_libraries(unittests-opentxs opentxs-core ${GTEST_BOTH_LIBRARIES})
