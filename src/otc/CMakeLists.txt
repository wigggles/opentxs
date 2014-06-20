# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

set(cxx-sources
  main.cpp
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(
  ${dependency_include_dir}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otlib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otextensions
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otapi
  ${PROJECT_SOURCE_DIR}/deps
)

add_executable(opentxs ${cxx-sources})

target_link_libraries(opentxs opentxs-api opentxs-ext opentxs-core)
