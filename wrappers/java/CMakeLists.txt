# Copyright (c) 2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(OTX_WRAPPER_NAME "opentxs-java")

set(CMAKE_SWIG_FLAGS -package org.opentransactions.otapi)

if(ANDROID)
  # These variables are not needed so set them to dummy values
  set(JAVA_AWT_LIBRARY "${LIB_ANDROID_LOG}")
  set(JAVA_JVM_LIBRARY "${LIB_ANDROID_LOG}")
  set(JAVA_INCLUDE_PATH2 "${CMAKE_CURRENT_SOURCE_DIR}")
  set(JAVA_AWT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

find_package(JNI REQUIRED)

include_directories(${JAVA_INCLUDE_PATH})

set(
  JAVA_SOURCE_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/otx_java
  CACHE INTERNAL ""
)
set(
  JAVA_BINARY_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/otx_java
  CACHE INTERNAL ""
)
file(MAKE_DIRECTORY ${JAVA_SOURCE_DIRECTORY})
file(MAKE_DIRECTORY ${JAVA_BINARY_DIRECTORY})

set(CMAKE_SWIG_OUTDIR ${JAVA_BINARY_DIRECTORY})

set_source_files_properties(
  OTX_.i
  PROPERTIES
  CPLUSPLUS
  ON
)

swig_add_library(
  ${OTX_WRAPPER_NAME}
  LANGUAGE
  java
  SOURCES
  OTX_.i
)
swig_link_libraries(${OTX_WRAPPER_NAME} ${JAVA_LIBRARIES})

set(MOD_NAME ${SWIG_MODULE_${OTX_WRAPPER_NAME}_REAL_NAME})

target_link_libraries(${MOD_NAME} opentxs::libopentxs)
target_include_directories(
  ${MOD_NAME} SYSTEM
  PRIVATE "${JNI_INCLUDE_DIRS}" "${JAVA_INCLUDE_PATH}" "${JAVA_INCLUDE_PATH2}"
)

if(NOT ANDROID)
  target_include_directories(${MOD_NAME} SYSTEM PRIVATE "${JAVA_INCLUDE_PATH2}")
endif()

if(NOT MSVC)
  target_compile_options(
    ${MOD_NAME}
    PRIVATE -Wno-error -Wno-old-style-cast -Wno-zero-as-null-pointer-constant
  )
endif()

set_target_properties(
  ${MOD_NAME}
  PROPERTIES
    NO_SONAME
    ON
    OUTPUT_NAME
    ${OTX_WRAPPER_NAME}
)

install(
  TARGETS
    ${MOD_NAME}
    DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
    COMPONENT
    java
)
