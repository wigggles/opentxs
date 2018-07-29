# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-bundled-sqlite)

set(c-sources
  sqlite3.c
)

add_library(${MODULE_NAME} OBJECT
  ${c-sources}
)

target_include_directories(${MODULE_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIRS}>
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_source_files_properties(${c-sources} LANGUAGE C)
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
