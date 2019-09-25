# Copyright (c) 2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.14)
project(opentxs)
list(APPEND CMAKE_MODULE_PATH "${${PROJECT_NAME}_SOURCE_DIR}/cmake")
include(open-transactions-common)

# -----------------------------------------------------------------------------
# Setup

set_version_from_git()
set_build_type()
set_out_of_source_build()
update_git_submodules()

# -----------------------------------------------------------------------------
# Set option defaults

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPENTXS_PEDANTIC_DEFAULT ON)
  set(OPENTXS_BUILD_TESTS_DEFAULT ON)
  set(CMAKE_VERBOSE_MAKEFILE ON)
else()
  set(OPENTXS_PEDANTIC_DEFAULT OFF)
  set(OPENTXS_BUILD_TESTS_DEFAULT OFF)
endif()

if(WIN32)
  set(OT_USE_VCPKG_TARGETS_DEFAULT ON)
else()
  set(OT_USE_VCPKG_TARGETS_DEFAULT OFF)
endif()

# -----------------------------------------------------------------------------
# Options

option(OPENTXS_BUILD_TESTS "Build the unit tests."
       ${OPENTXS_BUILD_TESTS_DEFAULT})
option(OPENTXS_PEDANTIC_BUILD "Treat compiler warnings as errors."
       ${OPENTXS_PEDANTIC_DEFAULT})
option(OT_VALGRIND "Use Valgrind annotations." OFF)
option(OT_DHT "Enable OpenDHT support" OFF)
option(OT_STORAGE_FS "Enable filesystem backend for storage" OFF)
option(OT_STORAGE_SQLITE "Enable sqlite backend for storage" OFF)
option(OT_STORAGE_LMDB "Enable LMDB backend for storage" ON)
option(OT_CRYPTO_SUPPORTED_SOURCE_BIP47 "Enable support for BIP-47 nyms" ON)
option(OT_CRYPTO_SUPPORTED_KEY_HD "Enable HD key derivation support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_ED25519 "Enable ed25519 key support" ON)
option(OT_CRYPTO_SUPPORTED_KEY_RSA "Enable RSA key support" OFF)
option(OT_CRYPTO_SUPPORTED_KEY_SECP256K1 "Enable secp256k1 key support" ON)
option(OT_CRYPTO_USING_OPENSSL "Use OpenSSL crypto library" ON)
option(OT_CRYPTO_USING_LIBSECP256K1 "Use libsecp256k1 crypto library" ON)
option(OT_CRYPTO_USING_TREZOR "Use trezor crypto library" ON)
option(OT_CASH_USING_LUCRE "Enable Lucre support" OFF)
option(OT_LUCRE_DEBUG "Output Lucre debug info" OFF)
option(OT_SCRIPT_USING_CHAI "Use chaiscript" ON)
option(OT_WITH_QT "Enable Qt5 model support for ui classes" OFF)
option(OT_WITH_JAVA "Build with Java binding" OFF)
option(OT_OPENSSL_FLAVOR_LIBRESSL "Assume OpenSSL library is LibreSSL" OFF)
option(OT_USE_VCPKG_TARGETS "Assume dependencies are managed by vcpkg"
       ${OT_USE_VCPKG_TARGETS_DEFAULT})

# -----------------------------------------------------------------------------
# Set compiler options

set_compiler_flags(17 ${OPENTXS_PEDANTIC_BUILD})

# temporarily disabled warnings
if(NOT MSVC)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-format-nonliteral -Wno-noexcept -Wno-redundant-move -Wno-shadow -Wno-sign-conversion -Wno-switch-enum -Wno-undef -Wno-unused-parameter"
  )
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wno-comma -Wno-disabled-macro-expansion -Wno-documentation -Wno-documentation-unknown-command -Wno-extra-semi-stmt -Wno-implicit-int-conversion -Wno-redundant-parens -Wno-reserved-id-macro -Wno-shorten-64-to-32 -Wno-unused-template"
  )
endif()

# -----------------------------------------------------------------------------
# Print system information and build options

print_build_details(OPENTXS_PEDANTIC_BUILD OPENTXS_BUILD_TESTS)

message(STATUS "Valgrind integration:   ${OT_VALGRIND}")

message(STATUS "Network plugins------------------------------")
message(STATUS "DHT:                    ${OT_DHT}")

message(STATUS "Storage backends-----------------------------")
message(STATUS "filesystem:             ${OT_STORAGE_FS}")
message(STATUS "sqlite                  ${OT_STORAGE_SQLITE}")
message(STATUS "LMDB                    ${OT_STORAGE_LMDB}")

message(STATUS "Nym ID sources------------------------------")
message(STATUS "BIP-47:                 ${OT_CRYPTO_SUPPORTED_SOURCE_BIP47}")

message(STATUS "Key algorithms-------------------------------")
message(STATUS "HD derivation:          ${OT_CRYPTO_SUPPORTED_KEY_HD}")
message(STATUS "ed25519:                ${OT_CRYPTO_SUPPORTED_KEY_ED25519}")
message(STATUS "RSA:                    ${OT_CRYPTO_SUPPORTED_KEY_RSA}")
message(STATUS "secp256k1               ${OT_CRYPTO_SUPPORTED_KEY_SECP256K1}")

message(STATUS "Crypto library providers---------------------")
message(STATUS "OpenSSL:                ${OT_CRYPTO_USING_OPENSSL}")
message(STATUS "libsecp256k1:           ${OT_CRYPTO_USING_LIBSECP256K1}")
message(STATUS "trezor-crypto:          ${OT_CRYPTO_USING_TREZOR}")

message(STATUS "Cash library providers---------------------")
message(STATUS "Lucre:                  ${OT_CASH_USING_LUCRE}")
message(STATUS "Lucre debug info:       ${OT_LUCRE_DEBUG}")

message(STATUS "Script engines-------------------------------")
message(STATUS "Chai:                   ${OT_SCRIPT_USING_CHAI}")

message(STATUS "UI-------------------------------------------")
message(STATUS "Qt5:                    ${OT_WITH_QT}")

message(STATUS "Bindings ------------------------------------")
message(STATUS "Java binding:           ${OT_WITH_JAVA}")

# -----------------------------------------------------------------------------
# Dependencies

find_system_libraries()

if(OPENTXS_BUILD_TESTS)
  find_package(GTest REQUIRED)
  enable_testing()
endif()

find_package(Protobuf REQUIRED)
find_package(opentxs-proto REQUIRED)
find_package(ZLIB REQUIRED)
find_package(
  Boost REQUIRED
  COMPONENTS system
  OPTIONAL_COMPONENTS stacktrace_basic
)

if(OT_USE_VCPKG_TARGETS)
  find_package(unofficial-sodium CONFIG REQUIRED)
else()
  find_package(Sodium REQUIRED)
endif()

if(OT_USE_VCPKG_TARGETS)
  find_package(ZeroMQ CONFIG REQUIRED)
else()
  find_package(ZeroMQ 4 REQUIRED)
endif()

if(OT_DHT)
  find_package(opendht 1.10 REQUIRED)
  find_package(GnuTLS REQUIRED)
endif()

if(OT_STORAGE_FS)
  find_package(Boost REQUIRED COMPONENTS filesystem iostreams)
endif()

if(OT_STORAGE_SQLITE)
  find_package(SQLite3 REQUIRED)
endif()

if(OT_STORAGE_LMDB)
  find_package(LMDB REQUIRED)
endif()

if(OT_CRYPTO_USING_OPENSSL)
  find_package(OpenSSL 1.0.1 REQUIRED)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  find_package(Libsecp256k1 REQUIRED)
endif()

if(OT_WITH_QT)
  find_package(
    Qt5
    COMPONENTS
      Core
      Gui
      Qml
      Widgets
    REQUIRED
  )
  set_target_properties(
    Qt5::Core
    PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE"
  )
  list(GET Qt5Core_INCLUDE_DIRS 0 QT_INCLUDE_DIR)
  include_directories(SYSTEM "${QT_INCLUDE_DIR}")
endif()

# -----------------------------------------------------------------------------
# Source Definitions

set_common_defines()

if(OT_VALGRIND)
  add_definitions(-DOT_VALGRIND=1)
else()
  add_definitions(-DOT_VALGRIND=0)
endif()

# Network

if(OT_DHT)
  set(DHT_EXPORT 1)
else()
  set(DHT_EXPORT 0)
endif()

# Storage backends

if(OT_STORAGE_FS)
  set(FS_EXPORT 1)
else()
  set(FS_EXPORT 0)
endif()

if(OT_STORAGE_SQLITE)
  set(SQLITE_EXPORT 1)
else()
  set(SQLITE_EXPORT 0)
endif()

if(OT_STORAGE_LMDB)
  set(LMDB_EXPORT 1)
else()
  set(LMDB_EXPORT 0)
endif()

if((NOT OT_STORAGE_FS) AND (NOT OT_STORAGE_SQLITE) AND (NOT OT_STORAGE_LMDB))
  message(FATAL_ERROR "At least one storage backend must be defined.")
endif()

# Nym sources

if(OT_CRYPTO_SUPPORTED_SOURCE_BIP47)
  if(NOT OT_CRYPTO_SUPPORTED_KEY_HD)
    message(FATAL_ERROR "BIP-47 support requires HD key support")
  endif()
  if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "BIP-47 support requires secp256k1 key support")
  endif()
  set(BIP47_EXPORT 1)
else()
  set(BIP47_EXPORT 0)
endif()

# Key types

if(OT_CRYPTO_SUPPORTED_KEY_HD)
  if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    message(FATAL_ERROR "HD key support requires secp256k1 key support")
  endif()
  if(NOT (OT_CRYPTO_USING_TREZOR))
    message(FATAL_ERROR "HD key support requires trezor-crypto")
  endif()
  set(HD_EXPORT 1)
else()
  set(HD_EXPORT 0)
endif()

if(NOT OT_CRYPTO_SUPPORTED_KEY_ED25519)
  set(ED25519_EXPORT 0)
  if(NOT OT_CRYPTO_SUPPORTED_KEY_RSA)
    if(NOT OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
      message(FATAL_ERROR "At least one type of key must be enabled")
    endif()
  endif()
else()
  set(ED25519_EXPORT 1)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_RSA)
  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "RSA support requires OpenSSL")
  endif()
  set(RSA_EXPORT 1)
else()
  set(RSA_EXPORT 0)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
  if(NOT (OT_CRYPTO_USING_LIBSECP256K1 OR OT_CRYPTO_USING_TREZOR))
    message(FATAL_ERROR "no provider for secp256k1 support configured")
  endif()
  set(SECP256K1_EXPORT 1)
else()
  set(SECP256K1_EXPORT 0)
endif()

# Crypto libraries

if(OT_CRYPTO_USING_OPENSSL)
  set(OPENSSL_EXPORT 1)
else()
  set(OPENSSL_EXPORT 0)
endif()

if(OT_CRYPTO_USING_LIBSECP256K1)
  if(OT_CRYPTO_SUPPORTED_KEY_SECP256K1)
    set(LIBSECP256K1_EXPORT 1)
  else()
    set(LIBSECP256K1_EXPORT 0)
    message(WARNING "Libsecp256k1 is pointless without secp256k1 keys.")
  endif()
else()
  set(LIBSECP256K1_EXPORT 0)
endif()

if(OT_CRYPTO_USING_TREZOR)
  set(TREZORCRYPTO_EXPORT 1)
else()
  set(TREZORCRYPTO_EXPORT 0)
endif()

if(OT_CRYPTO_SUPPORTED_KEY_HD)
  set(BIP32_EXPORT 1)
  set(BIP39_EXPORT 1)
else()
  set(BIP32_EXPORT 0)
  set(BIP39_EXPORT 0)
endif()

if(OT_CASH_USING_LUCRE)
  if(NOT OT_CRYPTO_USING_OPENSSL)
    message(FATAL_ERROR "Lucre support requires OpenSSL")
  endif()

  set(CASH_LUCRE_EXPORT 1)
  set(OT_CASH_EXPORT 1)
else()
  set(CASH_LUCRE_EXPORT 0)
  set(OT_CASH_EXPORT 0)
endif()

if(OT_LUCRE_DEBUG)
  add_definitions(-DOT_LUCRE_DEBUG=1)
else()
  add_definitions(-DOT_LUCRE_DEBUG=0)
endif()

if(FALSE)
  set(CASH_MM_EXPORT 1)
  set(OT_CASH_EXPORT 1)
else()
  set(CASH_MM_EXPORT 0)
endif()

# Script engines
if(OT_SCRIPT_USING_CHAI)
  set(SCRIPT_CHAI_EXPORT 1)
else()
  set(SCRIPT_CHAI_EXPORT 0)
endif()

if(OT_WITH_QT)
  set(OT_QT_EXPORT 1)
else()
  set(OT_QT_EXPORT 0)
endif()

if(OT_WITH_JAVA)
  set(OT_SWIG ON)
else()
  set(OT_SWIG OFF)
endif()

# -----------------------------------------------------------------------------
# Set in-source Version
configure_file("Version.hpp.in"
               "${opentxs_BINARY_DIR}/include/opentxs/Version.hpp" @ONLY)

install(
  FILES ${opentxs_BINARY_DIR}/include/opentxs/Version.hpp
  DESTINATION include/opentxs/
  COMPONENT dev
)

# -----------------------------------------------------------------------------
# Build source

include_directories("${opentxs_SOURCE_DIR}/include" "${opentxs_SOURCE_DIR}/src"
                    "${opentxs_BINARY_DIR}/include")
include_directories(SYSTEM "${Protobuf_INCLUDE_DIRS}")
include_directories(SYSTEM "${opentxs-proto_INCLUDE_DIRS}")

add_subdirectory(deps)
add_subdirectory(src)

if(OT_SWIG)
  add_subdirectory(wrappers)
endif()

if(OPENTXS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Package

generate_cmake_files()
generate_pkgconfig("-lopentxs")
generate_uninstall_target()
