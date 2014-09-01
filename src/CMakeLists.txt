# Copyright (c) Monetas AG, 2014

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PEDANTIC_CXX_FLAGS}")

add_subdirectory(core)
add_subdirectory(ext)
add_subdirectory(client)
add_subdirectory(opentxs)
add_subdirectory(opentxs-script)
