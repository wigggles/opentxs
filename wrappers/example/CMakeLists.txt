# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message("** Example wrapper")

set(OTX_WRAPPER_NAME "Example")

set(CMAKE_SWIG_FLAGS "")

find_package(SWIG 3.0.0 REQUIRED)
include(${SWIG_USE_FILE})

## Place any required custom modules in a ./cmake subdirectory
## pretend we are building a wrapper for php
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(PHP5)

## These are defined in our top-level opentxs.i which is included by
## our OTX_.i
include_directories(
  ${SWIG_INCLUDE_DIRS}
)

# module name is set in the wrapper's OTX_.i
set_source_files_properties(OTX_.i PROPERTIES CPLUSPLUS ON)

# use our module "example" found in the wrapper's OTX_.i
swig_add_module(${OTX_WRAPPER_NAME} php OTX_.i)
swig_link_libraries(${OTX_WRAPPER_NAME} ${PHP_LIBRARIES})

## cmake/swig does some funky stuff with names, fortunately they provide
## a macro to handle it. We use our $OTX_WRAPPER_NAME to reference it
## and shorten it to ${MOD_NAME}
##
set(MOD_NAME ${SWIG_MODULE_${OTX_WRAPPER_NAME}_REAL_NAME})


target_link_libraries(${MOD_NAME} ${OTX_LINK_LIBS})
set_target_properties(${MOD_NAME}
  PROPERTIES OUTPUT_NAME ${OTX_WRAPPER_NAME})
install(TARGETS ${MOD_NAME}
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT php)

