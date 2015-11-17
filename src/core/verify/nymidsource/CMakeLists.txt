set(cxx-sources
  NymIDSource.cpp
  NymIDSource_1.cpp
)

file(GLOB cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/opentxs/core/verify/*.hpp")

add_library(verify-nymidsource
  STATIC
  ${cxx-sources}
  ${cxx-headers}
)

add_dependencies(verify-nymidsource
    otprotob
)
