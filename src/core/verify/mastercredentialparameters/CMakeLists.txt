set(cxx-sources
  MasterCredentialParameters.cpp
  MasterCredentialParameters_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-masterparams
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-masterparams
  otprotob
  verify-nymidsource
)

target_link_libraries(verify-masterparams
  PUBLIC
  verify-nymidsource
)
