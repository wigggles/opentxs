# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

set(cxx-sources
	OpenTransactions.cpp
	ot_commands_ot.cpp
	ot_made_easy_ot.cpp
	OT_ME.cpp
	ot_opentxs_ot.cpp
	ot_otapi_ot.cpp
	ot_utility_ot.cpp
	OTAPI.cpp
	OTAPI_Exec.cpp
	OTClient.cpp
	OTMeChai.cpp
	OTMeCpp.cpp
	OTRecord.cpp
	OTRecordList.cpp
	OTServerConnection.cpp
   )


add_library(opentxs-api
	    STATIC
	    ${cxx-sources}
)


set(dependency_include_dir
    ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/
   )

include_directories(${dependency_include_dir}
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otlib
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otextensions
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/otapi
		    )

link_directories(${LIBRARY_OUTPUT_PATH})

target_link_libraries(opentxs-api opentxs-ext opentxs-core pthread ssl crypto pthread z dl zmq)
