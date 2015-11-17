set(cxx-sources
  Signature.cpp
  Signature_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-signature
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-signature
    otprotob
)
