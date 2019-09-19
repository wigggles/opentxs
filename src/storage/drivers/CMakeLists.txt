# Copyright (c) 2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  StorageFS.cpp
  StorageFSGC.cpp
  StorageFSArchive.cpp
  StorageLMDB.cpp
  StorageMemDB.cpp
  StorageMultiplex.cpp
  StorageSqlite3.cpp
)
set(cxx-install-headers "")
set(
  cxx-headers
  ${cxx-install-headers}
  StorageFS.hpp
  StorageFSGC.hpp
  StorageFSArchive.hpp
  StorageLMDB.hpp
  StorageMemDB.hpp
  StorageMultiplex.hpp
  StorageSqlite3.hpp
)

add_library(opentxs-storage-drivers OBJECT ${cxx-sources} ${cxx-headers})

if(FS_EXPORT)
  target_include_directories(
    opentxs-storage-drivers SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
  )
endif()

if(OT_STORAGE_SQLITE)
  target_include_directories(
    opentxs-storage-drivers SYSTEM
    PRIVATE ${SQLITE3_INCLUDE_DIRS}
  )
endif()

if(OT_STORAGE_LMDB)
  target_include_directories(
    opentxs-storage-drivers SYSTEM
    PRIVATE ${LMDB_INCLUDE_DIR}
  )
endif()

set_property(
  TARGET opentxs-storage-drivers
  PROPERTY POSITION_INDEPENDENT_CODE 1
)
