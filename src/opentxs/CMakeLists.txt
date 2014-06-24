# Copyright (c) Monetas AG, 2014

set(cxx-sources
  main.cpp
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ext
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/api
)

link_directories(
  ${CMAKE_BINARY_DIR}/lib
)

add_executable(opentxs ${cxx-sources})

target_link_libraries(opentxs opentxs-api)
