# Copyright (c) Monetas AG, 2014

set(cxx-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/anyoption/anyoption.cpp
  main.cpp
)

set(MODULE_NAME ot)
if (WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/winexe.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  add_executable(
    ${MODULE_NAME}
    ${cxx-sources}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_executable(${MODULE_NAME} ${cxx-sources})
endif()

target_link_libraries(ot opentxs-client)

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

install(TARGETS ot
        DESTINATION bin
        COMPONENT)
