## Work in progress
##
message("** Csharp Wrapper")

set(OTX_WRAPPER_NAME "Opentxs")

set(CMAKE_SWIG_FLAGS "")

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(CSharp REQUIRED)

include_directories(
  ${SWIG_INCLUDE_DIRS}
)

# place our csharp code in a subdir to keep it clean
set(CSHARP_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/otx_sharp CACHE INTERNAL "")
set(CSHARP_BINARY_DIRECTORY ${CSHARP_SOURCE_DIRECTORY} CACHE INTERNAL "")
file(MAKE_DIRECTORY ${CSHARP_SOURCE_DIRECTORY})  
set(CMAKE_SWIG_OUTDIR ${CSHARP_BINARY_DIRECTORY})
##


# -dllimport = native/glue lib
set(CMAKE_SWIG_FLAGS "-namespace Opentxs -dllimport opentxs-client -outdir ${CMAKE_SWIG_OUTDIR}" )
separate_arguments(CMAKE_SWIG_FLAGS)

# so file
set(CMAKE_SWIG_FLAGS -I${CMAKE_SOURCE_DIR})
set_source_files_properties(OTX_.i PROPERTIES CPLUSPLUS on)


swig_add_module(${OTX_WRAPPER_NAME} csharp OTX_.i)
swig_link_libraries(${OTX_WRAPPER_NAME} opentxs-client)

set(MOD_NAME ${SWIG_MODULE_${OTX_WRAPPER_NAME}_REAL_NAME})

# add_library(Opentxs-sharp opentxs-client)
target_link_libraries(${MOD_NAME} opentxs-client)
set_target_properties(${MOD_NAME}
                      PROPERTIES OUTPUT_NAME ${OTX_WRAPPER_NAME})


install(TARGETS ${MOD_NAME}
  DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT csharp)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cs.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AssemblyInfo.cs"
  @ONLY)
  
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/key.snk
  ${CMAKE_CURRENT_BINARY_DIR}/key.snk
  COPYONLY)

add_custom_command(
  OUTPUT "${GDCM_LIBRARY_DIR}/opentxs-sharp.dll"
  COMMAND ${CMAKE_CSHARP_COMPILER} ARGS "/t:library" "/out:${GDCM_LIBRARY_DIR}/opentxs-sharp.dll" "*.cs"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS "${swig_generated_file_fullname}"
      "${CMAKE_CURRENT_BINARY_DIR}/AssemblyInfo.cs"
    COMMENT "csc *.cs"
)
