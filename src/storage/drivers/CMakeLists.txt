if (OT_STORAGE_SQLITE)
  include_directories(SYSTEM
    ${SQLITE3_INCLUDE_DIRS}
  )
endif()

set(cxx-sources
  StorageFS.cpp
  StorageFSGC.cpp
  StorageFSArchive.cpp
  StorageMultiplex.cpp
  StorageSqlite3.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/storage/drivers/*.hpp"
)

file(GLOB cxx-interface-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/interface/storage/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
)

set(MODULE_NAME opentxs-storage-drivers)
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

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/storage/drivers
        COMPONENT dev)

install(FILES ${cxx-interface-install-headers}
        DESTINATION include/opentxs/interface/storage
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
