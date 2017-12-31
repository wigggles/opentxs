if (OT_STORAGE_SQLITE)
  include_directories(SYSTEM
    ${SQLITE3_INCLUDE_DIRS}
  )
endif()

set(cxx-sources
  BlockchainTransactions.cpp
  Contacts.cpp
  Contexts.cpp
  Credentials.cpp
  Issuers.cpp
  Node.cpp
  Mailbox.cpp
  Nym.cpp
  Nyms.cpp
  PeerReplies.cpp
  PeerRequests.cpp
  Root.cpp
  Seeds.cpp
  Servers.cpp
  Thread.cpp
  Threads.cpp
  Tree.cpp
  Units.cpp
)

file(GLOB cxx-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/storage/tree/*.hpp"
)

set(MODULE_NAME opentxs-storage-tree)
if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
