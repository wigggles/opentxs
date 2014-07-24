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

add_executable(ot ${cxx-sources})

target_link_libraries(ot opentxs-api)

if(WIN32)
  add_custom_command(TARGET ot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/deps/libzmq_4/bin/$<CONFIGURATION>/libzmq-mt-gd-4_1_0.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/libzmq-mt-gd-4_1_0.dll")

  add_custom_command(TARGET ot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ZLIB_INCLUDE_DIR}/../bin/zlib1.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/zlib1.dll")
endif()

install(TARGETS ot DESTINATION bin)
