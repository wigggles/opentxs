# Copyright (c) Monetas AG, 2014

set(cxx-sources
  opentxs.cpp
  main.cpp
)

add_executable(opentxs ${cxx-sources})

target_link_libraries(opentxs opentxs-client)

if(WIN32)
  add_custom_command(TARGET opentxs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/deps/libzmq_4/bin/$<CONFIGURATION>/libzmq-mt-gd-4_1_0.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/libzmq-mt-gd-4_1_0.dll")

  add_custom_command(TARGET opentxs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ZLIB_INCLUDE_DIR}/../bin/zlib1.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/zlib1.dll")
endif()

install(TARGETS opentxs
        DESTINATION bin
        COMPONENT)
