# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

set(cxx-sources
      OTSocket.cpp
   )


add_library(opentxs-ext
	    STATIC
	    ${cxx-sources}
)


set(dependency_include_dir
    ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
   )

include_directories(${dependency_include_dir}
		    ${dependency_include_dir}/libzmq_4/include
		    ${dependency_include_dir}/cppzmq
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otlib
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otextensions
		    )

link_directories(${LIBRARY_OUTPUT_PATH})

target_link_libraries(opentxs-ext opentxs-core ssl crypto z pthread dl zmq)
