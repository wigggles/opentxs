set(cxx-sources
  Credential.cpp
  Credential_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-credential
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-credential
  otprotob
  verify-keycredential
  verify-signature
)

target_link_libraries(verify-credential
  PUBLIC
  verify-keycredential
  verify-signature
)
