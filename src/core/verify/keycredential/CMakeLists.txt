set(cxx-sources
  KeyCredential.cpp
  KeyCredential_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-keycredential
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-keycredential
  otprotob
  verify-asymmetrickey
  verify-childparams
  verify-masterparams
)

target_link_libraries(verify-keycredential
  PUBLIC
  verify-asymmetrickey
  verify-childparams
  verify-masterparams
)
