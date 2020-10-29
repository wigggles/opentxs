# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources StorageMemDB.cpp StorageMultiplex.cpp)
set(cxx-install-headers "")
set(cxx-headers ${cxx-install-headers} StorageMemDB.hpp StorageMultiplex.hpp)

if(FS_EXPORT)
  list(
    APPEND
    cxx-sources
    StorageFS.cpp
    StorageFSGC.cpp
    StorageFSArchive.cpp
  )
  list(
    APPEND
    cxx-headers
    StorageFS.hpp
    StorageFSGC.hpp
    StorageFSArchive.hpp
  )
endif()

if(SQLITE_EXPORT)
  list(APPEND cxx-sources StorageSqlite3.cpp)
  list(APPEND cxx-headers StorageSqlite3.hpp)
endif()

if(LMDB_EXPORT)
  list(APPEND cxx-sources StorageLMDB.cpp)
  list(APPEND cxx-headers StorageLMDB.hpp)
endif()

add_library(opentxs-storage-drivers OBJECT ${cxx-sources} ${cxx-headers})

if(FS_EXPORT)
  target_link_libraries(opentxs-storage-drivers PRIVATE Boost::headers)
endif()

if(SQLITE_EXPORT)
  target_include_directories(
    opentxs-storage-drivers SYSTEM PRIVATE ${SQLITE3_INCLUDE_DIRS}
  )
endif()

if(LMDB_EXPORT)
  target_link_libraries(opentxs-storage-drivers PRIVATE lmdb)
endif()

target_link_libraries(opentxs-storage-drivers PRIVATE opentxs::messages)
