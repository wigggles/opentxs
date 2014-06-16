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
option(RPM                 "Build a RPM instead of a tarball" OFF )

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

set(PEDANTIC_CXX_FLAGS "-Wall -Werror -Wno-format -Wno-stack-protector -Wno-error=unused-variable ${WARN_CXX_FLAGS} -Wextra -pedantic -Wno-vla")

message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Flags:       ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:  ${CMAKE_CXX_LINK_FLAGS}")


#-----------------------------------------------------------------------------
# System libraries used for linking.

if(UNIX AND NOT APPLE)
  list(APPEND OPENTXS_SYSTEM_LIBRARIES rt)
  list(APPEND OPENTXS_SYSTEM_LIBRARIES dl)
endif()

if (MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif(MSVC)
if (WIN32)
  add_definitions(-D_WIN32_WINNT=0x0501)
endif(WIN32)


#-----------------------------------------------------------------------------
# Third party libraries

find_package(OpenSSL)
find_package(Protobuf)

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
  DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/opentxs)
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
         DESTINATION include/opentxs/irrxml
	 FILES_MATCHING PATTERN "*.hpp")
         
# Packaging
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_NAME "opentxs")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Lucas Betschart")
set(CPACK_PACKAGE_VENDOR "Monetas AG")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

# RPM
if(RPM)
  set(CPACK_GENERATOR "RPM")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/opentxs.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/opentxs.spec" @ONLY IMMEDIATE)
  set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/opentxs.spec")
endif(RPM)

include(CPack)
