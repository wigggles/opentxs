set(cxx-sources
  AsymmetricKey.cpp
  AsymmetricKey_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-asymmetrickey
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-asymmetrickey
    otprotob
)
