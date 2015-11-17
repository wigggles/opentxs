add_subdirectory(asymmetrickey)
add_subdirectory(childcredentialparameters)
add_subdirectory(credential)
add_subdirectory(keycredential)
add_subdirectory(mastercredentialparameters)
add_subdirectory(nymidsource)
add_subdirectory(signature)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/verify/*.hpp")

add_library(opentxs-verify
  STATIC
  ${cxx-headers}
)

target_link_libraries(opentxs-verify
  PUBLIC
  verify-asymmetrickey
  verify-childparams
  verify-credential
  verify-keycredential
  verify-masterparams
  verify-nymidsource
  verify-signature
)
