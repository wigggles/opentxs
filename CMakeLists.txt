# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)
project(opentxs)

set (VERSION_MAJOR  0)
set (VERSION_MINOR  92)
set (VERSION_BUGFIX 0)
set (VERSION "0.92.0")


#-----------------------------------------------------------------------------
# Options for building

option(BUILD_SHARED_LIBS   "Build as shared libraries." ON)
option(BUILD_VERBOSE       "Verbose build output." ON)
option(BUILD_TESTS         "Build the unit tests." ON)

option(KEYRING_WINDOWS     "Build with Windows Keyring" OFF)
option(KEYRING_MAC         "Build with Mac Keyring" OFF)
option(KEYRING_GNOME       "Build with Gnome OSX Keyring" OFF)
option(KEYRING_KWALLET     "Build with KWallet Keyring" OFF)
option(KEYRING_FLATFILE    "Build with Flatfile Keyring" OFF)
option(RPM                 "Build a RPM" OFF)
option(DEB                 "Build a DEB" OFF)

if(BUILD_VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE true)
endif()


#-----------------------------------------------------------------------------
# Print system information

message(STATUS "CMake version:   ${CMAKE_VERSION}")
message(STATUS "System:          ${CMAKE_SYSTEM}")
message(STATUS "Processor:       ${CMAKE_SYSTEM_PROCESSOR}")

message(STATUS "Verbose:         ${BUILD_VERBOSE}")
message(STATUS "Testing:         ${BUILD_TESTS}")
message(STATUS "Documentation:   ${BUILD_DOCUMENTATION}")
message(STATUS "Build RPM:       ${RPM}")
message(STATUS "Build DEB:       ${DEB}")


#-----------------------------------------------------------------------------
# Collect all binaries into bin subdirectory

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/lib)


#-----------------------------------------------------------------------------
# Check which compiler to use

if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD11)
  if (HAVE_STD11)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fPIC")
  else()
    message(FATAL_ERROR "No advanced standard C++ support (-std=c++11 not defined).")
  endif()
  set(WARN_CXX_FLAGS "-Wno-unused-result")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD11)
  if (HAVE_STD11)
    if (APPLE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -fPIC")
      set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -stdlib=libc++")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fPIC")
    endif()
  else()
    message(FATAL_ERROR "No C++11 support for Clang version. Please upgrade Clang to a version supporting C++11.")
  endif()
  set(WARN_CXX_FLAGS "-Wno-vla-extension -Wno-overloaded-virtual")
endif()

if (NOT MSVC)
  set(PEDANTIC_CXX_FLAGS "-Wall -Werror -Wno-format ${WARN_CXX_FLAGS} -Wextra -pedantic -Wno-vla")
endif()

message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Flags:       ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:  ${CMAKE_CXX_LINK_FLAGS}")

#-----------------------------------------------------------------------------
# Third party libraries

find_package(OpenSSL)
find_package(Protobuf)
find_package(ZLIB)

#-----------------------------------------------------------------------------
# System libraries used for linking.

if(UNIX AND NOT APPLE)
  list(APPEND OPENTXS_SYSTEM_LIBRARIES rt)
  list(APPEND OPENTXS_SYSTEM_LIBRARIES dl)
endif()

if (MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_UNICODE)
  if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0501)
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_INCLUDE_DIR}/../lib/VC)
  else()
    include_directories("${OPENSSL_INCLUDE_DIR}/../../OpenSSL-Win64/include")
    link_directories("${OPENSSL_INCLUDE_DIR}/../../OpenSSL-Win64/lib/VC")
  endif(WIN32)
  include_directories(${PROTOBUF_INCLUDE_DIR})
  include_directories(${ZLIB_INCLUDE_DIRS})
  link_directories(${ZLIB_LIBRARIES})
endif(MSVC)


#-----------------------------------------------------------------------------
# Force out-of-source build

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
message(FATAL_ERROR "CMake generation is not allowed within the source directory!
Remove the CMakeCache.txt file and try again from another folder, e.g.:

   rm CMakeCache.txt
   mkdir build
   cd build
   cmake ..
")
endif()


#-----------------------------------------------------------------------------
# Default to Release build

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()


#-----------------------------------------------------------------------------
# Build Unit tests

if(BUILD_TESTS)
  set(GTEST_ROOT ${opentxs_SOURCE_DIR}/deps/gtest)
  set(GTEST_FOUND ON)
  set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)
  set(GTEST_LIBRARY gtest)
  set(GTEST_MAIN_LIBRARY gtest_main)
  set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

  enable_testing()
endif(BUILD_TESTS)


############################################################################
## Build Documentation
#
# if(BUILD_DOCUMENTATION)
#    find_package(Doxygen)
#    if (DOXYGEN_FOUND)
#      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
#      add_custom_target(doc
#        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#        COMMENT "Generating API documentation with Doxygen" VERBATIM)
#      set(CPP-NETLIB_GENERATE_DOCS ON)
#    endif(DOXYGEN_FOUND)
# endif(BUILD_DOCUMENTATION)


#-----------------------------------------------------------------------------
# Get submodules

execute_process(COMMAND git "submodule" "update" "--init" "--recursive")


#-----------------------------------------------------------------------------
# OpenSSL workaround for OSX brew

if(APPLE)
  include_directories(/usr/local/Cellar/openssl/1.0.1e/include)
  link_directories(/usr/local/Cellar/openssl/1.0.1e/lib)
endif()


#-----------------------------------------------------------------------------
# Build source

add_definitions(-DCHAISCRIPT_NO_THREADS)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(tests)


#-----------------------------------------------------------------------------
# Install

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/opentxs
	DESTINATION bin)

IF (NOT WIN32)
  include(GNUInstallDirs)

  install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/lib/libopentxs-core.so
  ${CMAKE_CURRENT_BINARY_DIR}/lib/libopentxs-ext.so
  ${CMAKE_CURRENT_BINARY_DIR}/lib/libopentxs-api.so
  ${CMAKE_CURRENT_BINARY_DIR}/lib/liblucre.so
  ${CMAKE_CURRENT_BINARY_DIR}/lib/libbigint.so
  ${CMAKE_CURRENT_BINARY_DIR}/lib/libgtest.so
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/opentxs)
ENDIF()

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
         DESTINATION include)

install (FILES
	 ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/simple_ptr.hpp
	 ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/containers_fixes.hpp
	 ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/exceptions.hpp
	 ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/copy_functors.hpp
	 ${CMAKE_CURRENT_SOURCE_DIR}/deps/containers/simple_ptr.tpp
         DESTINATION include/opentxs/core/containers)

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/irrxml/
         DESTINATION include/irrxml
	 FILES_MATCHING PATTERN "*.hpp")

# Packaging
set(CPACK_GENERATOR "TGZ")

set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_NAME "opentxs")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Lucas Betschart")
set(CPACK_PACKAGE_VENDOR "Monetas AG")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Transactions library and command line client")
set(CPACK_PACKAGE_DESCRIPTION "
Open-Transactions democratizes financial and monetary actions.
You can use it for issuing currencies/stock, paying dividends, creating asset accounts, sending/receiving digital cash, writing/depositing cheques, cashier's cheques, creating basket currencies, trading on markets, scripting custom agreements, recurring payments, escrow, etc.
")

# RPM
if(RPM)
  set(CPACK_GENERATOR "${CPACK_GENERATOR};RPM")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/opentxs.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/opentxs.spec" @ONLY IMMEDIATE)
  set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/opentxs.spec")
endif(RPM)

#DEB
if(DEB)
  set(CPACK_GENERATOR "${CPACK_GENERATOR};DEB")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "openssl, libprotobuf")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${OPEN_TRANSACTIONS_WEBSITE})
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lucas Betschart")
endif(DEB)

include(CPack)
