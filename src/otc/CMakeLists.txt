# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

set(cxx-sources
	main.cpp
   )


add_executable(opentxs ${cxx-sources})

set(dependency_include_dir
    ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
   )

include_directories(${dependency_include_dir}
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otlib
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otextensions
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otapi
		    )

link_directories(${LIBRARY_OUTPUT_PATH})

target_link_libraries(opentxs opentxs-api opentxs-ext opentxs-core zmq ssl crypto z pthread dl)
