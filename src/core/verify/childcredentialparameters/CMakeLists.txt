set(cxx-sources
  ChildCredentialParameters.cpp
  ChildCredentialParameters_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-childparams
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-childparams
    otprotob
)
