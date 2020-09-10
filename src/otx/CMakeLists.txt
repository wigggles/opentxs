# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(client)
add_subdirectory(consensus)

set(cxx-sources Reply.cpp Request.cpp)
set(cxx-install-headers "${opentxs_SOURCE_DIR}/include/opentxs/otx/Reply.hpp"
                        "${opentxs_SOURCE_DIR}/include/opentxs/otx/Request.hpp"
)
set(cxx-headers ${cxx-install-headers} Reply.hpp Request.hpp)

add_library(opentxs-otx OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-otx PRIVATE opentxs::messages)
target_include_directories(
  opentxs-otx SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-otx PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/otx
  COMPONENT dev
)
